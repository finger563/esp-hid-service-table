#pragma once

#include <string>

#include <esp_bt.h>
#include <esp_gap_bt_api.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

/* Attributes State Machine */
enum
  {
    // HID Service, UUID: 0x1812
    IDX_SVC_HID,

    // Included Service Battery Service, UUID: 0x180F
    IDX_HID_INCL_BAT_SVC,

    // Included Service Device Information Service, UUID: 0x180A
    IDX_HID_INCL_DIS_SVC,

    // HID Information characteristic, UUID: 0x2A4A, Properties: read
    IDX_CHAR_HID_INFO,
    IDX_CHAR_VAL_HID_INFO,

    // HID Control Point characteristic, UUID: 0x2A4C, Properties: write without response
    IDX_CHAR_HID_CONTROL_POINT,
    IDX_CHAR_VAL_HID_CONTROL_POINT,

    // HID Report Map characteristic, UUID: 0x2A4B, Properties: read
    IDX_CHAR_HID_REPORT_MAP,
    IDX_CHAR_VAL_HID_REPORT_MAP,
    IDX_CHAR_EXT_HID_REPORT_MAP,

    // HID Protocol Mode characteristic, UUID: 0x2A4E, Properties: read, write without response
    IDX_CHAR_HID_PROTOCOL_MODE,
    IDX_CHAR_VAL_HID_PROTOCOL_MODE,

    // HID Report characteristic, UUID: 0x2A4D, Properties: read, notify
    IDX_CHAR_HID_REPORT_XB,
    IDX_CHAR_VAL_HID_REPORT_XB,
    IDX_CHAR_CFG_HID_REPORT_XB,
    IDX_CHAR_REP_HID_REPORT_XB,

    // HID Report characteristic, UUID: 0x2A4D, Properties: read, notify
    IDX_CHAR_HID_REPORT,
    IDX_CHAR_VAL_HID_REPORT,
    IDX_CHAR_REP_HID_REPORT,

    IDX_HID_NB,
  };


extern uint8_t *report_descriptor;
extern size_t report_descriptor_len;
extern const esp_gatts_attr_db_t hid_gatt_db[IDX_HID_NB];

void hid_service_table_set_report_descriptor(uint8_t *descriptor, size_t len);
void hid_service_table_set_included_battery_service_handles(uint16_t start_handle, uint16_t end_handle);
void hid_service_table_set_included_dev_info_service_handles(uint16_t start_handle, uint16_t end_handle);
