#pragma once

#include <cstddef>

#include <array>
#include <expected>
#include <string_view>
#include <variant>

#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>

#include "error.hpp"

namespace lsd {
struct Point {
  std::size_t x;

  std::size_t y;

  constexpr Point() = default;

  constexpr Point(const std::size_t x, const std::size_t y) : x{x}, y{y} {}
};

template <std::size_t WIDTH, std::size_t HEIGHT> class LEDMatrix {
public:
  LEDMatrix(const struct device *_device) : _device(_device) {}

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(std::size_t row, std::size_t col) const {
    if (row >= WIDTH || col >= HEIGHT) {
      return std::unexpected(InvalidIndexError());
    }

    const auto index = get_index(row, col);
    return &_pixels[index];
  }

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(const Point &point) const {
    return get_pixel(point.x, point.y);
  }

  std::expected<const led_rgb *, InvalidIndexError>
  get_pixel(const std::size_t index) const {
    if (index >= WIDTH * HEIGHT) {
      return std::unexpected(InvalidIndexError());
    }
    const auto point = get_point(index);
    return get_pixel(point);
  }

  std::expected<void, InvalidIndexError>
  set_pixel(std::size_t row, std::size_t col, const led_rgb &&color) {
    if (row >= WIDTH || col >= HEIGHT) {
      return std::unexpected(InvalidIndexError());
    }

    const auto index = get_index(row, col);
    _pixels[index] = std::move(color);

    return {};
  }

  std::expected<void, InvalidIndexError> set_pixel(const Point &point,
                                                   const led_rgb &&color) {
    return set_pixel(point.x, point.y, std::move(color));
  }

  std::expected<void, InvalidIndexError> set_pixel(const std::size_t index,
                                                   const led_rgb &&color) {
    if (index >= WIDTH * HEIGHT) {
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
        led_strip_update_rgb(_device, std::data(_pixels), WIDTH * HEIGHT);
    if (result) {
      return std::unexpected(result);
    }

    return {};
  }

private:
  const device *_device;

  std::array<led_rgb, WIDTH * HEIGHT> _pixels{};

  std::size_t get_index(const std::size_t row, const std::size_t col) const {
    const auto mapped_col = row % 2 == 0 ? WIDTH - col - 1 : col;
    return row * WIDTH + mapped_col;
  }

  Point get_point(std::size_t index) {
    const auto row = index / WIDTH;
    const auto col = index % WIDTH;
    return {row, col};
  }
};

}; // namespace lsd
