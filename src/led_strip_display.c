#include "led_strip_display.h"

#define PIXELS_SIZE 14

static const uint8_t digit_width = 3;

static const uint8_t digit_height = 5;

static const int8_t digit_pixels[10][PIXELS_SIZE] = {
    {0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 13, 14, -1},
    {2, 4, 5, 6, 8, 11, 14, -1},
    {0, 1, 2, 5, 6, 7, 8, 9, 12, 13, 14, -1},
    {0, 1, 2, 5, 6, 7, 8, 11, 12, 13, 14, -1},
    {0, 3, 5, 6, 7, 8, 11, 14, -1},
    {0, 1, 2, 3, 6, 7, 8, 11, 12, 13, 14, -1},
    {0, 1, 2, 3, 6, 7, 8, 9, 11, 12, 13, 14, -1},
    {0, 1, 2, 5, 8, 11, 14, -1},
    {0, 1, 2, 3, 5, 6, 7, 8, 9, 11, 12, 13, 14, -1},
    {0, 1, 2, 3, 5, 6, 7, 8, 11, 14, -1}};

const int lsd_err_ok = 0;

const int lsd_err_not_a_digit = -1;

const int8_t *get_digit_pixels(uint8_t digit)
{
	return digit_pixels[digit];
}

int lsd_display_digit(lsd_led_strip_t *strip, uint8_t position, uint8_t digit)
{
	if (digit > 9)
	{
		return lsd_err_not_a_digit;
	}

	uint8_t start_pos = position * (digit_width + strip->gap_x);
	uint8_t start_row = start_pos / strip->cols * (digit_height + strip->gap_y);
	uint8_t start_col = start_pos % strip->cols;

	const int8_t *pixel = get_digit_pixels(digit);
	for (; *pixel > -1; pixel++)
	{
		uint8_t dpos = *pixel;
		uint8_t drow = dpos / digit_width;
		uint8_t dcol = dpos % digit_width;

		uint8_t row = drow + start_row;
		uint8_t col = dcol + start_col;
		if (row % 2 == 0)
		{
			col = strip->cols - col - 1;
		}

		strip->pixels[row * strip->cols + col].r = 255;
	}

	return lsd_err_ok;
}

int lsd_commit(lsd_led_strip_t *strip, const struct device *dev)
{
	return led_strip_update_rgb(dev, strip->pixels, strip->rows * strip->cols);
}
