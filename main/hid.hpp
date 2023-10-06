#pragma once

// see: https://usb.org/sites/default/files/hut1_4.pdf

namespace hid {
// top-level types
static constexpr int USAGE_PAGE = 0x05;
static constexpr int USAGE_PAGE_16 = 0x06;
static constexpr int USAGE = 0x09;
static constexpr int USAGE_16 = 0x0A;
static constexpr int LOGICAL_MINIMUM = 0x15;
static constexpr int LOGICAL_MINIMUM_16 = 0x16;
static constexpr int LOGICAL_MINIMUM_32 = 0x17;
static constexpr int USAGE_MINIMUM = 0x19;
static constexpr int USAGE_MINIMUM_16 = 0x1A;
static constexpr int LOGICAL_MAXIMUM = 0x25;
static constexpr int LOGICAL_MAXIMUM_16 = 0x26;
static constexpr int LOGICAL_MAXIMUM_32 = 0x27;
static constexpr int USAGE_MAXIMUM = 0x29;
static constexpr int USAGE_MAXIMUM_16 = 0x2A;
static constexpr int PHYSICAL_MINIMUM = 0x35;
static constexpr int PHYSICAL_MINIMUM_16 = 0x36;
static constexpr int PHYSICAL_MAXIMUM = 0x45;
static constexpr int PHYSICAL_MAXIMUM_16 = 0x46;
static constexpr int UNIT_EXPONENT = 0x55;
static constexpr int UNIT = 0x65;
static constexpr int UNIT_16 = 0x66;
static constexpr int REPORT_SIZE = 0x75;
static constexpr int INPUT = 0x81;
static constexpr int INPUT_16 = 0x82;
static constexpr int REPORT_ID = 0x85;
static constexpr int OUTPUT = 0x91;
static constexpr int REPORT_COUNT = 0x95;
static constexpr int REPORT_COUNT_16 = 0x96;
static constexpr int START_COLLECTION = 0xA1;
static constexpr int FEATURE = 0xB1;
static constexpr int FEATURE_16 = 0xB2;
static constexpr int END_COLLECTION = 0xC0;

// usage page types
static constexpr int GENERIC_DESKTOP = 0x01;
static constexpr int SIMULATION_CONTROLS = 0x02;
static constexpr int VR_CONTROLS = 0x03;
static constexpr int SPORT_CONTROLS = 0x04;
static constexpr int GAME_CONTROLS = 0x05;
static constexpr int GENERIC_DEVICE_CONTROLS = 0x06;
static constexpr int KEYBOARD_CONTROLS = 0x07;
static constexpr int LEDS = 0x08;
static constexpr int BUTTON = 0x09;
static constexpr int CONSUMER = 0x0C;
static constexpr int PHYSICAL_INTERFACE = 0x0F;

// usage types
static constexpr int POINTER = 0x01;
static constexpr int MOUSE = 0x02;
static constexpr int JOYSTICK = 0x04;
static constexpr int GAMEPAD = 0x05;
static constexpr int KEYBOARD = 0x06;
static constexpr int KEYPAD = 0x07;
static constexpr int MULTI_AXIS_CONTROLLER = 0x08;
static constexpr int BATTERY_STRENGTH = 0x20;
static constexpr int AXIS_X = 0x30;
static constexpr int AXIS_Y = 0x31;
static constexpr int AXIS_Z = 0x32;
static constexpr int AXIS_RX = 0x33;
static constexpr int AXIS_RY = 0x34;
static constexpr int AXIS_RZ = 0x35;
static constexpr int SLIDER = 0x36;
static constexpr int DIAL = 0x37;
static constexpr int WHEEL = 0x38;
static constexpr int HAT_SWITCH = 0x39;
static constexpr int START = 0x3D;
static constexpr int SELECT = 0x3E;
static constexpr int AXIS_VX = 0x40;
static constexpr int AXIS_VY = 0x41;
static constexpr int AXIS_VZ = 0x42;
static constexpr int AXIS_VBRX = 0x43;
static constexpr int AXIS_VBRY = 0x44;
static constexpr int AXIS_VBRZ = 0x45;
static constexpr int DURATION = 0x50;
static constexpr int MAGNITUDE = 0x70;
static constexpr int LOOP_COUNT = 0x7C;
static constexpr int SYSTEM_CONTROL = 0x80;
static constexpr int SYSTEM_CONTEXT_MENU = 0x84;
static constexpr int SYSTEM_MAIN_MENU = 0x85;
static constexpr int SYSTEM_APP_MENU = 0x86;
static constexpr int SYSTEM_MENU_HELP = 0x87;
static constexpr int SYSTEM_MENU_EXIT = 0x88;
static constexpr int SYSTEM_MENU_SELECT = 0x89;
static constexpr int SYSTEM_MENU_RIGHT = 0x8A;
static constexpr int SYSTEM_MENU_LEFT = 0x8B;
static constexpr int SYSTEM_MENU_UP = 0x8C;
static constexpr int SYSTEM_MENU_DOWN = 0x8D;
static constexpr int DPAD_UP = 0x90;
static constexpr int DPAD_DOWN = 0x91;
static constexpr int DPAD_RIGHT = 0x92;
static constexpr int DPAD_LEFT = 0x93;
static constexpr int DC_ENABLE_ACTUATORS = 0x97;
static constexpr int START_DELAY = 0xA7;

// simulation controls
static constexpr int RUDDER = 0xBA;
static constexpr int THROTTLE = 0xBB;
static constexpr int ACCELERATOR = 0xC4;
static constexpr int BRAKE = 0xC5;
static constexpr int STEERING = 0xC8;

// consumer controls
static constexpr int POWER = 0x30;
static constexpr int RESET = 0x31;
static constexpr int SLEEP = 0x32;
static constexpr int MENU = 0x40;
static constexpr int MENU_PICK = 0x41;
static constexpr int MENU_UP = 0x42;
static constexpr int MENU_DOWN = 0x43;
static constexpr int MENU_LEFT = 0x44;
static constexpr int MENU_RIGHT = 0x45;
static constexpr int MENU_ESCAPE = 0x46;
static constexpr int MEDIA_SELECT_HOME = 0x9A;
static constexpr int SNAPSHOT = 0x65;
static constexpr int RECORD = 0xB2;
static constexpr int INVOKE_CAPTURE = 0xD0;
static constexpr int START_STOP_GAME_RECORD = 0xD1;
static constexpr int HISTORICAL_GAME_CAPTURE = 0xD2;
static constexpr int GAME_SCREENSHOT = 0xD3;
static constexpr int VOLUME_INCREMENT = 0xE9;
static constexpr int VOLUME_DECREMENT = 0xEA;
static constexpr int AC_EXIT = 0x204;
static constexpr int AC_PROPERTIES = 0x209;
static constexpr int AC_HOME = 0x223;
static constexpr int AC_BACK = 0x224;

// unit types
static constexpr int ANGULAR_POSITION = 0x12;

// collection types
static constexpr int PHYSICAL = 0x00;
static constexpr int APPLICATION = 0x01;
static constexpr int LOGICAL = 0x02;
static constexpr int REPORT = 0x03;

static constexpr int BUTTON_1 = 1;
static constexpr int BUTTON_2 = 2;
static constexpr int BUTTON_3 = 3;
static constexpr int BUTTON_4 = 4;
static constexpr int BUTTON_5 = 5;
static constexpr int BUTTON_6 = 6;
static constexpr int BUTTON_7 = 7;
static constexpr int BUTTON_8 = 8;
static constexpr int BUTTON_9 = 9;
static constexpr int BUTTON_10 = 10;
static constexpr int BUTTON_11 = 11;
static constexpr int BUTTON_12 = 12;
static constexpr int BUTTON_13 = 13;
static constexpr int BUTTON_14 = 14;
static constexpr int BUTTON_15 = 15;
static constexpr int BUTTON_16 = 16;
static constexpr int BUTTON_17 = 17;
static constexpr int BUTTON_18 = 18;

static constexpr int FORM_FIT_GAMEPAD = 0x3A;

namespace input {
static constexpr int CONST = (1 << 0);
static constexpr int VARIABLE = (1 << 1);
static constexpr int RELATIVE = (1 << 2);
static constexpr int WRAP = (1 << 3);
static constexpr int NON_LINEAR = (1 << 4);
static constexpr int NO_PREFERRED = (1 << 5);
static constexpr int NULL_STATE = (1 << 6);
static constexpr int VOLATILE = (1 << 7);
static constexpr int BUFFERED_BYTES = (1 << 8);

static constexpr int SEL = (0);               ///< Selector type, contained in a named array
static constexpr int SV = (CONST | VARIABLE); ///< Static value, read-only multiple bit value
static constexpr int SF = (CONST | VARIABLE); ///< Static flag, read only single bit value
static constexpr int DV = (VARIABLE);         ///< Dynamic value, read-write multiple bit value
static constexpr int DF = (VARIABLE);         ///< Dynamic flag, read-write single bit value
} // namespace input

static constexpr uint8_t low_byte(uint16_t v) { return v & 0xFF; }

static constexpr uint8_t high_byte(uint16_t v) { return v >> 8; }
} // namespace hid
