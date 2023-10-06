#include "battery_service_table.hpp"

/// Battery Service
static const uint16_t battery_svc = ESP_GATT_UUID_BATTERY_SERVICE_SVC;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_write_no_resp       = ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_read_notify         = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write          = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_no_resp  = ESP_GATT_CHAR_PROP_BIT_READ  | ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static const uint16_t bat_lev_uuid     = ESP_GATT_UUID_BATTERY_LEVEL;
static const uint8_t  bat_lev_ccc[2]   = {0x00, 0x00};
static const uint16_t char_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;

#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

/// characteristic presentation information
struct prf_char_pres_fmt
{
  /// Unit (The Unit is a UUID)
  uint16_t unit;
  /// Description
  uint16_t description;
  /// Format
  uint8_t format;
  /// Exponent
  uint8_t exponent;
  /// Name space
  uint8_t name_space;
};

uint8_t battery_level = 50;

/// Battery Service Attribute Table
const esp_gatts_attr_db_t bas_att_db[BAS_IDX_NB] =
  {
    // Battery Service Declaration
    [BAS_IDX_SVC] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(battery_svc), (uint8_t *)&battery_svc}},

    // Battery level Characteristic Declaration
    [BAS_IDX_BATT_LVL_CHAR] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

    // Battery level Characteristic Value
    [BAS_IDX_BATT_LVL_VAL] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&bat_lev_uuid, ESP_GATT_PERM_READ,
      sizeof(uint8_t), sizeof(uint8_t), &battery_level}},

    // Battery level Characteristic - Client Characteristic Configuration Descriptor
    [BAS_IDX_BATT_LVL_NTF_CFG] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(bat_lev_ccc), (uint8_t *)bat_lev_ccc}},

    // Battery level report Characteristic Declaration
    [BAS_IDX_BATT_LVL_PRES_FMT] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&char_format_uuid, ESP_GATT_PERM_READ,
      sizeof(struct prf_char_pres_fmt), 0, NULL}},
  };
