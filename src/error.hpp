#pragma once

#include <variant>

namespace lsd {
class Error {
public:
  virtual ~Error();

  virtual const char *message() const = 0;
};

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
  return std::visit([](const auto &error) { return error.message(); }, error);
}

}; // namespace lsd
