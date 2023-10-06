#pragma once

#include <esp_bt.h>
#include <esp_gatt_defs.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

/// Battery Service Attributes Indexes
enum
{
    BAS_IDX_SVC,

    BAS_IDX_BATT_LVL_CHAR,
    BAS_IDX_BATT_LVL_VAL,
    BAS_IDX_BATT_LVL_NTF_CFG,
    BAS_IDX_BATT_LVL_PRES_FMT,

    BAS_IDX_NB,
};

extern uint8_t battery_level;
extern const esp_gatts_attr_db_t bas_att_db[BAS_IDX_NB];
