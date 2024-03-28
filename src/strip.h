#ifndef STRIP_H
#define STRIP_H

#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>

#define LED_STRIP_NODE DT_ALIAS(led_strip)
#define PIXELS_COUNT DT_PROP(LED_STRIP_NODE, chain_length)

int strip_set(size_t index, struct led_rgb color);
int strip_set_all(struct led_rgb color);
struct led_rgb strip_get(size_t index);
int strip_update(void);

#endif
