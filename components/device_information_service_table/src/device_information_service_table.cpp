#include "device_information_service_table.hpp"

/// Device Info Service
static const uint16_t dev_info_svc = ESP_GATT_UUID_DEVICE_INFO_SVC; // 0x180A
static const uint16_t pnp_id_uuid  = ESP_GATT_UUID_PNP_ID; // 0x2A50
static const uint16_t manufacturer_name_uuid = ESP_GATT_UUID_MANU_NAME; // 0x2A29
static const uint16_t model_number_uuid      = ESP_GATT_UUID_MODEL_NUMBER_STR; // 0x2A24
static const uint16_t serial_number_uuid     = ESP_GATT_UUID_SERIAL_NUMBER_STR; // 0x2A25

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;

static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;

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

/// Device Info PnP ID
// Vendor ID Source : 0x02 = USB Implementers Forum assigned Vendor ID value
// Vendor ID        : 0x045E = Microsoft Corporation
// Product ID       : 0x02FD = Xbox One Controller
// Product Version  : 0x0100 = v1.0.0
uint64_t pnp_id = 0x010002FD045E02;
// it should only be 7 bytes long
static constexpr uint16_t PNP_ID_SIZE = 7;

/// Device Info Manufacturer Name
uint8_t manufacturer_name[MANUFACTURER_NAME_MAX_LEN] = "Microsoft Corporation";
uint16_t manufacturer_name_length = strlen((char*)manufacturer_name);

/// Device Info Model Number
uint8_t model_number[MODEL_NUMBER_MAX_LEN] = "0x045E";
uint16_t model_number_length = strlen((char*)model_number);

/// Device Info Serial Number
uint8_t serial_number[SERIAL_NUMBER_MAX_LEN] = "000000000001";
uint16_t serial_number_length = strlen((char*)serial_number);

uint16_t dis_handle_table[DIS_IDX_NB];

/// Device Info Service Attribute Table
const esp_gatts_attr_db_t dis_att_db[DIS_IDX_NB] =
  {
    // Device Info Service Declaration
    [DIS_IDX_SVC] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(dev_info_svc), (uint8_t *)&dev_info_svc}},

    // Device Info PNP Characteristic Declaration
    [DIS_IDX_PNP_CHAR] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    // Device Info PNP Characteristic Value
    [DIS_IDX_PNP_VAL] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&pnp_id_uuid, ESP_GATT_PERM_READ,
      PNP_ID_SIZE, PNP_ID_SIZE, (uint8_t *)&pnp_id}},

    // Device Info Manufacturer Name Characteristic Declaration
    [DIS_IDX_MANUFACTURER_NAME_CHAR] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    // Device Info Manufacturer Name Characteristic Value
    [DIS_IDX_MANUFACTURER_NAME_VAL] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&manufacturer_name_uuid, ESP_GATT_PERM_READ,
      MANUFACTURER_NAME_MAX_LEN, manufacturer_name_length, (uint8_t *)&manufacturer_name}},

    // Device Info Model Number Characteristic Declaration
    [DIS_IDX_MODEL_NUMBER_CHAR] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    // Device Info Model Number Characteristic Value
    [DIS_IDX_MODEL_NUMBER_VAL] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&model_number_uuid, ESP_GATT_PERM_READ,
      MODEL_NUMBER_MAX_LEN, model_number_length, (uint8_t *)&model_number}},

    // Device Info Serial Number Characteristic Declaration
    [DIS_IDX_SERIAL_NUMBER_CHAR] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

    // Device Info Serial Number Characteristic Value
    [DIS_IDX_SERIAL_NUMBER_VAL] =
    {{ESP_GATT_AUTO_RSP},
     {ESP_UUID_LEN_16, (uint8_t *)&serial_number_uuid, ESP_GATT_PERM_READ,
      SERIAL_NUMBER_MAX_LEN, serial_number_length, (uint8_t *)&serial_number}},
  };
