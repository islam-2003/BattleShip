#pragma once
#include "clickable_shape.hh"


class Scrollbar { // only vertical
  sf::Vector2f thumbSize;
  sf::Color trackColor;
  sf::RectangleShape track; 
  ClickableShape thumb;

 public:
  sf::Vector2f trueSize; 
  sf::Vector2f screenSize;
  Scrollbar(sf::Vector2f coord, sf::Vector2f wh, sf::Vector2f screenSize = sf::Vector2f(0,1000), sf::Vector2f trueSize = sf::Vector2f(0,1000), sf::Color trackColor = sf::Color(150,150,150), sf::Color idleThumb = sf::Color(100,100,100), sf::Color hoverThumb = sf::Color(200,200,200), sf::Color activeThumb = sf::Color(80,80,80));
  bool isPressed();
  void display(std::shared_ptr<sf::RenderWindow> window);
  void update(sf::Vector2f mousePos,bool &click, float newHeight);
  sf::Vector2f getPosThumb();  //returns top left
  sf::Vector2f getCenterThumb();
  sf::Vector2f getPosTrack();

};


