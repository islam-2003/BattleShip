#include "utils.hpp"
#include "game.hpp"
#include "database.hpp"



class Server {
  /* Serveur sur lequel la partie se déroule */
  Database database;
  vector<Game*> activeGames;
 public:
  void clientThread();
  void createMatchThread();
  void createGame(string gameMode, int turnTime, int gameTime);
  void removeGame(Game*);
  void connectUser();
};