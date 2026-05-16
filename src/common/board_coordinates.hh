#pragma once

#include <optional>
#include <string>
#include <charconv>
#include <vector>

using std::string, std::string_view, std::vector;

/** A pair of 0-indexed board coordinates.
 *
 * {0, 0} is top-left.
 *
 * NOTE: This is not the coordinates of a pixel on the screen. */
class BoardCoordinates final {
  size_t _x;
  size_t _y;

 public:
  struct Transform {
    ssize_t x, y;
  };

  constexpr BoardCoordinates() : _x{}, _y{} {}
  constexpr BoardCoordinates(size_t x, size_t y) : _x{x}, _y{y} {}
  explicit  BoardCoordinates(std::string_view coordinates);

  [[nodiscard]] constexpr inline size_t x() const { return _x; }
  [[nodiscard]] constexpr inline size_t y() const { return _y; }

  bool operator==(BoardCoordinates other) const { return x() == other.x() && y() == other.y(); }

  BoardCoordinates& operator+=(Transform other) { _x += other.x; _y += other.y; return *this; }

  void set(size_t x, size_t y) {
    _x = x;
    _y = y;
  }

  /** Read string into size_t pair. If input is malformed, either x or y can produce MAX_SIZE_T */
  [[nodiscard]] static std::pair<size_t, size_t> parse(std::string_view string_coordinates);

  /** Whether c is in [A-Za-z] */
  constexpr static bool isalpha(char c) {
    // Contrary to std::isalpha, works with char and not modified by locale
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
  }

  static vector<BoardCoordinates> stringToVector(string_view str);

  /** {0, 0} returns "A1" */
  [[nodiscard]] inline string toString() const { return xToString() + yToString(); }

  /** returns the x / letter part of toString() */
  [[nodiscard]] inline string xToString() const {
    string result{};
    size_t      n = _x + 1;
    while (n > 0) {
      result = static_cast<char>('A' + (n - 1) % 26) + result;
      n      = (n - 1) / 26;
    }
    return result;
  }

  /** returns the y / number part of toString() */
  [[nodiscard]] inline string yToString() const { return std::to_string(_y + 1); }
};

/** Put bc.toString() on os */
std::ostream& operator<<(std::ostream& os, BoardCoordinates bc);

