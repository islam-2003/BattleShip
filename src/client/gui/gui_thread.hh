#pragma once

#include <thread>
#include <functional>
#include <unistd.h>

#include "gui_game.hh"
#include "gui_menu_display.hh"

struct menu_data_t {
  std::shared_ptr<MenuView  const> const view;
  std::shared_ptr<MenuControl>     const control;
  std::shared_ptr<LobbyView const>       lobby;
  observer_ptr<SessionInfo  const>       session;
  ClientTimer* timer;
};

class GUIThread {
  std::unique_ptr<GUIMenuDisplay> menu;
  std::unique_ptr<GUIGame>        game;
  ClientTimer* timer;

  bool gameExists = false;

  std::jthread gui_thread;
  std::mutex thread_mutex;

  std::queue<NM::Message> gui_queue;

 public:
  GUIThread(menu_data_t&& menu_data) : gui_thread(std::bind_front(&GUIThread::run, this), menu_data), 
                                                  timer(menu_data.timer) {}

  void menuHandleServer(const NM::Message& message) {
    std::scoped_lock lock(thread_mutex);
    menu->handleServer(message);
  }

  void gameHandleServer(const NM::Message& message) {
    std::scoped_lock lock(thread_mutex);
    game->handleServer(message);
  }

  void startGame(std::shared_ptr<sf::RenderWindow> window, ClientTimer* timer) {
    bool mode = 1;
    Lobby::parameter_t params = menu->getLobbyParameters();
    std::string_view you = "You";
    std::shared_ptr<ClientView> board_view = std::make_shared<ClientView>(you, menu->playerNames(), 1);
    board_view->setGamemode(std::get<3>(params));
    std::shared_ptr<ClientControl> board_control = std::make_shared<ClientControl>(board_view, timer);
    if (std::get<3>(params) == GameModel::GameMode::CLASSIC && !menu->commanderModeSelected()){
      board_control->acceptFaction(NM::Message(Networkable::Request::GAME, NM::Message::Faction(GameModel::Faction::CLASSIC)));
      mode = 0;
    }
    game = std::make_unique<GUIGame>(window, board_view, board_control, timer, mode);
  }

  void switchActiveState() {
    if (menu->is_active)
      menu->is_active = 0;
    if (gameExists)
      game->is_active = 1;
  }

  [[nodiscard]] std::optional<NM::Message> popMessage() {
    std::scoped_lock lock(thread_mutex);
    if (gui_queue.empty())
      return std::nullopt;

    auto message = gui_queue.front();
    gui_queue.pop();
    return message;
  }

  void run(std::stop_token token, menu_data_t&& menu_data) {
    auto&& [view, control, lobby, session, timer] = menu_data;

    std::shared_ptr<sf::RenderWindow> window = std::make_shared<sf::RenderWindow>(sf::VideoMode(1500, 1000), "Battleship");
    window->setPosition({100, 100});

    menu = std::make_unique<GUIMenuDisplay>(window, view, control, lobby, session);

    while (!token.stop_requested() && window->isOpen()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(33));  // Limit frames to 30 per second
      std::scoped_lock lock(thread_mutex);
      if (menu->is_active) {
        NM::Message message = menu->pollEvent();
        if (!message.empty()) {
          gui_queue.push(message);
        }
        menu->display();
      }
      if (!menu->is_active && !gameExists) {
        startGame(window, timer);
        gameExists = 1;
      }
      else if (!menu->is_active && game->is_active) {
        if (!game->getCommander() && menu->commanderModeSelected())
          game->setCommander();
        NM::Message message = game->pollEvent();
        if (!message.empty()) {
          gui_queue.push(message);
        }
        game->display();
      }
    }
  }

  GUIThread(GUIThread&&)      = delete;
  GUIThread(const GUIThread&) = delete;
};
