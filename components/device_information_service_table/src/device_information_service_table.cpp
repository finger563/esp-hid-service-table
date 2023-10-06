#include "device_information_service_table.hpp"

/// Device Info Service
static const uint16_t dev_info_svc = ESP_GATT_UUID_DEVICE_INFO_SVC;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint16_t char_format_uuid = ESP_GATT_UUID_CHAR_PRESENT_FORMAT;

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

/// Device Info Service Attribute Table
const esp_gatts_attr_db_t dis_att_db[DIS_IDX_NB] =
  {
  };
