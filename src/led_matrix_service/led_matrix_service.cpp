#include "led_matrix_service.hpp"

#include <zephyr/kernel.h>

#define LSD_SERVICE_UUID_VAL                                                   \
  BT_UUID_128_ENCODE(0x4fd3af2a, 0x10e8, 0x474f, 0x84d7, 0x722bcfd3efc3)

#define LSD_WIDTH_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)

#define LSD_HEIGHT_UUID_VAL                                                    \
  BT_UUID_128_ENCODE(0x730134f2, 0xe403, 0x4956, 0xa998, 0xa4649f0fa1bf)

#define LSD_X_UUID_VAL                                                         \
  BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)

#define LSD_Y_UUID_VAL                                                         \
  BT_UUID_128_ENCODE(0xf1fb7f33, 0xa652, 0x4de0, 0x83f9, 0xbf3288f83f6f)

#define LSD_BRIGHTNESS_UUID_VAL                                                \
  BT_UUID_128_ENCODE(0xa1e0f55c, 0x2d1b, 0x4fca, 0xae9d, 0xefb3248c202a)

#define LSD_COLOR_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

constexpr bt_uuid_128 SERVICE_UUID = BT_UUID_INIT_128(LSD_SERVICE_UUID_VAL);

constexpr bt_uuid_128 WIDTH_UUID = BT_UUID_INIT_128(LSD_WIDTH_UUID_VAL);

constexpr bt_uuid_128 HEIGHT_UUID = BT_UUID_INIT_128(LSD_HEIGHT_UUID_VAL);

constexpr bt_uuid_128 X_UUID = BT_UUID_INIT_128(LSD_X_UUID_VAL);

constexpr bt_uuid_128 Y_UUID = BT_UUID_INIT_128(LSD_Y_UUID_VAL);

constexpr bt_uuid_128 BRIGHTNESS_UUID =
    BT_UUID_INIT_128(LSD_BRIGHTNESS_UUID_VAL);

constexpr bt_uuid_128 COLOR_UUID = BT_UUID_INIT_128(LSD_COLOR_UUID_VAL);

struct Data {
  std::uint16_t width{LED_MATRIX_SERVICE_WIDTH};

  std::uint16_t height{LED_MATRIX_SERVICE_HEIGHT};

  std::uint16_t x;

  std::uint16_t y;

  std::uint8_t brightness;

  std::array<std::uint8_t, 3> color;
};

static Data data;

ssize_t read_width_cb(bt_conn* conn, const bt_gatt_attr* attr, void* buf,
                      uint16_t len, uint16_t offset) {
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data.width,
                           sizeof(data.width));
}

BT_GATT_SERVICE_DEFINE(
    led_matrix_service,
    BT_GATT_PRIMARY_SERVICE(const_cast<bt_uuid_128 *>(&SERVICE_UUID)),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&WIDTH_UUID),
                           BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN,
                           &read_width_cb<MATRIX_SIZE>, NULL, &data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&HEIGHT_UUID),
                           BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN,
                           &read_height_cb<MATRIX_SIZE>, NULL, &data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&X_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_x_cb<MATRIX_SIZE>, &write_x_cb<MATRIX_SIZE>,
                           &data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&Y_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_y_cb<MATRIX_SIZE>, &write_y_cb<MATRIX_SIZE>,
                           &data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&BRIGHTNESS_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_brightness_cb<MATRIX_SIZE>,
                           &write_brightness_cb<MATRIX_SIZE>, &data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&COLOR_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_color_cb<MATRIX_SIZE>,
                           &write_color_cb<MATRIX_SIZE>, &data));
