#pragma once

#include "clickable_shape.hh"
#include "../client_board.hh"
#include "../../common/board_common.hh"

using std::string;

class Cell {
 private:
  sf::Color color;
  sf::Color hoverColor;
  sf::Color pressedColor;
  std::string position = "";
  sf::Vector2i positionVec;
  ClickableShape cell;
  GameModel::CellType type = GameModel::CellType::WATER;
  
  std::shared_ptr<ClientView const>  model;
  std::shared_ptr<ClientControl>   _control;

 public:
  Cell(); 
  Cell(sf::Vector2f coords, sf::Vector2f size,sf::Vector2i pos, std::shared_ptr<ClientView const> model,  std::shared_ptr<ClientControl>  control, short unsigned int borderSize = 2);
  void updateGame(const sf::Vector2f mousePos , bool &click , bool your_board,BoardCoordinates old, GameModel::GameStage game_state);
  void update(const sf::Vector2f mousePos, bool &click){cell.update(mousePos, click);}
  void updateGUI(bool your_board, BoardCoordinates old, GameModel::GameStage game_state);
  void display(std::shared_ptr<sf::RenderWindow> window);
  bool isPressed();
  std::string getPosition(){return position;}
  void multUpdate(const sf::Vector2f mousePos, int & click, bool& lock, bool your_board,BoardCoordinates old, GameModel::GameStage game_state);
  void setColors();
  void setType(GameModel::CellType newType){type = newType; setColors(); cell.setColors(color,hoverColor,pressedColor);}
  GameModel::CellType getType(){return type;}

};
