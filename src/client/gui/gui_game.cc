#include "gui_game.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"  // Wontfix

NM::Message GUIGame::pollEvent() {
  sf::Event ev;
  updateMousePos();
  updateState();
  if (commanderMode)
    updateAbilityButton();
  NM::Message res{};
  if (focus)
    res = updateButtonPressed(index);

  while (window->pollEvent(ev)) {
    switch(ev.type) {
      case sf::Event::Closed:
        window->close();
        break;
      case sf::Event::KeyPressed:
        if (ev.key.code == sf::Keyboard::Escape) {
          window->close();
          break;
        }
      case sf::Event::MouseMoved: 
        break;
      case sf::Event::MouseButtonPressed:
      	focus = 1;
        break;
      case sf::Event::MouseButtonReleased:
        click = 0;
        break;
      case sf::Event::KeyReleased:
        if (ev.key.code == sf::Keyboard::Enter)
          boat_lock = 0;
        break;
      case sf::Event::LostFocus:
        focus = 0;
        break;
      case sf::Event::GainedFocus:
        focus = 1;
        break;
      default:
        break;
    }
  }
  return res;
}


void GUIGame::updateState() {
  _game_state = _board->gameState();
   

  switch (_game_state) {
    using enum GameModel::GameStage;
    case FACTIONSELECT:
    break;
    case SELECTION:
      _game_state = PLACEMENT;
      break;
    case PLACEMENT:
      break;
    case ATTACKSELECT:
    case COMBAT:
      selected_ability = BASIC;
      screenTitle.setString("Your Turn");
      screenTitle.setPosition({1500/2-screenTitle.getGlobalBounds().width/2, 50});
      break;
    case OTHERTURN:
      screenTitle.setString("Enemy Turn");
      screenTitle.setPosition({1500/2-screenTitle.getGlobalBounds().width/2, 50});
      break;
    default :
      break;
  }
}
void GUIGame::setCommander() { commanderMode = 1; }
bool GUIGame::getCommander() { return commanderMode; }

void GUIGame::display() {
  board_left.updateState(_game_state);
  board_right.updateState(_game_state);
  
  window->clear(sf::Color(139, 183, 240));
    
  if (commanderMode && _game_state != GameModel::GameStage::PLACEMENT && _game_state != GameModel::GameStage::FACTIONSELECT) {
    basicAbility.update(mousePosGame,click);
    ability1.update(mousePosGame,click);
    ability2.update(mousePosGame,click);
    
    basicAbility.display(window);
    ability1.display(window);
    ability2.display(window);
    displayEnergy();
  }

  if (_game_state == GameModel::GameStage::FACTIONSELECT)
      confirm.display(window);


  if (commanderMode && _game_state == GameModel::GameStage::FACTIONSELECT) {
    displayBackground("Choose a faction", {1500/2 - texture.getSize().x/2, 160}, {1500/2 - mainMenu.getGlobalBounds().width/2 , 50});
    displayFactions();
  }
  else
    displayObserver();

  if (_game_state == GameModel::GameStage::PLACEMENT) {
    displayInventory(index); 
    for(int i = 0; i < boatButtons.size(); i++)
      boatButtons[i].displayWithoutText(window);

      if (_control->isValidPlace())
        board_left.resetOldSelection();

      if (focus && boat_lock) //locks placement if no boat selected
        board_left.placeBoats(mousePosGame, click);
  }
  else if (_game_state == GameModel::GameStage::OTHERTURN) {
    board_left.updateGUI();
  }
  else if (_game_state == GameModel::GameStage::ATTACKSELECT || _game_state == GameModel::GameStage::COMBAT) {
    board_right.updateGUI();
  }
  else if (_board->isFinished()) {
    setUpEnd(); 
    displayEnd();
  }

  window->display();
}

void GUIGame::displayObserver() { //display just what can be seen by observers
    window->draw(leftBoard);
    window->draw(rightBoard);
    window->draw(screenTitle);
    board_left.display(window);
    board_right.display(window);

}


void GUIGame::updateMousePos() {
  mousePosScreen = sf::Mouse::getPosition();
  mousePosWindow = sf::Mouse::getPosition(*window);
  mousePosGame = window->mapPixelToCoords((sf::Mouse::getPosition(*window)));
}


void GUIGame::setUpEnd() {
  if (_board->victor() == GameModel::Victor::RIGHT) //CHECK THE BOARDS HERE TOO
    screenTitle.setString("Right Player Won!"); //change title instead
  else if (_board->victor() == GameModel::Victor::LEFT)
    screenTitle.setString("Left Player Won!");
  else
    screenTitle.setString("Tie!");
}

void GUIGame::displayEnd() {
  window->draw(screenTitle);
  
}


std::string GUIGame::getShipName(Boat::Type boat) {
  switch(boat) {
    case Boat::Type::Destroyer:
      return "Destroyer";
    case Boat::Type::Cruiser:
      return "Cruiser";
    case Boat::Type::Battleship:
      return "Battleship";
    case Boat::Type::Carrier:
      return "Carrier";
    case Boat::Type::Z_Tetromino:
      return "Z_Tetromino";
    case Boat::Type::J_Tetromino:
      return "J_Tetromino";
    case Boat::Type::T_Tetromino:
      return "T_Tetromino";
    case Boat::Type::SENTINEL:
      return "Sentinel";
    default:
      return "Unnamed";
  }
  return "";
}

std::string GUIGame::getShipNumber(Boat::Type boat) {
  switch(boat) {
    case Boat::Type::Destroyer:
      return "1";
    case Boat::Type::Cruiser:
      return "2";
    case Boat::Type::Battleship:
      return "3";
    case Boat::Type::Carrier:
      return "4";
    case Boat::Type::Z_Tetromino:
      return "5";
    case Boat::Type::J_Tetromino:
      return "6";
    case Boat::Type::T_Tetromino:
      return "7";
    case Boat::Type::SENTINEL:
      return "Sentinel";
    default:
      return "Unnamed";
  }
  return "";
}

void GUIGame::displayInventory(unsigned int &index) { 
  sf::Text boat;
  boat.setString("boat");
  boat.setFont(font);
  boat.setFillColor(sf::Color::Black);
  boat.setCharacterSize(20);
  window->draw(inventoryBox);
  while(board_left.getBoats().size()>boatButtons.size())
    boatButtons.push_back(Button{sf::Vector2f{300,675 +1*30-5}, sf::Vector2f{25,25}, &font, 20, "", sf::Color::Green , sf::Color::Blue, sf::Color::Red});
  
  while(board_left.getBoats().size()<boatButtons.size())
   boatButtons.erase(boatButtons.begin()+boatButtons.size()-1);

    for (size_t i = 0; i < board_left.getBoats().size(); i++) {
      boatButtons[i].setText(getShipNumber(board_left.getBoats()[i]));
      boatButtons[i].setPosition(sf::Vector2f{300,675 +(i+1)*30-5});
      boat.setString(getShipName(board_left.getBoats()[i]));
      boat.setPosition({125,675 +(i+1)*25});
      window->draw(boat);
    }
}

void GUIGame::displayEnergy() {
  energy.setSize(sf::Vector2f(70, 50));
  energy.setPosition(sf::Vector2f(750, 780));
  energy.setFillColor(sf::Color::Blue);
  for (int i = 0; i < _board->getEnergy(); i++) {
    energy.setPosition(sf::Vector2f(energy.getPosition().x+100, 780));
    window->draw(energy);
  }
}

std::string GUIGame::updateAbilityButton() { //should add energy checks
  if (basicAbility.isPressed()) {
    selected_ability = BASIC;
    return "1";
  }
  else if (ability1.isPressed()) {
    selected_ability = ABILITY1;
    return "2";
  }
  else if (ability2.isPressed()) {
    selected_ability = ABILITY2;
    return "3";
  }
  return "";
}

void GUIGame::setText(sf::Text &text, string message, sf::Font &font, unsigned int characterSize, sf::Color fillColor, sf::Vector2f position) {
  text.setString(message);
  text.setFont(font);
  text.setCharacterSize(characterSize);
  text.setFillColor(fillColor);
  text.setPosition(position);
}

NM::Message GUIGame::updateButtonPressed(unsigned int &index) {
  confirm.update(mousePosGame, click);

  if (_game_state == GameModel::GameStage::FACTIONSELECT && confirm.isPressed()) { 
    std::string faction = factionSelected();
    if (faction == "")
      return {};
    NM::Message message = _control->factionSelect(faction);
    return message;
  }

  if (_game_state==GameModel::GameStage::PLACEMENT) {

    for (int i = 0; i < boatButtons.size(); i++) {//selecting boat
      boatButtons[i].update(mousePosGame,click); //should check bcs of box position
      if (boatButtons[i].isPressed() && boatPlaced) {
        boatPlaced = 0;
        index = i;
        boat_lock = 1;
        NM::Message message = _control->select(boatButtons[i].getText());
        boatButtons.erase(boatButtons.begin()+i); // only if confirmed
        return message;
      }
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
      boatPlaced = 1;
      if (_control->isValidPlace())
        boat_lock = 0;
      return _control->confirm_gui(click);
    }
  }
  if (_game_state==GameModel::GameStage::ATTACKSELECT) {
    std::string abilitySelect;
    switch (selected_ability) {
      case BASIC:
        abilitySelect = "1";
        break;
      case ABILITY1:
        abilitySelect = "2";
        break;
      case ABILITY2:
        abilitySelect = "3";
        break;
    }
    return _control->selectAbility(abilitySelect);
  }

  if (_game_state==GameModel::GameStage::COMBAT && !click) {
    NM::Message ret = board_right.fire(mousePosGame, click); 


    return ret;
  }


  return {};
}

void GUIGame::displayFactions() {
  if (!fleetPirate.loadFromFile("imgs/fleet_pirate.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite pirate_f;
  pirate_f.setTexture(fleetPirate);
  pirate_f.setPosition({pirate.getSize().width/2-pirate_f.getLocalBounds().width/4+250, 700});

  if (!fleetCaptain.loadFromFile("imgs/fleet_captain.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite captain_f;
  captain_f.setTexture(fleetCaptain);
  captain_f.setPosition({captain.getSize().width/2-captain_f.getLocalBounds().width/4+850, 700});

  sf::Vector2f targetSizePirate(229.0f, 152.0f);
  sf::Vector2f targetSizeCaptain(213.0f, 207.0f);
  pirate_f.setScale(targetSizePirate.x/pirate_f.getLocalBounds().width, targetSizePirate.y/pirate_f.getLocalBounds().height);
  captain_f.setScale(targetSizeCaptain.x/captain_f.getLocalBounds().width, targetSizeCaptain.y/captain_f.getLocalBounds().height);
  
  setText(pirateTitre, "Pirate", fontBritanic, 40, sf::Color::Black, sf::Vector2f(450-pirateTitre.getGlobalBounds().width/2, 640));
  setText(pirateAbilities[0], "Ability 1: Straight line attack (horizontal ", font, 20, sf::Color::Black, sf::Vector2f(275, 890));
  setText(pirateAbilities[1], "or vertical).", font, 20, sf::Color::Black, sf::Vector2f(275, 920));
  setText(pirateAbilities[2], "Ability 2: + shape attack. ", font, 20, sf::Color::Black, sf::Vector2f(275, 950));
  pirate.update(mousePosGame, click);
  pirate.displayWithoutText(window);
  window->draw(pirateTitre);
  window->draw(pirateAbilities[0]);
  window->draw(pirateAbilities[1]);
  window->draw(pirateAbilities[2]);
  setText(captainTitre, "Captain", fontBritanic, 40, sf::Color::Black, sf::Vector2f(1050-pirateTitre.getGlobalBounds().width/2, 640));
  setText(captainAbilities[0], "Ability 1: Oblic attack.", font, 20, sf::Color::Black, sf::Vector2f(875, 920));
  setText(captainAbilities[1], "Abitity 2: X shape attack.", font, 20, sf::Color::Black, sf::Vector2f(875, 950));
  captain.update(mousePosGame, click);
  captain.displayWithoutText(window);
  setText(boat, "Fleet:", font, 20, sf::Color::Black, sf::Vector2f(275, 700));
  window->draw(boat);
  setText(boat, "Fleet:", font, 20, sf::Color::Black, sf::Vector2f(875, 700));
  window->draw(boat);
  window->draw(captainTitre);
  window->draw(captainAbilities[0]);
  window->draw(captainAbilities[1]);
  window->draw(pirate_f);
  window->draw(captain_f);
}

std::string GUIGame::factionSelected() { 
  if (captain.isSelected())
    return "2";
  else if (pirate.isSelected())
    return "1";
  else 
    return "";
}

void GUIGame::displayBackground(string title,sf::Vector2f boat_pos, sf::Vector2f title_pos) {
   
  if (!texture.loadFromFile("imgs/cute_boat.png"))
    std::cout<<"Failed to load the image"<<std::endl;

  sf::Sprite boat;
  boat.setTexture(texture);
  boat.setPosition(boat_pos);

  setText(mainMenu, title, fontBritanic, 80, sf::Color::Black, title_pos);
  mainMenu.setStyle(sf::Text::Bold);
  window->draw(boat);
  window->draw(mainMenu);
  confirm.setPosition(sf::Vector2f(400/2-200/2+1025, texture.getSize().y/2-90/2+160));
}

#pragma GCC diagnostic pop
