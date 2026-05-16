#pragma once

#include <ranges>
#include <algorithm>
#include <span>

#include "../common/board_coordinates.hh"
#include "../common/board_common.hh"
#include "../common/boat.hh"

namespace ranges = std::ranges;
using std::span;

class ClientBoat final : public Boat {
  vector<BoardCoordinates> coordinates;

 public:
  ClientBoat(Type type, int id, vector<BoardCoordinates> coordinates) : Boat{type, id}, coordinates{coordinates} {}

  bool operator==(const ClientBoat& other) { return type == other.type && id == other.id && ranges::equal(coordinates, other.coordinates); }

  [[nodiscard]] inline BoardCoordinates       origin()         { return coordinates.front(); }
  [[nodiscard]] inline span<BoardCoordinates> getCoordinates() { return coordinates; }

  [[nodiscard]] inline bool contains(BoardCoordinates c) const {
    return ranges::find(coordinates, c) != coordinates.end();
  }
  [[nodiscard]] inline bool contains(const ClientBoat& other) const {
    return ranges::any_of(other.coordinates, [this](BoardCoordinates c) { return contains(c); });
  }
  [[nodiscard]] inline bool inBoundsH() const {
    return ranges::none_of(coordinates, [](BoardCoordinates c) { return c.x() >= BOARDSIZE; });
  }
  [[nodiscard]] inline bool inBoundsV() const {
    return ranges::none_of(coordinates, [](BoardCoordinates c) { return c.y() >= BOARDSIZE; });
  }

  [[nodiscard]] std::string to_string()  const;

  void shift(BoardCoordinates::Transform transform);
  inline void setCoordinates(span<BoardCoordinates const> new_coordinates) { coordinates.assign(new_coordinates.begin(), new_coordinates.end()); }

  inline void addCoordinate(BoardCoordinates new_coordinates) { coordinates.push_back(new_coordinates); }
};
