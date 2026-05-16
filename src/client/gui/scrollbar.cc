
#include "scrollbar.hh"


Scrollbar::Scrollbar(sf::Vector2f coord, sf::Vector2f wh, sf::Vector2f sS, sf::Vector2f tS, sf::Color tc, sf::Color it, sf::Color ht, sf::Color pt) {
  track.setPosition(coord);
  track.setSize(wh);
  trackColor = tc;
  thumb = {coord, wh, it, ht, pt, 1};
  track.setFillColor(trackColor);
  trueSize = tS;
  screenSize = sS;
}

bool Scrollbar::isPressed() {
  return thumb.isPressed();
}

void Scrollbar::display(std::shared_ptr<sf::RenderWindow> window) {
  window->draw(track);
  thumb.display(window);
}

void Scrollbar::update(sf::Vector2f mousePos,bool &click, float newHeight) {
  //update thumb pos
  if (track.getGlobalBounds().contains(mousePos) && thumb.isPressed()) {
    if (mousePos.y - thumb.getGlobalBounds().height/2 < track.getPosition().y)
      thumb.setPosition(sf::Vector2f(thumb.getPosition().x, track.getPosition().y));
    else if (mousePos.y + thumb.getGlobalBounds().height/2 > track.getPosition().y + track.getGlobalBounds().height)
      thumb.setPosition(sf::Vector2f(thumb.getPosition().x, track.getPosition().y + track.getGlobalBounds().height - thumb.getGlobalBounds().height));
    else
      thumb.setPosition(sf::Vector2f(thumb.getPosition().x, mousePos.y - thumb.getGlobalBounds().height/2));
  }

  //update thumbsize
  if (newHeight > trueSize.y) {
    trueSize.y = newHeight;
    thumb.setSize(sf::Vector2f(thumb.getGlobalBounds().width, track.getGlobalBounds().height*(screenSize.y-screenSize.x)/(trueSize.y-trueSize.x))); 
  }
  thumb.update(mousePos,click);
}

sf::Vector2f Scrollbar::getPosThumb() {
  return thumb.getPosition();
}

sf::Vector2f Scrollbar::getPosTrack() {
  return track.getPosition();
}

sf::Vector2f Scrollbar::getCenterThumb() {
  return sf::Vector2f(thumb.getGlobalBounds().left+thumb.getGlobalBounds().width/2, thumb.getGlobalBounds().top+thumb.getGlobalBounds().height/2);
}
