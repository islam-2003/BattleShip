#include "client_boat.hh"

std::string ClientBoat::to_string() const {
  switch (type) {
    using enum Boat::Type;
    case Destroyer:
      return "Destroyer";
    case Cruiser:
      return "Cruiser";
    case Battleship:
      return "Battleship";
    case Carrier:
      return "Carrier";
    case Z_Tetromino:
      return "Z Tetromino";
    case J_Tetromino:
      return "J Tetromino";
    case T_Tetromino:
      return "T Tetromino";
    default:
      throw NotImplementedError("Ship type not implemented");
  }
}

void ClientBoat::shift(BoardCoordinates::Transform transform) {
  for (auto&& coordinate : coordinates)
    coordinate += transform;
}
