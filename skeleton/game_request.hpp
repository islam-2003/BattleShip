#include "utils.hpp"
#include "account.hpp"

class GameRequests {
  /* Cette classe permet de gérer les requêtes de partie */
  vector<Account*> requests;
 public:
  void accept();
  void decline();
  void sendRequest(Account* account);
};