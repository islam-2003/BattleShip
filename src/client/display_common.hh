#pragma once

#include <memory>
#include <experimental/memory>

#include "../common/serializer.hh"
#include "client_menu_controller.hh"
#include "client_menu_view.hh"
#include "client_timer.hh"
#include "client_board.hh"

using std::experimental::observer_ptr;

class SessionInfo;  // Forward-declared class from client.hh

class MenuDisplay {
 protected:
  std::shared_ptr<MenuView const> const menu;
  std::shared_ptr<MenuControl>    const control;
  std::shared_ptr<LobbyView const>      lobby;
  observer_ptr<SessionInfo const>       session;

 public:
  MenuDisplay(std::shared_ptr<MenuView const>  menu,
              std::shared_ptr<MenuControl>     control,
              std::shared_ptr<LobbyView const> lobby,
              observer_ptr<SessionInfo const>  session)
    : menu{std::move(menu)},
      control{std::move(control)},
      lobby{std::move(lobby)},
      session{session} {}
      
  Lobby::parameter_t getLobbyParameters() const { return lobby->parameters(); }
  void handleServer(const NM::Message& message);

  [[nodiscard]] auto playerNames() const { return std::pair{lobby->getLeft(), lobby->getRight()}; }
};

class GameDisplay {
 protected:
  std::shared_ptr<ClientView const> const _board;
  std::shared_ptr<ClientControl>    const _control;
  epr::observer_ptr<ClientTimer const>     timer;

#ifndef GUI
  bool first_refresh = true;
#endif

 public:
  GameDisplay(std::shared_ptr<ClientView const> board,
              std::shared_ptr<ClientControl>    control,
              ClientTimer* timer)
    : _board{std::move(board)}, _control{std::move(control)}, timer{timer} {}

  void handleServer(const NM::Message& message);
};
