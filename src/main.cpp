#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "led_matrix.hpp"
#include "segmented_display.hpp"
#include "service.hpp"

using namespace lsd;

#define LED_STRIP_NODE DT_ALIAS(led_strip)

constexpr std::uint16_t MATRIX_WIDTH{16};

constexpr std::uint16_t MATRIX_HEIGHT{16};

constexpr std::uint16_t MATRIX_SIZE{MATRIX_WIDTH * MATRIX_HEIGHT};

LOG_MODULE_REGISTER(lsd, LOG_LEVEL_DBG);

static const device *led_strip_device = DEVICE_DT_GET(LED_STRIP_NODE);

static LEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT> matrix{led_strip_device};

static ServiceData<MATRIX_WIDTH * MATRIX_HEIGHT> service_data;

bool bonding{false};

LSS_SVC_DEFINE(SERVICE, MATRIX_SIZE, &service_data);

static SegmentedDisplay<MATRIX_WIDTH, MATRIX_HEIGHT> segmented_display{&matrix};

void display_passkey(bt_conn *conn, unsigned int passkey) {
  bonding = true;

  segmented_display.clear();

  char digits[] = "000000";
  for (std::size_t i = 0; i < std::size(digits) - 1; i++) {
    digits[std::size(digits) - 2 - i] = '0' + passkey % 10;
    passkey /= 10;
  }

  segmented_display.write(digits);
}

void cancel_auth(bt_conn *conn) {
  bonding = false;
  segmented_display.clear();
}

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

int main() {
  int err{bt_enable(NULL)};
  if (err) {
    LOG_ERR("Bluetooth initialization failed (err %d)", err);
    return 0;
  }

  err = settings_load();
  if (err) {
    LOG_ERR("Settings loading failed (err %d)", err);
    return 0;
  }

  const auto adv_result{start_advertising()};
  if (!adv_result) {
    LOG_ERR("Advertising failed (err %d)", adv_result.error());
    return 0;
  }

  while (true) {
    if (!bonding) {
      update<MATRIX_WIDTH, MATRIX_HEIGHT>(matrix, service_data);
    }
    const auto commit_result{matrix.commit()};
    if (!commit_result) {
      LOG_ERR("Matrix commit failed (err %d)", commit_result.error());
      return 0;
    }
    k_sleep(K_MSEC(100));
  }

  return 0;
}
