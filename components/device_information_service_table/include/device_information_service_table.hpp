#pragma once

#include <cstring>

#include <esp_bt.h>
#include <esp_gatt_defs.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

/// Device Information Service Attributes Indexes
enum
{
    DIS_IDX_SVC,

    DIS_IDX_PNP_CHAR,
    DIS_IDX_PNP_VAL,

    DIS_IDX_MANUFACTURER_NAME_CHAR,
    DIS_IDX_MANUFACTURER_NAME_VAL,

    DIS_IDX_MODEL_NUMBER_CHAR,
    DIS_IDX_MODEL_NUMBER_VAL,

    DIS_IDX_SERIAL_NUMBER_CHAR,
    DIS_IDX_SERIAL_NUMBER_VAL,

    DIS_IDX_NB,
};

extern uint64_t pnp_id;
static constexpr uint16_t MANUFACTURER_NAME_MAX_LEN = 100;
extern uint8_t manufacturer_name[MANUFACTURER_NAME_MAX_LEN];
extern uint16_t manufacturer_name_length;

static constexpr uint16_t MODEL_NUMBER_MAX_LEN = 100;
extern uint8_t model_number[MODEL_NUMBER_MAX_LEN];
extern uint16_t model_number_length;

static constexpr uint16_t SERIAL_NUMBER_MAX_LEN = 100;
extern uint8_t serial_number[SERIAL_NUMBER_MAX_LEN];
extern uint16_t serial_number_length;

extern const esp_gatts_attr_db_t dis_att_db[DIS_IDX_NB];
extern uint16_t dis_handle_table[DIS_IDX_NB];
