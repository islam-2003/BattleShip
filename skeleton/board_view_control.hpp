#include "utils.hpp"
#include "board_view.hpp"
#include "board_control.hpp"
#include "board_coordinates.hpp"
#include "board_display.hh"

class BoardViewControl final : public BoardView, public BoardControl {
  class Cell {
    CellType           _type{WATER};
    std::optional<int> _ship_id{nullopt};

   public:
    /** Default constructor: creates an unkown/Water cell */
    constexpr Cell() = default;
    constexpr Cell(CellType type, std::optional<int> ship_id);
    [[nodiscard]] constexpr inline CellType           type() const;
    [[nodiscard]] constexpr inline std::optional<int> shipId() const;
  };

  std::weak_ptr<BoardDisplay> _display{};
  uint8_t                     _game_state{0};
  string                      _input_result="";
  bool                        _left_turn{true};
  bool                        _is_finished{false};
  bool                        _is_victory{false};

  array<array<Cell, 10>, 10> _side_left,
                             _side_right;

  multiset<BoatType>         _boats_left  = {TWO_BY_ONE, TWO_BY_ONE, THREE_BY_ONE, FOUR_BY_ONE, FIVE_BY_ONE},  // Hard-coded for this implementation
                             _boats_right = {TWO_BY_ONE, TWO_BY_ONE, THREE_BY_ONE, FOUR_BY_ONE, FIVE_BY_ONE};

  set<int>                   _used_id_left,
                             _used_id_right;

  [[nodiscard]] Cell getCell(bool left_turn, BoardCoordinates position) const;

  void setCell(bool left_turn, BoardCoordinates position, Cell cell);

  void removeFromInventory(BoatType boatType);

  [[nodiscard]] std::optional<int> shipId(bool left_side, BoardCoordinates position) const;

  [[nodiscard]] bool check() const;

 public:
  BoardViewControl();
  BoardViewControl(const BoardViewControl&)            = delete;
  BoardViewControl(BoardViewControl&&)                 = delete;
  BoardViewControl& operator=(const BoardViewControl&) = delete;
  BoardViewControl& operator=(BoardViewControl&&)      = delete;

  [[nodiscard]] uint8_t  gameState()    const override;
  [[nodiscard]] string   inputResult()  const override;
  [[nodiscard]] bool     leftTurn()     const override;
  [[nodiscard]] bool     isFinished()   const override;
  [[nodiscard]] bool     isVictory()    const override;
  [[nodiscard]] size_t   width()        const override;
  [[nodiscard]] size_t   height()       const override;
  [[nodiscard]] multiset<BoatType>
                         getInventory() const override;
  [[nodiscard]] CellType cellType(bool             left_side,
                                  BoardCoordinates position) const override;
  [[nodiscard]] bool isSameShip(bool left_side, BoardCoordinates first,
                                BoardCoordinates second) const override;
  [[nodiscard]] bool rotate(vector<BoardCoordinates> ship_cells, int d);
  [[nodiscard]] bool confirm(vector<BoardCoordinates> ship_cells);
  [[nodiscard]] bool select(char boat_type) override;
  [[nodiscard]] bool move(char input) override;
  [[nodiscard]] bool fire(const BoardCoordinates position) override;
  void quit() override;

  ~BoardViewControl() override = default;
};