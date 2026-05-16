#include <iostream>
#include <ranges>

#include "board_coordinates.hh"

namespace views = std::ranges::views;

BoardCoordinates::BoardCoordinates(std::string_view coordinates) {
  auto [x, y] = BoardCoordinates::parse(coordinates);
  set(x, y);
}

std::pair<size_t, size_t> BoardCoordinates::parse(std::string_view string_coordinates) {
  auto alphabetic = [](char c) { return isalpha(c); };
  auto to_size_t  = [](char c) { return static_cast<size_t>(std::toupper(c) - 'A' + 1); };
  size_t result = 0;
  for (size_t x : string_coordinates | views::take_while(alphabetic) | views::transform(to_size_t)) {
    result = result * 26 + x;
  }
  
  auto y_coordinates = string_coordinates | views::drop_while(alphabetic);
  unsigned long long parsed = 0;
  std::from_chars(y_coordinates.data(), y_coordinates.data() + y_coordinates.size(), parsed);

  if (result == 0 || parsed == 0) {
    std::cerr << "Failed conversion string coordinates to BoardCoordinates\n";
  }

  return {result - 1, parsed - 1};
}

vector<BoardCoordinates> BoardCoordinates::stringToVector(string_view str) {
  vector<size_t> delimiters{0};

  size_t it = 0;
  while (it != str.size()) {
    if ((isdigit(str[it]) && it == 0))
      break;
    if ((isdigit(str[it])) && (it == str.size() - 1 || isalpha(str[it + 1]))) {
      delimiters.push_back(it + 1);
    }
    ++it;
  }

  vector<BoardCoordinates> coordinates;
  for (size_t i = 0; i < delimiters.size() - 1; ++i) {
    coordinates.push_back(BoardCoordinates{string{str.data() + delimiters[i], str.data() + delimiters[i + 1]}});
  }

  return coordinates;
}

std::ostream& operator<<(std::ostream& os, BoardCoordinates bc) {
  os << bc.toString();
  return os;
}
