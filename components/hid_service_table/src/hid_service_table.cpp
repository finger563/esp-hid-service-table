#include "hid_service_table.hpp"

/* Service */
// NOTE: these UUIDs are specified at 16-bit, which means that 1) they are not
// in reverse order, and 2) the rest of their full 128-bit UUIDs are the standard
// Bluetooth SIG base UUID (0x0000XXXX-0000-1000-8000-00805F9B34FB), and the 16-bit
// UUIDs are the last 2 bytes of the first 4 bytes of the full UUIDs (XXXX).
static const uint16_t GATTS_SERVICE_UUID_HID            = 0x1812;
static const uint16_t GATTS_CHAR_UUID_HID_INFO          = 0x2A4A;
static const uint16_t GATTS_CHAR_UUID_HID_CONTROL_POINT = 0x2A4C;
static const uint16_t GATTS_CHAR_UUID_HID_REPORT_MAP    = 0x2A4B;
static const uint16_t GATTS_CHAR_UUID_HID_REPORT        = 0x2A4D;
static const uint16_t GATTS_CHAR_UUID_HID_PROTOCOL_MODE = 0x2A4E;

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
 *  the data length must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
 */
#define HID_REPORT_MAX_LEN          255
#define HID_REPORT_MAP_MAX_LEN      512
#define HID_INFO_LEN                4
#define HID_PROTOCOL_MODE_LEN       1
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

static std::string manufacturer_name = CONFIG_MANUFACTURER_NAME;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;       // 0x2800
static const uint16_t include_service_uuid         = ESP_GATT_UUID_INCLUDE_SERVICE;   // 0x2802
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;      // 0x2803
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;// 0x2902
static const uint16_t hid_report_uuid              = ESP_GATT_UUID_HID_REPORT;        // 0x2A4D
static const uint16_t hid_report_map_ext_desc_uuid = ESP_GATT_UUID_EXT_RPT_REF_DESCR; // 0x2907
static const uint16_t hid_report_ref_descr_uuid    = ESP_GATT_UUID_RPT_REF_DESCR;     // 0x2908
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_write_no_resp       = ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_read_notify         = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write          = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_no_resp  = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint8_t notify_ccc[2]    = {0x01, 0x00}; // LSb corresponds to notifications (1 if enabled, 0 if disabled), next bit (bit 1) corresponds to indications - 1 if enabled, 0 if disabled
static const uint8_t ccc[2]           = {0x00, 0x00}; // LSb corresponds to notifications (1 if enabled, 0 if disabled), next bit (bit 1) corresponds to indications - 1 if enabled, 0 if disabled
static const uint8_t char_value[16]   = {0x00};
static const uint8_t protocol_mode[1] = {0x01}; // 0x01 = report mode, 0x00 = boot mode

struct hid_info_t {
  /// bcdHID
  uint16_t bcdHID;
  /// bCountryCode
  uint8_t bCountryCode;
  /// Flags
  uint8_t flags;
};

#define HID_FLAGS_REMOTE_WAKE           0x01      // RemoteWake
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x02      // NormallyConnectable
#define HID_FLAGS                                           \
  (HID_FLAGS_REMOTE_WAKE | HID_FLAGS_NORMALLY_CONNECTABLE)
static const uint8_t hid_info[] = {
  0x11, 0x01,             // bcdHID (USB HID version 1.11)
  0x00,                   // bCountryCode
  HID_FLAGS               // Flags
};

static esp_gatts_incl_svc_desc_t hid_incl_svc = {0};

static const uint16_t hid_ext_report_ref = ESP_GATT_UUID_BATTERY_LEVEL;

static const uint8_t hid_report_ref[] = {
  0x01, // report ID mouse in
  0x01, // report type (1 = input, 2 = output, 3 = feature)
};
static const uint8_t hid_report_ref_feature[] = {
  0x00, // report ID feature
  0x03, // report type (1 = input, 2 = output, 3 = feature)
};
const uint8_t *report_descriptor = NULL;
size_t report_descriptor_len = 0;

/* Full Database Description - Used to add attributes into the database */
const esp_gatts_attr_db_t hid_gatt_db[IDX_HID_NB] =
  {
    // Service Declaration
    [IDX_SVC_HID]           =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
                           sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_HID), (uint8_t *)&GATTS_SERVICE_UUID_HID}},

    // Include service UUID for the battery service which should be included within this service
    [IDX_HID_INCL_SVC]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&include_service_uuid,
                           ESP_GATT_PERM_READ,
                           sizeof(esp_gatts_incl_svc_desc_t), sizeof(esp_gatts_incl_svc_desc_t),
                           (uint8_t *)&hid_incl_svc}},
    /* Characteristic Declaration */
    [IDX_CHAR_HID_INFO]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    [IDX_CHAR_VAL_HID_INFO] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_INFO, ESP_GATT_PERM_READ,
                           sizeof(hid_info_t), sizeof(hid_info), (uint8_t *)&hid_info}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_CONTROL_POINT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write_no_resp}},
    [IDX_CHAR_VAL_HID_CONTROL_POINT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_CONTROL_POINT, ESP_GATT_PERM_WRITE,
                           sizeof(uint8_t), 0, NULL}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_REPORT_MAP]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
    [IDX_CHAR_VAL_HID_REPORT_MAP]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_REPORT_MAP, ESP_GATT_PERM_READ,
                           HID_REPORT_MAP_MAX_LEN, sizeof(report_descriptor), (uint8_t *)&report_descriptor}},
    [IDX_CHAR_EXT_HID_REPORT_MAP]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&hid_report_map_ext_desc_uuid, ESP_GATT_PERM_READ,
                           sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&hid_ext_report_ref}},

    /* Characteristic Declaration */
    [IDX_CHAR_HID_PROTOCOL_MODE]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},
    [IDX_CHAR_VAL_HID_PROTOCOL_MODE]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_PROTOCOL_MODE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                           sizeof(uint8_t), sizeof(protocol_mode), (uint8_t *)&protocol_mode}},


    /* Characteristic Declaration */
    [IDX_CHAR_HID_REPORT_XB]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},
    [IDX_CHAR_VAL_HID_REPORT_XB]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_REPORT, ESP_GATT_PERM_READ,
                           HID_REPORT_MAX_LEN, 0, NULL}},
    [IDX_CHAR_CFG_HID_REPORT_XB]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ,
                           sizeof(uint16_t), 0, NULL}},
    [IDX_CHAR_REP_HID_REPORT_XB]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid, ESP_GATT_PERM_READ,
                           sizeof(hid_report_ref), sizeof(hid_report_ref), (uint8_t *)&hid_report_ref}},


    /* Characteristic Declaration */
    [IDX_CHAR_HID_REPORT]      =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
                           CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}},
    [IDX_CHAR_VAL_HID_REPORT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HID_REPORT, ESP_GATT_PERM_READ,
                           HID_REPORT_MAX_LEN, 0, NULL}},
    [IDX_CHAR_REP_HID_REPORT]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&hid_report_ref_descr_uuid, ESP_GATT_PERM_READ,
                           sizeof(hid_report_ref_feature), sizeof(hid_report_ref_feature), (uint8_t *)hid_report_ref_feature}},


  };

void hid_service_table_set_included_service_handles(uint16_t start_handle, uint16_t end_handle) {
  hid_incl_svc.start_hdl = start_handle;
  hid_incl_svc.end_hdl = end_handle;
}
