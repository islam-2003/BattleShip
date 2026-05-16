#include "utils.hpp"
#include "board_coordinates.hpp"

class BoardControl {
 protected:
  BoardControl(const BoardControl&)            = default;
  BoardControl(BoardControl&&)                 = default;
  BoardControl& operator=(const BoardControl&) = default;
  BoardControl& operator=(BoardControl&&)      = default;

 public:
  BoardControl() = default;

  virtual bool select(char)           = 0;
  virtual bool move(char)             = 0;
  virtual bool fire(BoardCoordinates) = 0;
  virtual void quit()                 = 0;

  virtual ~BoardControl() = default;
};