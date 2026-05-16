#pragma once

#include <vector>
#include <string>
#include <span>
#include <ranges>
#include <algorithm>

#include "../common/board_coordinates.hh"
#include "not_implemented_error.hh"

namespace ranges = std::ranges;
namespace views = ranges::views;
using std::string, std::vector, std::span;

class Ability{
  

  public:
    enum class Type : uint8_t {
    Basic,
    Diagonal,
    XBomb,
    Linear,
    PlusBomb,
    A_SENTINEL
  };

  private:
    Type type;
    int cost = 1;
  
  public:
    Ability(Ability::Type type = Ability::Type::Basic) : type{type}{}

    [[nodiscard]] inline Type getType() const { return type; }
    void setType(Type abilityType);

    [[nodiscard]] inline int getCost() const { return cost; }
    void setCost(int value) {cost = value;}

  static vector<BoardCoordinates> assembleByType(Type type, BoardCoordinates origin) {
    vector<BoardCoordinates> coordinates{origin};
    switch (type) {
      using enum Type;
      case Basic:
        break;
      case Linear:
        coordinates.emplace_back(BoardCoordinates{origin.x() + 1, origin.y()});
        coordinates.emplace_back(BoardCoordinates{origin.x() - 1, origin.y()});
        break;
      case Diagonal:
        coordinates.emplace_back(BoardCoordinates{origin.x() + 1, origin.y() + 1});
        coordinates.emplace_back(BoardCoordinates{origin.x() - 1, origin.y() - 1});
        break;
      case XBomb:
        coordinates.emplace_back(BoardCoordinates{origin.x() + 1, origin.y()-1});
        coordinates.emplace_back(BoardCoordinates{origin.x() + 1, origin.y()+1});
        coordinates.emplace_back(BoardCoordinates{origin.x() - 1, origin.y()-1});
        coordinates.emplace_back(BoardCoordinates{origin.x() - 1, origin.y()+1});
        break;
      case PlusBomb:
        coordinates.emplace_back(BoardCoordinates{origin.x() + 1, origin.y()});
        coordinates.emplace_back(BoardCoordinates{origin.x() - 1, origin.y()});
        coordinates.emplace_back(BoardCoordinates{origin.x(), origin.y() + 1});
        coordinates.emplace_back(BoardCoordinates{origin.x(), origin.y() - 1});
        break;
      default:
        throw NotImplementedError("Ability type not implemented");
    }

    return coordinates;
  }
};