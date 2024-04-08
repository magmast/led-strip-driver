#pragma once

#include <variant>

namespace lsd {
class Error {
public:
  virtual ~Error();

  virtual const char *message() const = 0;
};

constexpr const char *message(const Error &error) { return error.message(); }

class InvalidIndexError : public Error {
public:
  const char *message() const override { return "Invalid index."; }
};

class InvalidCharacterError : public Error {
public:
  const char *message() const override { return "Invalid character."; }
};

using WriteError = std::variant<InvalidIndexError, InvalidCharacterError>;

constexpr const char *message(const WriteError &error) {
  return std::visit([](const auto &error) { return message(error); }, error);
}

struct Vector2 {
  std::size_t x;

  std::size_t y;

  constexpr Vector2() = default;

  constexpr Vector2(const std::size_t x, const std::size_t y) : x{x}, y{y} {}
};
}; // namespace lsd
