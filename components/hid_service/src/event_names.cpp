#include "event_names.hpp"

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(*a))

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
