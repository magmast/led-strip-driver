#pragma once

#include <variant>

#include "common.hpp"
#include "led_matrix.hpp"

namespace lsd {
template <Vector2 SIZE> class LEDSegmentedDisplay {
public:
  LEDSegmentedDisplay(LEDMatrix<SIZE> *matrix) : _matrix{matrix} {}

  void set_color(const led_rgb &&color) { _color = std::move(color); }

  void clear() {
    _cursor = {0, 0};
    _matrix->clear();
  }

  std::expected<void, WriteError> write(const char ch) {
    if (ch < '0' || ch > '9') {
      return std::unexpected(InvalidCharacterError());
    }

    const auto index = ch - '0';
    const auto character = _CHARACTERS[index];

    for (std::size_t row = 0; row < 5; row++) {
      for (std::size_t col = 0; col < 3; col++) {
        const auto bit = 1 << (row * 3 + col);
        const auto color = (character & bit) ? _color : led_rgb{0, 0, 0};

        const auto result = _matrix->set_pixel(row + _cursor.y * SEGMENT_HEIGHT,
                                               col + _cursor.x * SEGMENT_WIDTH,
                                               std::move(color));
        if (!result) {
          return result;
        }
      }
    }

    _cursor.x++;
    if (_cursor.x * SEGMENT_WIDTH + _CHAR_WIDTH >= SIZE.x) {
      _cursor.x = 0;
      _cursor.y++;
    }

    if (_cursor.y * SEGMENT_HEIGHT + _CHAR_HEIGHT >= SIZE.y) {
      _cursor.y = 0;
    }

    return {};
  }

  std::expected<void, WriteError> write(const std::string_view text) {
    for (const auto ch : text) {
      const auto result = write(ch);
      if (!result) {
        return result;
      }
    }

    return {};
  }

private:
  static constexpr std::uint8_t SEGMENT_WIDTH = 4;

  static constexpr std::uint8_t SEGMENT_HEIGHT = 7;

  static constexpr std::uint8_t _CHAR_WIDTH = 3;

  static constexpr std::uint8_t _CHAR_HEIGHT = 5;

  static constexpr std::array<uint16_t, 10> _CHARACTERS{
      0b111101101101111, // 0
      0b100100101110100, // 1
      0b111001111100111, // 2
      0b111100111100111, // 3
      0b100100111101101, // 4
      0b111100111001111, // 5
      0b111101111001111, // 6
      0b100100100100111, // 7
      0b111101111101111, // 8
      0b111100111101111, // 6
  };

  LEDMatrix<SIZE> *_matrix;

  Vector2 _cursor;

  led_rgb _color{64, 64, 64};
};
}; // namespace lsd
