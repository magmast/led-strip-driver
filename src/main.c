#include <zephyr/logging/log.h>

#include "led_strip_svc.h"

#define LED_STRIP_NODE DT_ALIAS(led_strip)
#define PIXELS_COUNT DT_PROP(LED_STRIP_NODE, chain_length)

LOG_MODULE_REGISTER(led_strip_driver, LOG_LEVEL_DBG);

const struct device *led_strip = DEVICE_DT_GET(LED_STRIP_NODE);

static lss_svc_data_t strip_svc_data;

LSS_SVC_DEFINE(strip_svc, &strip_svc_data);

static const struct bt_data adv_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, LSS_SVC_UUID_VAL),
};

int main(void)
{
        strip_svc_data = lss_svc_data_create(PIXELS_COUNT);

        int err = bt_enable(NULL);
        if (err)
        {
                LOG_ERR("Bluetooth init failed (err %d)\n", err);
                return err;
        }

        err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, adv_data, ARRAY_SIZE(adv_data), NULL, 0);
        if (err)
        {
                LOG_ERR("Advertising failed to start (err %d)\n", err);
                return err;
        }

        while (true)
        {
                lss_update(led_strip, &strip_svc_data);
                k_sleep(K_SECONDS(1));
        }

        return 0;
}
