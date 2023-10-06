# ESP HID Service Table

Example providing HID over GATT (HoG) profile for BLE on ESP using the ESP-IDF
GATT Service Table APIs.

https://github.com/finger563/esp-hid-service-table/assets/213467/f972932c-6285-44ac-a576-f49e8d22fc9b

## Implementation

The way this example works is to create the following service tables in order:

- Battery Service (0x180F)
- Device Information Service (0x180A)
- HID Service (0x1812)

The HID service is the last service created in the table, and uses the
`include_service_uuid` (0x2802) attribute to include the Battery and Device
Information services in the HID service.

This example was based on the ESP-IDF [ble_hid_device_demo
example](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/ble/ble_hid_device_demo),
which performs a similar function for a mouse/keyboard input device also using
the GATT Service Table APIs (see
[hid_device_le_prf.c](https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/ble_hid_device_demo/main/hid_device_le_prf.c))

## Cloning

Since this repo contains a submodule, you need to make sure you clone it
recursively, e.g. with:

``` sh
git clone --recurse-submodules git@github.com:finger563/esp-hid-service-table
```

Alternatively, you can always ensure the submodules are up to date after cloning
(or if you forgot to clone recursively) by running:

``` sh
git submodule update --init --recursive
```

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Output

![CleanShot 2023-10-06 at 16 06 58](https://github.com/finger563/esp-hid-service-table/assets/213467/589c8a6e-8ba2-4419-84a7-7ddaaa60fb9f)
