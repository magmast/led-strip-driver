#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include "led_matrix.hpp"
#include "led_segmented_display.hpp"
#include "service.hpp"

using namespace lsd;

#define LED_STRIP_NODE DT_ALIAS(led_strip)

constexpr Vector2 MATRIX_SIZE{16, 16};

LOG_MODULE_REGISTER(lsd, LOG_LEVEL_DBG);

static const device *led_strip_device = DEVICE_DT_GET(LED_STRIP_NODE);

static LEDMatrix<MATRIX_SIZE> matrix{led_strip_device};

static ServiceData<MATRIX_SIZE> service_data;

BT_GATT_SERVICE_DEFINE(
    bt_service,
    BT_GATT_PRIMARY_SERVICE(const_cast<bt_uuid_128 *>(&SERVICE_UUID)),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&WIDTH_UUID),
                           BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN,
                           &read_width_cb<MATRIX_SIZE>, NULL, &service_data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&HEIGHT_UUID),
                           BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN,
                           &read_height_cb<MATRIX_SIZE>, NULL, &service_data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&X_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_x_cb<MATRIX_SIZE>, &write_x_cb<MATRIX_SIZE>,
                           &service_data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&Y_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_y_cb<MATRIX_SIZE>, &write_y_cb<MATRIX_SIZE>,
                           &service_data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&BRIGHTNESS_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_brightness_cb<MATRIX_SIZE>,
                           &write_brightness_cb<MATRIX_SIZE>, &service_data),
    BT_GATT_CHARACTERISTIC(reinterpret_cast<const bt_uuid *>(&COLOR_UUID),
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN,
                           &read_color_cb<MATRIX_SIZE>,
                           &write_color_cb<MATRIX_SIZE>, &service_data));

static LEDSegmentedDisplay<MATRIX_SIZE> segmented_display{&matrix};

void display_passkey(bt_conn *conn, unsigned int passkey) {
  segmented_display.clear();

  char digits[] = "000000";
  for (std::size_t i = 0; i < std::size(digits) - 1; i++) {
    digits[std::size(digits) - 2 - i] = '0' + passkey % 10;
    passkey /= 10;
  }

  segmented_display.write(digits);
}

void cancel_auth(bt_conn *conn) { segmented_display.clear(); }

void handle_security_changed(bt_conn *conn, bt_security_t level,
                             bt_security_err err) {
  cancel_auth(conn);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .security_changed = handle_security_changed,
};

std::expected<void, int> start_advertising() {
  static const bt_data data[] = {
      BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
      BT_DATA_BYTES(BT_DATA_UUID128_ALL, LSD_SERVICE_UUID_VAL),
  };

  static const bt_le_adv_param params = {
      .id = BT_ID_DEFAULT,
      .sid = 0,
      .secondary_max_skip = 0,
      .options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
      .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
      .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
  };

  static const bt_conn_auth_cb auth_callbacks = {
      .passkey_display = &display_passkey,
      .cancel = &cancel_auth,
  };

  int err{bt_conn_auth_cb_register(&auth_callbacks)};
  if (err) {
    return std::unexpected(err);
  }

  err = bt_le_adv_start(&params, std::data(data), std::size(data), NULL, 0);
  if (err) {
    return std::unexpected(err);
  }

  return {};
}

void handle_bt_ready(int err) {
  if (err) {
    LOG_ERR("Bluetooth initialization failed (err %d)", err);
    return;
  }

  err = settings_load();
  if (err) {
    LOG_ERR("Settings loading failed (err %d)", err);
    return;
  }

  const auto adv_result{start_advertising()};
  if (!adv_result) {
    LOG_ERR("Advertising failed (err %d)", adv_result.error());
    return;
  }
}

int main() {
  settings_subsys_init();

  int err{bt_enable(handle_bt_ready)};
  if (err) {
    LOG_ERR("Bluetooth initialization failed (err %d)", err);
    return 0;
  }

  while (true) {
    matrix.set_pixel(15, 15, {255, 0, 0});
    const auto commit_result{matrix.commit()};
    if (!commit_result) {
      LOG_ERR("Matrix commit failed (err %d)", commit_result.error());
      return 0;
    }

    k_sleep(K_MSEC(100));
  }

  return 0;
}
