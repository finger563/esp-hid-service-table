#include <chrono>
#include <thread>

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

  espp::Logger logger({.tag = "Template", .level = espp::Logger::Verbosity::DEBUG});

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

  // set the HID report descriptor (to be the same as the xbox elite wireless controller)
  hid_service_set_report_descriptor(xb::report_descriptor, sizeof(xb::report_descriptor));

  // initialize the hid service table
  hid_service_init();

  // make a simple task that prints "Hello World!" every second
  espp::Task task({
      .name = "Input Report Task",
        .callback = [&](auto &m, auto &cv) -> bool {
          if (hid_service_is_connected()) {
            logger.debug("[{:.3f}] Sending new input report!", elapsed());
            static xb::InputReport report;
            static constexpr size_t report_size = sizeof(xb::InputReport);
            static uint8_t report_data[report_size];
            // toggle the up/down on the left joystick (axis_y, center is 32768, up is 65535, down is 0)
            if (report.axis_y == 0) {
              report.axis_y = 65535;
            } else if (report.axis_y == 65535) {
              report.axis_y = 0;
            } else if (report.axis_y == 32768) {
              report.axis_y = 0;
            } else {
              report.axis_y = 32768;
            }
            // copy the report into the report data
            memcpy(report_data, &report, report_size);
            // send an input report
            hid_service_send_input_report(report_data, report_size);
          }

          std::unique_lock<std::mutex> lock(m);
          cv.wait_for(lock, 1s);
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
