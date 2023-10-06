#pragma once

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

const char *ble_gap_evt_str(uint8_t event);
const char *ble_gatts_evt_str(uint8_t event);

const char *esp_ble_key_type_str(esp_ble_key_type_t key_type);
const char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req);
