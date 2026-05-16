#include "utils.hpp"
#include "account.hpp"

class Client{
  /* Cette classe gère la création de compte et la connexion d'un client */
  Account* account;
 public:
  void connect(string username, string password);
  void registerAccount(string username, string password);
  void disconnect();
};