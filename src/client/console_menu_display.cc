#include <iomanip>
#include <limits>
#include <iostream>
#include <string>
#include <span>

#include "console_menu_display.hh"
#include "client.hh"
#include "../common/network_io.hh"
#include "../common/utils.hh"
#include "../common/serializer.hh"

namespace ranges = std::ranges;
using std::string, std::span, std::string_view; 

void ConsoleMenuDisplay::displayLogin() const {
  output <<
    "============ Login Screen ============\n\n"

    "Please enter:\n"
    "> login    <username> <password>\n"
    "> register <username> <password>\n";
}

void ConsoleMenuDisplay::displayMain() const {
  output <<
    "============ Main menu ============\n\n"

    "Select number:\n"
    "> 1. Match browser\n"
    "> 2. Open friends list\n"
    "> 3. Log out\n"
    "> 4. View last replay\n";
  // Please update the relevant enum class when modifying this
}

void ConsoleMenuDisplay::displayBrowser() const {
  using enum NM::setc::Color;
  constexpr static uint8_t MAXSIZE = Lobby::LOBBY_NAME_MAXSIZE;

  output << NM::setc{BLACK, GREEN}
         << "  ID  | "
         << std::left << std::setw(MAXSIZE - 1) << "Name"
         << " | Players | Started | Password \n"
         << NM::setc{};
  output << menu->getMatches()
         << "\n\n> Exit browser: '/q'\n"
         <<     "> Refresh all:  '/r\n"
         <<     "> Create game:  '/c <" << std::to_string(MAXSIZE - 1) << " character-long name> <optional password>'\n"
         <<     "> Join:         '/j <name> <optional password>'\n";
}

void ConsoleMenuDisplay::displayFriends() const {
  output 
    << "============ Friend Menu ============\n\n"

       "-------------- Friends --------------\n"
    << NM::print_range{session->getFriends()}
    << "\n----- Received Friend Requests ------\n"
    << NM::print_range{session->getInbound()}
    << "\n------- Sent Friend Requests --------\n"
    << NM::print_range{session->getOutbound()}
    << "\n----- Received Game Invite From -----\n"
    << NM::print_range{session->getGameRequests()}
    << "\n-------------------------------------\n\n"

       "Enter number and username:\n"
       "> 1. Send   <friend> request\n"
       "> 2. Accept <friend> request\n"
       "> 3. Reject <friend> request\n"
       "> 4. Remove <friend>\n"
       "> 5. Chat   <friend>\n"
       "> 6. Accept game invite from <friend>\n"
       "> 7. Return to Main Menu \n";
  // Please update the relevant enum class when modifying this
}

void ConsoleMenuDisplay::displayChat() const {
  output
    << "============ Chat Menu ============\n"
    << NM::print_range{menu->currentChat()} << "\n"
    << "> Exit chat: '/q'\n";
}

void ConsoleMenuDisplay::displayLobby() const {
  output << *lobby;

  if (lobby->kind() == LobbyView::Kind::HOST)
    output << ">> Host Parameters:\n"
              "> Time is always interpreted in seconds. The turn time should only\n"
              "  be specified if you are switching to the turn-based time limit.\n"
              "> Time per turn cannot be less than or equal to game time.\n"
              "  Game time cannot be more than 1 hour.\n"
              "> Gamemode:        '/g <(1)Classic or (2)Commanders>'\n"
              "> Time limit:      '/t <(1)Per Match or (2)Per Turn> <game time> <turn time>'\n"
              "> Confirm & Start: '/c'\n\n";

  output << "> Leave:           '/q'\n"
         << "> Invite player:   '/i <username>'\n"
         << "> Switch slots:    '/s <left, right, spec>'\n";
}

ConsoleMenuDisplay::ConsoleMenuDisplay(std::ostream& out, std::istream& in,
                                       std::shared_ptr<MenuView const>  menu,
                                       std::shared_ptr<MenuControl>     control,
                                       std::shared_ptr<LobbyView const> lobby,
                                       observer_ptr<SessionInfo const> session)
  : MenuDisplay{std::move(menu), std::move(control), std::move(lobby), session},
    ConsoleDisplay{out, in} {}

void ConsoleMenuDisplay::display() {
  std::system("clear");  // Don't touch this

  switch(menu->currentState()) {
    using enum MenuView::MenuState;
    case LOGIN:
      displayLogin();
      break;
    case MAIN:
      displayMain();
      break;
    case BROWSER:
      displayBrowser();
      break;
    case FRIENDS:
      displayFriends();
      break;
    case CHAT:
      displayChat();
      break;
    case LOBBY:
      displayLobby();
      break;
    default:
      throw std::runtime_error("MenuDisplay reached unreachable case in display()");
  }
  output << ">> " << std::flush;
}

NM::Message ConsoleMenuDisplay::handleInput() {
  string line;
  std::getline(std::cin, line);

  if (line.empty()) return {};

  switch(menu->currentState()) {
    using enum MenuView::MenuState;
    case LOGIN:
      return control->formatAuth(line);
    case MAIN:
      return control->formatMain(line);
    case BROWSER:
      return control->formatBrowser(line);
    case FRIENDS:
      return control->formatFriends(line);
    case CHAT:
      return control->formatChat(line);
    case LOBBY:
      return control->formatLobby(line);
    default:
      throw NotImplementedError("State not implemented");
  }
}
