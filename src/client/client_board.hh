#pragma once

#include <array>
#include <span>
#include <memory>
#include <string>
#include <cstdint>
#include <experimental/memory>

#include "../common/board_coordinates.hh"
#include "../common/board_common.hh"
#include "../common/ability.hh"
#include "client_boat.hh"
#include "client_timer.hh"
#include "client_menu_controller.hh"


using std::array, std::span, std::string;
namespace epr = std::experimental;

namespace NM {
  class Message;  // Forward-declared from serializer.hh
}

/** Client-side view of the game.
 *
 * Contains a grid of special characters for displaying.
 * Contains copies of server-side state. */
class ClientView : public GameModel {
 public:
  enum class OverlayCell : uint8_t {
    NONE,
    RETICLE
  };

 private:
  GameModel::GameMode _gamemode{GameModel::GameMode::CLASSIC};

  GameStage _game_state{ (_gamemode == GameModel::GameMode::CLASSIC) ? GameStage::FACTIONSELECT : GameStage::SELECTION };

  array<array<OverlayCell, BOARDSIZE>, BOARDSIZE> _overlay;
  array<array<CellType,    BOARDSIZE>, BOARDSIZE> _view_left, _view_right;
  vector<BoardCoordinates> _selection{BoardCoordinates{5,5}};

  vector<Boat::Type> inventory;
  vector<Ability> abilities;

  vector<ClientBoat> your_fleet;
  vector<ClientBoat> enemy_fleet;

  Ability::Type last_ability;
  int current_energy{1};

  bool _your_turn{true};

  void findFreeSpot(ClientBoat& new_boat);

  std::string name_you;
  std::string name_other;
  bool is_left;

public:
  ClientView(std::string_view you, std::pair<std::string, std::string> names, bool is_spectator);
  ClientView(std::string_view left, std::string_view right);
  
  void setSelection(span<BoardCoordinates> s)   { _selection = {}; for(auto c : s) _selection.emplace_back(c); }
  void setSelection(vector<BoardCoordinates> s) { _selection = s; }
  void setSelectionCell(OverlayCell type, BoardCoordinates c) { _overlay[c.y()][c.x()] = type; }
  void clearSelection() { for (auto&& row : _overlay) row.fill(OverlayCell::NONE); }
  void setGamemode(GameModel::GameMode gm) { _gamemode = gm; } 

  [[nodiscard]] inline vector<BoardCoordinates> getSelection() const { return _selection; }
  [[nodiscard]] inline ClientBoat&         getLastBoat()          { return your_fleet.back(); }
  [[nodiscard]] inline GameModel::GameMode getGamemode()    const { return _gamemode; } 
  [[nodiscard]] inline Ability::Type       getLastAbility() const { return last_ability; }
  [[nodiscard]] inline int                 getEnergy()      const { return current_energy; }
  [[nodiscard]] inline vector<Ability>     getAbilities()   const { return abilities; }
  [[nodiscard]] inline GameStage           gameState()      const { return _game_state; }
  [[nodiscard]] inline vector<Boat::Type>  getInventory()   const { return inventory; }
  [[nodiscard]] inline OverlayCell         getSelectionCell(BoardCoordinates c) const { return _overlay.at(c.y()).at(c.x()); }
  [[nodiscard]] inline CellType            getCell(bool your_side, BoardCoordinates c) const { return (your_side ?
                                                                                            _view_left.at(c.y()).at(c.x()) :
                                                                                            _view_right.at(c.y()).at(c.x())); }

  [[nodiscard]] inline bool   isFinished() const { return _is_finished; }
  [[nodiscard]] inline Victor victor()     const { return _victor; }
  [[nodiscard]] inline size_t width()      const { return _view_left.at(0).size(); }
  [[nodiscard]] inline size_t height()     const { return _view_left.size();       }
  [[nodiscard]] inline bool   leftTurn()   const { return _your_turn; }

  inline void setGameState(GameStage new_state)  { _game_state = new_state; }
  inline void setTurn(bool new_turn)             { _your_turn = new_turn; }
  inline void setLastAbility(Ability::Type type) { last_ability = type; }
  inline void setEnergy(int value)               { current_energy = value; }
  
  [[nodiscard]] bool overlaps(ClientBoat& new_boat) const;
  [[nodiscard]] bool inInventory(Boat::Type type)   const;
  [[nodiscard]] bool isSameShip(BoardCoordinates first, BoardCoordinates second, bool your_side) const;

  [[nodiscard]] std::optional<int> whichShip(BoardCoordinates coordinates) const;

  void fillInventory(GameModel::Faction faction);
  void fillAbilities(GameModel::Faction faction);  // Rework
  void addShip(Boat::Type type);
  void setCell(bool your_side, BoardCoordinates coordinate, CellType type);

  [[nodiscard]] inline epr::observer_ptr<ClientBoat> getBoatWith(int id, bool your_side) {
    auto&& fleet = your_side ? your_fleet : enemy_fleet;
    auto it = ranges::find(fleet, id, &ClientBoat::getId);
    if (it != fleet.end())
      return std::experimental::make_observer(&*it);
    return nullptr;
  }
  inline void pushBoat(ClientBoat new_boat, bool your_side) { your_side ? your_fleet.push_back(new_boat) : enemy_fleet.push_back(new_boat); }

  [[nodiscard]] auto getNames() const { return std::tuple{is_left, name_you, name_other}; }

};

class ClientControl : public GameControl {
  std::weak_ptr<ClientView> _view;
  epr::observer_ptr<ClientTimer> timer;
  std::shared_ptr<MenuView> menu;

  void do_move_selection(BoardCoordinates::Transform transform);
  void do_move(BoardCoordinates::Transform transform);
  void rotate(bool orientation);
  NM::Message confirm();

  bool acceptPlaceGUI = false;

public:
  ClientControl(std::weak_ptr<ClientView> view, ClientTimer* timer)
    : _view{std::forward<std::weak_ptr<ClientView>>(view)},
      timer{timer} {}

  [[nodiscard]] NM::Message factionSelect(string faction);
  [[nodiscard]] NM::Message select(string boat_type);
  [[nodiscard]] NM::Message move(char input);

  void move_gui(BoardCoordinates::Transform transform);
  void rotate_gui(bool input);
  [[nodiscard]] NM::Message confirm_gui(bool& click);
  bool isValidPlace();
  BoardCoordinates getFirstCoordinate();
  void setSelectionGUI(vector<BoardCoordinates> s);
  [[nodiscard]] NM::Message fireGUI();
  
  [[nodiscard]] NM::Message executeFire();
  [[nodiscard]] NM::Message selectAbility(string ability);
  
  [[nodiscard]] NM::Message fire(char input);
  
  void acceptFaction(const NM::Message& message);
  void acceptSelect (const NM::Message& message);
  void acceptPlace  (const NM::Message& message);
  void acceptStart  (const NM::Message& message);
  void acceptFire   (const NM::Message& message);
  void acceptAbility(const NM::Message& message);

  void setTurn(bool new_turn);
  NM::Message goToMenu(string line);
  void endGame(const NM::Message& message);

  void quit() {
    if (auto p = _view.lock())
      p->setVictor(GameModel::Victor::NONE);
  }
};
