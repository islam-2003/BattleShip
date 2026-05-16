#pragma once

#include <string>
#include <algorithm>

/**
 * Repeats any string-like by the given amount.
 */
[[nodiscard]] inline std::string operator*(std::string_view str, size_t amount) {
  std::string result;
  result.reserve(str.size() * amount);
  for (size_t i = 0; i < amount; ++i) {
    result += str;
  }
  return result;
}

class ConsoleDisplay {
 protected:
  [[nodiscard]] static constexpr size_t length(std::string_view s) {
    // In UTF-8, continuation bytes begin with 0b10, so, does not count these bytes.
    return static_cast<size_t>(
        std::ranges::count_if(s, [](char c) noexcept { return (c & '\xC0') != '\x80'; }));
  }

  std::ostream& output;
  std::istream& input;

  ConsoleDisplay(std::ostream& out, std::istream& in) : output{out}, input{in} {}
};
