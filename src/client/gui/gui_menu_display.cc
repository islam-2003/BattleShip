#include "gui_menu_display.hh"
#include "../client.hh"
#include "../../common/network_io.hh"
#include "../../common/utils.hh"
#include "../../common/serializer.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"


GUIMenuDisplay::GUIMenuDisplay(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<MenuView const>  view, std::shared_ptr<MenuControl> control,
                std::shared_ptr<LobbyView const> lobby, observer_ptr<SessionInfo const> session)
                : MenuDisplay{std::move(view), std::move(control), std::move(lobby), session}, window(window) {
  if (!fontBritanic.loadFromFile("fonts/BRITANIC.TTF")) {
    std::cout<<"can't load britanic font"<<std::endl;
  }
  if (!fontTIMES.loadFromFile("fonts/TIMES.TTF")) {
    std::cout<<"can't load times font"<<std::endl;
  }
  commander.addNeighbor(&classic);
  classic.addNeighbor(&commander);
        
  playerTimer.addNeighbor(&gameTimer);
  gameTimer.addNeighbor(&playerTimer);

  setupShape();
}

NM::Message GUIMenuDisplay::pollEvent() {
  updateScreenState();
  updateMousePos();
  NM::Message res {} ;
  if (focus)
    res = updateButtonPressed();
  sf::Event event;
  while(window->pollEvent(event)) {
    switch(event.type) {
      case sf::Event::Closed:
        window->close();
        break;

      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape) {
          window->close();
        }
        if (screenState == CHAT && event.key.code == sf::Keyboard::Enter) {
          res = control->formatChat(messageText.getText());
          messageText.clear();
        }
        if (screenState == JOIN && event.key.code == sf::Keyboard::Enter) {
          res = control->formatBrowser("/j "+join.getText()+" "+joinPW.getText());
          join.clear();
          joinPW.clear();
        }
        if (screenState == LOBBY && event.key.code == sf::Keyboard::Enter) {
          res = control->formatLobby("/i "+inviteFriend.getText());
          inviteFriend.clear();
        }
        break;
      case sf::Event::LostFocus:
        focus = 0;
        break;
      case sf::Event::GainedFocus:
        focus = 1;
        break;

      case sf::Event::MouseButtonPressed:
        focus = 1;
        if (userRectangle.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          userText.setSelected(true);
          PWText.setSelected(false);
        }
        if (PWRectangle.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          PWText.setSelected(true);
          userText.setSelected(false);
        }
        if (messageRectangle.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          messageText.setSelected(true);
        }
        if (addFriendRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          addFriend.setSelected(true);
        }
        if (lobbyNameRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          lobbyName.setSelected(true);
          lobbyPW.setSelected(false);
        }
        if (lobbyPWRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          lobbyPW.setSelected(true);
          lobbyName.setSelected(false);
        }
        if (joinRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          joinPW.setSelected(false);
          join.setSelected(true);
        }
        if (joinPWRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          join.setSelected(false);
          joinPW.setSelected(true);
        }
        if (roundTimeRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          roundTimeText.setSelected(true);
          gameTimeText.setSelected(false);
          playerTimeText.setSelected(false);
          inviteFriend.setSelected(false);
        }
        if (gameTimeRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          gameTimeText.setSelected(true);
          roundTimeText.setSelected(false);
          playerTimeText.setSelected(false);
          inviteFriend.setSelected(false);
        }
        if (playerTimeRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          playerTimeText.setSelected(true);
          gameTimeText.setSelected(false);
          roundTimeText.setSelected(false);
          inviteFriend.setSelected(false);
        }
        if (inviteFriendRect.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
          inviteFriend.setSelected(true);
          gameTimeText.setSelected(false);
          roundTimeText.setSelected(false);
          playerTimeText.setSelected(false);
        }
        break;

      case sf::Event::MouseButtonReleased:
        click = 0;
        break;

      case sf::Event::TextEntered:
        if (userText.isSelected) {
          userText.typedOn(event);
          if (userText.getGlobalBounds().width > userRectangle.getSize().x) {  //scroll
            float offsetX = userText.getGlobalBounds().width - userRectangle.getSize().x;
            userText.setPosition({userRectangle.getPosition().x - offsetX, userText.getPosition().y});
          }
          else
            userText.setPosition({505, userText.getPosition().y});
        }
        
        if (PWText.isSelected) {
          PWText.typedOn(event);
          if (PWText.getGlobalBounds().width > PWRectangle.getSize().x) {
            float offsetX = PWText.getGlobalBounds().width - PWRectangle.getSize().x;
            PWText.setPosition({PWRectangle.getPosition().x - offsetX, PWText.getPosition().y});
          }
          else
            PWText.setPosition({505, PWText.getPosition().y});
        }

        if (messageText.isSelected) {
          messageText.typedOn(event);
          if (messageText.getGlobalBounds().width > messageRectangle.getSize().x) {
            float offsetX = messageText.getGlobalBounds().width - messageRectangle.getSize().x;
            messageText.setPosition({messageRectangle.getPosition().x - offsetX, messageText.getPosition().y});
          }
          else
            messageText.setPosition({50, messageText.getPosition().y});
        }

        if (addFriend.isSelected) {
          addFriend.typedOn(event);
        }

        if (join.isSelected) {
          join.typedOn(event);
          if (join.getGlobalBounds().width > joinRect.getSize().x) {
            float offsetX = join.getGlobalBounds().width - joinRect.getSize().x;
            join.setPosition({joinRect.getPosition().x - offsetX, join.getPosition().y});
          }
          else
            join.setPosition({1005, join.getPosition().y});
        }

        if (joinPW.isSelected) {
          joinPW.typedOn(event);
          if (joinPW.getGlobalBounds().width > joinPWRect.getSize().x) {
            float offsetX = joinPW.getGlobalBounds().width - joinPWRect.getSize().x;
            joinPW.setPosition({joinPWRect.getPosition().x - offsetX, joinPW.getPosition().y});
          }
          else
            joinPW.setPosition({1005, joinPW.getPosition().y});
        }

        if (lobbyName.isSelected) {
          lobbyName.typedOn(event);
          //scroll
          if (lobbyName.getGlobalBounds().width > lobbyNameRect.getSize().x) {
            float offsetX = lobbyName.getGlobalBounds().width - lobbyNameRect.getSize().x;
            lobbyName.setPosition({lobbyNameRect.getPosition().x - offsetX, lobbyName.getPosition().y});
          }
          else
            lobbyName.setPosition({505, lobbyName.getPosition().y});
        }

        if (lobbyPW.isSelected) {
          lobbyPW.typedOn(event);
          if (lobbyPW.getGlobalBounds().width > lobbyPWRect.getSize().x) {
            float offsetX = lobbyPW.getGlobalBounds().width - lobbyPWRect.getSize().x;
            lobbyPW.setPosition({lobbyPWRect.getPosition().x - offsetX, lobbyPW.getPosition().y});
          }
          else
            lobbyPW.setPosition({505, lobbyPW.getPosition().y});
        }

        if (roundTimeText.isSelected) {
          roundTimeText.typedOn(event);
          if (roundTimeText.getGlobalBounds().width > roundTimeRect.getSize().x) {
            float offsetX = roundTimeText.getGlobalBounds().width - roundTimeRect.getSize().x;
            roundTimeText.setPosition({roundTimeRect.getPosition().x - offsetX, roundTimeText.getPosition().y});
          }
          else
            roundTimeText.setPosition({370, roundTimeText.getPosition().y});
        }

        if (gameTimeText.isSelected) {
          gameTimeText.typedOn(event);
          if (gameTimeText.getGlobalBounds().width > gameTimeRect.getSize().x) {
            float offsetX = gameTimeText.getGlobalBounds().width - gameTimeRect.getSize().x;
            gameTimeText.setPosition({gameTimeRect.getPosition().x - offsetX, gameTimeText.getPosition().y});
          }
          else
            gameTimeText.setPosition({370, gameTimeText.getPosition().y});
        }

        if (playerTimeText.isSelected) {
          playerTimeText.typedOn(event);
          if (playerTimeText.getGlobalBounds().width > playerTimeRect.getSize().x) {
            float offsetX = playerTimeText.getGlobalBounds().width - playerTimeRect.getSize().x;
            playerTimeText.setPosition({playerTimeRect.getPosition().x - offsetX, playerTimeText.getPosition().y});
          }
          else
            playerTimeText.setPosition({370, playerTimeText.getPosition().y});
        }

        if (inviteFriend.isSelected) {
          inviteFriend.typedOn(event);
          if (inviteFriend.getGlobalBounds().width > inviteFriendRect.getSize().x) {
            float offsetX = inviteFriend.getGlobalBounds().width - inviteFriendRect.getSize().x;
            inviteFriend.setPosition({inviteFriendRect.getPosition().x - offsetX, inviteFriend.getPosition().y});
          }
          else
            inviteFriend.setPosition({708, inviteFriend.getPosition().y});
        }

        break;

      default:
        break;
    }
  }

  return res;
}
#pragma GCC diagnostic pop

void GUIMenuDisplay::updateScreenState() {
  switch(menu->currentState()) {
    case MenuView::MenuState::LOGIN:
      if (screenState != REGISTER_SCREEN && screenState != LOGIN)
        screenState = START;
      break;
    case MenuView::MenuState::MAIN:
      if (screenState != SENT && screenState != RECEIVED && screenState != INVITE)
        screenState = MAINMENU;
      break;
    case MenuView::MenuState::BROWSER:
      if (screenState != JOIN && screenState != CREATE)
        screenState = BROWSER;
      break;
    case MenuView::MenuState::LOBBY:
      if (screenState != SENT && screenState != RECEIVED)
        screenState = LOBBY;
      break;
    case MenuView::MenuState::CHAT:
      screenState = CHAT;
    default:
      break;
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"  // Wontfix
void GUIMenuDisplay::display() {
  sf::Color menuColor(139, 183, 240);

  window->clear(menuColor);
  switch(screenState) {
    case START:
      displayBackground("BATTLESHIP", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      displayStart();
      break;
    case LOGIN:
    case REGISTER_SCREEN:
      if (screenState == LOGIN)
        displayBackground("Login Screen", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      else if (screenState == REGISTER_SCREEN)
        displayBackground("Register Screen", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      displayLogin();
      break;
    case CHAT:
      displayChat();
      break;
    case MAINMENU:
      displayBackground("Main Menu", {1000/2 - texture.getSize().x/2, 160}, {1000/2 - mainMenu.getGlobalBounds().width/2, 50});
      displayMainMenu();
      displayFriendList();
      break;
    case SENT:
      displaySent();
      displayBackground("Add Friends", {1000/2 - texture.getSize().x/2, 160}, {1000/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    case RECEIVED:
      displayRequest();
      displayBackground("Friend(s) Request(s)", {1000/2 - texture.getSize().x/2, 160}, {1000/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    case LOBBY:
      if (lobby->kind() == LobbyView::Kind::HOST) {
        displayLobbyHost();
        displayBackground("Lobby", {1000/2 - texture.getSize().x/2, 160}, {1000/2 - mainMenu.getGlobalBounds().width/2, 50});
      }
      else if (lobby->kind() == LobbyView::Kind::OTHER) {
        displayLobbySlot();
        displayBackground("Lobby", {1000/2 - texture.getSize().x/2, 160}, {1000/2 - mainMenu.getGlobalBounds().width/2, 50});
      }
      break;
    case BROWSER:
      displayBrowser();
      displayBackground("Game Browser", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    case JOIN:
      displayJoin();
      displayBackground("Join a Game",{windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    case CREATE:
      displayLobbyName();
      displayBackground("Game creation", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    case INVITE:
      displayGameInvite();
      displayBackground("Game(s) invitation(s)", {windowWidth/2 - texture.getSize().x/2, 160}, {windowWidth/2 - mainMenu.getGlobalBounds().width/2, 50});
      break;
    default:
      break;
  }
  window->display();
}

void GUIMenuDisplay::updateMousePos() {
  mousePosScreen = sf::Mouse::getPosition();
  mousePosWindow = sf::Mouse::getPosition(*window);
  mousePosGame   = window->mapPixelToCoords((sf::Mouse::getPosition(*window)));
}

NM::Message GUIMenuDisplay::updateButtonPressed() {
  if (screenState == START) {
    messageText.clear();
    addFriend.clear();
    if (login.isPressed()) {
      screenState = LOGIN;
      registerlimit++;
      sf::Mouse::setPosition(sf::Vector2i(windowWidth/2, windowHeight/2));
    }
    else if (regist.isPressed()) {
      screenState = REGISTER_SCREEN;
      registerlimit++;
      sf::Mouse::setPosition(sf::Vector2i(windowWidth/2, windowHeight/2));
    }
    else if (mainmenu.isPressed()) {
      screenState = MAINMENU;
      sf::Mouse::setPosition(sf::Vector2i(windowWidth/2, windowHeight/2));
    }
  }

  else if (screenState == LOGIN) {
    if (confirm.isPressed() && !std::all_of(userText.getText().begin(), userText.getText().end(),isspace)) {
      NM::Message ret = control->formatAuth("login "+userText.getText()+" "+PWText.getText());
      userText.clear();
      PWText.clear();
      return ret;
    }
  }

  else if (screenState == REGISTER_SCREEN) {
    if (confirm.isPressed() && !std::all_of(userText.getText().begin(), userText.getText().end(),isspace)) {
      NM::Message ret = control->formatAuth("register "+userText.getText()+" "+PWText.getText());
      userText.clear();
      PWText.clear();
      return ret;
    }
  }

  else if (screenState == MAINMENU) {
    userText.clear();
    PWText.clear();
    messageText.clear();
    addFriend.clear();
    if (gameBrowser.isPressed()) {
      return control->formatMain("1");
    }
    else if (logout.isPressed()) { //else if only same screenState buttons
      registerlimit = 0;
      sf::Mouse::setPosition(sf::Vector2i(windowWidth/2, windowHeight/2));
      return control->formatMain("3");
    }
    else if(gameInvite.isPressed()) {
      screenState = INVITE;
    }
  }

  else if (screenState == SENT) {
    if (confirm.isPressed() && !std::all_of(addFriend.getText().begin(), addFriend.getText().end(),isspace)) {
      NM::Message ret = control->formatFriends("1 "+ addFriend.getText());
      addFriend.clear();
      return ret;
    }
  }

  else if (screenState == BROWSER) {
    join.clear();
    joinPW.clear();
    registerlimit = 0;
    if (quit.isPressed())
      return control->formatBrowser("/q");
    else if (createGame.isPressed()) {
      screenState = CREATE;
      registerlimit++;
      lobbyName.clear();
      lobbyPW.clear();
    }
    else if (refresh.isPressed())
      return control->formatBrowser("/r");
  }

  else if (screenState == JOIN) {
    if (refresh.isPressed())
      return control->formatBrowser("/r");
  }

  else if (screenState == LOBBY) {
    addFriend.clear();
    if (quitLobby.isPressed())
      return control->formatLobby("/q");
    
    else if (joinLeft.isPressed())
      return control->formatLobby("/s left");

    else if (joinRight.isPressed())
      return control->formatLobby("/s right");

    else if (spec.isPressed())
      return control->formatLobby("/s spec");
    
    else if (commander.hasSelectionChanged() && commander.isSelected()) {
      commander.changeSelection();
      return control->formatLobby("/g 2");
    }
    else if (classic.hasSelectionChanged() && classic.isSelected()){
      classic.changeSelection();
      return control->formatLobby("/g 1");
    }
      
    else if (roundTimer.isPressed() && launchGame.isPressed())
      return control->formatLobby("/t 2 "+roundTimeText.getText());

    else if (gameTimer.isSelected() && launchGame.isPressed())
      return control->formatLobby("/t 1 "+gameTimeText.getText());

    else if (launchGame.isPressed()) {
      return control->formatLobby("/c");
    }
  }

  else if (screenState == CREATE) {
    inviteFriend.clear();
    roundTimeText.clear();
    gameTimeText.clear();
    playerTimeText.clear();

    if (confirm.isPressed() && !lobbyName.getText().empty() && !std::all_of(lobbyName.getText().begin(), lobbyName.getText().end(), isspace) && !std::all_of(lobbyPW.getText().begin(), lobbyPW.getText().end(),isspace)) {
      return control->formatBrowser("/c "+lobbyName.getText()+" "+lobbyPW.getText());
    }
    else if (quit.isPressed()) {
      screenState = BROWSER;
      registerlimit = 0;
    }
  }

  if (registerlimit == 1) { 
    if (screenState == CREATE) {
      lobbyName.setSelected(true);
      lobbyPW.setSelected(false);
      registerlimit = 2;
    }

    else if (screenState == JOIN) {
      join.setSelected(true);
      joinPW.setSelected(false);
      registerlimit = 2;
    }

    else {
      userText.setSelected(true);
      PWText.setSelected(false);
      registerlimit = 2;
    }
  }

// Buttons that only exist in GUI mode, have no equivalent in console mode (don't need to use controller or view)
  if (conversation.isPressed())                         screenState = CHAT;
  if (quit.isPressed())                                 screenState = MAINMENU;  // quit button is used in SENT and RECEIVED screens
  if (quitLogin.isPressed())                            screenState = START;    
  if (requestsreceived.isPressed())                     screenState = RECEIVED;
  if (requestsSent.isPressed())                         screenState = SENT;
  registerlimit = 0;
  if (joinGame.isPressed()) {                            
    screenState = JOIN;
    registerlimit++;
    join.clear();
  }

  if (screenState == JOIN && quitLobby.isPressed()) {
    screenState = BROWSER;
    join.clear();
  }

  if (quitChat.isPressed()) {
    NM::Message ret = control->formatChat("/q");
    screenState = MAINMENU;
    return ret;
  }

  for (int i = 0; i < acceptRequest.size(); i++) {
    if (acceptRequest[i].isPressed()) {
      NM::Message ret = control->formatFriends("2 "+acceptRequest[i].getText());
      acceptRequest.erase(acceptRequest.begin()+i);
      declineRequest.erase(declineRequest.begin()+i);
      return ret;
    }
  }

  for (int i = 0; i < declineRequest.size(); i++) {
    if (declineRequest[i].isPressed()) {
      NM::Message ret = control->formatFriends("3 "+declineRequest[i].getText());
      acceptRequest.erase(acceptRequest.begin()+i);
      declineRequest.erase(declineRequest.begin()+i);
      return ret;
    }
  }

  for (int i = 0; i < removeFriend.size(); i++) {
    if (removeFriend[i].isPressed()) {
      NM::Message ret = control->formatFriends("4 "+removeFriend[i].getText());
      removeFriend.erase(removeFriend.begin()+i);
      chatButton.erase(chatButton.begin()+i);
      return ret;
    }
  }
  
  for (int i = 0; i < chatButton.size(); i++) {
    if (chatButton[i].isPressed()) {
      NM::Message ret = control->formatFriends("5 "+chatButton[i].getText());
      return ret;
    }
  }

  for (int i = 0; i < acceptGameInvite.size(); i++) {
    if (acceptGameInvite[i].isPressed()) {
      NM::Message ret = control->formatFriends("6 "+acceptGameInvite[i].getText());
      acceptGameInvite.erase(acceptGameInvite.begin()+i);
      return ret;
    }
  }
  return {};
}

void GUIMenuDisplay::setupShape() {
  setText(loginMenu[0], username, fontBritanic, 50, sf::Color::Black, sf::Vector2f(500, 585));
  setText(loginMenu[1], password, fontBritanic, 50, sf::Color::Black, sf::Vector2f(500, 785));

  setRectangle(userRectangle, sf::Vector2f(500, 60), sf::Vector2f(500, 650), sf::Color::White, sf::Color::Black, 2);
  setTexbox(userText, sf::Vector2f(505, 647), fontTIMES, true, 12);

  setRectangle(PWRectangle, sf::Vector2f(500, 60), sf::Vector2f(500, 850), sf::Color::White, sf::Color::Black, 2);
  setTexbox(PWText, sf::Vector2f(505, 847), fontTIMES);

  setRectangle(messageRectangle, sf::Vector2f(1300,40), sf::Vector2f(45, 905), sf::Color::White, sf::Color::Black, 2);
  setTexbox(messageText, sf::Vector2f(50, 902), fontTIMES, true, 60);

  //hiding the text that goes out of bounds
  setRectangle(hideUser,       sf::Vector2f(498, 60),   sf::Vector2f(0, 650),  sf::Color(139, 183, 240));
  setRectangle(hidePW,         sf::Vector2f(498, 60),   sf::Vector2f(0, 850),  sf::Color(139, 183, 240));
  setRectangle(hideChat,       sf::Vector2f(43, 40),    sf::Vector2f(0, 905),  sf::Color(139, 183, 240));
  setRectangle(hideChatBehind, sf::Vector2f(1400, 105), sf::Vector2f(0, 895),  sf::Color(139, 183, 240));
  setRectangle(hideFReq,       sf::Vector2f(998, 605),  sf::Vector2f(0, 0),    sf::Color(139, 183, 240));
  setRectangle(hideFSentTop,   sf::Vector2f(998, 638),  sf::Vector2f(0, 0),    sf::Color(139, 183, 240));
  setRectangle(hideFSentBot,   sf::Vector2f(998, 215),  sf::Vector2f(0, 785),  sf::Color(139, 183, 240));
  setRectangle(hideObsTop,     sf::Vector2f(290, 200),  sf::Vector2f(50, 0),   sf::Color(139, 183, 240));
  setRectangle(hideObsBot,     sf::Vector2f(340, 530),  sf::Vector2f(0, 470),  sf::Color(139, 183, 240));
  setRectangle(hideGameName,   sf::Vector2f(998, 60),   sf::Vector2f(0, 250),  sf::Color(139, 183, 240));
  setRectangle(hideGamePW,     sf::Vector2f(998, 60),   sf::Vector2f(0, 450),  sf::Color(139, 183, 240));
  setRectangle(hideGamesTop,   sf::Vector2f(1450, 612), sf::Vector2f(50, 0),   sf::Color(139, 183, 240));
  setRectangle(hideGamesBot,   sf::Vector2f(1450, 70),  sf::Vector2f(50, 930), sf::Color(139, 183, 240));
  setRectangle(hideTime,       sf::Vector2f(363, 233),  sf::Vector2f(0, 767),  sf::Color(139, 183, 240));
  setRectangle(hideInvite,     sf::Vector2f(701, 40),   sf::Vector2f(0, 180),  sf::Color(139, 183, 240));
  setRectangle(hideLobbyName,  sf::Vector2f(498, 60),   sf::Vector2f(0, 670),  sf::Color(139, 183, 240));
  setRectangle(hideLobbyPW,    sf::Vector2f(498, 60),   sf::Vector2f(0, 870),  sf::Color(139, 183, 240));
  setRectangle(hideGameInvite, sf::Vector2f(1500, 605), sf::Vector2f(0, 0),    sf::Color(139, 183, 240));

  setRectangle(friendListRect, sf::Vector2f(500,1000), sf::Vector2f(1000,0),   sf::Color (114, 172, 247), sf::Color::Black, 2);
  setRectangle(addFriendRect,  sf::Vector2f(500,60),   sf::Vector2f(250, 850), sf::Color::White,          sf::Color::Black, 2);
  setTexbox(addFriend, sf::Vector2f(255, 847), fontTIMES, true, 12);

  setRectangle(browserRect, sf::Vector2f(1400, 60), sf::Vector2f(50, 610), sf::Color(67, 142, 240));
  setText(browserText[0], "ID",       fontBritanic, 40, sf::Color::Black, sf::Vector2f(70, 615));
  setText(browserText[1], "Name",     fontBritanic, 40, sf::Color::Black, sf::Vector2f(200, 615));
  setText(browserText[2], "Players",  fontBritanic, 40, sf::Color::Black, sf::Vector2f(800, 615));
  setText(browserText[3], "Started",  fontBritanic, 40, sf::Color::Black, sf::Vector2f(1000, 615));
  setText(browserText[4], "Password", fontBritanic, 40, sf::Color::Black, sf::Vector2f(1200, 615));

  setRectangle(joinRect, sf::Vector2f(480, 60), sf::Vector2f(1000, 250), sf::Color::White, sf::Color::Black, 2);
  setTexbox(join, sf::Vector2f(1005, 247), fontTIMES, true, 30);

  setRectangle(joinPWRect, sf::Vector2f(480, 60), sf::Vector2f(1000, 450), sf::Color::White, sf::Color::Black, 2);
  setTexbox(joinPW, sf::Vector2f(1005, 447), fontTIMES);

  setRectangle(lobbyNameRect, sf::Vector2f(500, 60), sf::Vector2f(500, 670), sf::Color::White, sf::Color::Black, 2);
  setTexbox(lobbyName, sf::Vector2f(505, 667), fontTIMES, true, 30);
  setText(chooseName, "Choose a name (max 30 characters)", fontBritanic, 50, sf::Color::Black, sf::Vector2f(500, 605));

  setRectangle(lobbyPWRect, sf::Vector2f(500, 60), sf::Vector2f(500, 870), sf::Color::White, sf::Color::Black, 2);
  setTexbox(lobbyPW, sf::Vector2f(505, 867), fontTIMES);
  setText(choosePW, "Choose a password (optional)",  fontBritanic, 50, sf::Color::Black, sf::Vector2f(500, 805));

  setRectangle(roundTimeRect, sf::Vector2f(100, 30), sf::Vector2f(365, 910), sf::Color::White, sf::Color::Black, 2);
  setTexbox(roundTimeText, sf::Vector2f(370, 907), fontTIMES);

  setRectangle(gameTimeRect, sf::Vector2f(100, 30), sf::Vector2f(365, 770), sf::Color::White, sf::Color::Black, 2);
  setTexbox(gameTimeText, sf::Vector2f(370, 767), fontTIMES);

  setRectangle(playerTimeRect, sf::Vector2f(100, 30), sf::Vector2f(365, 840), sf::Color::White, sf::Color::Black, 2);
  setTexbox(playerTimeText, sf::Vector2f(370, 837), fontTIMES);

  setRectangle(inviteFriendRect, sf::Vector2f(250, 40), sf::Vector2f(703, 180), sf::Color::White, sf::Color::Black, 2);
  setTexbox(inviteFriend, sf::Vector2f(708, 174), fontTIMES, true, 12);
  setText(inviteFriendText, "Invite a friend", fontBritanic, 30, sf::Color::Black, sf::Vector2f(703, 140));
}

void GUIMenuDisplay::displayStart() { 
  login.update(mousePosGame,click);
  login.display(window);
  regist.update(mousePosGame,click);
  regist.display(window);
}

void GUIMenuDisplay::displayBackground(string title,sf::Vector2f boat_pos, sf::Vector2f title_pos) {
  if (!texture.loadFromFile("imgs/cute_boat.png"))
    std::cout<<"Failed to load the image"<<std::endl;

  sf::Sprite boat;
  boat.setTexture(texture);
  boat.setPosition(boat_pos);

  setText(mainMenu, title, fontBritanic, 80, sf::Color::Black, title_pos);
  mainMenu.setStyle(sf::Text::Bold);
  window->draw(boat);
  window->draw(mainMenu);
}

void GUIMenuDisplay::displayLogin() {
  window->draw(userRectangle);
  userText.drawTo(*window);
  window->draw(PWRectangle);
  PWText.drawTo(*window);
  confirm.setPosition(sf::Vector2f(1125, 340));
  confirm.update(mousePosGame,click);
  confirm.display(window);
  window->draw(hidePW);
  window->draw(hideUser);
  
  for (int i = 0; i < 2; i++)
    window->draw(loginMenu[i]);
  quitLogin.update(mousePosGame, click);
  quitLogin.display(window);
}

void GUIMenuDisplay::displayMainMenu() {
  gameBrowser.update(mousePosGame,click);
  gameBrowser.display(window);
  gameInvite.update(mousePosGame, click);
  gameInvite.display(window);
  logout.update(mousePosGame,click);
  logout.display(window);
}

void GUIMenuDisplay::displayFriendList() {
  int i = 185;
 
  window->draw(friendListRect);
  scrollbarFriendList.display(window);
  sf::Text friends;
  float posLast = 900;
  int count = 1;
  int listSize = session->getFriends().size();

  if (!bubbleChat.loadFromFile("imgs/chat.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite chat;
  chat.setTexture(bubbleChat);
  bubbleChat.setSmooth(true);
  sf::Vector2f targetSize(30.0f, 30.0f);

  if (!noButton.loadFromFile("imgs/red_cross.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite no;
  no.setTexture(noButton);

  while (listSize > removeFriend.size()) {
    removeFriend.push_back({sf::Vector2f(1010, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)), sf::Vector2f(30, 30), &fontBritanic, 15, "", sf::Color::Transparent, sf::Color::Transparent, sf::Color::Transparent});
    chatButton.push_back({sf::Vector2f(1400, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)), sf::Vector2f(30, 30), &fontBritanic, 15, "", sf::Color::Transparent, sf::Color::Transparent, sf::Color::Transparent});
  }

  int iter = 0;
  for (auto& f:session->getFriends()) {
    setText(friends, f, fontTIMES, 30, sf::Color::Black, sf::Vector2f(1050, i - posLast/900*(scrollbarFriendList.getPosThumb().y - scrollbarFriendList.getPosTrack().y)*(listSize/20)));
    window->draw(friends);
    chat.setPosition({1400, i+4 - posLast/900*(scrollbarFriendList.getPosThumb().y - scrollbarFriendList.getPosTrack().y)*(listSize/20)});
    chat.setScale(targetSize.x/chat.getLocalBounds().width, targetSize.y/chat.getLocalBounds().height);
    no.setPosition({1010, i+5- posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)});
    no.setScale(targetSize.x/no.getLocalBounds().width, targetSize.y/no.getLocalBounds().height);

    if (removeFriend.size() > 0) {
      removeFriend[iter].setText(f);
      removeFriend[iter].setPosition(sf::Vector2f(1010, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)));
      removeFriend[iter].update(mousePosGame,click);
      removeFriend[iter].displayWithoutText(window);
      chatButton[iter].setText(f);
      chatButton[iter].setPosition(sf::Vector2f(1400, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)));
      chatButton[iter].update(mousePosGame, click);
      chatButton[iter].displayWithoutText(window);
    }
    window->draw(no);
    window->draw(chat);
    i += 35;
    if (count == listSize) {
      posLast = friends.getPosition().y + friends.getGlobalBounds().height;
    }
    count++;
    iter++;
  }
  scrollbarFriendList.update(mousePosGame,click, posLast);
  requestsreceived.update(mousePosGame,click);
  requestsreceived.display(window);
  requestsSent.update(mousePosGame,click);
  requestsSent.display(window);
}

void GUIMenuDisplay::displayRequest() {
  scrollbarFRequest.display(window);
  displayFriendList();
  int i = 605;
  sf::Text friends;
  float posLast = 900;
  int count = 1;
  int listSize = session->getInbound().size();
  setText(EnterUsername, "Enter a username", fontBritanic, 50, sf::Color::Black, sf::Vector2f(250, 785));
  if (!yesButton.loadFromFile("imgs/green_tick.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite yes;
  yes.setTexture(yesButton);
  
  if (!noButton.loadFromFile("imgs/red_cross.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite no;
  no.setTexture(noButton);

  sf::Vector2f targetSize(30.0f, 30.0f);

  while (listSize > acceptRequest.size()) { 
    acceptRequest.push_back({sf::Vector2f(586, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)),sf::Vector2f(30, 30), &fontBritanic, 15, "", sf::Color(53, 219, 70), sf::Color(61, 252, 80), sf::Color(100,100,100)});
    declineRequest.push_back({sf::Vector2f(621, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)),sf::Vector2f(30, 30), &fontBritanic, 15, "", sf::Color(204, 45, 45), sf::Color(204, 59, 59), sf::Color(100,100,100)});
  }
 
  int iter = 0;
  for (auto &f:session->getInbound()) {
    setText(friends, f, fontTIMES, 30, sf::Color::Black, sf::Vector2f(200, i- posLast/900*(scrollbarFRequest.getPosThumb().y- scrollbarFRequest.getPosTrack().y)*(listSize/10)));
    window->draw(friends);
    yes.setPosition({586, i+5- posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)});
    no.setPosition({621, i+5- posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)});
    yes.setScale(targetSize.x/yes.getLocalBounds().width, targetSize.y/yes.getLocalBounds().height);
    no.setScale(targetSize.x/no.getLocalBounds().width, targetSize.y/no.getLocalBounds().height);

    if (acceptRequest.size() > 0) {
      acceptRequest[iter].setText(f);
      declineRequest[iter].setText(f);
      acceptRequest[iter].setPosition(sf::Vector2f(586, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)));
      declineRequest[iter].setPosition(sf::Vector2f(621, i+6 - posLast/900*(scrollbarFRequest.getPosThumb().y - scrollbarFRequest.getPosTrack().y)*(listSize/10)));
      acceptRequest[iter].update(mousePosGame,click);
      acceptRequest[iter].displayWithoutText(window);
      declineRequest[iter].update(mousePosGame,click);
      declineRequest[iter].displayWithoutText(window);
    }

    window->draw(yes);
    window->draw(no);
    
    i += 35;
    if (count == listSize) {
      posLast = friends.getPosition().y + friends.getGlobalBounds().height;
    }
    count++;
    iter++;

  }
  scrollbarFRequest.update(mousePosGame,click,posLast);
  quit.setPosition({940, windowHeight - 90});
  quit.update(mousePosGame,click);
  quit.display(window);
  window->draw(hideFReq);
}

void GUIMenuDisplay::displaySent() {
  displayFriendList();
  scrollbarFSent.display(window);
  int count = 1;
  int listSize = session->getOutbound().size();
  float posLast = 785;
  
  int i = 640;
  sf::Text sent;
  sf::Text messageSent; 
  
  setText(messageSent, "Pending request(s)", fontBritanic, 30, sf::Color::Black, sf::Vector2f((texture.getSize().x/2-258/2)+(1000/2 - texture.getSize().x/2), 605));
  
  for (auto &f:session->getOutbound()) {
    setText(sent, f, fontTIMES, 30, sf::Color::Black, sf::Vector2f(texture.getSize().x/2-messageSent.getGlobalBounds().width/2+(1000/2 - texture.getSize().x/2), i- posLast/785*(scrollbarFSent.getPosThumb().y- scrollbarFSent.getPosTrack().y)*(listSize/4)));
    window->draw(sent);
   
    i += 35;
    if (count == listSize) {
      posLast = sent.getPosition().y + sent.getGlobalBounds().height;
    }
    count++;
  }
  scrollbarFSent.update(mousePosGame,click,posLast);
  
  window->draw(hideFSentTop);
  window->draw(hideFSentBot);
  window->draw(messageSent);
  window->draw(addFriendRect);
  addFriend.drawTo(*window);
  window->draw(EnterUsername);
  confirm.setPosition(sf::Vector2f(750,texture.getSize().y/2+160));
  confirm.update(mousePosGame, click);
  confirm.display(window);
  quit.setPosition({940, windowHeight - 90});
  quit.update(mousePosGame,click);
  quit.display(window);
}

void GUIMenuDisplay::displayChat() {
  scrollbarChat.display(window);
  int i = 10; 
  int count = 1;
  int listSize = menu->currentChat().size();
  sf::Text message;

  float posLast = 885; 
  for (auto &m:menu->currentChat()) { 
    setText(message, m, fontTIMES, 20, sf::Color::White, sf::Vector2f(30, i - posLast/(885)* (scrollbarChat.getPosThumb().y - scrollbarChat.getPosTrack().y)*(listSize/20)));    ///+ 10));
    window->draw(message);
    i += 25;
    if (count == listSize) {
      posLast = message.getPosition().y + message.getGlobalBounds().height;
    }
    count++;
  }
  scrollbarChat.update(mousePosGame, click, posLast);
  
  window->draw(hideChatBehind);
  window->draw(messageRectangle);
  messageText.drawTo(*window);
  window->draw(hideChat);
  
  quitChat.update(mousePosGame,click);
  quitChat.display(window);
}

void GUIMenuDisplay::displayLobbyHost() {
  commander.update(mousePosGame, click);
  classic.update(mousePosGame, click);
  gameTimer.update(mousePosGame, click);
  playerTimer.update(mousePosGame, click);
  roundTimer.update(mousePosGame, click);
  launchGame.update(mousePosGame, click);
  launchGame.display(window);
  window->draw(gameTimeRect);
  gameTimeText.drawTo(*window);
  window->draw(playerTimeRect);
  playerTimeText.drawTo(*window);
  window->draw(roundTimeRect);
  roundTimeText.drawTo(*window);
  window->draw(hideTime);
  window->draw(inviteFriendRect);
  inviteFriend.drawTo(*window);
  window->draw(inviteFriendText);
  window->draw(hideInvite);
  displayLobbySlot();
}

void GUIMenuDisplay::displayLobbySlot() {
  displayFriendList();
  scrollbarObservers.display(window);
  int listSize = lobby->getSpectators().size();
  float posLast = 880;
  sf::Text left;
  sf::Text right;
  sf::Text observer;
  int i = 200;
  int count = 1;
  
  setText(gameSettings, "Game Settings", fontBritanic, 40, sf::Color::Black, sf::Vector2f(100, 605));
  setText(playerLeft,   "Left Player",   fontBritanic, 40, sf::Color::Black, sf::Vector2f(485, 605));
  setText(playerRight,  "Right Player",  fontBritanic, 40, sf::Color::Black, sf::Vector2f(740, 605));
  setText(observers,    "Observers",     fontBritanic, 40, sf::Color::Black, sf::Vector2f(50, 125));
  setText(gameMode,     "Game Mode",     fontBritanic, 40, sf::Color::Black, sf::Vector2f(30, 680));
  setText(timer,        "Timer",         fontBritanic, 40, sf::Color::Black, sf::Vector2f(30, 760));
  
  setText(left, lobby->getLeft(), fontTIMES, 20, sf::Color::Black, sf::Vector2f(485, 680));
  setText(right, lobby->getRight(), fontTIMES, 20, sf::Color::Black, sf::Vector2f(740, 680)); 
  
  for (auto &o:lobby->getSpectators()) {
    setText(observer, o, fontTIMES, 20, sf::Color::Black, sf::Vector2f(50, i- posLast/880*(scrollbarObservers.getPosThumb().y- scrollbarObservers.getPosTrack().y)*(listSize/10)));
    window->draw(observer);
    i += 35;
    if (count == listSize) {
      posLast = observer.getPosition().y + observer.getGlobalBounds().height;
    }
    count++;
  }
  window->draw(hideObsTop);
  window->draw(hideObsBot);
  window->draw(left);
  window->draw(right);
  scrollbarObservers.update(mousePosGame,click,posLast);
  window->draw(gameSettings);
  window->draw(playerLeft);
  window->draw(playerRight);
  window->draw(observers);
  window->draw(gameMode);
  window->draw(timer); 
  commander.display(window);
  classic.display(window);
  gameTimer.display(window);
  playerTimer.display(window);
  roundTimer.display(window);
  spec.update(mousePosGame, click);
  spec.display(window);
  joinLeft.update(mousePosGame, click);
  joinLeft.display(window);
  joinRight.update(mousePosGame, click);
  joinRight.display(window);
  quitLobby.setPosition({940,  windowHeight - 90});
  quitLobby.update(mousePosGame, click);
  quitLobby.display(window);
}

void GUIMenuDisplay::displayGames() {
  scrollbarGames.display(window);
  float posLast = 990;
  int count = 1;
  int i = 680;
  int id = 1;
  NM::Message::Matches matches = menu->getMatches();
  int listSize = matches.getMatch().size();

  for (auto& match : matches.getMatch()) {
    setText(matchInfo[0], std::string(match.name.data()), fontBritanic, 25, sf::Color::Black, sf::Vector2f(200, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    setText(matchInfo[1], std::to_string(match.players), fontBritanic, 25, sf::Color::Black, sf::Vector2f(850, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    if (match.started)
      setText(matchInfo[2], "Yes", fontBritanic, 25, sf::Color::Green, sf::Vector2f(1050, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    else
      setText(matchInfo[2], "No", fontBritanic, 25, sf::Color::Red, sf::Vector2f(1050, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    if (match.password)
      setText(matchInfo[3], "Yes", fontBritanic, 25, sf::Color::Green, sf::Vector2f(1250, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    else
      setText(matchInfo[3], "No", fontBritanic, 25, sf::Color::Red, sf::Vector2f(1250, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    setText(matchInfo[4], std::to_string(id), fontBritanic, 25, sf::Color::Black, sf::Vector2f(80, i- posLast/990*(scrollbarGames.getPosThumb().y- scrollbarGames.getPosTrack().y)*(listSize/10)));
    window->draw(matchInfo[0]);
    window->draw(matchInfo[1]);
    window->draw(matchInfo[2]);
    window->draw(matchInfo[3]);
    window->draw(matchInfo[4]);
    i += 40;
    id++;
    
    if (count == listSize)
      posLast = matchInfo[0].getPosition().y + matchInfo[0].getGlobalBounds().height;
    count++;

  }
  window->draw(browserRect);
  for (int i = 0; i < 5; i++) {
    window->draw(browserText[i]);
  }
  window->draw(hideGamesTop);
  scrollbarGames.update(mousePosGame,click,posLast);
}

void GUIMenuDisplay::displayBrowser() {
  displayGames();
  createGame.update(mousePosGame, click);
  createGame.display(window);
  joinGame.update(mousePosGame,click);
  joinGame.display(window);
  refresh.update(mousePosGame, click);
  refresh.display(window);
  quit.setPosition({1400, 900});
  quit.update(mousePosGame,click);
  quit.display(window);
}

void GUIMenuDisplay::displayJoin() {
  setText(gameName, "Enter game's name :" , fontBritanic, 45, sf::Color::Black, sf::Vector2f(1000, 180));
  setText(gamePW, "Enter game's password :", fontBritanic, 45, sf::Color::Black, sf::Vector2f(1000, 380));
  
  displayGames();
  window->draw(hideGamesBot);
  window->draw(joinRect);
  join.drawTo(*window);
  refresh.update(mousePosGame, click);
  refresh.display(window);
  window->draw(joinPWRect);
  joinPW.drawTo(*window);
  window->draw(hideGameName);
  window->draw(hideGamePW);
  window->draw(gameName);
  window->draw(gamePW);
  quitLobby.setPosition({1400, 900});
  quitLobby.update(mousePosGame,click);
  quitLobby.display(window);
}

void GUIMenuDisplay::displayLobbyName() {
  window->draw(lobbyNameRect);
  lobbyName.drawTo(*window);
  window->draw(lobbyPWRect);
  lobbyPW.drawTo(*window);
  window->draw(hideLobbyName);
  window->draw(hideLobbyPW);
  window->draw(chooseName);
  window->draw(choosePW);
  confirm.setPosition(sf::Vector2f(1125, 340));
  confirm.update(mousePosGame,click);
  confirm.display(window);
  quit.setPosition({1400, 900});
  quit.update(mousePosGame,click);
  quit.display(window);
}

void GUIMenuDisplay::displayGameInvite() {
  scrollbarGamesInvite.display(window);
  int i = 605;
  sf::Text invitations;
  float posLast = 955;
  int listSize = session->getGameRequests().size();
  int count = 1;

  if (!acceptGame.loadFromFile("imgs/green_tick.png"))
    std::cout<<"Failed to load the image"<<std::endl;
  sf::Sprite acceptGamesprite;
  acceptGamesprite.setTexture(acceptGame);

  sf::Vector2f targetSize(50.0f, 50.0f);
  while (listSize > acceptGameInvite.size()) {
    acceptGameInvite.push_back({sf::Vector2f(1200, i+6), sf::Vector2f(50, 50), &fontBritanic, 15, "", sf::Color(53, 219, 70), sf::Color(61, 252, 80), sf::Color(100,100,100)});
  }

  int iter = 0;
  for (auto& invite:session->getGameRequests()) {
    setText(invitations, invite, fontTIMES, 50, sf::Color::Black, sf::Vector2f(500, i- posLast/990*(scrollbarGamesInvite.getPosThumb().y- scrollbarGamesInvite.getPosTrack().y)*(listSize/10)));
    window->draw(invitations);
    acceptGamesprite.setPosition({1200, i+5});
    acceptGamesprite.setScale(targetSize.x/acceptGamesprite.getLocalBounds().width, targetSize.y/acceptGamesprite.getLocalBounds().height);
    if (acceptGameInvite.size() > 0) {
      acceptGameInvite[iter].setText(invite);
      acceptGameInvite[iter].setPosition(sf::Vector2f(1200, i+6));
      acceptGameInvite[iter].update(mousePosGame, click);
      acceptGameInvite[iter].displayWithoutText(window);
    }
    window->draw(acceptGamesprite);
    i += 55;
    if (count == listSize)
      posLast = matchInfo[0].getPosition().y + matchInfo[0].getGlobalBounds().height;
    count++;
  }
  scrollbarGamesInvite.update(mousePosGame,click,posLast);
  quit.setPosition({1400, 900});
  quit.update(mousePosGame,click);
  quit.display(window);
}

void GUIMenuDisplay::setRectangle(sf::RectangleShape &rectangle, sf::Vector2f size, sf::Vector2f position, sf::Color fillColor, sf::Color outineColor, int outlineThickness) {
  rectangle.setSize(size);
  rectangle.setOutlineColor(outineColor);
  rectangle.setOutlineThickness(outlineThickness);
  rectangle.setPosition(position);
  rectangle.setFillColor(fillColor);
}

void GUIMenuDisplay::setText(sf::Text &text, string message, sf::Font &font, unsigned int characterSize, sf::Color fillColor, sf::Vector2f position) {
  text.setString(message);
  text.setFont(font);
  text.setCharacterSize(characterSize);
  text.setFillColor(fillColor);
  text.setPosition(position);
}

void GUIMenuDisplay::setTexbox(Textbox &textbox, sf::Vector2f position, sf::Font &font, bool limit, int limChar) {
  textbox.setPosition(position);
  textbox.setLimit(limit, limChar);
  textbox.setFont(font);
}

bool GUIMenuDisplay::commanderModeSelected() {
  if (commander.isSelected())
    return true;
  return false;
}

#pragma GCC diagnostic pop
