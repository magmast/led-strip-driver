#include "led_strip_svc.h"

#include <math.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_DECLARE(led_strip_driver, LOG_LEVEL_DBG);

const struct bt_uuid_128 lss_svc_uuid = BT_UUID_INIT_128(LSS_SVC_UUID_VAL);

const struct bt_uuid_128 lss_length_chrc_uuid = BT_UUID_INIT_128(LSS_LENGTH_CHRC_UUID_VAL);

const struct bt_uuid_128 lss_brightness_chrc_uuid = BT_UUID_INIT_128(LSS_BRIGHTNESS_CHRC_UUID_VAL);

const struct bt_uuid_128 lss_index_chrc_uuid = BT_UUID_INIT_128(LSS_INDEX_CHRC_UUID_VAL);

const struct bt_uuid_128 lss_color_chrc_uuid = BT_UUID_INIT_128(LSS_COLOR_CHRC_UUID_VAL);

lss_svc_data_t lss_svc_data_create(uint16_t length)
{
    lss_svc_data_t data = {
        .length = length,
        .brightness = 255,
        .index = 0,
        .color = k_malloc(sizeof(uint8_t) * length * 3),
    };

    memset(data.color, 0, sizeof(uint8_t) * length * 3);

    return data;
}

void lss_svc_data_free(lss_svc_data_t *data)
{
    k_free(data->color);
}

ssize_t lss_read_length_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    LOG_DBG("Reading length\n");

    if (offset)
    {
        LOG_WRN("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &svc->length, sizeof(svc->length));
}

ssize_t lss_read_brightness_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    LOG_DBG("Reading brightness\n");

    if (offset)
    {
        LOG_WRN("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &svc->brightness, sizeof(svc->brightness));
}

ssize_t lss_write_brightness_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    LOG_DBG("Writing brightness\n");

    if (offset)
    {
        LOG_WRN("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (len != 1)
    {
        LOG_WRN("Invalid attribute length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    struct lss_svc_data *svc = attr->user_data;
    svc->brightness = *(uint8_t *)buf;

    return len;
}

ssize_t lss_read_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    LOG_DBG("Reading index\n");

    if (offset)
    {
        LOG_WRN("Invalid offset\n");
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
    LOG_DBG("Writing index\n");

    if (offset)
    {
        LOG_WRN("Invalid offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    struct lss_svc_data *svc = attr->user_data;
    int16_t *value = (int16_t *)buf;

    if (len != sizeof(svc->index))
    {
        LOG_WRN("Invalid attribute length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (*value < -1 || *value >= svc->length)
    {
        LOG_WRN("Invalid index\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_PDU);
    }

    svc->index = *value;

    return len;
}

ssize_t lss_read_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset)
{
    LOG_DBG("Reading color\n");

    struct lss_svc_data *svc = attr->user_data;
    size_t byte_idx = svc->index * 3 + offset;
    uint8_t *color = &svc->color[byte_idx];

    return bt_gatt_attr_read(conn, attr, buf, len, offset, color, MIN(510, svc->length * 3 - byte_idx));
}

ssize_t lss_write_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags)
{
    LOG_DBG("Writing color\n");

    struct lss_svc_data *svc = attr->user_data;
    bytecpy(svc->color + svc->index * 3 + offset, buf, len);

    return len;
}

int lss_update(const struct device *dev, lss_svc_data_t *svc)
{
    LOG_DBG("Updating LED strip\n");

    struct led_rgb *pixels = k_malloc(sizeof(struct led_rgb) * svc->length);
    for (size_t i = 0; i < svc->length; i++)
    {
        struct led_rgb *pixel = &pixels[i];
        uint8_t *byte = &svc->color[i * 3];
        pixel->r = *byte * ((float)svc->brightness / 255);
        pixel->g = *(byte + 1) * ((float)svc->brightness / 255);
        pixel->b = *(byte + 2) * ((float)svc->brightness / 255);
    }

    int err = led_strip_update_rgb(dev, pixels, svc->length);

    k_free(pixels);

    return err;
}