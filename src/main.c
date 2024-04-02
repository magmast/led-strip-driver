#include <zephyr/logging/log.h>

#include "led_strip_display.h"
#include "led_strip_svc.h"

#define LED_STRIP_NODE DT_ALIAS(led_strip)
#define PIXELS_COUNT DT_PROP(LED_STRIP_NODE, chain_length)

LOG_MODULE_REGISTER(led_strip_driver, LOG_LEVEL_DBG);

void display_passkey(struct bt_conn *conn, unsigned int passkey);

void hide_passkey(struct bt_conn *conn);

void handle_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);

static const struct device *strip_dev = DEVICE_DT_GET(LED_STRIP_NODE);

static const struct bt_data adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LSS_SVC_UUID_VAL),
};

static struct led_rgb pixels[PIXELS_COUNT];

static lss_svc_data_t strip_svc_data;

static lsd_led_strip_t strip_display = {
    .rows = 16,
    .cols = 16,
    .gap_x = 1,
    .gap_y = 1,
    .pixels = pixels,
};

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .security_changed = handle_security_changed,
};

static const struct bt_conn_auth_cb bt_auth_cb = {
    .passkey_display = &display_passkey,
    .cancel = &hide_passkey,
};

static bool bonding = false;

LSS_SVC_DEFINE(strip_svc, &strip_svc_data);

int main(void)
{
	memset(pixels, 0, sizeof(pixels));
	strip_svc_data = lss_svc_data_create(PIXELS_COUNT);

	int err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	err = bt_conn_auth_cb_register(&bt_auth_cb);
	if (err)
	{
		LOG_ERR("Failed to register auth cb (err %d)\n", err);
		return 0;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, adv_data, ARRAY_SIZE(adv_data), NULL, 0);
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return 0;
	}

	while (true)
	{
		if (!bonding)
		{
			lss_update(&strip_svc_data, strip_dev, pixels);
		}
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

void display_passkey(struct bt_conn *conn, unsigned int passkey)
{
	LOG_INF("Display passkey\n");
	bonding = true;

	uint8_t digits[] = {0, 0, 0, 0, 0, 0};
	for (int i = 0; i < 6; i++)
	{
		digits[5 - i] = passkey % 10;
		passkey /= 10;
	}

	for (int i = 0; i < 6; i++)
	{
		lsd_display_digit(&strip_display, i, digits[i]);
	}

	int err = lsd_commit(&strip_display, strip_dev);
	if (err)
	{
		LOG_ERR("Failed to display passkey (err %d)\n", err);
		return;
	}
}

void hide_passkey(struct bt_conn *conn)
{
	LOG_INF("Hiding passkey\n");
	bonding = false;

	memset(pixels, 0, sizeof(pixels));
	int err = led_strip_update_rgb(strip_dev, pixels, PIXELS_COUNT);
	if (err)
	{
		LOG_ERR("Failed to hide passkey (err %d)\n", err);
		return;
	}
}

void handle_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	if (level >= BT_SECURITY_L2)
	{
		hide_passkey(conn);
	}
}
