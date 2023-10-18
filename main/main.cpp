#include <chrono>
#include <thread>

#include <esp_random.h>

#include "hid_service.hpp"

#include "logger.hpp"
#include "task.hpp"

#include "xbox.hpp"

using namespace std::chrono_literals;

extern "C" void app_main(void) {
  static auto start = std::chrono::high_resolution_clock::now();
  static auto elapsed = [&]() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start).count();
  };

  espp::Logger logger({.tag = "HID Service Table Example", .level = espp::Logger::Verbosity::DEBUG});

  logger.info("Bootup");

  // Initialize NVS.
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK( ret );

  logger.info("Device name: '{}'", CONFIG_DEVICE_NAME);

  // Initialize the bluetooth subsystem
  ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  if (ret) {
    logger.error("esp_bt_controller_mem_release failed: {}", ret);
    return;
  }

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret) {
    logger.error("enable controller failed");
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
    logger.error("enable controller failed");
    return;
  }

  logger.info("init bluetooth");
  ret = esp_bluedroid_init();
  if (ret) {
    logger.error("init bluetooth failed");
    return;
  }
  ret = esp_bluedroid_enable();
  if (ret) {
    logger.error("enable bluetooth failed");
    return;
  }

  // initialize the hid service table
  hid_service_init(CONFIG_DEVICE_NAME);

  // set the device plug and play ID (vendor ID, product ID, product version)
  uint16_t vendor_id = CONFIG_VENDOR_ID;
  uint16_t product_id = CONFIG_PRODUCT_ID;
  uint16_t product_version = CONFIG_PRODUCT_VERSION;
  hid_service_set_pnp_id(vendor_id, product_id, product_version);

  // set the manufacturer name
  std::string manufacturer_name = CONFIG_MANUFACTURER_NAME;
  hid_service_set_manufacturer_name(manufacturer_name);

  // set the model number string, which is the product ID in hex
  std::string model_number = fmt::format("{:04x}", product_id);
  hid_service_set_model_number(model_number);

  // set the serial number
  uint32_t random_number = esp_random();
  std::string serial_number = fmt::format("{:010d}", random_number);
  hid_service_set_serial_number(serial_number);

  // set the HID report descriptor (to be the same as the xbox elite wireless controller)
  hid_service_set_report_descriptor((uint8_t*)xb::report_descriptor, sizeof(xb::report_descriptor));

  // make a task to send input reports every second
  espp::Task task({
      .name = "Input Report Task",
        .callback = [&](auto &m, auto &cv) -> bool {
          auto start = std::chrono::steady_clock::now();
          if (hid_service_is_connected()) {
            logger.debug("[{:.3f}] Sending new input report!", elapsed());
            static xb::InputReport report;
            static constexpr size_t report_size = sizeof(xb::InputReport);
            static constexpr uint16_t AXIS_UP_VALUE = 65534;
            static constexpr uint16_t AXIS_DOWN_VALUE = 0;
            static constexpr uint16_t AXIS_CENTER_VALUE = 32767;
            static bool go_up = true;
            // toggle the 'b' button, which acts as the 'back' button on Android
            // report.btn_2 = !report.btn_2; // NOTE: disabling this because it's really annoying...
            // toggle the up/down on the left joystick (axis_y, center is 32768, up is 65535, down is 0)
            if (go_up) {
              report.axis_y = AXIS_UP_VALUE;
              hid_service_send_input_report((const uint8_t*)&report, report_size);

            } else {
              report.axis_y = AXIS_DOWN_VALUE;
              hid_service_send_input_report((const uint8_t*)&report, report_size);
            }
            // put it back to center
            report.axis_y = AXIS_CENTER_VALUE;
            hid_service_send_input_report((const uint8_t*)&report, report_size);
            // toggle the direction
            go_up = !go_up;
            // update the battery level
            static uint8_t battery_level = 0;
            battery_level = (battery_level) % 100 + 5;
            hid_service_set_battery_level(battery_level);
          }

          std::unique_lock<std::mutex> lock(m);
          cv.wait_until(lock, start + 1s);
          // we don't want to stop the task, so return false
          return false;
        },
        .stack_size_bytes = 4096,
        });
  task.start();

  // loop forever
  while (true) {
    std::this_thread::sleep_for(1s);
  }
}
