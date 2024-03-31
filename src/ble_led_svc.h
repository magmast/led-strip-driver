#ifndef BLE_LED_SVC_H
#define BLE_LED_SVC_H

#include <zephyr/device.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/led_strip.h>

#ifndef BLE_LED_SVC_UPDATE_LED_THREAD_STACK_SIZE
/**
 * @brief Size of the stack for the update LED thread.
 */
#define BLE_LED_SVC_UPDATE_LED_THREAD_STACK_SIZE 512
#endif

#ifndef BLE_LED_SVC_UPDATE_LED_THREAD_PRIORITY
/**
 * @brief Priority of the update LED thread.
 */
#define BLE_LED_SVC_UPDATE_LED_THREAD_PRIORITY 10
#endif

/**
 * @brief LED service UUID value.
 */
#define BLE_LED_SVC_UUID_VAL BT_UUID_128_ENCODE(0x4fd3af2a, 0x10e8, 0x474f, 0x84d7, 0x722bcfd3efc3)

/**
 * @brief Length characteristic UUID value.
 */
#define BLE_LED_SVC_LENGTH_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x410f7f12, 0xe051, 0x4b5d, 0xa8ed, 0x7d5619727b34)

/**
 * @brief Index characteristic UUID value.
 */
#define BLE_LED_SVC_INDEX_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x85289f22, 0xbaa7, 0x447b, 0xacb2, 0xd961c06ecabf)

/**
 * @brief Color characteristic UUID value.
 */
#define BLE_LED_SVC_COLOR_CHRC_UUID_VAL BT_UUID_128_ENCODE(0x0c903aa6, 0xde65, 0x44c4, 0x9cde, 0x8873267e16c0)

/**
 * @brief Defines a BLE LED service.
 * @param _name Name of the service.
 * @param _data Pointer to the service data.
 */
#define BLE_LED_SVC_DEFINE(_name, _data)                           \
    BT_GATT_SERVICE_DEFINE(                                        \
        _name,                                                     \
        BT_GATT_PRIMARY_SERVICE(&ble_led_svc_uuid),                \
        BT_GATT_CHARACTERISTIC(                                    \
            (const struct bt_uuid *)&ble_led_svc_length_chrc_uuid, \
            BT_GATT_CHRC_READ,                                     \
            BT_GATT_PERM_READ,                                     \
            &ble_led_svc_read_length_chrc, NULL, _data),           \
        BT_GATT_CHARACTERISTIC(                                    \
            (const struct bt_uuid *)&ble_led_svc_index_chrc_uuid,  \
            BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,                \
            BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,                \
            &ble_led_svc_read_index_chrc,                          \
            &ble_led_svc_write_index_chrc, _data),                 \
        BT_GATT_CHARACTERISTIC(                                    \
            (const struct bt_uuid *)&ble_led_svc_color_chrc_uuid,  \
            BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,                \
            BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,                \
            &ble_led_svc_read_color_chrc,                          \
            &ble_led_svc_write_color_chrc, _data))

/**
 * @brief LED service data.
 * @note Remember to allocate the color array equal to the length of the LED strip.
 */
struct ble_led_svc
{
    /**
     * @brief Device controlled by the service.
     */
    const struct device *device;

    /**
     * @brief Length of the LED strip.
     */
    uint16_t length;

    /**
     * @brief Index for color characteristic.
     * @details It's stored in a way seen by the user of the service. It has to be mapped to the actual index in the LED strip on use.
     */
    int16_t index;

    /**
     * @brief Color of the LED strip.
     */
    struct led_rgb *color;
};

/**
 * @brief LED service uuid.
 */
extern const struct bt_uuid_128 ble_led_svc_uuid;

/**
 * @brief Length characteristic uuid.
 */
extern const struct bt_uuid_128 ble_led_svc_length_chrc_uuid;

/**
 * @brief Index characteristic uuid.
 */
extern const struct bt_uuid_128 ble_led_svc_index_chrc_uuid;

/**
 * @brief Color characteristic uuid.
 */
extern const struct bt_uuid_128 ble_led_svc_color_chrc_uuid;

/**
 * @brief Callback for reading the length characteristic.
 */
ssize_t ble_led_svc_read_length_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for reading the index characteristic.
 */
ssize_t ble_led_svc_read_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for writing the index characteristic.
 */
ssize_t ble_led_svc_write_index_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

/**
 * @brief Callback for reading the color characteristic.
 */
ssize_t ble_led_svc_read_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    void *buf,
    uint16_t len,
    uint16_t offset);

/**
 * @brief Callback for writing the color characteristic.
 */
ssize_t ble_led_svc_write_color_chrc(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
    const void *buf,
    uint16_t len,
    uint16_t offset,
    uint8_t flags);

#endif