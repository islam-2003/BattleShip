#pragma once

#include <memory>
#include <ostream>
#include <istream>
#include <string>
#include <utility>
#include <vector>
#include <span>
#include <iostream>
#include <optional>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>

#include "../client_menu_controller.hh"
#include "../client_menu_view.hh"
#include "../../common/not_implemented_error.hh"
#include "../../common/network_io.hh"

enum button_state{IDLE, HOVER, PRESSED};

class ClickableShape {
  short unsigned state = IDLE;
  sf::RectangleShape shape;
  sf::Color idle;
  sf::Color hover;
  sf::Color pressed;
  short unsigned int borderSize = 0;
  bool clickable = 1;
  bool hold = 0;

  public:
    ClickableShape();
    ClickableShape(sf::Vector2f coords, sf::Vector2f dist, sf::Color idle, sf::Color hover, sf::Color pressed, bool hold = 0, short unsigned int borderSize = 0);

    void display(std::shared_ptr<sf::RenderWindow> window);
    void update(const sf::Vector2f mousePos, bool &click);
    bool isPressed();
    bool containsWithoutBorder(const sf::Vector2f mousePos);
    sf::RectangleShape getShape();
    short unsigned getState();
    void setPosition(sf::Vector2f coords);
    void setColors(sf::Color newIdle, sf::Color newHover, sf::Color newPressed);
    void setUnclickable();
    sf::Vector2f getPosition();
    sf::FloatRect getGlobalBounds();
    void setSize(sf::Vector2f size);
    void setColor();
    void press();
    void setState(short unsigned newState);
    bool getHold();
    void setOutlineColor(sf::Color color);
    void multUpdate(const sf::Vector2f mousePos,int & click, bool& lock);

};