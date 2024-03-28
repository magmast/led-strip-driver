#include <math.h>

#include "strip.h"

static const struct device *const strip = DEVICE_DT_GET(LED_STRIP_NODE);

static struct led_rgb pixels[PIXELS_COUNT];

size_t map_index(size_t index)
{
    size_t row = (size_t)floor(index / 16);

    if (row % 2 == 0)
    {
        return row * 16 + index % 16;
    }

    return row * 16 + 15 - index % 16;
}

int strip_set(size_t index, struct led_rgb color)
{
    if (index >= PIXELS_COUNT)
    {
        return -EINVAL;
    }

    index = map_index(index);
    pixels[index] = color;

    return 0;
}

int strip_set_all(struct led_rgb color)
{
    for (size_t i = 0; i < PIXELS_COUNT; i++)
    {
        strip_set(i, color);
    }

    return 0;
}

struct led_rgb strip_get(size_t index)
{
    if (index >= PIXELS_COUNT)
    {
        return (struct led_rgb){0, 0, 0};
    }

    index = map_index(index);
    return pixels[index];
}

int strip_update(void)
{
    return led_strip_update_rgb(strip, pixels, PIXELS_COUNT);
}