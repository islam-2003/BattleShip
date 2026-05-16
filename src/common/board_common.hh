#pragma once

#include <cstddef>
#include <string>
#include <chrono>

using std::string_view;

constexpr size_t BOARDSIZE = 10;

class GameModel {
 public:
  enum class GameStage : uint8_t {
    FACTIONSELECT,
    SELECTION,
    PLACEMENT,
    WAITING,
    ATTACKSELECT,
    COMBAT,
    OTHERTURN,
    GAMEFINISHED,
    SPECTATING
  };

  enum class GameMode : uint8_t {
    CLASSIC,
    COMMANDERS
  };

  enum class Faction : uint8_t {
    CLASSIC,
    PIRATE,
    CAPTAIN
  };

  enum class Victor : uint8_t {
    NONE,
    LEFT,
    RIGHT,
    STALEMATE,
    REPLAY
  };

  enum CellType : uint8_t {
    // Flags:
    IS_SHIP  = 0b001,
    IS_KNOWN = 0b010,
    IS_SUNK  = 0b100,

    // Non-ship types:
    WATER = 0,
    OCEAN = IS_KNOWN,

    // Ship states:
    UNDAMAGED = IS_SHIP,  // For your side only
    HIT       = IS_SHIP | IS_KNOWN,
    SUNK      = IS_SHIP | IS_KNOWN | IS_SUNK,
  };

  [[nodiscard]] inline Victor victor() const { return _victor; }

  inline void setVictor(Victor victor) { _is_finished = true; _victor = victor; }

 protected:
  Victor    _victor{ Victor::NONE };
  bool      _is_finished{ false };
};

class GameControl {
 public:
  // THESE DO NOT HAVE A NULL TERMINATOR
  static constexpr string_view BAD        {"YOU ARE BAD", 11};
  static constexpr string_view GAMEOVER   {"GO", 2};
  static constexpr string_view HOSTWIN    {"HW", 2};
  static constexpr string_view OPPONENTWIN{"OPW", 3};

  static constexpr string_view FACTION    {"F", 1};
  static constexpr string_view GOODFACTION{"GF", 2};

  static constexpr string_view SELECT    {"SELECT", 6};
  static constexpr string_view GOODSELECT{"GOOD SELECTION", 14};

  static constexpr string_view PLACE     {"PLACE", 5};
  static constexpr string_view GOODPLACE {"AP", 2};

  static constexpr string_view SELECTABILITY{"ABILITY", 7};
  static constexpr string_view GOODABILITY  {"GOOD ABILITY", 13};

  static constexpr string_view FIRE    {"FF", 2};
  static constexpr string_view GOODFIRE{"LF", 2};
};
