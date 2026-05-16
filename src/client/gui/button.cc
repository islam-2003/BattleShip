
#include "button.hh"


Button::Button(sf::Vector2f coords, sf::Vector2f size, sf::Font* font, unsigned int fontSize, std::string t, sf::Color ic, sf::Color hc, sf::Color pc, short unsigned int outlineSize) {
  text.setString(t);
  text.setFont(*font);
  text.setCharacterSize(fontSize);
  text.setFillColor(sf::Color::White);
  rectangle = {coords, size, ic, hc, pc, 0, outlineSize};
}

void Button::display(std::shared_ptr<sf::RenderWindow> window) {
  text.setPosition(rectangle.getShape().getPosition().x + (rectangle.getShape().getLocalBounds().width/2.f) - text.getGlobalBounds().width/2.f, rectangle.getShape().getPosition().y + (rectangle.getShape().getLocalBounds().height/4.f)/*-text.getGlobalBounds().height*/); 
  window->draw(rectangle.getShape());
  window->draw(text);
}

void Button::displayWithoutText(std::shared_ptr<sf::RenderWindow> window) { 
  window->draw(rectangle.getShape());
}

void Button::update(const sf::Vector2f mousePos, bool &click) {
  rectangle.update(mousePos,click);
}

bool Button::isPressed() {
  return rectangle.isPressed();
}

void Button::setPosition(sf::Vector2f coords) {
  rectangle.setPosition(coords);
}

std::string Button::getText() {
  return text.getString();
}

void Button::setText(std::string newText) {
  text.setString(newText);
}

void Button::press() {
  rectangle.press();
}

void Button::setState(short unsigned newState) {
  rectangle.setState(newState);
}

short unsigned Button::getState() {
  return rectangle.getState();
}

bool Button::containsWithoutBorder(const sf::Vector2f mousePos) {
  return rectangle.containsWithoutBorder(mousePos);
}

bool Button::getHold() {
  return rectangle.getHold();
}

void Button::setOutlineColor(sf::Color color) {
  rectangle.setOutlineColor(color);
}

sf::FloatRect Button::getSize() {
  return rectangle.getGlobalBounds();
}


SelectButton::SelectButton(sf::Vector2f coords, sf::Vector2f size, sf::Font* font, unsigned int fontSize, std::string text, sf::Color idle, sf::Color hover, sf::Color pressed, sf::Color outlineColor, short unsigned int outlineSize)
  : Button{coords, size, font, fontSize, text, idle, hover, pressed, outlineSize}, outline{outlineColor} {
    Button::setOutlineColor(sf::Color::Black);
}

bool SelectButton::isSelected() {
  return selected;
}

void SelectButton::setSelected(bool s) {
  selected = s; 
  if (selected) 
    Button::setOutlineColor(outline); 
  else 
    Button::setOutlineColor(sf::Color::Black);
}

void SelectButton::addNeighbor(SelectButton* neighbor) {
  neighbors.push_back(neighbor);
}

void SelectButton::update(const sf::Vector2f mousePos, bool &click) {
  Button::setState(IDLE);
  if (Button::containsWithoutBorder(mousePos)) {
    Button::setState(HOVER);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && (!click || Button::getHold())) {
      click = 1;
      Button::setState(PRESSED);
      setSelected(1);
      selectionChanged = 1;
      for (auto &n : neighbors)
        n->setSelected(0);
    }
  }
  Button::update(mousePos, click);
}

void SelectButton::changeSelection() {
  selectionChanged = 0;
}

bool SelectButton::hasSelectionChanged() {
  return selectionChanged;
}