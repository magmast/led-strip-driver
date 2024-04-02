#ifndef LED_STRIP_SVC_H
#define LED_STRIP_SVC_H

#include <zephyr/device.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/led_strip.h>

/**
 * @brief LED service UUID value.
 */
#define LSS_SVC_UUID_VAL BT_UUID_128_ENCODE(0x4fd3af2a, 0x10e8, 0x474f, 0x84d7, 0x722bcfd3efc3)

/**
 * @brief Length characteristic UUID value.
 */
#define LSS_LENGTH_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)

/**
 * @brief Brightness characteristic UUID value.
 */
#define LSS_BRIGHTNESS_CHRC_UUID_VAL BT_UUID_128_ENCODE(0xa1e0f55c, 0x2d1b, 0x4fca, 0xae9d, 0xefb3248c202a)

/**
 * @brief Index characteristic UUID value.
 */
#define LSS_INDEX_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)

/**
 * @brief Color characteristic UUID value.
 */
#define LSS_COLOR_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

/**
 * @brief Defines a BLE LED service.
 * @param _name Name of the service.
 * @param _data Pointer to the service data.
 */
#define LSS_SVC_DEFINE(_name, _data)                                  \
	BT_GATT_SERVICE_DEFINE(                                       \
	    _name,                                                    \
	    BT_GATT_PRIMARY_SERVICE(&lss_svc_uuid),                   \
	    BT_GATT_CHARACTERISTIC(                                   \
		(const struct bt_uuid *)&lss_length_chrc_uuid,        \
		BT_GATT_CHRC_READ,                                    \
		BT_GATT_PERM_READ_AUTHEN,                             \
		&lss_read_length_chrc,                                \
		NULL,                                                 \
		_data),                                               \
	    BT_GATT_CHARACTERISTIC(                                   \
		(const struct bt_uuid *)&lss_brightness_chrc_uuid,    \
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,               \
		BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN, \
		&lss_read_brightness_chrc,                            \
		&lss_write_brightness_chrc,                           \
		_data),                                               \
	    BT_GATT_CHARACTERISTIC(                                   \
		(const struct bt_uuid *)&lss_index_chrc_uuid,         \
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,               \
		BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN, \
		&lss_read_index_chrc,                                 \
		&lss_write_index_chrc,                                \
		_data),                                               \
	    BT_GATT_CHARACTERISTIC(                                   \
		(const struct bt_uuid *)&lss_color_chrc_uuid,         \
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,               \
		BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN, \
		&lss_read_color_chrc,                                 \
		&lss_write_color_chrc,                                \
		_data))

/**
 * @brief LED service uuid.
 */
extern const struct bt_uuid_128 lss_svc_uuid;

/**
 * @brief Length characteristic uuid.
 */
extern const struct bt_uuid_128 lss_length_chrc_uuid;

/**
 * @brief Brightness characteristic uuid.
 */
extern const struct bt_uuid_128 lss_brightness_chrc_uuid;

/**
 * @brief Index characteristic uuid.
 */
extern const struct bt_uuid_128 lss_index_chrc_uuid;

/**
 * @brief Color characteristic uuid.
 */
extern const struct bt_uuid_128 lss_color_chrc_uuid;
/**
 * @brief LED service data.
 * @note Remember to allocate the color array equal to the length of the LED strip.
 */
struct lss_svc_data
{
	/**
	 * @brief Length of the LED strip.
	 */
	uint16_t length;

	/**
	 * @brief Brightness of the LED strip.
	 */
	uint8_t brightness;

	/**
	 * @brief Index for color characteristic.
	 * @details It's stored in a way seen by the user of the service. It has to be mapped to the actual index in the LED strip on use.
	 */
	int16_t index;

	/**
	 * @brief Color of the LED strip.
	 */
	uint8_t *color;
};

typedef struct lss_svc_data lss_svc_data_t;

/**
 * @brief Creates a new LED service data and allocates memory for the color data.
 */
lss_svc_data_t lss_svc_data_create(uint16_t length);

/**
 * @brief Frees the memory allocated for the color data.
 */
void lss_svc_data_free(lss_svc_data_t *data);

/**
 * @brief Callback for reading the length characteristic.
 */
ssize_t lss_read_length_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for reading the brightness characteristic.
 */
ssize_t lss_read_brightness_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for writing the brightness characteristic.
 */
ssize_t lss_write_brightness_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

/**
 * @brief Callback for reading the index characteristic.
 */
ssize_t lss_read_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for writing the index characteristic.
 */
ssize_t lss_write_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

/**
 * @brief Callback for reading the color characteristic.
 */
ssize_t lss_read_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for writing the color characteristic.
 */
ssize_t lss_write_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

/**
 * @brief Updates the LED strip from the service data.
 */
int lss_update(lss_svc_data_t *svc, const struct device *dev, struct led_rgb *pixels);

#endif
