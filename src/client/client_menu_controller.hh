#pragma once

#include <array>
#include <span>
#include <string_view>
#include <experimental/memory>

#include "client_menu_view.hh"
#include "../common/serializer.hh"

using std::vector, std::string, std::span, std::experimental::observer_ptr;

class SessionInfo;  // Forward-declared class from client.hh

class MenuControl {
  std::weak_ptr<MenuView> view;
  std::weak_ptr<LobbyView> lobby;
  observer_ptr<SessionInfo> session;

  enum class MainOptions : uint8_t {
    BROWSER = 1,
    FRIENDS,
    LOGOUT,
    REPLAY,
    LAST_SENTINEL
  };
  enum class FriendsOptions : uint8_t {
    SEND = 1,
    ACCEPT,
    REJECT,
    REMOVE,
    CHAT,
    ACCEPTINVITE,
    RETURN,
    LAST_SENTINEL
  };

 public:
  MenuControl(std::weak_ptr<MenuView> view, std::weak_ptr<LobbyView> lobby, SessionInfo* session)
    : view{std::forward<std::weak_ptr<MenuView>>(view)},
      lobby{std::forward<std::weak_ptr<LobbyView>>(lobby)},
      session{session} {}

  void authentify    (const NM::Message& message);
  void startLobby    (const NM::Message& message);
  void joinLobby     (const NM::Message& message);
  void appendChat    (const NM::Message& message);
  void updateRelation(const NM::Message& message);
  void loadChat      (const NM::Message& message);
  void loadMatches   (const NM::Message& message);
  void quitLobby     ();
  void updateLobby   (const NM::Message& message);
  void updateLobbyMember(const NM::Message& message);

  [[nodiscard]] NM::Message formatAuth   (std::string_view line) const;
  [[nodiscard]] NM::Message formatMain   (std::string_view line) const;
  [[nodiscard]] NM::Message formatBrowser(std::string_view line) const;
  [[nodiscard]] NM::Message formatFriends(std::string_view line) const;
  [[nodiscard]] NM::Message formatChat   (std::string_view line) const;
  [[nodiscard]] NM::Message formatLobby  (std::string_view line) const;

};
