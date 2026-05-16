#include "utils.hpp"
#include "account.hpp"


class FriendList {
  /* Cette classe permet de gérer une liste d'amis */
  vector<Account*> friends;
  vector<Account*> receivedRequests;
  vector<Account*> sentRequests;
 public:
  void sendFriendRequest(Account* account);
  void acceptFriendRequest();
  void rejectFriendRequest();
  void removeFriend(Account* account);
  int search(Account* account, vector<string> v);
};