#include "hid_service.hpp"

static espp::Logger logger({.tag = "HID BLE", .level = espp::Logger::Verbosity::DEBUG});

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x1812
#define SVC_INST_ID                 0

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

static uint8_t adv_config_done       = 0;
static uint16_t hid_handle_table[IDX_HID_NB];

static SemaphoreHandle_t ble_cb_semaphore = NULL;
#define WAIT_BLE_CB() xSemaphoreTake(ble_cb_semaphore, portMAX_DELAY)
#define SEND_BLE_CB() xSemaphoreGive(ble_cb_semaphore)

static std::atomic<bool> connected{false};

static uint8_t service_uuid[16] = {
  // This is the HID service UUID: 00001812-0000-1000-8000-00805f9b34fb
    /* LSB <--------------------------------------------------------------------------------> MSB */
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};
esp_ble_adv_data_t adv_config = {
  .set_scan_rsp        = false,
  .include_name        = true,
  .include_txpower     = true,
  .min_interval        = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
  .max_interval        = 0x000C, // slave connection max interval, Time = max_interval * 1.25 msec
  .appearance          = 0x03C0,
  .manufacturer_len    = (uint16_t)strlen((const char*)manufacturer_name),
  .p_manufacturer_data = manufacturer_name,
  .service_data_len    = 0,
  .p_service_data      = NULL,
  .service_uuid_len    = sizeof(service_uuid),
  .p_service_uuid      = service_uuid,
  .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// config scan response data
esp_ble_adv_data_t scan_rsp_config = {
  .set_scan_rsp        = true,
  .include_name        = true,
  .include_txpower     = true,
  .appearance          = 0x03C0,
  .manufacturer_len    = (uint16_t)strlen((const char*)manufacturer_name),
  .p_manufacturer_data = manufacturer_name,
  .service_data_len    = 0,
  .p_service_data      = NULL,
  .service_uuid_len    = sizeof(service_uuid),
  .p_service_uuid      = service_uuid,
  .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
  .adv_int_min         = 0x20, // 40ms interval between advertisements (32 * 1.25ms)
  .adv_int_max         = 0x40, // 80ms interval between advertisements (64 * 1.25ms)
  .adv_type            = ADV_TYPE_IND,
  .own_addr_type       = BLE_ADDR_TYPE_PUBLIC, // TYPE_PUBLIC, TYPE_RPA_PUBLIC
  .channel_map         = ADV_CHNL_ALL,
  .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
  esp_gatts_cb_t gatts_cb;
  uint16_t gatts_if;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t service_handle;
  esp_gatt_srvc_id_t service_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_perm_t perm;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst hid_profile_tab[PROFILE_NUM] = {
  [PROFILE_APP_IDX] = {
    .gatts_cb = gatts_profile_event_handler,
    .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
  },
};


static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  switch (event) {
    /*
     * SCAN
     * */
  case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
    logger.debug("BLE GAP EVENT SCAN_PARAM_SET_COMPLETE");
    SEND_BLE_CB();
    break;
  }
  case ESP_GAP_BLE_SCAN_RESULT_EVT: {
    logger.debug("BLE GAP EVENT SCAN_RESULT");
    break;
  }
  case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT: {
    logger.debug("BLE GAP EVENT SCAN CANCELED");
    break;
  }

    /*
     * ADVERTISEMENT
     * */
  case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    logger.debug("BLE GAP ADV_DATA_RAW_SET_COMPLETE");
    adv_config_done &= (~ADV_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    logger.debug("BLE GAP ADV_DATA_SET_COMPLETE");
    adv_config_done &= (~ADV_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    logger.debug("BLE GAP SCAN_RSP_DATA_RAW_SET_COMPLETE");
    adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    logger.debug("BLE GAP SCAN_RSP_DATA_SET_COMPLETE");
    adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    /* advertising start complete event to indicate advertising start successfully or failed */
    if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
      logger.error("advertising start failed");
    }else{
      logger.info("advertising start successfully");
    }
    break;
  case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
      logger.error("Advertising stop failed");
    }
    else {
      logger.info("Stop adv successfully");
    }
    break;
  case ESP_GAP_BLE_ADV_TERMINATED_EVT:
    logger.debug("BLE GAP ADV_TERMINATED");
    break;

    /*
     * CONNECTION
     * */
  case ESP_GAP_BLE_GET_DEV_NAME_COMPLETE_EVT:
    logger.debug("BLE GAP GET_DEV_NAME_COMPLETE");
    // print the name
    logger.debug("BLE GAP DEVICE NAME: {}", param->get_dev_name_cmpl.name);
    break;

    /*
     * BOND
     * */
  case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
    logger.debug("BLE GAP REMOVE_BOND_DEV_COMPLETE");
    // log the bond that was removed
    // esp_log_buffer_hex(TAG, param->remove_bond_dev_cmpl.bd_addr, ESP_BD_ADDR_LEN);
    break;

  case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
    logger.debug("BLE GAP CLEAR_BOND_DEV_COMPLETE");
    break;

  case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
    logger.debug("BLE GAP GET_BOND_DEV_COMPLETE");
    break;

    /*
     * AUTHENTICATION
     * */
  case ESP_GAP_BLE_AUTH_CMPL_EVT:
    if (!param->ble_security.auth_cmpl.success) {
      logger.error("BLE GAP AUTH ERROR: {:#x}", param->ble_security.auth_cmpl.fail_reason);
    } else {
      logger.info("BLE GAP AUTH SUCCESS");
    }
    break;

  case ESP_GAP_BLE_KEY_EVT: // shows the ble key info share with peer device to the user.
    logger.debug("BLE GAP KEY type = {}",
                esp_ble_key_type_str(param->ble_security.ble_key.key_type));
    break;

  case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: { // ESP_IO_CAP_OUT
    int received_passkey = (int)param->ble_security.key_notif.passkey;
    // The app will receive this evt when the IO has Output capability and the peer device IO
    // has Input capability. Show the passkey number to the user to input it in the peer device.
    logger.debug("BLE GAP PASSKEY_NOTIF passkey: {}", received_passkey);
  }
    break;

  case ESP_GAP_BLE_NC_REQ_EVT: { // ESP_IO_CAP_IO
    int received_passkey = (int)param->ble_security.key_notif.passkey;
    // The app will receive this event when the IO has DisplayYesNO capability and the peer
    // device IO also has DisplayYesNo capability. show the passkey number to the user to
    // confirm it with the number displayed by peer device.
    logger.debug("BLE GAP NC_REQ passkey: {}", received_passkey);
    esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
  }
    break;

  case ESP_GAP_BLE_PASSKEY_REQ_EVT: // ESP_IO_CAP_IN
    // The app will receive this evt when the IO has Input capability and the peer device IO has
    // Output capability. See the passkey number on the peer device and send it back.
    logger.debug("BLE GAP PASSKEY_REQ");
    // TODO: uncomment below and set the passkey
    // esp_ble_passkey_reply(param->ble_security.key_notif.bd_addr, true, PASSKEY);
    break;

  case ESP_GAP_BLE_OOB_REQ_EVT:
    // OOB request event
    logger.warn("BLE GAP OOB_REQ not implemented");
    break;

  case ESP_GAP_BLE_SC_OOB_REQ_EVT:
    // secure connection oob request event
    logger.warn("BLE GAP SC_OOB_REQ not implemented");
    break;

  case ESP_GAP_BLE_SC_CR_LOC_OOB_EVT:
    // secure connection create oob data complete event
    logger.warn("BLE GAP SC_CR_LOC_OOB not implemented");
    break;

  case ESP_GAP_BLE_SEC_REQ_EVT:
    logger.debug("BLE GAP SEC_REQ");

    // Send the positive(true) security response to the peer device to accept the security
    // request. If not accept the security request, should send the security response with
    // negative(false) accept value.
    esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
    break;

  case ESP_GAP_BLE_PHY_UPDATE_COMPLETE_EVT:
    logger.debug("BLE GAP PHY_UPDATE_COMPLETE");
    break;

  case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
    if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
      logger.error("config local privacy failed, error status = {:#x}", (int)param->local_privacy_cmpl.status);
      break;
    }
    break;

  case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    logger.debug("update connection params status = {}, min_int = {}, max_int = {},conn_int = {},latency = {}, timeout = {}",
                (int)param->update_conn_params.status,
                (int)param->update_conn_params.min_int,
                (int)param->update_conn_params.max_int,
                (int)param->update_conn_params.conn_int,
                (int)param->update_conn_params.latency,
                (int)param->update_conn_params.timeout);
    break;
  default:
    logger.warn("UNHANDLED BLE GAP EVENT: {}", ble_gap_evt_str(event));
    break;
  }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
  uint64_t peer_address = 0;
  logger.debug("gatts_profile_event_handler: event = {}, gatts_if = {}",
               ble_gatts_evt_str(event),
               (int)gatts_if);
  switch (event) {
  case ESP_GATTS_REG_EVT:
    logger.debug("ESP_GATTS_REG_EVT");
    esp_ble_gap_set_device_name(CONFIG_DEVICE_NAME);
    esp_ble_gap_config_adv_data(&adv_config);
    esp_ble_gap_config_adv_data(&scan_rsp_config);
    esp_ble_gatts_create_attr_tab(bas_att_db, gatts_if, BAS_IDX_NB, 0);
    break;
  case ESP_GATTS_READ_EVT:
    for (int i = 0; i < 6; i++) {
      peer_address += ((uint64_t)param->read.bda[i]) << (i * 8);
    }
    logger.debug("ESP_GATTS_READ_EVT, peer_address: {:#x}", peer_address);
    {
      auto handle_id = param->read.handle;
      logger.debug("                          handle: {}, offset: {}, need_rsp: {}", handle_id, param->read.offset, param->read.need_rsp);
    }
    // TODO: write output/response?
    break;
  case ESP_GATTS_WRITE_EVT:
    for (int i = 0; i < 6; i++) {
      peer_address += ((uint64_t)param->write.bda[i]) << (i * 8);
    }
    logger.debug("ESP_GATTS_WRITE_EVT, peer_address: {:#x}", peer_address);
    logger.debug("                           handle: {}, value len: {}", param->write.handle, param->write.len);
    if (!param->write.is_prep){
      bool is_cfg_handle =
        hid_handle_table[IDX_CHAR_CFG_HID_REPORT_XB] == param->write.handle;

      if (is_cfg_handle && param->write.len == 2){
        logger.debug("Configuration of {}",
                     param->write.handle == hid_handle_table[IDX_CHAR_CFG_HID_REPORT_XB]
                     ? "IDX_CHAR_CFG_HID_REPORT_XB"
                     : "UNKNOWN CFG HANDLE");
        uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
        if (descr_value == 0x0001) {
          logger.info("notify enable");
        } else if (descr_value == 0x0002) {
          logger.info("indicate enable");
        } else if (descr_value == 0x0000) {
          logger.info("notify/indicate disable ");
        } else {
          logger.error("unknown descr value");
        }
      } else {
        // TODO: handle other writes?
      }
      /* send response when param->write.need_rsp is true*/
      if (param->write.need_rsp){
        logger.info("send response");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
      }
    }else{
      /* handle prepare write */
      logger.warn("ESP_GATTS_PREP_WRITE_EVT not implemented");
    }
    break;
  case ESP_GATTS_EXEC_WRITE_EVT:
    logger.warn("ESP_GATTS_EXEC_WRITE_EVT not implemented");
    break;
  case ESP_GATTS_SET_ATTR_VAL_EVT:
    logger.debug("ESP_GATTS_SET_ATTR_VAL_EVT, attr_handle {}, srvc_handle {}, status {}",
                (int)param->set_attr_val.attr_handle,
                (int)param->set_attr_val.srvc_handle,
                (int)param->set_attr_val.status);
    break;
  case ESP_GATTS_MTU_EVT:
    logger.debug("ESP_GATTS_MTU_EVT, MTU {}", (int)param->mtu.mtu);
    break;
  case ESP_GATTS_CONF_EVT:
    logger.debug("ESP_GATTS_CONF_EVT, status = {}, attr_handle {}", (int)param->conf.status, (int)param->conf.handle);

    break;
  case ESP_GATTS_START_EVT:
    logger.debug("SERVICE_START_EVT, status {}, service_handle {}", (int)param->start.status, (int)param->start.service_handle);
    break;
  case ESP_GATTS_CONNECT_EVT: {
    logger.debug("ESP_GATTS_CONNECT_EVT, conn_id = {}", (int)param->connect.conn_id);
    // update the connection id to each profile table
    hid_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
    esp_ble_conn_update_params_t conn_params = {0};
    memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
    /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
    conn_params.latency = 0;
    conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
    conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
    conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms

    //start sent the update connection parameters to the peer device.
    esp_ble_gap_update_conn_params(&conn_params);
    connected = true;
  }
    break;
  case ESP_GATTS_DISCONNECT_EVT:
    logger.debug("ESP_GATTS_DISCONNECT_EVT, reason = {:#x}", (int)param->disconnect.reason);
    connected = false;
    esp_ble_gap_start_advertising(&adv_params);
    break;
  case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
    if (param->add_attr_tab.num_handle == BAS_IDX_NB &&
        param->add_attr_tab.svc_uuid.uuid.uuid16 == ESP_GATT_UUID_BATTERY_SERVICE_SVC &&
        param->add_attr_tab.status == ESP_GATT_OK) {
      logger.info("create battery attribute table successfully, the number handle = {}", (int)param->add_attr_tab.num_handle);
      memcpy(bas_handle_table, param->add_attr_tab.handles, sizeof(bas_handle_table));
      auto start_handle = param->add_attr_tab.handles[BAS_IDX_SVC];
      hid_service_table_set_included_battery_service_handles(start_handle,
                                                             start_handle + BAS_IDX_NB - 1);
      esp_ble_gatts_create_attr_tab(dis_att_db, gatts_if, DIS_IDX_NB, 0);
    }
    if (param->add_attr_tab.num_handle == DIS_IDX_NB &&
        param->add_attr_tab.svc_uuid.uuid.uuid16 == ESP_GATT_UUID_DEVICE_INFO_SVC &&
        param->add_attr_tab.status == ESP_GATT_OK) {
      logger.info("create device information attribute table successfully, the number handle = {}", (int)param->add_attr_tab.num_handle);
      memcpy(dis_handle_table, param->add_attr_tab.handles, sizeof(dis_handle_table));
      auto start_handle = param->add_attr_tab.handles[DIS_IDX_SVC];
      hid_service_table_set_included_dev_info_service_handles(start_handle,
                                                              start_handle + DIS_IDX_NB - 1);
      esp_ble_gatts_create_attr_tab(hid_gatt_db, gatts_if, IDX_HID_NB, 0);
    }
    if (param->add_attr_tab.num_handle == IDX_HID_NB &&
        param->add_attr_tab.status == ESP_GATT_OK) {
      logger.info("create hid attribute table successfully, the number handle = {}", (int)param->add_attr_tab.num_handle);
      memcpy(hid_handle_table, param->add_attr_tab.handles, sizeof(hid_handle_table));
      esp_ble_gatts_start_service(hid_handle_table[IDX_SVC_HID]);
    } else {
      esp_ble_gatts_start_service(param->add_attr_tab.handles[0]);
    }
    break;
  }
  case ESP_GATTS_STOP_EVT:
  case ESP_GATTS_OPEN_EVT:
  case ESP_GATTS_CANCEL_OPEN_EVT:
  case ESP_GATTS_CLOSE_EVT:
  case ESP_GATTS_LISTEN_EVT:
  case ESP_GATTS_CONGEST_EVT:
  case ESP_GATTS_UNREG_EVT:
  case ESP_GATTS_DELETE_EVT:
  default:
    logger.debug("gatts_event_handler: unhandled event {}", (int)event);
    break;
  }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

  /* If event is register event, store the gatts_if for each profile */
  if (event == ESP_GATTS_REG_EVT) {
    if (param->reg.status == ESP_GATT_OK) {
      hid_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
    } else {
      logger.error("reg app failed, app_id {:04x}, status {}",
                   (int)param->reg.app_id,
                   (int)param->reg.status);
      return;
    }
  }
  do {
    int idx;
    for (idx = 0; idx < PROFILE_NUM; idx++) {
      /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
      if (gatts_if == ESP_GATT_IF_NONE || gatts_if == hid_profile_tab[idx].gatts_if) {
        if (hid_profile_tab[idx].gatts_cb) {
          hid_profile_tab[idx].gatts_cb(event, gatts_if, param);
        }
      }
    }
  } while (0);
}


static void send_indicate(uint8_t* data, size_t length, uint16_t handle, bool indicate=false) {
  if (!connected) {
    return;
  }
  uint16_t gatts_if = hid_profile_tab[PROFILE_APP_IDX].gatts_if;
  uint16_t conn_id = hid_profile_tab[PROFILE_APP_IDX].conn_id;
  logger.debug("Sending notification: gatts_if={}, conn_id={}, attr_handle={}, length={}",
               gatts_if, conn_id, handle, length);
  esp_err_t ret = esp_ble_gatts_send_indicate(gatts_if, conn_id, handle, length, data, indicate);
  if (ret) {
    logger.error("esp_ble_gatts_send_indicate failed: {:#x}", ret);
  }
}

/////////////////BLE///////////////////////

// Initializes BLE
bool hid_service_is_connected() {
  return connected;
}

void hid_service_init() {
  logger.info("Initializing BLE");

  // Set the type of authentication needed
  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
  // set the IO capability of the device
  esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
  // set the out of band configuration
  uint8_t oob_support = ESP_BLE_OOB_DISABLE;
  // set the key configuration
  uint8_t spec_auth = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;

  // esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, (void*)&PASSKEY, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, 1);
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, 1);
  esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &spec_auth, sizeof(uint8_t));

  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t key_size = 16; //the key size should be 7~16 bytes

  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, 1);
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, 1);
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, 1);

  esp_ble_gatts_register_callback(gatts_event_handler);
  esp_ble_gap_register_callback(gap_event_handler);
  esp_ble_gatts_app_register(ESP_APP_ID);
}

void hid_service_set_report_descriptor(uint8_t* descriptor, size_t descriptor_len) {
  logger.info("Setting report descriptor of length {}", report_descriptor_len);
  hid_service_table_set_report_descriptor(descriptor, descriptor_len);
  esp_ble_gatts_set_attr_value(hid_handle_table[IDX_CHAR_VAL_HID_REPORT_MAP], descriptor_len, descriptor);
}

void hid_service_send_input_report(const uint8_t* report, size_t report_len) {
  logger.info("Sending input report of length {}", report_len);
  send_indicate((uint8_t*)report, report_len, hid_handle_table[IDX_CHAR_VAL_HID_REPORT_XB]);
}

void hid_service_set_battery_level(const uint8_t level) {
  logger.info("Setting battery level to {}%", level);
  battery_level = level;
  send_indicate(&battery_level, sizeof(battery_level), bas_handle_table[BAS_IDX_BATT_LVL_VAL]);
}

void hid_service_set_pnp_id(const uint16_t vendor_id, const uint16_t product_id, const uint16_t product_version) {
  // format of pnp:
  // 0xVVVVPPPPIIIISS
  // VVVV: product version
  // PPPP: product id
  // IIII: vendor id
  // SS: vendor id source
  uint64_t pnp = 0x02; // vendor id source
  pnp |= (uint64_t)vendor_id << 8;
  pnp |= (uint64_t)product_id << 24;
  pnp |= (uint64_t)product_version << 40;
  logger.info("Building PnP from vendor_id={:#x}, product_id={:#x}, product_version={:#x}",
              vendor_id, product_id, product_version);
  logger.info("Setting PNP ID to {:#x}", pnp);
  // now actually set the variable
  pnp_id = pnp;
  // and make sure we send it
  esp_ble_gatts_set_attr_value(dis_handle_table[DIS_IDX_PNP_VAL], 7, (uint8_t*)&pnp_id);
}

void hid_service_set_manufacturer_name(std::string_view manufacturer_name_string_view) {
  logger.info("Setting manufacturer name to '{}'", manufacturer_name_string_view);
  // copy the manufacturer name into the manufacturer name buffer
  std::copy(manufacturer_name_string_view.begin(), manufacturer_name_string_view.end(), manufacturer_name);
  manufacturer_name_length = manufacturer_name_string_view.size();
  // and make sure we send it
  esp_ble_gatts_set_attr_value(dis_handle_table[DIS_IDX_MANUFACTURER_NAME_VAL], manufacturer_name_length, manufacturer_name);
}

void hid_service_set_model_number(std::string_view model_number_string_view) {
  logger.info("Setting model number to '{}'", model_number_string_view);
  // copy the model number into the model number buffer
  std::copy(model_number_string_view.begin(), model_number_string_view.end(), model_number);
  model_number_length = model_number_string_view.size();
  // and make sure we send it
  esp_ble_gatts_set_attr_value(dis_handle_table[DIS_IDX_MODEL_NUMBER_VAL], model_number_length, model_number);
}

void hid_service_set_serial_number(std::string_view serial_number_string_view) {
  logger.info("Setting serial number to '{}'", serial_number_string_view);
  // copy the serial number into the serial number buffer
  std::copy(serial_number_string_view.begin(), serial_number_string_view.end(), serial_number);
  serial_number_length = serial_number_string_view.size();
  // and make sure we send it
  esp_ble_gatts_set_attr_value(dis_handle_table[DIS_IDX_SERIAL_NUMBER_VAL], serial_number_length, serial_number);
}
