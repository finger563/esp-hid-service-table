#include <chrono>
#include <thread>

#include "hid_service_table.hpp"

#include "logger.hpp"
#include "task.hpp"

using namespace std::chrono_literals;

extern "C" void app_main(void) {
  static auto start = std::chrono::high_resolution_clock::now();
  static auto elapsed = [&]() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float>(now - start).count();
  };

  espp::Logger logger({.tag = "Template", .level = espp::Logger::Verbosity::DEBUG});

  logger.info("Bootup");

  hid_service_table_init();

  // make a simple task that prints "Hello World!" every second
  espp::Task task({
      .name = "Hello World",
      .callback = [&](auto &m, auto &cv) -> bool {
        logger.debug("[{:.3f}] Hello from the task!", elapsed());
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
