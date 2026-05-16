#pragma once

#include <memory>
#include <ostream>
#include <istream>
#include <string>
#include <utility>
#include <vector>
#include <span>
#include <iostream>
#include <optional>
#include <experimental/memory>

#include "client_menu_controller.hh"
#include "client_menu_view.hh"
#include "console_display.hh"
#include "display_common.hh"
#include "../common/serializer.hh"

using std::experimental::observer_ptr, std::vector, std::string;

class SessionInfo;  // Forward-declared class from client.hh

/** MenuDisplay using text.
 *
 * Different options to choose are represented on the screen.
 * Draw the input side for the client */
class ConsoleMenuDisplay : public MenuDisplay, public ConsoleDisplay {
 public:
  ConsoleMenuDisplay(std::ostream& out, std::istream& in,
                     std::shared_ptr<MenuView const> menu,
                     std::shared_ptr<MenuControl>    control,
                     std::shared_ptr<LobbyView const> lobby,
                     observer_ptr<SessionInfo const> session);

  void display();

  [[nodiscard]] NM::Message handleInput();

 private:
  void displayLogin  () const;
  void displayMain   () const;
  void displayBrowser() const;
  void displayFriends() const;
  void displayChat   () const;
  void displayLobby  () const;
};
