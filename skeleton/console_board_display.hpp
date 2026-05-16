#include "utils.hpp"
#include "board_display.hpp"
#include "board_view.hpp"
#include "board_control.hh"

class ConsoleBoardDisplay final : public BoardDisplay {
  std::ostream&                          _out;
  std::istream&                          _in;
  std::shared_ptr<BoardView const> const _board;
  std::shared_ptr<BoardControl> const    _control;

  uint8_t const _letter_width;
  uint8_t const _number_width;

  string const   _gap;
  size_t const   _grid_width;
  size_t const   _width;
  vector<string> _map_key;
  vector<string> _controls;


  static constexpr size_t length(const string& s);

  static string toString(BoardView::CellType type);

  [[nodiscard]] string createHeader() const;

  [[nodiscard]] string createGridLabel(bool my_side) const;

  [[nodiscard]] vector<string> createGrid(bool my_side) const;

  [[nodiscard]] static vector<string> createMapKey();

  [[nodiscard]] static vector<string> createControls();

  [[nodiscard]] vector<string> createInventory(multiset<BoardView::BoatType> inventory);

  [[nodiscard]] vector<string> createPrompt() const;

  void print(const vector<string>& lines);

  void printSideBySide(vector<string> left, vector<string> right);

  void clearBadInput();

 public:
  ConsoleBoardDisplay() = delete;

  ConsoleBoardDisplay(std::ostream& out, std::istream& in,
                      std::shared_ptr<BoardView const> board,
                      std::shared_ptr<BoardControl>    control);

  ConsoleBoardDisplay(const ConsoleBoardDisplay&)                  = default;
  ConsoleBoardDisplay(ConsoleBoardDisplay&&)                       = default;
  ConsoleBoardDisplay& operator=(const ConsoleBoardDisplay& other) = delete;
  ConsoleBoardDisplay& operator=(ConsoleBoardDisplay&&)            = delete;

  void update() override;

  /** Parse coordinates provided by user, check boundaries and call
   * BoardControl::fire. */
  void handleInput() override;


  ~ConsoleBoardDisplay() override = default;
};