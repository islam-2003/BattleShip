#include "clickable_shape.hh"

ClickableShape::ClickableShape() {}

ClickableShape::ClickableShape(sf::Vector2f coords, sf::Vector2f dist, sf::Color i, sf::Color h, sf::Color p, bool hold, short unsigned int bs) : idle{i}, hover{h}, pressed{p}, borderSize{bs}, hold{hold} {
  shape.setPosition(coords);
  shape.setSize(dist);
  shape.setFillColor(idle);
  shape.setOutlineThickness(borderSize);
  shape.setOutlineColor(sf::Color::Black);
}

void ClickableShape::display(std::shared_ptr<sf::RenderWindow> window) { 
  window->draw(shape);
}

void ClickableShape::update(const sf::Vector2f mousePos, bool &click) {
  state = IDLE;
  if (containsWithoutBorder(mousePos)) {
    state = HOVER;
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && (!click || hold)) {
      click = 1;
      state = PRESSED;
    }
  }
  setColor();
}

bool ClickableShape::isPressed() {
  if (state == PRESSED) {
    state = IDLE;
    return true;
  }
  return false;
}

bool ClickableShape::containsWithoutBorder(const sf::Vector2f mousePos) {
  return mousePos.x > shape.getGlobalBounds().left + borderSize && mousePos.x<shape.getGlobalBounds().left + shape.getGlobalBounds().width - borderSize 
          &&  mousePos.y>shape.getGlobalBounds().top + borderSize && mousePos.y<shape.getGlobalBounds().top + shape.getGlobalBounds().height - borderSize;
}

sf::RectangleShape ClickableShape::getShape() {
  return shape;
}
  
short unsigned ClickableShape::getState() {
  return state;
}

void ClickableShape::setPosition(sf::Vector2f coords) {
  shape.setPosition(coords);
}

void ClickableShape::setColors(sf::Color newIdle, sf::Color newHover, sf::Color newPressed) {
  idle = newIdle;
  hover = newHover;
  pressed = newPressed;
}

void ClickableShape::setUnclickable() {
  clickable = 0;
}

sf::Vector2f ClickableShape::getPosition() {
  return shape.getPosition();
}

sf::FloatRect ClickableShape::getGlobalBounds() {
  return shape.getGlobalBounds();
}

void ClickableShape::setSize(sf::Vector2f size) {
  shape.setSize(size);
}

void ClickableShape::press() {
  state = PRESSED;
}

void ClickableShape::setState(short unsigned newState) {
  state = newState;
}

bool ClickableShape::getHold() {
  return hold;
}

void ClickableShape::setOutlineColor(sf::Color color) {
  shape.setOutlineColor(color);
}

void ClickableShape::setColor() {
  switch (state) {
    case IDLE:
      shape.setFillColor(idle);
      break;
    case HOVER:
      shape.setFillColor(hover);
      break;
    case PRESSED:
      shape.setFillColor(pressed);
      break;
    default:
      shape.setFillColor(sf::Color::Black); //error
      break;
  }
}

void ClickableShape::multUpdate(const sf::Vector2f mousePos,int & click, bool& lock) {
  state = IDLE;
  if (containsWithoutBorder(mousePos)) {
    state = HOVER;
    if (clickable && sf::Mouse::isButtonPressed(sf::Mouse::Left)&& !lock) {
      click--;
      state = PRESSED;
      if (click == 0)
        lock =1;
    }
  }  
  setColor();
}


