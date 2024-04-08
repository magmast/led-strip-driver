#pragma once

#include <cstddef>

#include <array>
#include <expected>
#include <string_view>

#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>

#include "common.hpp"

LOG_MODULE_DECLARE(lsd, CONFIG_LOG_DEFAULT_LEVEL);

namespace lsd {
template <Vector2 SIZE> class LEDMatrix {
public:
  LEDMatrix(const struct device *_device) : _device(_device) {}

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(std::size_t row, std::size_t col) const {
    if (row >= SIZE.x || col >= SIZE.y) {
      return std::unexpected(InvalidIndexError());
    }

    const auto index = get_index(row, col);
    return &_pixels[index];
  }

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(const Vector2 &point) const {
    return get_pixel(point.x, point.y);
  }

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(const std::size_t index) const {
    if (index >= SIZE.x * SIZE.y) {
      return std::unexpected(InvalidIndexError());
    }
    const auto point = get_point(index);
    return get_pixel(point);
  }

  std::expected<void, InvalidIndexError>
  set_pixel(std::size_t row, std::size_t col, const led_rgb &&color) {
    if (row >= SIZE.x || col >= SIZE.y) {
      return std::unexpected(InvalidIndexError());
    }

    LOG_INF("Setting pixel at (%d, %d) to (%d, %d, %d)", row, col, color.r,
            color.g, color.b);

    const auto index = get_index(row, col);
    _pixels[index] = std::move(color);

    return {};
  }

  std::expected<void, InvalidIndexError> set_pixel(const Vector2 &point,
                                                   const led_rgb &&color) {
    return set_pixel(point.x, point.y, std::move(color));
  }

  std::expected<void, InvalidIndexError> set_pixel(const std::size_t index,
                                                   const led_rgb &&color) {
    if (index >= SIZE.x * SIZE.y) {
      return std::unexpected(InvalidIndexError());
    }
    const auto point = get_point(index);
    return set_pixel(point, std::move(color));
  }

  void fill(const led_rgb &color) {
    std::fill(std::begin(_pixels), std::end(_pixels), color);
  }

  void clear() { fill(led_rgb{0, 0, 0}); }

  std::expected<void, int> commit() {
    const auto result =
        led_strip_update_rgb(_device, std::data(_pixels), SIZE.x * SIZE.y);
    if (result) {
      return std::unexpected(result);
    }

    return {};
  }

private:
  const device *_device;

  std::array<led_rgb, SIZE.x * SIZE.y> _pixels{};

  std::size_t get_index(const std::size_t row, const std::size_t col) const {
    const auto mapped_col = row % 2 == 0 ? SIZE.x - col - 1 : col;
    return row * SIZE.x + mapped_col;
  }

  Vector2 get_point(std::size_t index) {
    const auto row = index / SIZE.x;
    const auto col = index % SIZE.x;
    return {row, col};
  }
};

}; // namespace lsd
