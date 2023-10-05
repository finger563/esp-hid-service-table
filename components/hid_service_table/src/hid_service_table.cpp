#include "hid_service_table.hpp"

static espp::Logger logger({.tag = "GFPS BLE", .level = espp::Logger::Verbosity::DEBUG});

// READ = only val
// READ/WRITE = val + cfg

/* Attributes State Machine */
enum
  {
    /// HID Service
    IDX_SVC_HID, // HID Service, UUID: 0x1812

    IDX_CHAR_HID_INFO, // HID Information characteristic, UUID: 0x2A4A, Properties: read
    IDX_CHAR_VAL_HID_INFO,

    IDX_CHAR_HID_CONTROL_POINT, // HID Control Point characteristic, UUID: 0x2A4C, Properties: write without response
    IDX_CHAR_VAL_HID_CONTROL_POINT,
    IDX_CHAR_CFG_HID_CONTROL_POINT,

    IDX_CHAR_HID_REPORT_MAP, // HID Report Map characteristic, UUID: 0x2A4B, Properties: read
    IDX_CHAR_VAL_HID_REPORT_MAP,

    IDX_CHAR_HID_REPORT, // HID Report characteristic, UUID: 0x2A4D, Properties: read, notify
    IDX_CHAR_VAL_HID_REPORT,

    IDX_CHAR_HID_PROTOCOL_MODE, // HID Protocol Mode characteristic, UUID: 0x2A4E, Properties: read, write without response
    IDX_CHAR_VAL_HID_PROTOCOL_MODE,
    IDX_CHAR_CFG_HID_PROTOCOL_MODE,

    IDX_HID_NB,
  };

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SVC_INST_ID                 0

/* Service */
// NOTE: these UUIDs are specified at 16-bit, which means that 1) they are not
// in reverse order, and 2) the rest of their full 128-bit UUIDs are the standard
// Bluetooth SIG base UUID (0x0000XXXX-0000-1000-8000-00805F9B34FB), and the 16-bit
// UUIDs are the last 2 bytes of the first 4 bytes of the full UUIDs (XXXX).
static const uint16_t GATTS_SERVICE_UUID_HID      = 0x1812;
static const uint16_t GATTS_CHAR_UUID_HID_INFO    = 0x2A4A;
static const uint16_t GATTS_CHAR_UUID_HID_CONTROL_POINT = 0x2A4C;
static const uint16_t GATTS_CHAR_UUID_HID_REPORT_MAP = 0x2A4B;
static const uint16_t GATTS_CHAR_UUID_HID_REPORT  = 0x2A4D;
static const uint16_t GATTS_CHAR_UUID_HID_PROTOCOL_MODE = 0x2A4E;

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
 *  the data length must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
 */
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

static uint8_t adv_config_done       = 0;

static uint16_t hid_handle_table[IDX_HID_NB];

static SemaphoreHandle_t ble_cb_semaphore = NULL;
#define WAIT_BLE_CB() xSemaphoreTake(ble_cb_semaphore, portMAX_DELAY)
#define SEND_BLE_CB() xSemaphoreGive(ble_cb_semaphore)

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(*a))

static uint8_t service_uuid[16] = {
  // This is the HID service UUID: 00001812-0000-1000-8000-00805f9b34fb
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};
esp_ble_adv_data_t adv_config = {
  .set_scan_rsp        = false,
  .include_name        = true,
  .include_txpower     = true,
  .min_interval        = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
  .max_interval        = 0x000C, // slave connection max interval, Time = max_interval * 1.25 msec
  .appearance          = 0x00,
  .manufacturer_len    = 0,
  .p_manufacturer_data = NULL,
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
  .appearance          = 0x00,
  .manufacturer_len    = 0,
  .p_manufacturer_data = NULL,
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

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint8_t ccc[2]           = {0x01, 0x00}; // LSb corresponds to notifications (1 if enabled, 0 if disabled), next bit (bit 1) corresponds to indications - 1 if enabled, 0 if disabled
static const uint8_t char_value[16]    = {0x00};

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[IDX_HID_NB] =
  {
    // Service Declaration
    [IDX_SVC_HID]           =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
                           sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_HID), (uint8_t *)&GATTS_SERVICE_UUID_HID}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_INFO]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_HID_INFO] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)GATTS_CHAR_UUID_HID_INFO, ESP_GATT_PERM_READ,
                           GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_CONTROL_POINT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_HID_CONTROL_POINT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)GATTS_CHAR_UUID_HID_CONTROL_POINT, ESP_GATT_PERM_WRITE,
                           GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_HID_CONTROL_POINT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_WRITE,
                           sizeof(uint16_t), sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_REPORT_MAP]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_HID_REPORT_MAP]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)GATTS_CHAR_UUID_HID_REPORT_MAP, ESP_GATT_PERM_READ,
                           GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_REPORT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_WRITE,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_HID_REPORT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)GATTS_CHAR_UUID_HID_REPORT, ESP_GATT_PERM_READ,
                           GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_PROTOCOL_MODE]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    /* Characteristic Value */
    [IDX_CHAR_VAL_HID_PROTOCOL_MODE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_PROTOCOL_MODE, ESP_GATT_PERM_READ,
                           GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [IDX_CHAR_CFG_HID_PROTOCOL_MODE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                           sizeof(uint16_t), sizeof(char_value), (uint8_t *)char_value}},

  };

static const char *ble_gap_evt_names[] = {
  "ADV_DATA_SET_COMPLETE",
  "SCAN_RSP_DATA_SET_COMPLETE",
  "SCAN_PARAM_SET_COMPLETE",
  "SCAN_RESULT",
  "ADV_DATA_RAW_SET_COMPLETE",
  "SCAN_RSP_DATA_RAW_SET_COMPLETE",
  "ADV_START_COMPLETE",
  "SCAN_START_COMPLETE",
  "AUTH_CMPL",
  "KEY",
  "SEC_REQ",
  "PASSKEY_NOTIF",
  "PASSKEY_REQ",
  "OOB_REQ",
  "LOCAL_IR",
  "LOCAL_ER",
  "NC_REQ",
  "ADV_STOP_COMPLETE",
  "SCAN_STOP_COMPLETE",
  "SET_STATIC_RAND_ADDR",
  "UPDATE_CONN_PARAMS",
  "SET_PKT_LENGTH_COMPLETE",
  "SET_LOCAL_PRIVACY_COMPLETE",
  "REMOVE_BOND_DEV_COMPLETE",
  "CLEAR_BOND_DEV_COMPLETE",
  "GET_BOND_DEV_COMPLETE",
  "READ_RSSI_COMPLETE",
  "UPDATE_WHITELIST_COMPLETE"
};

static const char *ble_gatts_evt_names[] = {
  "ESP_GATTS_REG_EVT",
  "ESP_GATTS_READ_EVT",
  "ESP_GATTS_WRITE_EVT",
  "ESP_GATTS_EXEC_WRITE_EVT",
  "ESP_GATTS_MTU_EVT",
  "ESP_GATTS_CONF_EVT",
  "ESP_GATTS_UNREG_EVT",
  "ESP_GATTS_CREATE_EVT",
  "ESP_GATTS_ADD_INCL_SRVC_EVT",
  "ESP_GATTS_ADD_CHAR_EVT",
  "ESP_GATTS_ADD_CHAR_DESCR_EVT",
  "ESP_GATTS_DELETE_EVT",
  "ESP_GATTS_START_EVT",
  "ESP_GATTS_STOP_EVT",
  "ESP_GATTS_CONNECT_EVT",
  "ESP_GATTS_DISCONNECT_EVT",
  "ESP_GATTS_OPEN_EVT",
  "ESP_GATTS_CANCEL_OPEN_EVT",
  "ESP_GATTS_CLOSE_EVT",
  "ESP_GATTS_LISTEN_EVT",
  "ESP_GATTS_CONGEST_EVT",
  "ESP_GATTS_RESPONSE_EVT",
  "ESP_GATTS_CREAT_ATTR_TAB_EVT",
  "ESP_GATTS_SET_ATTR_VAL_EVT",
  "ESP_GATTS_SEND_SERVICE_CHANGE_EVT"
};

const char *ble_gap_evt_str(uint8_t event) {
    if (event >= SIZEOF_ARRAY(ble_gap_evt_names)) {
        return "UNKNOWN";
    }
    return ble_gap_evt_names[event];
}

const char *ble_gatts_evt_str(uint8_t event) {
    if (event >= SIZEOF_ARRAY(ble_gatts_evt_names)) {
        return "UNKNOWN";
    }
    return ble_gatts_evt_names[event];
}

const char *esp_ble_key_type_str(esp_ble_key_type_t key_type) {
    const char *key_str = NULL;
    switch (key_type) {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;
    }
    return key_str;
}

const char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
   const char *auth_str = NULL;
   switch(auth_req) {
    case ESP_LE_AUTH_NO_BOND:
        auth_str = "ESP_LE_AUTH_NO_BOND";
        break;
    case ESP_LE_AUTH_BOND:
        auth_str = "ESP_LE_AUTH_BOND";
        break;
    case ESP_LE_AUTH_REQ_MITM:
        auth_str = "ESP_LE_AUTH_REQ_MITM";
        break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
        auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
        auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
        break;
    case ESP_LE_AUTH_REQ_SC_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
        break;
    default:
        auth_str = "INVALID BLE AUTH REQ";
        break;
   }

   return auth_str;
}

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
    logger.info("BLE GAP EVENT SCAN_RESULT");
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
    adv_config_done &= (~ADV_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    adv_config_done &= (~ADV_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
    if (adv_config_done == 0){
      esp_ble_gap_start_advertising(&adv_params);
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
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
    logger.info("BLE GAP ADV_TERMINATED");
    break;

    /*
     * CONNECTION
     * */
  case ESP_GAP_BLE_GET_DEV_NAME_COMPLETE_EVT:
    logger.debug("BLE GAP GET_DEV_NAME_COMPLETE");
    // print the name
    logger.info("BLE GAP DEVICE NAME: {}", param->get_dev_name_cmpl.name);
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
    logger.info("BLE GAP KEY type = {}",
                esp_ble_key_type_str(param->ble_security.ble_key.key_type));
    break;

  case ESP_GAP_BLE_PASSKEY_NOTIF_EVT: { // ESP_IO_CAP_OUT
    int received_passkey = (int)param->ble_security.key_notif.passkey;
    // The app will receive this evt when the IO has Output capability and the peer device IO
    // has Input capability. Show the passkey number to the user to input it in the peer device.
    logger.info("BLE GAP PASSKEY_NOTIF passkey: {}", received_passkey);
  }
    break;

  case ESP_GAP_BLE_NC_REQ_EVT: { // ESP_IO_CAP_IO
    int received_passkey = (int)param->ble_security.key_notif.passkey;
    // The app will receive this event when the IO has DisplayYesNO capability and the peer
    // device IO also has DisplayYesNo capability. show the passkey number to the user to
    // confirm it with the number displayed by peer device.
    logger.info("BLE GAP NC_REQ passkey: {}", received_passkey);
    esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
  }
    break;

  case ESP_GAP_BLE_PASSKEY_REQ_EVT: // ESP_IO_CAP_IN
    // The app will receive this evt when the IO has Input capability and the peer device IO has
    // Output capability. See the passkey number on the peer device and send it back.
    logger.info("BLE GAP PASSKEY_REQ");
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
    logger.info("BLE GAP SEC_REQ");

    // Send the positive(true) security response to the peer device to accept the security
    // request. If not accept the security request, should send the security response with
    // negative(false) accept value.
    esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
    break;

  case ESP_GAP_BLE_PHY_UPDATE_COMPLETE_EVT:
    logger.info("BLE GAP PHY_UPDATE_COMPLETE");
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
    logger.info("ESP_GATTS_REG_EVT");
    // TODO: set name here to what the user wants....
    esp_ble_gap_set_device_name(CONFIG_DEVICE_NAME);

    esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, IDX_HID_NB, SVC_INST_ID);
    break;
  case ESP_GATTS_READ_EVT:
    for (int i = 0; i < 6; i++) {
      peer_address += ((uint64_t)param->read.bda[i]) << (i * 8);
    }
    logger.debug("ESP_GATTS_READ_EVT, peer_address: {:#x}", peer_address);
    // TODO: write output/response?
    break;
  case ESP_GATTS_WRITE_EVT:
    for (int i = 0; i < 6; i++) {
      peer_address += ((uint64_t)param->write.bda[i]) << (i * 8);
    }
    logger.debug("ESP_GATTS_WRITE_EVT, peer_address: {:#x}", peer_address);
    logger.debug("                     handle: {}, value len: {}", param->write.handle, param->write.len);
    if (!param->write.is_prep){
      bool is_cfg_handle =
        hid_handle_table[IDX_CHAR_CFG_HID_CONTROL_POINT] == param->write.handle ||
        hid_handle_table[IDX_CHAR_CFG_HID_PROTOCOL_MODE] == param->write.handle;

      if (is_cfg_handle && param->write.len == 2){
        logger.debug("Configuration of {}",
                     param->write.handle == hid_handle_table[IDX_CHAR_CFG_HID_CONTROL_POINT]
                     ? "IDX_CHAR_CFG_HID_CONTROL_POINT"
                     : "IDX_CHAR_CFG_HID_PROTOCOL_MODE");
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
    // the length of gattc prepare write data must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
    logger.warn("ESP_GATTS_EXEC_WRITE_EVT not implemented");
    break;
  case ESP_GATTS_SET_ATTR_VAL_EVT:
    logger.info("ESP_GATTS_SET_ATTR_VAL_EVT, attr_handle {}, srvc_handle {}, status {}",
                (int)param->set_attr_val.attr_handle,
                (int)param->set_attr_val.srvc_handle,
                (int)param->set_attr_val.status);
    break;
  case ESP_GATTS_MTU_EVT:
    logger.info("ESP_GATTS_MTU_EVT, MTU {}", (int)param->mtu.mtu);
    break;
  case ESP_GATTS_CONF_EVT:
    logger.info("ESP_GATTS_CONF_EVT, status = {}, attr_handle {}", (int)param->conf.status, (int)param->conf.handle);

    break;
  case ESP_GATTS_START_EVT:
    logger.info("SERVICE_START_EVT, status {}, service_handle {}", (int)param->start.status, (int)param->start.service_handle);
    break;
  case ESP_GATTS_CONNECT_EVT: {
    logger.info("ESP_GATTS_CONNECT_EVT, conn_id = {}", (int)param->connect.conn_id);
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
  }
    break;
  case ESP_GATTS_DISCONNECT_EVT:
    logger.info("ESP_GATTS_DISCONNECT_EVT, reason = {:#x}", (int)param->disconnect.reason);
    esp_ble_gap_start_advertising(&adv_params);
    break;
  case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
    if (param->add_attr_tab.status != ESP_GATT_OK){
      logger.error("create attribute table failed, error code={:#x}", (int)param->add_attr_tab.status);
    }
    else if (param->add_attr_tab.num_handle != IDX_HID_NB){
      logger.error("create attribute table abnormally, num_handle ({}) \
                        doesn't equal to IDX_HID_NB({})", (int)param->add_attr_tab.num_handle, (int)IDX_HID_NB);
    }
    else {
      logger.info("create attribute table successfully, the number handle = {}", (int)param->add_attr_tab.num_handle);
      memcpy(hid_handle_table, param->add_attr_tab.handles, sizeof(hid_handle_table));
      esp_ble_gatts_start_service(hid_handle_table[IDX_SVC_HID]);
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


/////////////////BLE///////////////////////

// Initializes BLE
void hid_service_table_init() {
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
