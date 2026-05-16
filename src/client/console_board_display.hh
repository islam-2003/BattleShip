#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <span>
#include <iostream>
#include <experimental/memory>

#include "client_timer.hh"
#include "client_board.hh"
#include "console_display.hh"
#include "display_common.hh"
#include "../common/not_implemented_error.hh"
#include "../common/serializer.hh"

namespace epr = std::experimental;
namespace ranges = std::ranges;
using std::vector;
using std::span;

/** BoardDisplay using text.
 *
 * A grid is a side of the board as represented on the screen.
 * Draw both sides of the board as two grids side by side. */
class ConsoleGameDisplay : public GameDisplay, public ConsoleDisplay {
private:
  [[nodiscard]] static constexpr inline GameModel::CellType best(GameModel::CellType lhs, GameModel::CellType rhs) {
    using enum GameModel::CellType;
    if (!(lhs & IS_SHIP) || !(rhs & IS_SHIP)) {
      // std::cerr << "BoardView::best(" << static_cast<unsigned>(lhs) << ", "
      //   << static_cast<unsigned>(rhs) << ")" << std::endl;
      return WATER;
    }
    return lhs <= rhs ? lhs : rhs;
  }

  [[nodiscard]] static constexpr vector<string> createMapLegend() {
    using enum GameModel::CellType;
    return {
      (" > " + toString(OCEAN) + " Ocean          <"),
      (" > " + toString(UNDAMAGED) + " Undamaged ship <"),
      (" > " + toString(HIT) + " Hit ship       <"),
      (" > " + toString(SUNK) + " Sunk ship      <")
    };
  }

  [[nodiscard]] static constexpr vector<string> createFactionsLegend() {
    return {
      (" > 1: Pirate <"),
      (" > 2: Captain  <")
    }; 
  }

  [[nodiscard]] static constexpr vector<string> createControlsLegend() {
    return {
      (" > ZSQD - Movement <"),
      (" >    A - Rotate L <"),
      (" >    E - Rotate R <"),
      (" >    C - Confirm  <")
    };
  }

    [[nodiscard]] static constexpr vector<string> createAttackSelectionLegend() {
    return {
      (" > Z - UP <"),
      (" > S - DOWN <"),
      (" > Q - LEFT <"),
      (" > D - RIGHT <"),
      (" > C - Confirm  <")
    };
  }

  [[nodiscard]] static constexpr vector<string> createAbilitiesLegend() {
    return {
      (" > 1: Basic Ability (1E) <"),
      (" > 2:   Ability 1   (1E) <"),
      (" > 3:   Ability 2   (1E) <"),
    };
  }

  [[nodiscard]] static constexpr string toString(GameModel::CellType type) {
    switch (type) {
      using enum GameModel::CellType;
      case WATER:
        return " ";
      case OCEAN:
        return "╳";
      case UNDAMAGED:
        return "█";
      case HIT:
        return "▒";
      case SUNK:
        return "░";
      default:
        throw NotImplementedError("ConsoleGameDisplay unknown CellType");
    }
  }

  uint8_t const string_width;
  uint8_t const number_width;

  string const   grid_gap;
  size_t const   grid_width;
  size_t const   total_width;
  vector<string> cell_legend;
  vector<string> factions_legend;
  vector<string> controls_legend;
  vector<string> abilities_legend;
  vector<string> selection_legend;

  /** Create a header indicating who should play */
  [[nodiscard]] string createHeader() const;

  /** Create a grid label: Your / Their Fleet */
  [[nodiscard]] string createGridLabel(bool my_side) const;

  /** Create grid for a given side, each string is a line without ending '\n' */
  [[nodiscard]] vector<string> createGrid(bool my_side) const;

  /** Converts given inventory to string representation, for printing */
  [[nodiscard]] vector<string> formatInventory(span<Boat::Type const> inventory);

  /** Create user input prompt, each string is a line without ending '\n'.
   * Empty lines are added in the beginning of the prompt so it can be printed next to the
   * map key. */
  [[nodiscard]] vector<string> createPrompt(size_t padding) const;

  /** Print both parts side by side with grid_gap in between.
   * left and right are vector of lines without ending '\n'.
   * Both parts are fully printed, even if one part is shorter than the other.
   * If the left-hand part has lines of varying sizes, padding is added to ensure that the
   * right-hand part is properly aligned. Moreover, padding is also added so that the left
   * part is at least as wide as a grid.
   * The last line is printed without final '\n'. */
  void printSideBySide(vector<string> left, vector<string> right);

 public:
  ConsoleGameDisplay(std::ostream& out, std::istream& in,
                     std::shared_ptr<ClientView const> board,
                     std::shared_ptr<ClientControl>    control,
                     ClientTimer* timer);

  [[nodiscard]] NM::Message handleInput();
  void update();
};
