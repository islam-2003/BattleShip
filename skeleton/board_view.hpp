#include "utils.hpp"
#include "board_coordinates.hpp"


class BoardView {
 protected:
  BoardView(const BoardView&)            = default;
  BoardView(BoardView&&)                 = default;
  BoardView& operator=(const BoardView&) = default;
  BoardView& operator=(BoardView&&)      = default;

 public:
  enum CellType : uint8_t{
    IS_SHIP  = 0b001,
    IS_KNOWN = 0b010,
    IS_SUNK  = 0b100,

    WATER = 0,
    OCEAN = IS_KNOWN,

    UNDAMAGED = IS_SHIP,
    HIT       = IS_SHIP | IS_KNOWN,
    SUNK      = IS_SHIP | IS_KNOWN | IS_SUNK,
    IS_MINE,
  };

  enum BoatType : uint8_t{
    TWO_BY_ONE = 2,
    THREE_BY_ONE = 3,
    FOUR_BY_ONE = 4,
    FIVE_BY_ONE = 5
  };

  static constexpr inline CellType best(CellType lhs, CellType rhs);

  BoardView() = default;

  [[nodiscard]] virtual uint8_t  gameState()    const                      = 0;
  [[nodiscard]] virtual bool     leftTurn()     const                      = 0;
  [[nodiscard]] virtual bool     isFinished()   const                      = 0;
  [[nodiscard]] virtual bool     isVictory()    const                      = 0;
  [[nodiscard]] virtual size_t   width()        const                      = 0;
  [[nodiscard]] virtual size_t   height()       const                      = 0;
  [[nodiscard]] virtual multiset<BoatType>
                                 getInventory() const                      = 0;
  [[nodiscard]] virtual CellType cellType(bool             my_side,
                                          BoardCoordinates position) const = 0;
  [[nodiscard]] virtual bool     isSameShip(bool my_side, BoardCoordinates first,
                                            BoardCoordinates second) const = 0;

  virtual ~BoardView() = default;
};
