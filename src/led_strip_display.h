#ifndef LED_STRIP_DISPLAY_H
#define LED_STRIP_DISPLAY_h

#include <stdint.h>

#include <zephyr/drivers/led_strip.h>

extern const int lsd_err_ok;

extern const int lsd_err_not_a_digit;

/**
 * @brief Structure representing an LED strip used to display digits.
 */
struct lsd_led_strip
{
	/**
	 * @brief The number of rows in the LED strip.
	 */
	uint8_t rows;

	/**
	 * @brief The number of columns in the LED strip.
	 */
	uint8_t cols;

	/**
	 * @brief The gap between the digits in the x direction.
	 */
	uint8_t gap_x;

	/**
	 * @brief The gap between the digits in the y direction.
	 */
	uint8_t gap_y;

	/**
	 * @brief The pixels of the LED strip.
	 */
	struct led_rgb *pixels;
};

typedef struct lsd_led_strip lsd_led_strip_t;

/**
 * @brief Display a digit on the LED strip.
 * @param strip The LED strip to display the digit on.
 * @param position The position of the digit on the LED strip.
 * @param digit The digit to display.
 * @returns `lsd_err_ok` if the digit was displayed successfully, `lsd_err_not_a_digit` otherwise.
 * @note User is responsible for ensuring that digit does not overflow the LED strip rows.
 */
int lsd_display_digit(lsd_led_strip_t *strip, uint8_t position, uint8_t digit);

/**
 * @brief Commit the LED strip data to the hardware.
 */
int lsd_commit(lsd_led_strip_t *strip, const struct device *dev);

#endif
