#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>

#include "ble.h"
#include "strip.h"

static ssize_t read_chain_length_chrc(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);
static size_t read_led_index_chrc(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);
static size_t write_led_index_chrc(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);
static size_t read_color_chrc(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);
static ssize_t write_color_chrc(struct bt_conn *, const struct bt_gatt_attr *, const void *, uint16_t, uint16_t, uint8_t);

#define LED_SVC_UUID_VAL BT_UUID_128_ENCODE(0x4fd3af2a, 0x10e8, 0x474f, 0x84d7, 0x722bcfd3efc3)
#define CHAIN_LENGTH_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)
#define LED_INDEX_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)
#define COLOR_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

static const struct bt_uuid_128 led_svc_uuid = BT_UUID_INIT_128(LED_SVC_UUID_VAL);

static const struct bt_uuid_128 chain_length_chrc_uuid = BT_UUID_INIT_128(CHAIN_LENGTH_CHRC_UUID_VAL);

static const struct bt_uuid_128 led_index_chrc_uuid = BT_UUID_INIT_128(LED_INDEX_CHRC_UUID_VAL);

static const struct bt_uuid_128 color_chrc_uuid = BT_UUID_INIT_128(COLOR_CHRC_UUID_VAL);

static const struct bt_data adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LED_SVC_UUID_VAL)};

static uint16_t chain_length = PIXELS_COUNT;
static int16_t led_index;
static uint8_t color[3];

BT_GATT_SERVICE_DEFINE(led_service,
                       BT_GATT_PRIMARY_SERVICE(&led_svc_uuid),
                       BT_GATT_CHARACTERISTIC(&chain_length_chrc_uuid,
                                              BT_GATT_CHRC_READ,
                                              BT_GATT_PERM_READ,
                                              &read_chain_length_chrc,
                                              NULL,
                                              &chain_length),
                       BT_GATT_CHARACTERISTIC(&led_index_chrc_uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              &read_led_index_chrc,
                                              &write_led_index_chrc,
                                              &led_index),
                       BT_GATT_CHARACTERISTIC(&color_chrc_uuid,
                                              BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_WRITE,
                                              NULL,
                                              &write_color_chrc,
                                              &color));

int ble_init(void)
{
    int err = bt_enable(NULL);
    if (err)
    {
        return err;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, adv_data, ARRAY_SIZE(adv_data), NULL, 0);
    if (err)
    {
        return err;
    }

    return 0;
}

static ssize_t read_chain_length_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(chain_length));
}

static size_t read_led_index_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(led_index));
}

static size_t write_led_index_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *raw_buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    uint8_t *buf = ((uint8_t *)raw_buf) + offset;

    if (len != 2)
    {
        printk("Invalid data length.\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    int16_t new_index = (int16_t)buf[0] | ((int16_t)buf[1] << 8);

    if (led_index < -1 || led_index >= chain_length)
    {
        printk("Invalid LED index.\n");
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    led_index = new_index;

    return len;
}

static size_t read_color_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    struct led_rgb c = strip_get(led_index);
    color[0] = c.r;
    color[1] = c.g;
    color[2] = c.b;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(color));
}

static ssize_t write_color_chrc(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *raw_buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    uint8_t *buf = ((uint8_t *)raw_buf) + offset;

    if (len != 3)
    {
        printk("Invalid data length.\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    struct led_rgb color = {
        .r = buf[0],
        .g = buf[1],
        .b = buf[2]};

    int err;
    if (led_index < 0)
    {
        err = strip_set_all(color);
    }
    else
    {
        err = strip_set(led_index, color);
    }

    if (err)
    {
        printk("Failed to write to LED strip (err %d)\n", err);
        buf[0] = (uint8_t)err;
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    return len;
}