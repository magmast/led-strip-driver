#include "led_strip_svc.h"

#include <math.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

const struct bt_uuid_128 lss_svc_uuid = BT_UUID_INIT_128(LSS_SVC_UUID_VAL);

const struct bt_uuid_128 lss_length_chrc_uuid = BT_UUID_INIT_128(LSS_LENGTH_CHRC_UUID_VAL);

const struct bt_uuid_128 lss_index_chrc_uuid = BT_UUID_INIT_128(LSS_INDEX_CHRC_UUID_VAL);

const struct bt_uuid_128 lss_color_chrc_uuid = BT_UUID_INIT_128(LSS_COLOR_CHRC_UUID_VAL);

K_FIFO_DEFINE(update_led_queue);

void update_led(void)
{
    while (true)
    {
        struct lss_svc_data *svc = k_fifo_get(&update_led_queue, K_FOREVER);
        int err = led_strip_update_rgb(svc->device, svc->color, svc->length);
        if (err)
        {
            printk("Failed to update LED strip (err %d)\n", err);
        }
    }
}

K_THREAD_DEFINE(
    update_led_thread, BLE_LED_SVC_UPDATE_LED_THREAD_STACK_SIZE, update_led,
    NULL, NULL, NULL, 10, 0, 0);

ssize_t lss_read_length_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    if (offset)
    {
        printk("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &svc->length, sizeof(svc->length));
}

ssize_t lss_read_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    if (offset)
    {
        printk("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &svc->index, sizeof(svc->index));
}

ssize_t lss_write_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    if (offset)
    {
        printk("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    int16_t *value = (int16_t *)buf;

    if (len != sizeof(svc->index))
    {
        printk("Invalid attribute length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (*value < -1 || *value >= svc->length)
    {
        printk("Invalid index\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_PDU);
    }

    svc->index = *value;

    return len;
}

// TODO(magmast): With hardcoded 16 pixels per row, this module cannot be reused for other LED matrices.
size_t map_index(int16_t index)
{
    size_t row = (size_t)floor(index / 16);

    if (row % 2 == 0)
    {
        return row * 16 + index % 16;
    }

    return row * 16 + 15 - index % 16;
}

ssize_t lss_read_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    if (offset)
    {
        printk("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    size_t idx = map_index(svc->index);

    if (idx < 0)
    {
        printk("Cannot read color for index %d\n", idx);
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    struct led_rgb *color = &svc->color[idx];

    return bt_gatt_attr_read(conn, attr, buf, len, offset, color, sizeof(*color));
}

ssize_t lss_write_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    if (offset)
    {
        printk("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    uint8_t *value = (uint8_t *)buf + offset;

    if (len != 3)
    {
        printk("Invalid attribute length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    size_t idx = map_index(svc->index);

    if (idx > 0)
    {
        printk("Write color rgb(%d, %d, %d) for index %d\n", value[0], value[1], value[2], idx);
        struct led_rgb *color = &svc->color[idx];
        color->r = value[0];
        color->g = value[1];
        color->b = value[2];
    }
    else
    {
        memset(svc->color, 0, sizeof(*svc->color) * svc->length);
    }

    k_fifo_put(&update_led_queue, svc);

    return len;
}