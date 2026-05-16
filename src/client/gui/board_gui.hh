#pragma once

#include <array>

#include "cell.hh"
#include "../../common/board_common.hh"

using std::vector;

class Board {
  array<array<Cell,BOARDSIZE>,BOARDSIZE> board;
  vector<sf::Text> board_text;
  sf::Font *font;
  std::shared_ptr<ClientView const> const _model;
  std::shared_ptr<ClientControl> const _control;
  GameModel::GameStage game_state = GameModel::GameStage::PLACEMENT;
  bool your_board;
  BoardCoordinates old_selection = {0,0};
  GameModel::CellType lastCellSelected =  GameModel::CellType::UNDAMAGED;

 public:
  Board(sf::Vector2f coords, sf::Vector2f cellSize, sf::Font *font, std::shared_ptr<ClientView const> model, std::shared_ptr<ClientControl> control, bool your_board, unsigned int size = 10);
  void update(const sf::Vector2f mousePos, bool &click);
  void display(std::shared_ptr<sf::RenderWindow> window);
  void placeBoats(const sf::Vector2f mousePos, bool &click);
  void multUpdate(sf::Vector2f pos, int & click, bool& lock);
  void updateState(GameModel::GameStage state);
  vector<Boat::Type> getBoats() const;
  void resetOldSelection();
  NM::Message fire(const sf::Vector2f mousePos, bool &click);
  GameModel::CellType getLastCellSelected();
  void updateGUI();

};
