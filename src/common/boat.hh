#pragma once

#include <cstdint>
#include <string>
#include <span>

#include "not_implemented_error.hh"
#include "board_coordinates.hh"
#include "board_common.hh"

class Boat {
 public:
  enum class Type : uint8_t {
    Destroyer,
    Cruiser,
    Battleship,
    Carrier,
    Z_Tetromino,
    J_Tetromino,
    T_Tetromino,
    SENTINEL
  };

  static vector<Type> createInventory(GameModel::Faction faction);
  static vector<BoardCoordinates> assembleByType(Type type, BoardCoordinates origin);
  static bool isCorrect(Type type, std::span<BoardCoordinates const> coordinates);

  Boat(Boat::Type type, int id) : type{type}, id{id} {}

  bool operator==(const Boat& other) { return id == other.id; }

  [[nodiscard]] inline int  getId()   const { return id; }
  [[nodiscard]] inline Type getType() const { return type; }

 protected:
  Type type;
  int id;
};
