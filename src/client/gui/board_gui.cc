
#include "board_gui.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"  // Wontfix


Board::Board(sf::Vector2f coords, sf::Vector2f cellSize, sf::Font* font, std::shared_ptr<ClientView const> model, std::shared_ptr<ClientControl>  control, bool your_board, unsigned int size) : /*boats(boats),*/ font(font),_model(model), _control(control), your_board(your_board) {
  for (size_t x = 0; x < BOARDSIZE; x++) {
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y] = {sf::Vector2f(coords.x + (x + 1) * 50,coords.y + (y) * 50),cellSize,sf::Vector2i(x,y), model,control};  //modify position after
    }
  }
  std::string letters = "ABCDEFGHIJ";
  sf::Text text;
  for (int i = 0; i < 10; i++) {
    text.setString(letters[i]);
    text.setFont(*font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition({coords.x + cellSize.x + cellSize.x / 4 + (i) * 50,coords.y - 40});
    board_text.push_back(text);
    text.setString(std::to_string(i + 1));
    text.setPosition({coords.x + 20,coords.y + cellSize.y / 4 + (i) * 50});
    board_text.push_back(text);
  }
}

void Board::update(const sf::Vector2f mousePos, bool& click) {
  for (size_t x = 0; x < BOARDSIZE; x++)
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y].updateGame(mousePos, click,your_board,old_selection,game_state);
      old_selection = {x,y};
    }
}

void Board::display(std::shared_ptr<sf::RenderWindow> window) {
  for (size_t x = 0; x < BOARDSIZE; x++) {
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y].display(window);
    }
  }
  for (auto& m : board_text)
    window->draw(m);
}

void Board::placeBoats(const sf::Vector2f mousePos, bool& click) {
  static bool APressed = false;
  static bool EPressed = false;
  for (size_t x = 0; x < BOARDSIZE; x++) {
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y].update(mousePos, click);
      if (board[x][y].isPressed()) {
        board[x][y].updateGUI(your_board,old_selection, game_state);
        _control->move_gui({x-old_selection.x(),y-old_selection.y()});
        old_selection = {x,y};
      }
      board[x][y].updateGUI(your_board,old_selection, game_state); //needed for diplaying the boats
    }
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && !APressed) {
    _control->rotate_gui(false);
    APressed = true;
  }
  else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
    APressed = false;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !EPressed) {
      _control->rotate_gui(true);
      EPressed = true;
  }
  else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
    EPressed = false;
  }
}


void Board::multUpdate(sf::Vector2f pos, int& click, bool& lock) {  //isn't use
  for (size_t x = 0; x < BOARDSIZE; x++)
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y].multUpdate(pos, click, lock, your_board, old_selection, game_state);
      old_selection = {x,y};
    }
}

void Board::updateState(GameModel::GameStage state) {
  game_state = state;
}

vector<Boat::Type> Board::getBoats() const {
  return _model->getInventory();
}

void Board::resetOldSelection() {
  old_selection = _control->getFirstCoordinate();
}

NM::Message Board::fire(const sf::Vector2f mousePos, bool &click){ // doesn't work
  for (size_t x = 0; x < BOARDSIZE; x++)
    for (size_t y = 0; y < BOARDSIZE; y++) {
      board[x][y].update(mousePos, click);
      if (board[x][y].isPressed()) {
        lastCellSelected = board[x][y].getType();
        board[x][y].updateGUI(your_board,old_selection, game_state); 
        _control->setSelectionGUI({{x,y}}); 
        return _control->fireGUI();
      }
      board[x][y].updateGUI(your_board,old_selection, game_state); 
    }
  return {};
}

GameModel::CellType Board::getLastCellSelected() {
  return lastCellSelected;
}

void Board::updateGUI() {
  for (size_t x = 0; x < BOARDSIZE; x++)
    for (size_t y = 0; y < BOARDSIZE; y++)
      board[x][y].updateGUI(your_board,old_selection, game_state);  
}

#pragma GCC diagnostic pop
