#pragma once

#include <poll.h>
#include <memory>
#include <array>

#include "client_board.hh"
#include "console_board_display.hh"

#include "client_menu_view.hh"
#include "client_menu_controller.hh"
#include "console_menu_display.hh"

#include "../common/network_io.hh"

#ifdef GUI
#include "gui/gui_thread.hh"
#endif

using std::string, std::string_view, std::array;

class SessionInfo {
  enum Relationship : uint8_t {
    FRIENDS,
    REQUESTS_INBOUND,
    REQUESTS_OUTBOUND,
    GAME_REQUESTS
  };

  array<vector<string>, 4> relationships;
  string username;

  void remove(vector<string>& relation, string_view username);

 public:
  SessionInfo() = default;

  void newSession(const NM::Message::Account& account);
  void updateRelation(const NM::Message::Relationship& relation);
  void clear() noexcept;

  [[nodiscard]] inline string             getUsername()     const { return username; }
  [[nodiscard]] inline span<string const> getFriends()      const { return relationships.at(FRIENDS); }
  [[nodiscard]] inline span<string const> getInbound()      const { return relationships.at(REQUESTS_INBOUND); }
  [[nodiscard]] inline span<string const> getOutbound()     const { return relationships.at(REQUESTS_OUTBOUND); }
  [[nodiscard]] inline span<string const> getGameRequests() const { return relationships.at(GAME_REQUESTS); }

  SessionInfo(SessionInfo&&)      = delete;
  SessionInfo(const SessionInfo&) = delete;
};

class Client : protected Networkable {
  enum Descriptor : uint8_t {
    TIMER,
    USER,
    SERVER
  };

  enum class State : bool {
    MENU,
    PLAYING
  };

  int server_fd;
  array<pollfd, 3> poll_fds;
  
  State state;
  ClientTimer timer;

  SessionInfo session;
  
#ifndef GUI
  std::unique_ptr<ConsoleMenuDisplay> menu;
  std::unique_ptr<ConsoleGameDisplay> game;
#else
  std::unique_ptr<GUIThread> gui_thread;

#endif

  void startGame(Lobby::parameter_t params, bool spec);
  void failed();

 public:
  Client(string_view ip);
  ~Client();

  void watch();
};
