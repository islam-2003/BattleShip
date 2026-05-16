#pragma once


#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>

#define DELETE_KEY 8
#define ENTER_KEY 13
#define ESCAPE_KEY 27

class Textbox {
 private:
  sf::Text textbox;
  unsigned int size;
  bool hasLimit = false;
  size_t limit = 0;

  void deleteLastChar();
  void inputLogic(int charTyped);

 public:
  std::ostringstream text; 
  bool isSelected = false;
  Textbox(unsigned int size, sf::Color color, bool sel);
  unsigned int getSize();
  void setFont(sf::Font &font);
  void setPosition(sf::Vector2f point);
  void setLimit(bool ToF, int lim);
  void setSelected(bool sel);
  void setString(const std::string &text);
  std::string substr(int pos);
  std::string getText();
  void drawTo(sf::RenderWindow &window);
  void typedOn(sf::Event input);
  sf::Vector2f getPosition();
  sf::FloatRect getGlobalBounds();
  void clear();
};

