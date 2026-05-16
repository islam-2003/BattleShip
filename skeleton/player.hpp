#include "utils.hpp"
#include "board_view.hpp"


class Player{
  /* Cette classe gère les pouvoirs du joueur pendant une partie en mode Commandant */
  Faction _faction;
  int _energy_pts;
  vector<Skills> _skills;
  vector<BoardView::BoatType> _boats;
 public:
  void setFaction();
  void setEnergyPts();
  void setBoats();
  void setSkills();
  void useSkill();
  void getEnergyPts();
};

enum Faction{
  /*Les différentes factions disponible*/
  CLASSIC,
  PIRATE,
  CAPTAIN
};

enum Skills{
  /*Les différents pouvoir disponible*/
  STANDARD,
  RADAR,
  LONG_HIT,
  DIAGONAL_HIT
};