#include "utils.hpp"
#include "account.hpp"

class Database {
  /* Base de données pour stocker les comptes et les conversations */
  vector<Account*> accountList;
  vector<vector<FILE>> chats;
 public:
  void searchAccount(Account* account);
  void addAccount(Account*);
  void addChat(Account* A, Account* B);
  void updateChatLog(Account* A, Account* B);
};