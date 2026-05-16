#pragma once

#include "clickable_shape.hh"

using std::vector;

class Button {
 private:
  sf::Font* font;
  sf::Text text;
  ClickableShape rectangle;

 public:
  Button(sf::Vector2f coords, sf::Vector2f size, sf::Font* font, unsigned int fontSize, std::string text, sf::Color idle, sf::Color hover, sf::Color pressed, short unsigned outlineSize = 0);
  void display(std::shared_ptr<sf::RenderWindow> window);
  void displayWithoutText(std::shared_ptr<sf::RenderWindow> window); //for accept/decline
  void update(const sf::Vector2f mousePos, bool &click);
  bool isPressed();
  void setPosition(sf::Vector2f coords);
  std::string getText();
  void setText(std::string newText);
  void press();
  void setState(short unsigned newState);
  short unsigned getState();
  bool containsWithoutBorder(const sf::Vector2f mousePos);
  bool getHold();
  void setOutlineColor(sf::Color color);
  sf::FloatRect getSize();
};


class SelectButton: public Button {
  bool selected = 0;
  sf::Color outline;
  vector<SelectButton*> neighbors;
  bool selectionChanged = 0;
 public:
  SelectButton(sf::Vector2f coords, sf::Vector2f size, sf::Font* font, unsigned int fontSize, std::string text, sf::Color idle, sf::Color hover, sf::Color pressed, sf::Color outlineColor = sf::Color::Red, short unsigned int outlineSize = 3);
  bool isSelected();
  void setSelected(bool s);
  void addNeighbor(SelectButton* neighbor);
  void update(const sf::Vector2f mousePos, bool &click);
  void changeSelection();
  bool hasSelectionChanged();
};
