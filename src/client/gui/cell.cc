
#include "cell.hh"

Cell::Cell(sf::Vector2f coords, sf::Vector2f size, sf::Vector2i pos, std::shared_ptr<ClientView const> model,  std::shared_ptr<ClientControl>  control, short unsigned int borderSize): model(model), _control(control) {
  setColors();
  cell = {coords, size, color, hoverColor, pressedColor, 0, borderSize};
  std::string letters = "abcdefghij";
  position += letters[pos.x] + std::to_string(pos.y+1);
  positionVec = pos;
}

Cell::Cell() {
  cell = {sf::Vector2f(0,0), sf::Vector2f(50,50), sf::Color::Red, sf::Color::Red, sf::Color::Red,0,2};
  positionVec = sf::Vector2i(0,0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
void Cell::setColors() {  
  switch (type) {
    case GameModel::CellType::WATER: //need to check how to label other side cause "IS_SHIP is taken by "UNDAMAGED"
      color = {62, 126, 179};
      hoverColor = {115, 166, 209};
      pressedColor = {60, 94, 144};
      break;
    case GameModel::CellType::UNDAMAGED:
      color = {100, 100, 100};
      hoverColor = {150, 150, 150}; 
      pressedColor = {80, 80, 80};
      break;
    case GameModel::CellType::OCEAN: //hit water
      color = {58, 92, 142};
      hoverColor = {106, 140, 142};
      pressedColor = {34, 58, 142};
      break;
    case GameModel::CellType::HIT:
      color = {160, 100, 100};
      hoverColor = {210, 150, 150}; 
      pressedColor = {140, 80, 80};
      break;
    case GameModel::CellType::SUNK:
      color = {55, 69, 87};
      hoverColor = {86, 103, 125};
      pressedColor = {51, 63, 79};
      break;
    default:
      throw std::runtime_error("Invalid color for Cell");
  }
}
#pragma GCC diagnostic pop

void Cell::updateGUI(bool your_board, BoardCoordinates old, GameModel::GameStage game_state) {
  if ((game_state == GameModel::GameStage::PLACEMENT && your_board) || (game_state== GameModel::GameStage::OTHERTURN && your_board)) {
    setType(model->getCell(your_board, BoardCoordinates{position}));
  }
  
  if ((game_state == GameModel::GameStage::ATTACKSELECT || game_state == GameModel::GameStage::COMBAT) && !your_board)
    setType(model->getCell(your_board, BoardCoordinates{position}));

  switch (type) {
    case ClientView::OCEAN:
      cell.setUnclickable(); 
      break;
    case ClientView::HIT:
      cell.setUnclickable(); 
      break;
    default:
      break;
  }
}

void Cell::updateGame(const sf::Vector2f mousePos, bool &click, bool your_board, BoardCoordinates old, GameModel::GameStage game_state) {
  cell.update(mousePos, click);
  if (cell.isPressed()) {
     updateGUI(your_board,old, game_state);
  }
}

void Cell::multUpdate(const sf::Vector2f mousePos, int & click, bool& lock, bool your_board, BoardCoordinates old, GameModel::GameStage game_state) {
  cell.multUpdate(mousePos,click,lock);
    if (cell.isPressed()) {
      updateGUI(your_board,old,game_state);
    }
}

bool Cell::isPressed() {
  return cell.isPressed();
}

void Cell::display(std::shared_ptr<sf::RenderWindow> window){ 
  window->draw(cell.getShape());
}
