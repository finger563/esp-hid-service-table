idf_component_register(
  INCLUDE_DIRS "include"
  SRC_DIRS "src"
  REQUIRES "bt" "mbedtls" "nvs_flash" "logger" "task" "timer"
)
