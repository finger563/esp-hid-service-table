#pragma once

#include <atomic>
#include <chrono>
#include <string>

#include <esp_bt.h>
#include <esp_gap_bt_api.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_device.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>
#include <esp_random.h>
#include <nvs_flash.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "logger.hpp"
#include "task.hpp"
#include "timer.hpp"

#include "battery_service_table.hpp"
#include "device_information_service_table.hpp"
#include "hid_service_table.hpp"
#include "event_names.hpp"

bool hid_service_is_connected();
esp_bd_addr_t *hid_service_get_peer_address();
void hid_service_init(std::string_view device_name_string_view);
void hid_service_set_device_name(std::string_view device_name_string_view);
void hid_service_set_report_descriptor(uint8_t* report_descriptor, size_t report_descriptor_len);
void hid_service_send_input_report(const uint8_t* report, size_t report_len);
void hid_service_set_battery_level(const uint8_t level);
void hid_service_set_pnp_id(const uint16_t vendor_id, const uint16_t product_id, const uint16_t product_version);
void hid_service_set_manufacturer_name(std::string_view manufacturer_name_string_view);
void hid_service_set_model_number(std::string_view model_number_string_view);
void hid_service_set_serial_number(std::string_view serial_number_string_view);
