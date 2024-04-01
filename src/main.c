#include "led_strip_svc.h"

#define PIXELS_COUNT 256

#define LED_STRIP_NODE DT_ALIAS(led_strip)

struct lss_svc_data strip_svc_data = {
    .length = PIXELS_COUNT,
    .device = DEVICE_DT_GET(LED_STRIP_NODE),
};

BLE_LED_SVC_DEFINE(strip_svc, &strip_svc_data);

static const struct bt_data adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LSS_SVC_UUID_VAL),
};

int main(void)
{
        size_t color_size = sizeof(struct led_rgb) * strip_svc_data.length;
        strip_svc_data.color = k_malloc(color_size);
        memset(strip_svc_data.color, 0, color_size);

        int err = bt_enable(NULL);
        if (err)
        {
                printk("Bluetooth init failed (err %d)\n", err);
                return err;
        }

        err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, adv_data, ARRAY_SIZE(adv_data), NULL, 0);
        if (err)
        {
                printk("Advertising failed to start (err %d)\n", err);
                return err;
        }

        return 0;
}
