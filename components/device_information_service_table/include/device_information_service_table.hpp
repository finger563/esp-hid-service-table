#pragma once

#include <esp_bt.h>
#include <esp_gatt_defs.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

    // // Device Information Service, UUID: 0x180A
    // IDX_SVC_DEV_INFO,

    // // Device Information characteristic PnP ID, UUID: 0x2A50, Properties: read
    // IDX_CHAR_DEV_INFO_PNP_ID,

    // // Device Information characteristic Manufacturer Name String, UUID: 0x2A29, Properties: read
    // IDX_CHAR_DEV_INFO_MANUFACTURER_NAME,

    // // Device Information characteristic Model Number String, UUID: 0x2A24, Properties: read
    // IDX_CHAR_DEV_INFO_MODEL_NUMBER,

    // // Device Information characteristic Serial Number String, UUID: 0x2A25, Properties: read
    // IDX_CHAR_DEV_INFO_SERIAL_NUMBER,

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

extern const esp_gatts_attr_db_t dis_att_db[DIS_IDX_NB];
