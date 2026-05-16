#include "utils.hpp"
#include "game.hpp"
#include "chat.hpp"
#include "friend_list.hpp"
#include "game_request.hpp"


class Account {
  /* Cette classe gère tout ce que l'utilisateur peut faire en dehors d'une partie */
  string username;
  string password;
  FriendList friendLists;
  Game* game; 
  GameRequests gameRequest;
  Chat chats;
 public:
  void changePassword();
  bool inGame();
  void joinGame(Game* game);
  string getUsername();
  void modifyFriendList(string action, Account* account);
  void chat(string s, Account* account);
  void createGame(string gameMode, int turnTime, int gameTime);
  void joinAsViewer(Game *game);
};
