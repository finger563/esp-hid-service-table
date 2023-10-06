#pragma once

#include "hid.hpp"

namespace xb {

  // This is the
  struct InputReport {
    uint16_t axis_x;
    uint16_t axis_y;
    uint16_t axis_z;
    uint16_t axis_rz;
    uint16_t brake : 10;
  uint16_t : 6; // unused bits
    uint16_t accelerator : 10;
  uint16_t : 6; // unused bits
    uint8_t hat : 4;
  uint8_t : 4; // unused bits
    uint16_t btn_1 : 1; // a
    uint16_t btn_2 : 1; // b
    uint16_t btn_3 : 1; // unused
    uint16_t btn_4 : 1; // x
    uint16_t btn_5 : 1; // y
    uint16_t btn_6 : 1; // unused
    uint16_t btn_7 : 1; // l1
    uint16_t btn_8 : 1; // r1
    uint16_t btn_9 : 1; // unused
    uint16_t btn_10 : 1; // unused
    uint16_t btn_11 : 1; // unused
    uint16_t btn_12 : 1; // menu
    uint16_t btn_13 : 1; // mode
    uint16_t btn_14 : 1; // l3
    uint16_t btn_15 : 1; // r3
  uint16_t : 1; // unused bits
    uint8_t options : 1;
    uint8_t capture : 1;
    uint8_t home : 1;
  uint8_t : 5; // unused bits
  } __attribute__((packed));

  static constexpr uint8_t report_descriptor[] = {
    hid::USAGE_PAGE,
    hid::GENERIC_DESKTOP,
    hid::USAGE,
    hid::GAMEPAD,
    hid::START_COLLECTION,
    hid::APPLICATION,
    hid::REPORT_ID,
    0x01,

    // x/y axes ([0,65535])
    hid::USAGE,
    hid::POINTER,
    hid::START_COLLECTION,
    hid::PHYSICAL,
    hid::USAGE,
    hid::AXIS_X,
    hid::USAGE,
    hid::AXIS_Y,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM_32,
    hid::low_byte(65534),
    hid::high_byte(65534),
    0x00,
    0x00,
    hid::REPORT_COUNT,
    0x02,
    hid::REPORT_SIZE,
    0x10,
    hid::INPUT,
    0x02,
    hid::END_COLLECTION, //     Physical

    // rx/ry axes ([0,65535])
    hid::USAGE,
    hid::POINTER,
    hid::START_COLLECTION,
    hid::PHYSICAL,
    hid::USAGE,
    hid::AXIS_Z,
    hid::USAGE,
    hid::AXIS_RZ,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM_32,
    hid::low_byte(65534),
    hid::high_byte(65534),
    0x00,
    0x00,
    hid::REPORT_COUNT,
    0x02,
    hid::REPORT_SIZE,
    0x10,
    hid::INPUT,
    0x02,
    hid::END_COLLECTION, //     Physical

    // brake ([0,1023])
    hid::USAGE_PAGE,
    hid::SIMULATION_CONTROLS,
    hid::USAGE,
    hid::BRAKE,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM_16,
    hid::low_byte(1023),
    hid::high_byte(1023),
    hid::REPORT_COUNT,
    0x01,
    hid::REPORT_SIZE,
    0x0A,
    hid::INPUT,
    0x02,
    // padding for brake
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM,
    0x00,
    hid::REPORT_SIZE,
    0x06,
    hid::REPORT_COUNT,
    0x01,
    hid::INPUT,
    0x03, // constant, variable, abs

    // accelerator ([0,1023])
    hid::USAGE_PAGE,
    hid::SIMULATION_CONTROLS,
    hid::USAGE,
    hid::ACCELERATOR,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM_16,
    hid::low_byte(1023),
    hid::high_byte(1023),
    hid::REPORT_COUNT,
    0x01,
    hid::REPORT_SIZE,
    0x0A,
    hid::INPUT,
    0x02,
    // padding for rz
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM,
    0x00,
    hid::REPORT_SIZE,
    0x06,
    hid::REPORT_COUNT,
    0x01,
    hid::INPUT,
    0x03, // constant, variable, abs

    // hat switch
    hid::USAGE_PAGE,
    hid::GENERIC_DESKTOP,
    hid::USAGE,
    hid::HAT_SWITCH,
    hid::LOGICAL_MINIMUM,
    0x01,
    hid::LOGICAL_MAXIMUM,
    0x08,
    hid::PHYSICAL_MINIMUM,
    0x00,
    hid::PHYSICAL_MAXIMUM_16,
    hid::low_byte(315),
    hid::high_byte(315),
    hid::UNIT_16,
    0x14,
    0x00, // system: english rotation, length: centimeter
    hid::REPORT_SIZE,
    0x04,
    hid::REPORT_COUNT,
    0x01,
    hid::INPUT,
    0x42, // (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    // padding for hat switch
    hid::REPORT_SIZE,
    0x04,
    hid::REPORT_COUNT,
    0x01,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM,
    0x00,
    hid::PHYSICAL_MINIMUM,
    0x00,
    hid::PHYSICAL_MAXIMUM,
    0x00,
    hid::UNIT,
    0x00, // none
    hid::INPUT,
    0x03, // constant

    // buttons (15)
    hid::USAGE_PAGE,
    hid::BUTTON,
    hid::USAGE_MINIMUM,
    0x01,
    hid::USAGE_MAXIMUM,
    0x0F,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM,
    0x01,
    hid::REPORT_SIZE,
    0x01,
    hid::REPORT_COUNT,
    0x0F,
    hid::INPUT,
    0x02,
    // padding for buttons
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM,
    0x00,
    hid::REPORT_SIZE,
    0x01,
    hid::REPORT_COUNT,
    0x01,
    hid::INPUT,
    0x03, // constant

    // back button
    hid::USAGE_PAGE,
    hid::CONSUMER, //   Usage Page (Consumer)
    hid::REPORT_COUNT,
    0x01, //   Report Count (1)
    hid::REPORT_SIZE,
    0x01, //   Report Size (1)
    hid::USAGE_16,
    hid::low_byte(hid::AC_BACK),
    hid::high_byte(hid::AC_BACK), //   Usage (AC Back) // options
    hid::LOGICAL_MINIMUM,
    0x00, //   Logical Minimum (0)
    hid::LOGICAL_MAXIMUM,
    0x01, //   Logical Maximum (1)
    hid::INPUT,
    0x02, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    hid::LOGICAL_MINIMUM,
    0x00, //   Logical Minimum (0)
    hid::LOGICAL_MAXIMUM,
    0x00, //   Logical Maximum (0)
    hid::REPORT_SIZE,
    0x07, //   Report Size (7)
    hid::REPORT_COUNT,
    0x01, //   Report Count (1)
    hid::INPUT,
    0x03, //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    // battery level
    hid::USAGE_PAGE,
    hid::GENERIC_DEVICE_CONTROLS,
    hid::USAGE,
    hid::BATTERY_STRENGTH,
    hid::REPORT_ID,
    0x04,
    hid::LOGICAL_MINIMUM,
    0x00,
    hid::LOGICAL_MAXIMUM_16,
    0xFF,
    0x00,
    hid::REPORT_SIZE,
    0x08,
    hid::REPORT_COUNT,
    0x01,
    hid::INPUT,
    0x02,

    // end
    hid::END_COLLECTION // Application
  };

} // namespace xb
