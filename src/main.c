#include <stdbool.h>

#include <zephyr/kernel.h>

#include "ble.h"
#include "strip.h"

int main(void)
{
        ble_init();

        while (true)
        {
                int err = strip_update();
                if (err)
                {
                        printk("Failed to update LED strip: %d\n", err);
                }
                k_sleep(K_SECONDS(1));
        }
}
