#include "client_board.hh"

#include <iostream>
#include <ranges>
#include <functional>
#include <algorithm>

#include "../common/serializer.hh"
#include "../common/network_io.hh"
#include "../common/utils.hh"
#include "../common/boat.hh"

using namespace NM;
using enum Networkable::Request;
namespace ranges = std::ranges;
namespace views = ranges::views;

//                   ╔═══════════╗
//                   ║ BoardView ║
//                   ╚═══════════╝

void ClientView::findFreeSpot(ClientBoat& new_boat) {
  while (!new_boat.inBoundsH() || overlaps(new_boat)) {
    if (!new_boat.inBoundsH()) {
      int line_feed = new_boat.origin().x();
      new_boat.shift({-line_feed, 1});
    } else {
      new_boat.shift({1, 0});
    }
  }

  if (!new_boat.inBoundsV())
    throw std::runtime_error("Could not find free spot for ship");  // We want to use the entire inventory
}

ClientView::ClientView(std::string_view you, std::pair<std::string, std::string> names, bool is_spectator) {
  if (!is_spectator) {
    name_you   = you;
    name_other = you == names.first ? names.second : names.first;
    is_left = you == names.first;
  } else {
    name_you = names.first;
    name_other = names.second;
    is_left = true;
  }
  for (auto&& row : _view_left)
    row.fill(CellType::WATER);
  for (auto&& row : _view_right)
    row.fill(CellType::WATER);
  for (auto&& row : _overlay)
    row.fill(OverlayCell::NONE);
}

ClientView::ClientView(std::string_view left, std::string_view right) {
  name_you = left;
  name_other = right;
  is_left = true;

  for (auto&& row : _view_left)
    row.fill(CellType::WATER);
  for (auto&& row : _view_right)
    row.fill(CellType::WATER);
  for (auto&& row : _overlay)
    row.fill(OverlayCell::NONE);
}

bool ClientView::overlaps(ClientBoat& new_boat) const {
  return ranges::any_of(your_fleet | views::take(your_fleet.size() - 1),  // Assume new_boat is already in fleet
                        [&new_boat](const ClientBoat& boat) { return boat.contains(new_boat); });
}

bool ClientView::inInventory(Boat::Type type) const {
  auto it = ranges::find(inventory, type);
  return (it != inventory.end()); 
}

bool ClientView::isSameShip(BoardCoordinates first, BoardCoordinates second, bool your_side) const {
  return your_side ? ranges::any_of(your_fleet,  [&first, &second](const ClientBoat& boat) { return boat.contains(first) && boat.contains(second); })
                   : ranges::any_of(enemy_fleet, [&first, &second](const ClientBoat& boat) { return boat.contains(first) && boat.contains(second); });
}

std::optional<int> ClientView::whichShip(BoardCoordinates coordinates) const {
  auto it = ranges::find_if(your_fleet, [&coordinates](const ClientBoat& boat) { return boat.contains(coordinates); });
  if (it != your_fleet.end())
    return it->getId();
  return std::nullopt;
}

void ClientView::fillInventory(GameModel::Faction faction) {
  inventory = Boat::createInventory(faction);
}

void ClientView::fillAbilities(GameModel::Faction faction) {
  switch (faction) {
    using enum Ability::Type;
    using enum GameModel::Faction;
    case CLASSIC:
      abilities = { Ability(Basic), Ability(Basic), Ability(Basic) };
      break;
    case PIRATE:
      abilities = { Ability(Basic), Ability(Diagonal), Ability(XBomb) };
      break;
    case CAPTAIN:
      abilities = {Ability(Basic),Ability(Linear),Ability(PlusBomb)};
      break;
    default:
      throw NotImplementedError("Ability set doesn't exist: 97 cb");
  }
}

void ClientView::addShip(Boat::Type type) {
  if (inventory.size() == 0)
    return;

  auto it = ranges::find(inventory, type);
  if (it == inventory.end())
    return;
  inventory.erase(it);

  int id = your_fleet.size() == 0 ? 1 : your_fleet.back().getId() + 1;
  ClientBoat new_boat{type, id, Boat::assembleByType(type, {0, 0})};
  findFreeSpot(your_fleet.emplace_back(ClientBoat{type, id, Boat::assembleByType(type, {0, 0})}));
  
  for (auto&& c : your_fleet.back().getCoordinates()) {
    _view_left.at(c.y()).at(c.x()) = CellType::UNDAMAGED;
  }
}

void ClientView::setCell(bool your_side, BoardCoordinates coordinate, GameModel::CellType type) {
  auto&& side = your_side ? _view_left : _view_right;
  side[coordinate.y()][coordinate.x()] = type;
}

//                   ╔═══════════════╗
//                   ║ ClientControl ║
//                   ╚═══════════════╝

void ClientControl::do_move_selection(BoardCoordinates::Transform transform) {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return;

  Ability chosen_ability = view->getLastAbility();
  if (view->getGamemode() == GameModel::GameMode::CLASSIC)  // Fixes exception thrown in assemleByType
    chosen_ability = Ability::Type::Basic;

  ClientBoat boat = ClientBoat(Boat::Type::SENTINEL, -1, chosen_ability.assembleByType(chosen_ability.getType(), view->getSelection().at(0)));

  span<BoardCoordinates> cells = boat.getCoordinates();
  vector<BoardCoordinates> old_cells(cells.begin(), cells.end());

  boat.shift(transform);
  if (!boat.inBoundsH() || !boat.inBoundsV()) {
    boat.setCoordinates(old_cells);
  }

  for (auto&& cell : old_cells)
    view->setSelectionCell(ClientView::OverlayCell::NONE, cell);
  for (auto&& cell : cells)
    view->setSelectionCell(ClientView::OverlayCell::RETICLE, cell);

  std::cerr << "ClientControl: moved selection\n";

  view->setSelection(boat.getCoordinates());
}

void ClientControl::do_move(BoardCoordinates::Transform transform) {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return;

  ClientBoat& boat = view->getLastBoat();
  span<BoardCoordinates> cells = boat.getCoordinates();
  vector<BoardCoordinates> old_cells(cells.begin(), cells.end());

  do {
    boat.shift(transform);
    if (!boat.inBoundsH() || !boat.inBoundsV()) {
      std::cerr << "ClientControl: Outside of grid\n";
      boat.setCoordinates(old_cells);
      return;
    }
  } while (view->overlaps(boat));

  for (auto&& cell : old_cells)
    view->setCell(true, cell, GameModel::CellType::WATER);
  for (auto&& cell : cells)
    view->setCell(true, cell, GameModel::CellType::UNDAMAGED);

  std::cerr << "ClientControl: moved ship\n";
}

void ClientControl::rotate(bool orientation) {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return;

  ClientBoat& boat = view->getLastBoat();
  span<BoardCoordinates> cells = boat.getCoordinates();
  vector<BoardCoordinates> old_cells(cells.begin(), cells.end());

  BoardCoordinates fulcrum = cells[0];
  array<std::function<void(BoardCoordinates&)>, 2> rotations{
   [&fulcrum](BoardCoordinates& ship_cell) { ship_cell.set(fulcrum.x() - ship_cell.y() + fulcrum.y(), fulcrum.y() - fulcrum.x() + ship_cell.x()); },
   [&fulcrum](BoardCoordinates& ship_cell) { ship_cell.set(fulcrum.x() - fulcrum.y() + ship_cell.y(), fulcrum.y() - ship_cell.x() + fulcrum.x()); }
  };

  const auto& rotation = orientation ? rotations.at(0) : rotations.at(1);
  ranges::for_each(cells | std::views::drop(1), rotation);

  if (view->overlaps(boat) || !boat.inBoundsH() || !boat.inBoundsV()) {
    std::cerr << "ClientControl: Cannot rotate\n";
    boat.setCoordinates(old_cells);
    return;
  }

  for (auto&& cell : old_cells | std::views::drop(1))
    view->setCell(true, cell, GameModel::CellType::WATER);
  for (auto&& cell : cells | std::views::drop(1))
    view->setCell(true, cell, GameModel::CellType::UNDAMAGED);

  std::cerr << "ClientControl: Rotation\n";
}

Message ClientControl::confirm() {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return {};

  ClientBoat& boat = view->getLastBoat();

  if (view->getGamemode() == GameModel::GameMode::CLASSIC) {
    for (BoardCoordinates ship_cell : boat.getCoordinates()) {
      array<BoardCoordinates, 4> adjacents{ {
        {ship_cell.x(),     ship_cell.y() - 1},  // Up
        {ship_cell.x() - 1, ship_cell.y()},      // Left
        {ship_cell.x(),     ship_cell.y() + 1},  // Down
        {ship_cell.x() + 1, ship_cell.y()}       // Right
      } };

      auto is_adjacent = [this, &ship_cell, &view](BoardCoordinates adjacent) {
        return (!(adjacent.x() >= view->width() || adjacent.y() >= view->height()) &&
                view->getCell(view->leftTurn(), adjacent) == GameModel::CellType::UNDAMAGED &&  // Temp
                !view->isSameShip(adjacent, ship_cell, true));
        };
      if (ranges::any_of(adjacents, is_adjacent)) {
        std::cerr << "ClientControl: Adjacent to another ship" << '\n';
        return {};
      }
    }
  }

  std::cerr << "ClientControl: Confirmation" << '\n';
  return Message(GAME, Message::Confirmation(boat.getCoordinates(), boat.getId(), boat.getType()));
}

Message ClientControl::factionSelect(string faction) {
  std::shared_ptr<ClientView> view = _view.lock();

  std::optional<uint8_t> faction_int = from_string(faction);

  if (!view || !faction_int || *faction_int > 2 || *faction_int == 0)
    return {};

  return Message(GAME, Message::Faction(static_cast<GameModel::Faction>(*faction_int)));
}

Message ClientControl::select(string boat_type) {
  std::shared_ptr<ClientView> view = _view.lock();

  std::optional<uint8_t> boat_int = from_string(boat_type);

  if (!view || !boat_int || *boat_int - 1 >= to_underlying(Boat::Type::SENTINEL)) 
    return {};

  Boat::Type boat = static_cast<Boat::Type>(*boat_int - 1);

  if (!(view->inInventory(boat))) {
    std::cerr << "ClientControl received an invalid ship selection: " << boat_type << '\n';
    return {};
  }

  return Message(GAME, Message::BoatSelection(boat));
}

Message ClientControl::move(char input) {
  switch (std::toupper(input)) {
    case 'Z':
      do_move({0, -1});
      break;
    case 'Q':
      do_move({-1,  0});
      break;
    case 'S':
      do_move({0,  1});
      break;
    case 'D':
      do_move({1,  0});
      break;
    case 'A':
      rotate(false);
      break;
    case 'E':
      rotate(true);
      break;
    case 'C':
      return confirm();
    default:
      std::cerr << "BoardViewControl received an invalid move command: " << input << ". Unknown command" << '\n';
  }

  return {};
}

void ClientControl::move_gui(BoardCoordinates::Transform transform) {
  do_move(transform);
}

void ClientControl::rotate_gui(bool input) {
  rotate(input);
}

Message ClientControl::confirm_gui(bool& click) {
  if (!click) {
    click = 1;
    return confirm();
  }
  return {};
}

bool ClientControl::isValidPlace() {
  return acceptPlaceGUI;
}

void ClientControl::setSelectionGUI(vector<BoardCoordinates> s) {
  std::shared_ptr<ClientView> view = _view.lock();
  view->setSelection(s);
}

BoardCoordinates ClientControl::getFirstCoordinate() {  
  std::shared_ptr<ClientView> view = _view.lock();
  return view->getLastBoat().getCoordinates()[0];
}

Message ClientControl::fireGUI() {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return {};
  return executeFire();
}

Message ClientControl::fire(char input) {
  std::shared_ptr<ClientView> view = _view.lock();
  if (!view) return {};
  switch (std::toupper(input)) {
    case 'Z':
      do_move_selection({ 0, -1 });
      break;
    case 'Q':
      do_move_selection({-1,  0 });
      break;
    case 'S':
      do_move_selection({ 0,  1 });
      break;
    case 'D':
      do_move_selection({ 1,  0 });
      break;
    case 'C': 
      return executeFire();
    default:
      std::cerr << "BoardViewControl received an invalid move command: " << input << ". Unknown command" << '\n';
  }

  return {};
}

Message ClientControl::selectAbility(string ability) {
  std::shared_ptr<ClientView> view = _view.lock();

  std::optional<int> ability_int = from_string(ability);

  if (!view || !ability_int.has_value()) return {};

  (*ability_int)--;
  if (*ability_int < 0 || *ability_int > 2) {
    std::cerr << "ClientControl received an invalid ability selection: " << ability << '\n';
    return {};
  }
  return Message(GAME, Message::AbilitySelection(view->getAbilities().at(*ability_int).getType()));
}

Message ClientControl::executeFire() {
  std::shared_ptr<ClientView> view = _view.lock();
  BoardCoordinates position = view->getSelection().at(0);
  view->setSelection({BoardCoordinates{5,5}});
  view->clearSelection();

  if (!view || position.x() >= view->width() || position.y() >= view->height())
    return {};

  Ability chosen_ability = view->getLastAbility();
  if (view->getGamemode() == GameModel::GameMode::CLASSIC)
    chosen_ability = Ability::Type::Basic;

  switch (chosen_ability.getType()) {
    using enum Ability::Type;
    case Basic:
      if (view->getCell(false, position) != GameModel::CellType::WATER) {
        // view->setMessage(position.toString() + ": Invalid Target");
        std::cerr << "ClientControl received an invalid fire target: " << position << '\n';
        return {};
      }
      std::cerr << "ClientControl received a valid fire target: " << position << '\n';
      return Message(GAME, Message::ClientFire(vector{position}, Basic));
    default:
      vector<BoardCoordinates> area = chosen_ability.assembleByType(chosen_ability.getType(),position);
      return Message(GAME, Message::ClientFire(area, chosen_ability.getType()));
    }
}

void ClientControl::acceptFaction(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();

  auto faction_selection = message.extract<Message::Faction>();

  if (!view || !faction_selection) {
    std::cerr << "Faction Selection not accepted\n";
    return;
  }

  GameModel::Faction faction = faction_selection->data();

  view->fillInventory(faction);
  view->fillAbilities(faction);

  std::cerr << "ClientControl received a valid faction selection: " << to_string(faction) << '\n';
  view->setGameState(GameModel::GameStage::SELECTION);
}

void ClientControl::acceptSelect(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();

  auto boat_selection = message.extract<Message::BoatSelection>();

  if (!view || !boat_selection) {
    std::cerr << "Selection not accepted\n";
    return;
  }

  Boat::Type boat = boat_selection->data();

  // Second check
  if (!(view->inInventory(boat))) {
    std::cerr << "ClientControl received an invalid ship selection: " << to_string(boat) << '\n';
    return;
  }

  view->addShip(boat);
  std::cerr << "ClientControl received a valid ship selection: " << to_string(boat) << '\n';
  view->setGameState(GameModel::GameStage::PLACEMENT);
}

void ClientControl::acceptPlace(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();

  auto confirmation = message.extract<Message::Confirmation>();

  if (!view || !confirmation) {
    std::cerr << "Placement not accepted\n";
    //acceptPlaceGUI = false; //added for GUI
    return;
  }

  auto&& [coordinates, id, type] = confirmation->data();
  ClientBoat boat(type, id, coordinates);

  if (view->getLastBoat() != boat) {
    std::cerr << "Placement not accepted\n";
    acceptPlaceGUI = false; //added for GUI
    return;
  }

  std::cerr << "ClientControl received a valid ship placement: " << to_string(type) << '\n';
  acceptPlaceGUI = true; //added for GUI

  view->setGameState(GameModel::GameStage::SELECTION);

  if (view->getInventory().size() == 0) {
    view->setGameState(GameModel::GameStage::WAITING);
    view->setTurn(false);
  }
}

void ClientControl::acceptStart(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();

  auto start = message.extract<Message::StartCombat>();

  if (!view || !start) {
    std::cerr << "Invalid start message\n";
    return;
  }

  bool left = start->data();
  view->setTurn(left);
  if (left) {
  #ifndef GUI
    timer->start();
  #endif 
    if (view->getGamemode() == GameModel::GameMode::CLASSIC)
      view->setGameState(GameModel::GameStage::COMBAT);
    else
      view->setGameState(GameModel::GameStage::ATTACKSELECT);
  } else {
    view->setGameState(GameModel::GameStage::OTHERTURN);
  }
#ifndef GUI
  if (timer->type() == Timer::Type::GLOBAL)
    timer->start();
#endif
}

void ClientControl::acceptAbility(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();

  auto ability = message.extract<Message::AbilitySelection>();

  if (!view || !ability) {
    std::cerr << "Ability Selection not accepted\n";
    return;
  }

  Ability::Type type = ability->data();

  if (to_underlying(type) >= to_underlying(Ability::Type::A_SENTINEL)) {
    std::cerr << "ClientControl received an invalid ability selection: " << to_string(type) << '\n';
    return;
  }

  view->setLastAbility(type);
  std::cerr << "ClientControl received a valid ship selection: " << to_string(type) << '\n';
  view->setGameState(GameModel::GameStage::COMBAT);
}

void ClientControl::acceptFire(const Message& message) {
  std::shared_ptr<ClientView> view = _view.lock();
  auto response = message.extract<Message::ServerFire>();

  if (!view || !response) {
    std::cerr << "Invalid firing message\n";
    return;
  }

  auto&& [new_cells, your_board, your_turn, new_energy] = response->data();

  view->setEnergy(new_energy);

  for (auto&& cell : new_cells)
    view->setCell(your_board, cell.c, cell.new_state);

  if (!your_board || view->gameState() == GameModel::GameStage::SPECTATING) {
    for (auto&& cell : new_cells) {
      auto&& boat = view->getBoatWith(cell.id, your_board);
      if (!boat) {
        ClientBoat enemy_boat(Boat::Type::SENTINEL, cell.id, {cell.c});
        view->pushBoat(enemy_boat, your_board);
      } else {
        boat->addCoordinate(cell.c);
      }
    }
  }
#ifndef GUI
  if (timer)
    timer->swap(your_turn);
#endif
  setTurn(your_turn);
}

void ClientControl::setTurn(bool new_turn) {
  std::shared_ptr<ClientView> view = _view.lock();

  if (view) {
    if (view->gameState() == GameModel::GameStage::SPECTATING)
      return;
    
    if (new_turn) {
      if (view->getGamemode() == GameModel::GameMode::CLASSIC)
        view->setGameState(GameModel::GameStage::COMBAT);
      else
        view->setGameState(GameModel::GameStage::ATTACKSELECT);
    } else {
      view->setGameState(GameModel::GameStage::OTHERTURN);
    }
    view->setTurn(new_turn);
  }
}

NM::Message ClientControl::goToMenu(string line){
    std::shared_ptr<ClientView> view = _view.lock();
    if (!view) {
        return {};
    }
    if (line == "/q") {
        //view->setGameState(GameModel::GameStage::GAMEFINISHED);
        return NM::Message(Networkable::Request::BACK_TO_LOBBY);
    }
    return {};
}

void ClientControl::endGame(const Message& message) {
    auto victor = message.extract<Message::GameEnd>();
    std::shared_ptr<ClientView> view = _view.lock();

    if (!view || !victor) {
        std::cerr << "Invalid victor message\n";
        return;
    }
    view->setVictor(victor->data());
#ifndef GUI
    timer->stop();
#endif
}
