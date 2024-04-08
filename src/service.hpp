#pragma once

#include <cstdint>

#include <algorithm>
#include <expected>
#include <span>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>

/**
 * @brief LED service UUID value.
 */
#define LSD_SERVICE_UUID_VAL                                                   \
  BT_UUID_128_ENCODE(0x4fd3af2a, 0x10e8, 0x474f, 0x84d7, 0x722bcfd3efc3)

/**
 * @brief Width characteristic UUID value.
 */
#define LSD_WIDTH_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)

/**
 * @brief Height characteristic UUID value.
 */
#define LSD_HEIGHT_UUID_VAL                                                    \
  BT_UUID_128_ENCODE(0x730134f2, 0xe403, 0x4956, 0xa998, 0xa4649f0fa1bf)

/**
 * @brief X characteristic UUID value.
 */
#define LSD_X_UUID_VAL                                                         \
  BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)

/**
 * @brief Y characteristic UUID value.
 */
#define LSD_Y_UUID_VAL                                                         \
  BT_UUID_128_ENCODE(0xf1fb7f33, 0xa652, 0x4de0, 0x83f9, 0xbf3288f83f6f)

/**
 * @brief Brightness characteristic UUID value.
 */
#define LSD_BRIGHTNESS_UUID_VAL                                                \
  BT_UUID_128_ENCODE(0xa1e0f55c, 0x2d1b, 0x4fca, 0xae9d, 0xefb3248c202a)

/**
 * @brief Color characteristic UUID value.
 */
#define LSD_COLOR_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

namespace lsd {
constexpr bt_uuid_128 SERVICE_UUID = BT_UUID_INIT_128(LSD_SERVICE_UUID_VAL);

constexpr bt_uuid_128 WIDTH_UUID = BT_UUID_INIT_128(LSD_WIDTH_UUID_VAL);

constexpr bt_uuid_128 HEIGHT_UUID = BT_UUID_INIT_128(LSD_HEIGHT_UUID_VAL);

constexpr bt_uuid_128 X_UUID = BT_UUID_INIT_128(LSD_X_UUID_VAL);

constexpr bt_uuid_128 Y_UUID = BT_UUID_INIT_128(LSD_Y_UUID_VAL);

constexpr bt_uuid_128 BRIGHTNESS_UUID =
    BT_UUID_INIT_128(LSD_BRIGHTNESS_UUID_VAL);

constexpr bt_uuid_128 COLOR_UUID = BT_UUID_INIT_128(LSD_COLOR_UUID_VAL);

constexpr std::uint8_t INITIAL_BRIGHTNESS{255};

constexpr std::uint16_t INITIAL_INDEX{0};

template <Vector2 SIZE> struct ServiceData {
  LEDMatrix<SIZE> *matrix;

  std::uint16_t width{SIZE.x};

  std::uint16_t height{SIZE.y};

  std::uint16_t x;

  std::uint16_t y;

  std::uint8_t brightness{INITIAL_BRIGHTNESS};

  std::array<std::uint8_t, 3> color{};
};

template <Vector2 SIZE>
ssize_t read_width_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                      const std::uint16_t len, const std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<SIZE> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->width,
                           sizeof(data->width));
}

template <Vector2 SIZE>
ssize_t read_height_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                       const std::uint16_t len, const std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<SIZE> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->height,
                           sizeof(data->height));
}

template <Vector2 SIZE>
ssize_t read_x_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                  const std::uint16_t len, const std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<SIZE> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->x,
                           sizeof(data->x));
}

template <Vector2 SIZE>
ssize_t write_x_cb(bt_conn *conn, const bt_gatt_attr *attr, const void *buf,
                   const std::uint16_t len, const std::uint16_t offset,
                   const std::uint8_t flags) {
  auto *data = reinterpret_cast<ServiceData<SIZE> *>(attr->user_data);
  if (len != sizeof(data->x)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  data->x = *reinterpret_cast<const std::uint16_t *>(buf);
  return sizeof(data->x);
}

template <Vector2 SIZE>
ssize_t read_y_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                  const std::uint16_t len, const std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<SIZE> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->y,
                           sizeof(data->y));
}

template <Vector2 SIZE>
ssize_t write_y_cb(bt_conn *conn, const bt_gatt_attr *attr, const void *buf,
                   const std::uint16_t len, const std::uint16_t offset,
                   const std::uint8_t flags) {
  auto *data = reinterpret_cast<ServiceData<SIZE> *>(attr->user_data);
  if (len != sizeof(data->y)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  data->y = *reinterpret_cast<const std::uint16_t *>(buf);
  return sizeof(data->y);
}

template <Vector2 SIZE>
ssize_t read_brightness_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                           const std::uint16_t len,
                           const std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<SIZE> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->brightness,
                           sizeof(data->brightness));
}

template <Vector2 SIZE>
ssize_t write_brightness_cb(bt_conn *conn, const bt_gatt_attr *attr,
                            const void *buf, const std::uint16_t len,
                            const std::uint16_t offset,
                            const std::uint8_t flags) {
  if (len != 1) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  auto *data = reinterpret_cast<ServiceData<SIZE> *>(attr->user_data);
  data->brightness = *reinterpret_cast<const uint8_t *>(buf);

  return len;
}

template <Vector2 SIZE>
ssize_t read_color_cb(bt_conn *conn, const bt_gatt_attr *attr, void *buf,
                      std::uint16_t len, std::uint16_t offset) {
  auto *data = reinterpret_cast<ServiceData<SIZE> *>(attr->user_data);
  const auto result = data->matrix->get_pixel(data->x, data->y);
  if (!result) {
    return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
  }

  const auto *color = result.value();
  data->color[0] = color->r;
  data->color[1] = color->g;
  data->color[2] = color->b;

  return bt_gatt_attr_read(conn, attr, buf, len, offset, std::data(data->color),
                           std::size(data->color));
}

template <Vector2 SIZE>
ssize_t write_color_cb(bt_conn *conn, const bt_gatt_attr *attr, const void *buf,
                       std::uint16_t len, std::uint16_t offset,
                       std::uint8_t flags) {
  auto *data = reinterpret_cast<ServiceData<SIZE> *>(attr->user_data);
  if (len != std::size(data->color)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  const auto *color = reinterpret_cast<const std::uint8_t *>(buf);
  std::copy_n(color, len, std::data(data->color));

  const struct led_rgb pixel {
    .r = data->color[0], .g = data->color[1], .b = data->color[2]
  };

  const auto result =
      data->matrix->set_pixel(data->x, data->y, std::move(pixel));
  if (!result) {
    return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
  }

  return len;
}
} // namespace lsd
