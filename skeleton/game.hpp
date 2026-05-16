#include "utils.hpp"
#include "account.hpp"
#include "board_view_control.hpp"

class Game {
  /* Cette classe gère une partie en cours */
  int gameTime;
  int turnTime;
  Account* player_l;
  Account* player_r;
  BoardViewControl board_l;
  BoardViewControl board_l;
 public:
  void addPlayer(Account* other);
  void removePlayer(Account* other);
};