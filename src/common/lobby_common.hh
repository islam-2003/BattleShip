#pragma once

#include "board_common.hh"
#include "timer.hh"

namespace chrono = std::chrono;

class Lobby {
 public:
  using parameter_t = std::tuple<chrono::seconds, chrono::seconds, Timer::Type, GameModel::GameMode>;

  constexpr static uint8_t LOBBY_NAME_MAXSIZE = 31;
  constexpr static uint8_t MAX_PLAYERS = 10;

  inline auto parameters() const { return std::tuple{game_time, turn_time, tt, gt}; }

  inline bool updateParameters(parameter_t data) {
    if (std::get<0>(data) > chrono::seconds(3600))
      return false;
    if (std::get<2>(data) == Timer::Type::TURN && std::get<1>(data) >= std::get<0>(data))
      return false;
    std::tie(game_time, turn_time, tt, gt) = data;
    return true;
  }

 protected:
  std::array<char, LOBBY_NAME_MAXSIZE> name;
  std::string password;

  chrono::seconds game_time{Timer::DEFAULTLIMIT};
  chrono::seconds turn_time{Timer::DEFAULTTURN};
  Timer::Type tt{Timer::Type::GLOBAL};
  GameModel::GameMode gt{GameModel::GameMode::CLASSIC};
};
