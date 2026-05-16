#include "client.hh"
#include <iostream>
#include <ranges>
#include <fstream>

#include "../common/serializer.hh"

namespace views = std::ranges::views;

void SessionInfo::newSession(const NM::Message::Account& account) {
  std::tie(username,
           relationships.at(FRIENDS),
           relationships.at(REQUESTS_INBOUND),
           relationships.at(REQUESTS_OUTBOUND),
           relationships.at(GAME_REQUESTS)) = account.data();
}

void SessionInfo::remove(vector<string>& relation, string_view username) {
  auto it = ranges::find(relation, username);
  if (it != relation.end())
    relation.erase(it);
}

void SessionInfo::updateRelation(const NM::Message::Relationship& relation) {
  auto&& [you, other, kind] = relation.data();

  switch (kind) {
    using enum NM::Message::Relationship::Kind;
    case SENDING:
      relationships.at(REQUESTS_OUTBOUND).push_back(other);
      break;
    case RECEIVING:
      relationships.at(REQUESTS_INBOUND).push_back(other);
      break;
    case ACCEPTING:
      relationships.at(FRIENDS).push_back(other);
      remove(relationships.at(REQUESTS_OUTBOUND), other);
      remove(relationships.at(REQUESTS_INBOUND), other);
      break;
    case REJECTING:
      remove(relationships.at(REQUESTS_OUTBOUND), other);
      remove(relationships.at(REQUESTS_INBOUND), other);
      break;
    case REMOVING:
      remove(relationships.at(FRIENDS), other);
      break;
    case SENDINGGAME:
      relationships.at(GAME_REQUESTS).push_back(other);
      break;
    default:
      throw std::runtime_error("Updating relation failed with unknown enum value");
  }
}

void SessionInfo::clear() noexcept {
  username.clear();
  username.shrink_to_fit();
  for (auto&& relationship : relationships) {
    relationship.clear();
    relationship.shrink_to_fit();
  }
}

void Client::startGame(Lobby::parameter_t params, bool spec) {
#ifndef GUI
  timer.make(std::get<2>(params), std::get<0>(params), std::get<1>(params));
  std::shared_ptr<ClientView>    board   = std::make_shared<ClientView>(session.getUsername(), menu->playerNames(), spec);
  board->setGamemode(std::get<3>(params));
  std::shared_ptr<ClientControl> control = std::make_shared<ClientControl>(board, &timer);
  if (std::get<3>(params) == GameModel::GameMode::CLASSIC)
    control->acceptFaction(NM::Message(Networkable::Request::GAME, NM::Message::Faction(GameModel::Faction::CLASSIC)));
  if (spec)
    board->setGameState(GameModel::GameStage::SPECTATING);
  game = std::make_unique<ConsoleGameDisplay>(std::cout, std::cin, board, control, &timer);
#endif
}

void Client::failed() {
  std::cout << "Could not connect to server!\n";
  server_fd = -1;
}

Client::Client(string_view ip) : state{State::MENU} {
  if (timer.fd() == -1) {
    std::cout << "Could not start timer\n";
    return;
  }
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return;
  }
  sockaddr_in address{
    .sin_family{AF_INET},
    .sin_port{htons(PORT)}
  };
  if (inet_pton(AF_INET, ip.data(), &address.sin_addr) != 1) {
    failed();
    return;
  }
  if (connect(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    failed();
    return;
  }
  if (set_sigaction() == -1) {
    write_message(server_fd, NM::Message(Networkable::Request::DISCONNECT));
    failed();
    return;
  }
  poll_fds = { {{timer.fd(), POLLIN}, {STDIN_FILENO, POLLIN}, {server_fd, POLLIN}}};

  std::shared_ptr<LobbyView>   lobby   = std::make_shared<LobbyView>();
  std::shared_ptr<MenuView>    view    = std::make_shared<MenuView>();
  std::shared_ptr<MenuControl> control = std::make_shared<MenuControl>(view, lobby, &session);

#ifdef GUI
  gui_thread = std::make_unique<GUIThread>(menu_data_t{view, control, lobby, std::experimental::make_observer(&session), &timer});
#else
  menu       = std::make_unique<ConsoleMenuDisplay>(std::cout, std::cin, view, control, lobby, std::experimental::make_observer(&session));
#endif

  std::cout << "Connected to server!\n";
}

Client::~Client() {
  std::cout << "\nClosing client...\n";
  if (server_fd != -1) {
    write_message(server_fd, NM::Message(Networkable::Request::DISCONNECT));
    close(server_fd);
  }
}

void Client::watch() {
  if (server_fd == -1) return;

  while (true) {
#ifndef GUI
    switch (state) {
      using enum State;
      case MENU:
          menu->display();
        break;
      case PLAYING:
          game->update();
        break;
      default:
        throw NotImplementedError("Activity not implemented");
    }

    poll(poll_fds.data(), 3, -1);
#else
    poll(poll_fds.data(), 3, 500);
#endif
    if (is_interrupted)
      return;

#ifndef GUI
    if (poll_fds[USER].revents & POLLIN) {

      NM::Message result;
      switch (state) {
        using enum State;
        case MENU:
          result = menu->handleInput();
          break;
        case PLAYING:
          result = game->handleInput();
          break;
        default:
          throw std::runtime_error("Unknown client state");
      }

      if (result.empty())
        continue;

      write_message(server_fd, std::move(result));

    }
#else
    if (auto message = gui_thread->popMessage()) {
      write_message(server_fd, std::move(*message));
    }
#endif
    else if (poll_fds[SERVER].revents & POLLIN) {

      NM::Message message = read_message(server_fd);

      if (message.empty()) {
        std::cout << "Connection lost to server!\n";
        return;
      } else if (message.request() == Networkable::Request::DISCONNECT) {
        std::cout << "Server has shut down!\n";
        return;
      }
      
      if (message.request() == Networkable::Request::RECORDING) {
        std::ofstream ofs("./last.replay", std::ios::binary);
        auto body = message.get_body();
        ofs.write(reinterpret_cast<const char*>(body.data()), body.size());
      }

      switch (state) {
        using enum State;
        case MENU:
          if (message.request() == Networkable::Request::START_GAME || message.request() == Networkable::Request::START_SPECTATING) {
            state = PLAYING;
            bool spectator = message.request() == Networkable::Request::START_SPECTATING;
            #ifndef GUI
            startGame(menu->getLobbyParameters(), spectator);
            #else
            gui_thread->switchActiveState();
            #endif
            break;
          }
#ifndef GUI
          menu->handleServer(message);
#else
          gui_thread->menuHandleServer(message);
#endif
          break;

        case PLAYING:
#ifndef GUI
          if (message.request() == Networkable::Request::BACK_TO_LOBBY) {
            state = State::MENU;
            game = nullptr;
            break;
          }
#endif
#ifndef GUI
          game->handleServer(message);
#else
          gui_thread->gameHandleServer(message);
#endif
          break;
        default:
          throw ("Client.cc handle state not implemented");
      }

    } else if (poll_fds[SERVER].revents & (POLLPRI | POLLRDHUP | POLLERR | POLLHUP | POLLNVAL)) {

      std::cout << "Unexpected behaviour on server: " << poll_fds[USER].revents << '\n';
      return;

    }
    
  #ifndef GUI
    if (poll_fds[TIMER].revents & POLLIN) {
      if (timer.on())
        timer.update();
      if (timer.turnDone() || timer.done())
        write_message(server_fd, NM::Message(Networkable::Request::OUT_OF_TIME));
    }
  #endif 
 
  }
}



int main(int argc, char* argv[]) {
  string ip = "127.0.0.1";
  if (argc > 1) ip = argv[1];
  Client c(ip);
  std::system("clear");  // Stop touching this FOR THE LOVE OF GOD
  c.watch();

  return 0;
}
