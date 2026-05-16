#include "utils.hpp"
#include "account.hpp"

class Chat {
  /* Cette classe gère la communication entre deux utilisateurs */
  vector<Account*> dm;
 public:
  void sendMessage(string s, Account* account);
  string receiveMessage(Account* account);
  bool checkExistence();
};