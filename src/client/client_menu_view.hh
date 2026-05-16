#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <array>
#include <span>
#include <memory>
#include <string>
#include <cstdint>
#include <iomanip>
#include <chrono>

#include "../common/utils.hh"
#include "../common/serializer.hh"
#include "../common/lobby_common.hh"

namespace chrono = std::chrono;
using std::array, std::span, std::string, std::string_view, std::vector;

class LobbyView : public Lobby {
 public:
  enum class Kind {
    HOST,
    OTHER
  };

  LobbyView() {}
  inline void newLobby(NM::Message::HostLobby&& details) { std::tie(name, password) = details.data(); k = Kind::HOST; }
  inline void joinLobby(NM::Message::JoinLobby&& details) {
    k = Kind::OTHER;
    auto&& [lobby, params, clients] = details.data();
    std::tie(name, password) = lobby.data();
    std::tie(game_time, turn_time, tt, gt) = params.data();
    for (auto&& client : clients) {
      auto&& [name, slot] = client.data();
      switch (slot) {
        using enum NM::Message::SlotLobby::Slot;
        case LEFT:
          left = name;
          break;
        case RIGHT:
          right = name;
          break;
        case SPECTATOR:
          spectators.push_back(name);
          break;
        default:
          break;
      }
    }
  }

  [[nodiscard]] inline Kind kind() const { return k; }

  inline void clear() { *this = LobbyView(); }

  void updateMember(const NM::Message::SlotLobby member) {
    auto&& [name, slot] = member.data();
    std::erase(spectators, name);
    if (name == left)
      left = "";
    if (name == right)
      right = "";

    switch (slot) {
      using enum NM::Message::SlotLobby::Slot;
      case SPECTATOR:
        spectators.push_back(name);
        break;
      case LEFT:
        left = name;
        break;
      case RIGHT:
        right = name;
        break;
      case QUITTING:
      default:
        break;
    }
  }

  friend std::ostream& operator<<(std::ostream& output, const LobbyView& lobby) {
    using Color = NM::setc::Color;

    output << ">> Welcome to [ " << NM::setc{Color::YELLOW} << lobby.name.data() << NM::setc{} << " ] << \n"
              ">> Password   [ " << NM::setc{Color::YELLOW} << lobby.password    << NM::setc{} << " ] <<\n\n"

           << ">> Gamemode: " << (lobby.gt == GameModel::GameMode::CLASSIC ? NM::color_string{"CLASSIC",    Color::GREEN}
                                                                           : NM::color_string{"COMMANDERS", Color::CYAN})
                              << "\n"
           << ">> Time per: " << (lobby.tt == Timer::Type::GLOBAL ? NM::color_string{"MATCH", Color::YELLOW}
                                                                  : NM::color_string{"TURN",  Color::RED})
                              << " (" << (lobby.tt == Timer::Type::GLOBAL ? lobby.game_time.count()
                                                                          : lobby.turn_time.count()) << "s)\n";
     if (lobby.tt == Timer::Type::TURN)
      output << ">> Global time: (" << lobby.game_time.count() << "s)\n\n";
     else
      output << '\n';

     output << "> Left: "  << lobby.left  << "\n"
           << "> Right: " << lobby.right << "\n"
           << "> Spectators: " << NM::print_range{lobby.spectators, " "} << "\n\n";

    return output;
  }
 std::string getLeft() const { return left; }
 std::string getRight() const { return right; }
 vector<std::string> getSpectators() const { return spectators; }
 private:
  string left;
  string right;
  vector<string> spectators;
  Kind k;
};

/** Client-side view of the menu
 *
 * Contains a vector of different option's menu for displaying.
 * Contains copies of server-side state.
 * **/
class MenuView {
 public:
  enum class MenuState : uint8_t {
    LOGIN,
    MAIN,
    BROWSER,
    FRIENDS,
    CHAT, 
    LOBBY,
  };

 private:
  MenuState state;

  NM::Message::Matches matches;
  std::shared_ptr<LobbyView> lobby;

  string current_recipient;
  vector<string> chat;

 public:
  MenuView() : state{MenuState::LOGIN} {}

  [[nodiscard]] inline MenuState          currentState()     const { return state; }
  [[nodiscard]] inline string             currentRecipient() const { return current_recipient; }
  [[nodiscard]] inline span<string const> currentChat()      const { return chat; }

  inline void setState(MenuState new_state) { state = new_state; }

  void loadMatches(const NM::Message::Matches& _matches) { matches = _matches; }
  [[nodiscard]] inline const NM::Message::Matches& getMatches() const { return matches; }
  inline void clearBrowser() { matches.clear(); matches.shrink_to_fit(); }

  void loadChat(const NM::Message::ChatLog& log) { std::tie(current_recipient, chat) = log.data(); }

  void appendChat(string_view name, string_view line) {
    chat.emplace_back(string{name} + ": " + string{line});
  }

  void appendChat(const NM::Message::ChatUpdate& update) {
    auto&& [sender, receiver, line] = update.data();
    chat.emplace_back(sender + ": " + line);
  }

  void clearChat() {
    current_recipient.clear();
    chat.clear();
    current_recipient.shrink_to_fit();
    chat.shrink_to_fit();
  }
};
