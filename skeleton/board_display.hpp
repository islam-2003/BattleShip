#include "utils.hpp"




class BoardDisplay {
 protected:
  BoardDisplay(const BoardDisplay&)            = default;
  BoardDisplay(BoardDisplay&&)                 = default;
  BoardDisplay& operator=(const BoardDisplay&) = default;
  BoardDisplay& operator=(BoardDisplay&&)      = default;

 public:
  BoardDisplay() = default;
  virtual void update()      = 0;
  virtual void handleInput() = 0;
  virtual void hideBoeard()  = 0;

  virtual ~BoardDisplay() = default;
};