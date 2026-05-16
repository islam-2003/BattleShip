#pragma once

#include "button.hh"
#include "textbox.hh"
#include "scrollbar.hh"
#include "../console_menu_display.hh"  // Enlever ?
#include "../client_menu_view.hh"
#include "../display_common.hh"
#include <optional>

using std::vector;
using std::span;

class SessionInfo;  // Forward-declared class from client.hh

enum screenState{START, LOGIN, REGISTER_SCREEN, CHAT, MAINMENU, SENT, RECEIVED, LOBBY, FACTION, BROWSER, JOIN, CREATE, INVITE}; 
                                                

inline int WINDOW_WIDTH = 1500;                                                                
inline int WINDOW_HEIGHT = 1000;
             

class GUIMenuDisplay : public MenuDisplay {                      
 public: 
  bool is_active = 1;
  GUIMenuDisplay(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<MenuView const>  view, std::shared_ptr<MenuControl> control,
                 std::shared_ptr<LobbyView const> lobby, observer_ptr<SessionInfo const> session);
  
  NM::Message updateButtonPressed();

  void scroll(Textbox &textbox, sf::RectangleShape &rectangle);

  void setupShape();

  /** Parse Numbers provided by user, check boundaries and call
   * BoardControl::fire. */
  [[nodiscard]] vector<string> handleInput();
  NM::Message pollEvent();
  void updateScreenState();
  void display();
  void updateMousePos();
  void displayStart();
  void displayLogin();
  void displayChat();
  void displayMainMenu();
  void displayBackground(string title, sf::Vector2f boat_pos, sf::Vector2f title_pos);
  void displayFriendList(); 
  void displayRequest();
  void displaySent();
  void displayLobbyHost();
  void displayLobbySlot();
  void displayBrowser();
  void displayJoin();
  void displayGames();
  void displayLobbyName();
  void displayGameInvite();
  void setRectangle(sf::RectangleShape &rectangle, sf::Vector2f size,  sf::Vector2f position, sf::Color fillColor = sf::Color::White, sf::Color outineColor = sf::Color::White, int outlineThickness = 0);
  void setText(sf::Text &text,string message, sf::Font &font,unsigned int characterSize, sf::Color fillColor, sf::Vector2f position);
  void setTexbox(Textbox &textbox, sf::Vector2f position, sf::Font &font, bool limit = false, int limChar = 0);
  bool commanderModeSelected();

 private:
  int  windowWidth = 1500;
  int  windowHeight = 1000;
  bool click = 0;
  bool focus = 0;

  sf::Font fontBritanic;
  sf::Font fontArial;
  sf::Font fontTIMES;
  
  std::shared_ptr<sf::RenderWindow> window;
  sf::Texture texture;
  sf::Texture yesButton;
  sf::Texture noButton;
  sf::Texture bubbleChat;
  sf::Texture fleetPirate;
  sf::Texture fleetCaptain;
  sf::Texture acceptGame;
  sf::Text mainMenu;
  

  //x,y xoordinates = top left corner
  Button login            {sf::Vector2f(windowWidth/2-200/2, 700), sf::Vector2f(200, 90), &fontBritanic, 35, "Login",    sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button regist           {sf::Vector2f(windowWidth/2-200/2, 850), sf::Vector2f(200, 90), &fontBritanic, 35, "Register", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  Button mainmenu         {sf::Vector2f(100, 500), sf::Vector2f(200, 90), &fontBritanic, 35, "Main Menu", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
 
  Button gameBrowser      {sf::Vector2f(1000/2-200/2, 600), sf::Vector2f(200, 90), &fontBritanic, 30, "Game Browser", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  
  Button conversation     {sf::Vector2f(1000/2+50, 800),    sf::Vector2f(200, 90), &fontBritanic, 35, "Chat",         sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button logout           {sf::Vector2f(1000/2-200/2, 800), sf::Vector2f(200, 90), &fontBritanic, 35, "Logout",       sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button gameInvite       {sf::Vector2f(1000/2-200/2, 700), sf::Vector2f(200, 90), &fontBritanic, 35, "Game invite",  sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  Button requestsreceived {sf::Vector2f(1000,  0), sf::Vector2f(500, 90), &fontBritanic, 30, "Request(s) Received", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button requestsSent     {sf::Vector2f(1000, 90), sf::Vector2f(500, 90), &fontBritanic, 30, "Request(s) Sent",     sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  Button quit             {sf::Vector2f(940, windowHeight - 90),   sf::Vector2f(50, 50), &fontBritanic, 15, "Quit", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button quitChat         {sf::Vector2f(1385, windowHeight - 90),  sf::Vector2f(50, 50), &fontBritanic, 15, "Quit", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button quitLogin        {sf::Vector2f(1385, windowHeight - 90),  sf::Vector2f(50, 50), &fontBritanic, 15, "Quit", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button quitLobby        {sf::Vector2f(940, windowHeight - 90),   sf::Vector2f(50, 50), &fontBritanic, 15, "Quit", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  SelectButton commander  {sf::Vector2f(250, 680), sf::Vector2f(100, 50), &fontBritanic, 15, "Commander",   sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  SelectButton classic    {sf::Vector2f(360, 680), sf::Vector2f(100, 50), &fontBritanic, 15, "Classic",     sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  SelectButton gameTimer  {sf::Vector2f(250, 760), sf::Vector2f(100, 50), &fontBritanic, 15, "Game",        sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  SelectButton playerTimer{sf::Vector2f(250, 830), sf::Vector2f(100, 50), &fontBritanic, 15, "Player",      sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button       roundTimer {sf::Vector2f(250, 900), sf::Vector2f(100, 50), &fontBritanic, 15, "Round",       sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button       launchGame {sf::Vector2f(800, 340), sf::Vector2f(150, 70), &fontBritanic, 20, "Launch Game", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button       spec       {sf::Vector2f(80, 470),  sf::Vector2f(100, 50), &fontBritanic, 20, "Join",        sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button       joinLeft   {sf::Vector2f(520, 800), sf::Vector2f(100, 50), &fontBritanic, 20, "Join",        sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button       joinRight  {sf::Vector2f(770, 800), sf::Vector2f(100, 50), &fontBritanic, 20, "Join",        sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  Button confirm          {sf::Vector2f(1125, 340), sf::Vector2f(200, 90), &fontBritanic, 30, "Confirm", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};

  Button createGame       {sf::Vector2f(100, 100), sf::Vector2f(200, 90), &fontBritanic, 35, "Create Game", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button refresh          {sf::Vector2f(100, 300), sf::Vector2f(200, 90), &fontBritanic, 35, "Refresh",     sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  Button joinGame         {sf::Vector2f(100, 200), sf::Vector2f(200, 90), &fontBritanic, 35, "Join Game",   sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  
  sf::Vector2i mousePosScreen;
  sf::Vector2i mousePosWindow;
  sf::Vector2f mousePosGame;

  short unsigned screenState = START;

  //scroll
  sf::RectangleShape hideUser;
  sf::RectangleShape hidePW;
  sf::RectangleShape hideChat;
  sf::RectangleShape hideChatBehind;
  sf::RectangleShape hideFReq;
  sf::RectangleShape hideFSentTop;
  sf::RectangleShape hideFSentBot;
  sf::RectangleShape hideObsTop;
  sf::RectangleShape hideObsBot;
  sf::RectangleShape hideGamesTop;
  sf::RectangleShape hideGamesBot;
  sf::RectangleShape hideGameName;
  sf::RectangleShape hideGamePW;
  sf::RectangleShape hideTime;
  sf::RectangleShape hideInvite;
  sf::RectangleShape hideLobbyName;
  sf::RectangleShape hideLobbyPW;
  sf::RectangleShape hideGameInvite;

  sf::Text loginMenu[2];
  string username = "Username (max 12 characters)";
  string password = "Password";
  sf::Text EnterUsername;
  sf::RectangleShape userRectangle;
  sf::RectangleShape PWRectangle;
  sf::RectangleShape messageRectangle;
  sf::RectangleShape friendListRect;
  sf::RectangleShape addFriendRect;
  sf::RectangleShape browserRect;
  sf::RectangleShape joinRect;
  sf::RectangleShape joinPWRect;
  sf::RectangleShape lobbyNameRect;
  sf::RectangleShape lobbyPWRect;
  sf::RectangleShape roundTimeRect;
  sf::RectangleShape gameTimeRect;
  sf::RectangleShape playerTimeRect;
  sf::RectangleShape inviteFriendRect;
  Textbox userText       {50, sf::Color::Black, true};
  Textbox PWText         {50, sf::Color::Black, true};
  Textbox messageText    {30, sf::Color::Black, true};
  Textbox addFriend      {50, sf::Color::Black, true};
  Textbox join           {50, sf::Color::Black, true};
  Textbox joinPW         {50, sf::Color::Black, true};
  Textbox lobbyName      {50, sf::Color::Black, true};
  Textbox lobbyPW        {50, sf::Color::Black, false};
  Textbox roundTimeText  {30, sf::Color::Black, false};
  Textbox gameTimeText   {30, sf::Color::Black, false};
  Textbox playerTimeText {30, sf::Color::Black, false};
  Textbox inviteFriend   {40, sf::Color::Black, false};
  

  unsigned short int registerlimit = 0; //sets username as first textbox 

  Scrollbar scrollbarChat        {sf::Vector2f(1475, 10),    sf::Vector2f(25, 980), sf::Vector2f(10, 885),  sf::Vector2f(10, 885)}; 
  Scrollbar scrollbarFRequest    {sf::Vector2f(1500/2, 605), sf::Vector2f(10, 375), sf::Vector2f(615, 900), sf::Vector2f(0, 1000)};
  Scrollbar scrollbarFriendList  {sf::Vector2f(1480, 185),   sf::Vector2f(10, 795), sf::Vector2f(185, 900), sf::Vector2f(0, 1000)};
  Scrollbar scrollbarFSent       {sf::Vector2f(750, 640),    sf::Vector2f(10, 145), sf::Vector2f(640, 785), sf::Vector2f(0, 785)};
  Scrollbar scrollbarObservers   {sf::Vector2f(290, 200),    sf::Vector2f(10, 250), sf::Vector2f(200, 450), sf::Vector2f(0, 450)};
  Scrollbar scrollbarGames       {sf::Vector2f(1485, 690),   sf::Vector2f(10, 250), sf::Vector2f(690, 940), sf::Vector2f(0, 940)};
  Scrollbar scrollbarGamesInvite {sf::Vector2f(1300, 605),   sf::Vector2f(10, 350), sf::Vector2f(690, 955), sf::Vector2f(0, 955)};

  sf::Text gameSettings;
  sf::Text playerLeft;
  sf::Text playerRight;
  sf::Text observers;
  sf::Text gameMode;
  sf::Text timer;
  sf::Text browserText[5];
  sf::Text chooseName;
  sf::Text choosePW;
  sf::Text matchInfo[5];
  sf::Text inviteFriendText;
  sf::Text gameName;
  sf::Text gamePW;

  vector<Button> acceptRequest;
  vector<Button> declineRequest;
  vector<Button> removeFriend;
  vector<Button> chatButton;
  vector<Button> acceptGameInvite;
};
