#include "utils.hpp"


class BoardCoordinates final {
  size_t _x;
  size_t _y;

 public:
  constexpr BoardCoordinates(size_t x, size_t y);

  [[nodiscard]] constexpr inline size_t x() const;
  [[nodiscard]] constexpr inline size_t y() const;

  void set(size_t x, size_t y);

  constexpr static bool isalpha(char c);

  [[nodiscard]] static std::optional<size_t> parseX(const string& x_string);
  [[nodiscard]] static std::optional<size_t> parseY(const string& y_string);

  [[nodiscard]] inline string toString() const;
  [[nodiscard]] inline string xToString() const;
  [[nodiscard]] inline string yToString() const;
};

std::ostream& operator<<(std::ostream& os, const BoardCoordinates& bc);
std::istream& operator>>(std::istream& is, BoardCoordinates& bc);
