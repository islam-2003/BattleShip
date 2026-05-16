#include "boat.hh"

#include <algorithm>
#include <ranges>
#include <functional>

namespace ranges = std::ranges;
namespace views = std::ranges::views;

vector<Boat::Type> Boat::createInventory(GameModel::Faction faction) {
  switch (faction) {
    using enum Boat::Type;
    using enum GameModel::Faction;
    case CLASSIC:
      // return { Destroyer, Destroyer };
      return { Destroyer, Destroyer, Cruiser, Battleship, Carrier };
    case CAPTAIN:
      return { Z_Tetromino, J_Tetromino, Cruiser, T_Tetromino, Destroyer };
    case PIRATE:
      return { Destroyer, Cruiser, Cruiser, Battleship, Carrier };
    default:
      throw NotImplementedError("Faction doesn't exist");
  }
}

vector<BoardCoordinates> Boat::assembleByType(Type type, BoardCoordinates origin) {
    vector<BoardCoordinates> coordinates{ origin };
    switch (type) {
        using enum Type;
        case Carrier:
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 4, origin.y() });
            [[fallthrough]];
        case Battleship:
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 3, origin.y() });
            [[fallthrough]];
        case Cruiser:
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 2, origin.y() });
            [[fallthrough]];
        case Destroyer:
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 1, origin.y() });
            break;
        case Z_Tetromino:
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 1, origin.y() });
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 1, origin.y() + 1 });
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 2, origin.y() + 1 });
            break;
        case J_Tetromino:
            coordinates.emplace_back(BoardCoordinates{ origin.x(),     origin.y() + 1 });
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 1, origin.y() + 1 });
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 2, origin.y() + 1 });
            break;
        case T_Tetromino:
            coordinates.emplace_back(BoardCoordinates{ origin.x(),     origin.y() + 1 });
            coordinates.emplace_back(BoardCoordinates{ origin.x(),     origin.y() + 2 });
            coordinates.emplace_back(BoardCoordinates{ origin.x() + 1, origin.y() + 1 });
            break;
        default:
            throw NotImplementedError("Boat type not implemented");
    }

    return coordinates;
}

bool Boat::isCorrect(Type type, std::span<BoardCoordinates const> coordinates) {
  vector<BoardCoordinates> coordinates_template = assembleByType(type, coordinates[0]);

  if (coordinates.size() != coordinates_template.size())
    return false;

  auto&& fulcrum = coordinates[0];

  std::array<std::function<BoardCoordinates(const BoardCoordinates&)>, 3> rotations{
    // 90°, 180°, -90°
    [&fulcrum](const BoardCoordinates& ship_cell) { 
      return BoardCoordinates{fulcrum.x() - fulcrum.y() + ship_cell.y(), fulcrum.y() - ship_cell.x() + fulcrum.x()};
    },
    [&fulcrum](const BoardCoordinates& ship_cell) { 
      return BoardCoordinates{ship_cell.x() - 2 * (ship_cell.x() - fulcrum.x()), ship_cell.y() - 2 * (ship_cell.y() - fulcrum.y())};
    },
    [&fulcrum](const BoardCoordinates& ship_cell) { 
      return BoardCoordinates{fulcrum.x() - ship_cell.y() + fulcrum.y(), fulcrum.y() - fulcrum.x() + ship_cell.x()};
    }
  };

  auto matches = [&coordinates, &coordinates_template](std::function<BoardCoordinates(const BoardCoordinates&)> transform) {
    return ranges::equal(coordinates, coordinates_template | views::transform(transform));
  };

  return ranges::equal(coordinates, coordinates_template) || ranges::any_of(rotations, matches);
}
