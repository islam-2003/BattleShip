#pragma once

#include <array>
#include <experimental/memory>

#include "board_gui.hh"
#include "gui_menu_display.hh"
#include "../client_timer.hh"
#include "../client_board.hh"

using std::vector;
namespace epr = std::experimental;

enum ability{BASIC, ABILITY1, ABILITY2};

class GUIGame : public GameDisplay {
  std::shared_ptr<sf::RenderWindow> window;
  bool commanderMode = 0;

  std::ostream& output;
  std::istream& input;
  bool leftTurn = 1;
  bool click = 0;
  unsigned int index = 0;
  bool boat_lock = 0;
  bool boatPlaced = 1;
  bool focus = 0;
  //bool your_turn = 1;
  //bool game_started = 1;


  unsigned short selected_ability = BASIC;
  GameModel::GameStage _game_state{commanderMode ? GameModel::GameStage::FACTIONSELECT : GameModel::GameStage::PLACEMENT};



  sf::Vector2i mousePosScreen;
  sf::Vector2i mousePosWindow;
  sf::Vector2f mousePosGame;
  sf::Font font;
  sf::Font fontBritanic;

  Button confirm{sf::Vector2f(900,850),sf::Vector2f(150,75),&font, 20, "Confirm", sf::Color(150,150,150),sf::Color(200,200,200),sf::Color(100,100,100)};


  int screenwidth = 1500;
  int screenheight = 1000;

  vector<Button> boatButtons;


  Board board_left{sf::Vector2f(50.f,150),sf::Vector2f(50.f,50.f), &font,_board, _control, 1}; 
  Board board_right{sf::Vector2f(1500/2+50,150),sf::Vector2f(50.f,50.f), &font,_board, _control, 0};

  Button basicAbility{sf::Vector2f(1500/2+50,700),sf::Vector2f(150,75),&font, 20, "Shoot", sf::Color(150,150,150),sf::Color(200,200,200),sf::Color(100,100,100)}; 
  Button ability1{sf::Vector2f(1500/2+110,700),sf::Vector2f(150,75),&font, 20, "Ability 1", sf::Color(150,150,150),sf::Color(200,200,200),sf::Color(100,100,100)}; 
  Button ability2{sf::Vector2f(1500/2+170,700),sf::Vector2f(150,75),&font, 20, "ability 2", sf::Color(150,150,150),sf::Color(200,200,200),sf::Color(100,100,100)};

  sf::RectangleShape inventoryBox{sf::Vector2f(300,300)};
  sf::RectangleShape energy;

  sf::Text leftBoard;
  sf::Text rightBoard;
  sf::Text screenTitle;


  //faction
  SelectButton pirate     {sf::Vector2f(250, 625), sf::Vector2f(400, 350), &fontBritanic, 15, "", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  SelectButton captain    {sf::Vector2f(850, 625), sf::Vector2f(400, 350), &fontBritanic, 15, "", sf::Color(150,150,150), sf::Color(200,200,200), sf::Color(100,100,100)};
  sf::Texture texture;
  sf::Texture fleetPirate;
  sf::Texture fleetCaptain;
  sf::Text pirateTitre;
  sf::Text captainTitre;
  sf::Text pirateAbilities[3];
  sf::Text captainAbilities[2];
  sf::Text boat;
  sf::Text mainMenu;

 public: 
  bool is_active = 1;

  GUIGame(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<ClientView const> board, std::shared_ptr<ClientControl> control,
                     ClientTimer* timer, bool mode = 0, std::ostream& out = std::cout, std::istream& in = std::cin)
        : GameDisplay{std::move(board), std::move(control), timer},
          window(window), commanderMode(mode),
          output(out), input(in) {
          if (!font.loadFromFile("fonts/arial.ttf")) {
              std::cout<<"font error"<<std::endl;
          }
           if (!fontBritanic.loadFromFile("fonts/BRITANIC.TTF")) {
              std::cout<<"can't load britanic font"<<std::endl;
          }
          setText(screenTitle, "Placement", font, 40, sf::Color::White, sf::Vector2f(1500/2-187/2, 50)); 
          setText(leftBoard, "Left's fleet", font, 40, sf::Color::White, sf::Vector2f(50+225-182/5, 50));
          setText(rightBoard, "Right's fleet", font, 40, sf::Color::White, sf::Vector2f(1500/2+50+225-207/5, 50));


          pirate.addNeighbor(&captain);
          captain.addNeighbor(&pirate);

          inventoryBox.setPosition({100,675});
          inventoryBox.setFillColor(sf::Color::White);
          inventoryBox.setOutlineThickness(2);
          inventoryBox.setOutlineColor(sf::Color::Black);

          }


  void updateState();
  void setCommander();
  bool getCommander();
  void display();
  void displayObserver();
  void updateMousePos();
  void setUpEnd();
  void displayEnd();
  void tick();
  void displayInventory(unsigned int &index); 
  void displayEnergy();
  std::string getShipName(Boat::Type boat);
  std::string updateAbilityButton();
  void setText(sf::Text &text, string message, sf::Font &font, unsigned int characterSize, sf::Color fillColor, sf::Vector2f position);
  NM::Message updateButtonPressed(unsigned int &index);
  NM::Message pollEvent(); 
  void displayFactions();
  std::string factionSelected();
  void displayBackground(string title, sf::Vector2f boat_pos, sf::Vector2f title_pos);
  std::string getShipNumber(Boat::Type boat);


};
