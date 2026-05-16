#include <iomanip>
#include <limits>
#include <iostream>
#include <string>
#include <span>
#include <ranges>
#include <fstream>

#include "console_menu_display.hh"
#include "client.hh"
#include "../common/network_io.hh"
#include "../common/utils.hh"

namespace ranges = std::ranges;
namespace views = ranges::views;
using std::string, std::string_view, std::span, std::vector;

void replayMenuLoop();

void MenuControl::authentify(const NM::Message& message) {
  auto menu = view.lock();
  auto account = message.extract<NM::Message::Account>();
  if (menu && account) {
    menu->setState(MenuView::MenuState::MAIN);
    session->newSession(*account);
  }
}

void MenuControl::startLobby(const NM::Message& message) {
  auto menu = view.lock();
  auto l = lobby.lock();
  auto match_details = message.extract<NM::Message::HostLobby>();
  if (!menu || !l || !match_details)
    return;

  menu->setState(MenuView::MenuState::LOBBY);
  l->newLobby(std::move(*match_details));
}

void MenuControl::joinLobby(const NM::Message& message) {
  auto menu = view.lock();
  auto l = lobby.lock();
  auto match_details = message.extract<NM::Message::JoinLobby>();
  if (!menu || !l || !match_details)
    return;

  menu->setState(MenuView::MenuState::LOBBY);
  l->joinLobby(std::move(*match_details));
}

void MenuControl::updateRelation(const NM::Message& message) {
  if (auto relationship = message.extract<NM::Message::Relationship>())
    session->updateRelation(*relationship);
}

void MenuControl::loadMatches(const NM::Message& message) {
  auto menu = view.lock();
  auto matches = message.extract<NM::Message::Matches>();
  if (menu && matches) {
    menu->loadMatches(*matches);
  }
}

void MenuControl::quitLobby() {
  auto menu = view.lock();
  auto l = lobby.lock();
  if (!menu || !l)
    return;
  menu->setState(MenuView::MenuState::BROWSER);
  l->clear();
}

void MenuControl::updateLobby(const NM::Message& message) {
  auto menu = view.lock();
  auto l = lobby.lock();
  auto match_params = message.extract<NM::Message::LobbyParameters>();
  if (menu && l && match_params) {
    l->updateParameters(match_params->data());
  }
}

void MenuControl::updateLobbyMember(const NM::Message& message) {
  auto menu = view.lock();
  auto l = lobby.lock();
  auto member = message.extract<NM::Message::SlotLobby>();
  if (menu && l && member)
    l->updateMember(*member);
}

void MenuControl::loadChat(const NM::Message& message) {
  auto menu = view.lock();
  auto log = message.extract<NM::Message::ChatLog>();
  if (menu && log) {
    menu->loadChat(*log);
  }
}

void MenuControl::appendChat(const NM::Message& message) {
  auto menu = view.lock();
  auto update = message.extract<NM::Message::ChatUpdate>();
  if (!menu || !update)
    throw std::runtime_error("Server sent invalid chat message");

  menu->appendChat(*update);
}

NM::Message MenuControl::formatAuth(string_view line) const {
  auto format_request = [](auto&& r) { return string_view(r); };

  auto split = line | views::split(' ') | views::transform(format_request);
  vector<string_view> tokens{split.begin(), split.end()};

  if (tokens.size() == 3) {
    auto credentials = NM::Message::Credentials(tokens[1], tokens[2]);
    if (tokens[0] == "login")
      return NM::Message(Networkable::Request::LOGIN, std::move(credentials));
    if (tokens[0] == "register")
      return NM::Message(Networkable::Request::REGISTER, std::move(credentials));
  }

  return {};
}

NM::Message MenuControl::formatMain(std::string_view line) const {
  using enum MenuControl::MainOptions;

  auto menu = view.lock();
  std::optional<uint8_t> choice_int = NM::from_string(line);
  if (!menu || !choice_int || *choice_int >= static_cast<uint8_t>(LAST_SENTINEL))
    return {};

  auto choice = static_cast<MenuControl::MainOptions>(*choice_int);
  switch (choice) {
    case BROWSER:
      menu->setState(MenuView::MenuState::BROWSER);
      return NM::Message(Networkable::Request::GET_MATCHES);
  #ifndef GUI
    case FRIENDS:
      menu->setState(MenuView::MenuState::FRIENDS);
      break;
  #endif
    case LOGOUT:
      menu->setState(MenuView::MenuState::LOGIN);
      session->clear();
      return NM::Message(Networkable::Request::LOGOUT);
    case REPLAY:
      replayMenuLoop();  // Faster
      break;
    default:
      break;
  }

  return {};
}

NM::Message MenuControl::formatBrowser(std::string_view line) const {
  if (auto menu = view.lock()) {
    auto split = line | views::split(' ');
    vector<string_view> tokens{split.begin(), split.end()};
    if (tokens.empty())
      return {};

    if (tokens.front() == "/q") {
      menu->setState(MenuView::MenuState::MAIN);
      menu->clearBrowser();
      return {};
    }

    if (tokens.front() == "/r")
      return NM::Message(Networkable::Request::GET_MATCHES);

    if (tokens.size() == 2)
      tokens.emplace_back("");
    if (tokens.size() != 3 || tokens[1].size() >= Lobby::LOBBY_NAME_MAXSIZE)
      return {};

    if (tokens.front() == "/c")
      return NM::Message(Networkable::Request::HOST, NM::Message::HostLobby(tokens[1], tokens[2]));

    if (tokens.front() == "/j")
      return NM::Message(Networkable::Request::JOIN, NM::Message::HostLobby(tokens[1], tokens[2]));
  }
  return {};
}

NM::Message MenuControl::formatFriends(std::string_view line) const {
  using enum MenuControl::FriendsOptions;

  auto split = line | views::split(' ');
  vector<string_view> tokens{split.begin(), split.end()};

  if (tokens.empty() || (tokens.size() == 1 && tokens[0] != "7"))
    return {};

  auto menu = view.lock();
  std::optional<uint8_t> choice_int = NM::from_string(tokens.at(0));
  if (!menu || !choice_int || *choice_int >= static_cast<uint8_t>(LAST_SENTINEL))
    return {};

  auto invalid = [&tokens](span<string const> list, bool exists) {
    return tokens.size() != 2 ||
      exists ? ranges::find(list, tokens.at(1)) != list.end() :
      ranges::find(list, tokens.at(1)) == list.end();
    };

  auto choice = static_cast<MenuControl::FriendsOptions>(*choice_int);
  switch (choice) {
    using enum Networkable::Request;
    using enum NM::Message::Relationship::Kind;
    using Relationship = NM::Message::Relationship;
    case SEND:
      if (invalid(session->getOutbound(), true))
        break;
      return NM::Message(UPDATE_RELATIONSHIPS, Relationship(session->getUsername(), tokens.at(1), SENDING));
    case ACCEPT:
      if (invalid(session->getInbound(), false))
        break;
      return NM::Message(UPDATE_RELATIONSHIPS, Relationship(session->getUsername(), tokens.at(1), ACCEPTING));
    case REJECT:
      if (invalid(session->getInbound(), false))
        break;
      return NM::Message(UPDATE_RELATIONSHIPS, Relationship(session->getUsername(), tokens.at(1), REJECTING));
    case REMOVE:
      if (invalid(session->getFriends(), false))
        break;
      return NM::Message(UPDATE_RELATIONSHIPS, Relationship(session->getUsername(), tokens.at(1), REMOVING));
    case CHAT:
      if (invalid(session->getFriends(), false))
        break;
      menu->setState(MenuView::MenuState::CHAT);
      return NM::Message(LOAD_CHAT, NM::Message::ChatLog(tokens.at(1), {}));
    case ACCEPTINVITE:
      return NM::Message(Networkable::Request::JOINONINVITE, Relationship(session->getUsername(), tokens.at(1), JOINGAME));
    case RETURN:
      menu->setState(MenuView::MenuState::MAIN);
      break;
    default:
      break;
  }
  return {};
}

NM::Message MenuControl::formatChat(std::string_view line) const {
  using enum Networkable::Request;
  using ChatUpdate = NM::Message::ChatUpdate;

  if (auto menu = view.lock()) {
    if (line == "/q") {
      menu->setState(MenuView::MenuState::FRIENDS);
      menu->clearChat();
    } else {
      menu->appendChat(session->getUsername(), line);
      return NM::Message(CHAT_MESSAGE, ChatUpdate(session->getUsername(), menu->currentRecipient(), line));
    }
  }
  return {};
}

NM::Message MenuControl::formatLobby(std::string_view line) const {
  if (auto menu = view.lock()) {
    auto split = line | views::split(' ');
    vector<string_view> tokens{split.begin(), split.end()};
    if (tokens.empty())
      return {};

    auto l = lobby.lock();
    if (!l)
      return {};

    if (tokens.front() == "/q") {
      menu->setState(MenuView::MenuState::BROWSER);
      l->clear();
      return NM::Message(Networkable::Request::QUIT_LOBBY);
    }

    if (tokens.front() == "/s" && tokens.size() == 2) {
      NM::Message::SlotLobby::Slot slot = NM::Message::SlotLobby::Slot::QUITTING;
      if (tokens[1] == "left")
        slot = NM::Message::SlotLobby::Slot::LEFT;
      else if (tokens[1] == "right")
        slot = NM::Message::SlotLobby::Slot::RIGHT;
      else if (tokens[1] == "spec")
        slot = NM::Message::SlotLobby::Slot::SPECTATOR;
      else
        return {};

      return NM::Message(Networkable::Request::UPDATE_LOBBY_MEMBER, NM::Message::SlotLobby(session->getUsername(), slot));
    }

    // ---------- HOST SECTION ----------

    if (l->kind() != LobbyView::Kind::HOST)
      return {};

    if (tokens.front() == "/c")
      // Les paramètres doivent être déjà synchronisé pour tout le monde. Il ne faut rien attacher au message
      return NM::Message(Networkable::Request::START_GAME);

    if (tokens.front() == "/i" && tokens.size() == 2) {
      return NM::Message(Networkable::Request::UPDATE_RELATIONSHIPS, NM::Message::Relationship(session->getUsername(), tokens.at(1), NM::Message::Relationship::Kind::SENDINGGAME));
    }

    if ((tokens.front() == "/g" || tokens.front() == "/t") && tokens.size() >= 2 && (tokens[1] == "1" || tokens[1] == "2")) {
      auto&& [game_seconds, turn_seconds, time_type, game_type] = l->parameters();
      if (tokens.front() == "/g") {
        if (tokens[1] == "1")
          game_type = GameModel::GameMode::CLASSIC;
        if (tokens[1] == "2")
          game_type = GameModel::GameMode::COMMANDERS;
      }
      if (tokens.front() == "/t") {
        if (tokens[1] == "1")
          time_type = Timer::Type::GLOBAL;
        else if (tokens[1] == "2")
          time_type = Timer::Type::TURN;
        else
          return {};

        if ((time_type == Timer::Type::GLOBAL && tokens.size() != 3) ||
            (time_type == Timer::Type::TURN   && tokens.size() != 4))
          return {};

        auto&& new_game = NM::from_string(tokens[2]);
        if (!new_game)
          return {};
        game_seconds = chrono::seconds(*new_game);
          
        if (tokens.size() == 4) {
          auto&& new_turn = NM::from_string(tokens[3]);
          if (!new_turn)
            return {};
          turn_seconds = chrono::seconds(*new_turn);
        }
      }
      return NM::Message(Networkable::Request::UPDATE_LOBBY, NM::Message::LobbyParameters({game_seconds, turn_seconds, time_type, game_type}));
    }

  }

  return {};
}

// Emergency
void replayMenuLoop() {
  std::ifstream ifs("./last.replay", std::ios::binary);
  if (!ifs)
    return;
  std::vector<std::byte> bytes;

  std::streampos size;
  ifs.seekg(0, std::ios::end);
  size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  bytes.resize(size);
  ifs.read(reinterpret_cast<std::ifstream::char_type *>(bytes.data()), size);

  // Assume file is healthy
  auto recording = NM::Message::Recording::from_bytes(bytes);
  auto&& [left_name, right_name, moves] = recording.data();

  std::shared_ptr<ClientView>    board   = std::make_shared<ClientView>(left_name, right_name);
  board->setGameState(GameModel::GameStage::SPECTATING);
  board->setVictor(GameModel::Victor::REPLAY);
  std::shared_ptr<ClientControl> control = std::make_shared<ClientControl>(board, nullptr);
  auto game = std::make_unique<ConsoleGameDisplay>(std::cout, std::cin, board, control, nullptr);

  size_t current_index = 0;
  while (true) {

    game->update();
    
    std::cout << "> Next:  'n'\n";

    string line;
    std::getline(std::cin, line);

    if (line == "/q") {
      break;
    } else if (line == "n" && current_index < moves.size()) {
      control->acceptFire(NM::Message(Networkable::Request::GAME, std::move(moves[current_index])));
      current_index += 1;
    } else {
      break;
    }
  }


}
