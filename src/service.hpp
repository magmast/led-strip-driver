#pragma once

#include <cstdint>

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
 * @brief Length characteristic UUID value.
 */
#define LSD_LENGTH_UUID_VAL                                                    \
  BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)

/**
 * @brief Brightness characteristic UUID value.
 */
#define LSD_BRIGHTNESS_UUID_VAL                                                \
  BT_UUID_128_ENCODE(0xa1e0f55c, 0x2d1b, 0x4fca, 0xae9d, 0xefb3248c202a)

/**
 * @brief Index characteristic UUID value.
 */
#define LSD_INDEX_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)

/**
 * @brief Color characteristic UUID value.
 */
#define LSD_COLOR_UUID_VAL                                                     \
  BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

/**
 * @brief Defines a BLE LED service.
 * @param _name Name of the service.
 */
#define LSS_SVC_DEFINE(_name, _length, _data)                                  \
  BT_GATT_SERVICE_DEFINE(                                                      \
      _name,                                                                   \
      BT_GATT_PRIMARY_SERVICE(const_cast<bt_uuid_128 *>(&lsd::SERVICE_UUID)),  \
      BT_GATT_CHARACTERISTIC((const struct bt_uuid *)&lsd::LENGTH_UUID,        \
                             BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN,      \
                             &lsd::read_length_cb<_length>, NULL, _data),      \
      BT_GATT_CHARACTERISTIC((const struct bt_uuid *)&lsd::BRIGHTNESS_UUID,    \
                             BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,           \
                             BT_GATT_PERM_READ_AUTHEN |                        \
                                 BT_GATT_PERM_WRITE_AUTHEN,                    \
                             &lsd::read_brightness_cb<_length>,                \
                             &lsd::write_brightness_cb<_length>, _data),       \
      BT_GATT_CHARACTERISTIC(                                                  \
          (const struct bt_uuid *)&lsd::INDEX_UUID,                            \
          BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,                              \
          BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,                \
          &lsd::read_index_cb<_length>, &lsd::write_index_cb<_length>, _data), \
      BT_GATT_CHARACTERISTIC(                                                  \
          (const struct bt_uuid *)&lsd::COLOR_UUID,                            \
          BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,                              \
          BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,                \
          &lsd::read_color_cb<_length>, &lsd::write_color_cb<_length>, _data))

namespace lsd {
/**
 * @brief LED service uuid.
 */
constexpr struct bt_uuid_128 SERVICE_UUID =
    BT_UUID_INIT_128(LSD_SERVICE_UUID_VAL);

constexpr struct bt_uuid_128 LENGTH_UUID =
    BT_UUID_INIT_128(LSD_LENGTH_UUID_VAL);

constexpr struct bt_uuid_128 BRIGHTNESS_UUID =
    BT_UUID_INIT_128(LSD_BRIGHTNESS_UUID_VAL);

constexpr struct bt_uuid_128 INDEX_UUID = BT_UUID_INIT_128(LSD_INDEX_UUID_VAL);

constexpr struct bt_uuid_128 COLOR_UUID = BT_UUID_INIT_128(LSD_COLOR_UUID_VAL);

constexpr std::uint8_t INITIAL_BRIGHTNESS{255};

constexpr std::uint16_t INITIAL_INDEX{0};

using Length = std::uint16_t;

template <Length LENGTH> struct ServiceData {
  Length length{LENGTH};

  std::uint8_t brightness{INITIAL_BRIGHTNESS};

  Length index{INITIAL_INDEX};

  std::array<std::uint8_t, LENGTH * 3> color{};
};

template <Length LENGTH>
ssize_t read_length_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       void *buf, std::uint16_t len, std::uint16_t offset) {
  if (offset) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  const auto *data =
      reinterpret_cast<const ServiceData<LENGTH> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->length,
                           sizeof(data->length));
}

template <Length LENGTH>
ssize_t read_brightness_cb(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr, void *buf,
                           std::uint16_t len, std::uint16_t offset) {
  if (offset) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  const auto *data =
      reinterpret_cast<const ServiceData<LENGTH> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->brightness,
                           sizeof(data->brightness));
}

template <Length LENGTH>
ssize_t write_brightness_cb(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr, const void *buf,
                            std::uint16_t len, std::uint16_t offset,
                            std::uint8_t flags) {
  if (offset) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  if (len != 1) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  auto *data = reinterpret_cast<ServiceData<LENGTH> *>(attr->user_data);
  data->brightness = *reinterpret_cast<const uint8_t *>(buf);

  return len;
}

template <Length LENGTH>
ssize_t read_index_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                      void *buf, std::uint16_t len, std::uint16_t offset) {
  if (offset) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  const auto *data =
      reinterpret_cast<const ServiceData<LENGTH> *>(attr->user_data);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &data->index,
                           sizeof(data->index));
}

template <Length LENGTH>
ssize_t write_index_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       const void *buf, std::uint16_t len, std::uint16_t offset,
                       std::uint8_t flags) {
  if (offset) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }

  auto *data = reinterpret_cast<ServiceData<LENGTH> *>(attr->user_data);
  const auto *value{reinterpret_cast<const std::int16_t *>(buf)};

  if (len != sizeof(data->index)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
  }

  if (*value < -1 || *value >= LENGTH) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_PDU);
  }

  data->index = *value;

  return len;
}

template <Length LENGTH>
ssize_t read_color_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                      void *buf, std::uint16_t len, std::uint16_t offset) {
  const auto *data =
      reinterpret_cast<const ServiceData<LENGTH> *>(attr->user_data);

  const auto byte_idx{data->index * 3 + offset};
  const auto *color{&data->color[byte_idx]};

  return bt_gatt_attr_read(conn, attr, buf, len, offset, color,
                           MIN(510, LENGTH * 3 - byte_idx));
}

template <Length LENGTH>
ssize_t write_color_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       const void *buf, std::uint16_t len, std::uint16_t offset,
                       std::uint8_t flags) {
  auto *data = reinterpret_cast<ServiceData<LENGTH> *>(attr->user_data);
  bytecpy(data->color.begin() + data->index * 3 + offset, buf, len);

  return len;
}

template <Length WIDTH, Length HEIGHT>
std::expected<void, InvalidIndexError>
update(LEDMatrix<WIDTH, HEIGHT> &matrix, ServiceData<WIDTH * HEIGHT> &data) {
  for (std::size_t i = 0; i < WIDTH * HEIGHT; i++) {
    const auto *byte{&data.color[i * 3]};
    const auto brightness{static_cast<double>(data.brightness) / 255.0};
    struct led_rgb pixel {
      static_cast<std::uint8_t>((*byte) * brightness),
          static_cast<std::uint8_t>((*(byte + 1)) * brightness),
          static_cast<std::uint8_t>((*(byte + 2)) * brightness)
    };
    const auto result = matrix.set_pixel(i, std::move(pixel));
    if (!result) {
      return std::unexpected(result.error());
    }
  }
  return {};
};
} // namespace lsd
