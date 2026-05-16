#include <iomanip>
#include <limits>
#include <iostream>

#include "../common/board_common.hh"
#include "../common/network_io.hh"
#include "../common/utils.hh"
#include "console_display.hh"

#include "console_board_display.hh"

using namespace NM;
namespace ranges = std::ranges;
using std::string, std::string_view;
using Color = NM::setc::Color;

string ConsoleGameDisplay::createHeader() const {
  //                   ╔═════════════╗
  //                   ║ Left's Turn ║
  //                   ╚═════════════╝

  // 2de line:
  string who;
  string turn;
  auto&& [is_left, you, other] = _board->getNames();
  if (_board->isFinished()) {
    switch (_board->victor()) {
      using enum GameModel::Victor;
      case STALEMATE:
        who = "   Stalemate !   ";
        break;
      case LEFT:
        who = (is_left ? you : other) + " has Won!";
        break;
      case RIGHT:
        who = (is_left ? other : you) + " has Won!";
        break;
      case REPLAY:
        who = " Replay ";
        break;
      case NONE:
      default:
        throw std::runtime_error("Board has an invalid victor");
    }
  } else {
    who = _board->leftTurn() ? you : other;
    who += "'s Turn";
  }
  turn = "║ " + who + " ║";

  // margin:
  size_t margin_size = length(turn) > total_width ? 0 : (total_width - length(turn)) / 2;
  string margin(margin_size, ' ');
  // 1st and 3rd line:
  std::ostringstream oss;
  oss << (string("═") * (length(turn) - 2));
  string line = oss.str();
  oss.str("");  // clear oss

  // Recolor after getting length
  turn = NM::setc{Color::GREEN} + "║ " + NM::setc{} + who + NM::setc{Color::GREEN} + " ║" + NM::setc{};

  // Result:
  oss << margin << NM::setc{Color::GREEN} << "╔" << line << "╗\n"   << NM::setc{}
      << margin << turn << '\n'
      << margin << NM::setc{Color::GREEN} << "╚" << line << "╝\n\n" << NM::setc{};
  return oss.str();
}

string ConsoleGameDisplay::createGridLabel(bool my_side) const {
  auto&& [_, you, other] = _board->getNames();
  string player_left  = you   + "'s Fleet";
  string player_right = other + "'s Fleet";
  size_t label_size   = std::max(length(player_left), length(player_right));
  size_t margin_size  = label_size > grid_width ? 0 : (grid_width - label_size) / 2;
  string margin(margin_size, ' ');
  return margin + (my_side ? player_left : player_right);
}

vector<string> ConsoleGameDisplay::createGrid(bool my_side) const {
  vector<string>     grid;
  std::ostringstream oss("    ", std::ios_base::ate);

  // letters
  for (size_t i = 0; i < _board->width(); ++i) {
    oss << std::setw(string_width) << BoardCoordinates{i, 0}.xToString() << ' ';
  }
  grid.emplace_back(oss.str());

  // first line
  oss.str("   ┌");
  oss << (((string("─") * string_width) + "┬") * (_board->width() - 1));
  oss << "─┐";
  grid.emplace_back(oss.str());
  // body
  for (unsigned i = 0; i < _board->height(); ++i) {
    oss.str("");
    oss << std::setw(number_width) << i + 1 << " ";
    for (unsigned j = 0; j < _board->width(); ++j) {
      string border = "│";
      if (!my_side && _board->getSelectionCell({j, i}) == ClientView::OverlayCell::RETICLE) {
        oss << border << NM::setc{Color::GREEN} << "█" << NM::setc{};
      } else {
        GameModel::CellType content  = _board->getCell(my_side, {j, i});
        bool check = (j > 0 && _board->isSameShip({j, i}, {j - 1, i}, my_side));
        if (check) {
          GameModel::CellType previous = _board->getCell(my_side, {j - 1, i});
          auto b = best(content, previous);
          if (b == GameModel::CellType::WATER)
            border = "│";
          else
            border = toString(b);
        }
        oss << border << toString(content);
      }
    }
    oss << "│";
    grid.emplace_back(oss.str());
  }

  // last line
  oss.str("   └");
  oss << (((string("─") * string_width) + "┴") * (_board->width() - 1));
  oss << "─┘";
  grid.emplace_back(oss.str());

  return grid;
}

vector<string> ConsoleGameDisplay::formatInventory(span<Boat::Type const> inventory) {
  return {
    (" > 1: " + std::to_string(ranges::count(inventory, Boat::Type::Destroyer))   + " Destroyer(s)    <"),
    (" > 2: " + std::to_string(ranges::count(inventory, Boat::Type::Cruiser))     + " Cruiser(s)      <"),
    (" > 3: " + std::to_string(ranges::count(inventory, Boat::Type::Battleship))  + " Battleship(s)   <"),
    (" > 4: " + std::to_string(ranges::count(inventory, Boat::Type::Carrier))     + " Carrier(s)      <"),
    (" > 5: " + std::to_string(ranges::count(inventory, Boat::Type::Z_Tetromino)) + " Z Tetromino(es) <"),
    (" > 6: " + std::to_string(ranges::count(inventory, Boat::Type::J_Tetromino)) + " J Tetromino(es) <"),
    (" > 7: " + std::to_string(ranges::count(inventory, Boat::Type::T_Tetromino)) + " T Tetromino(es) <")
  };
}

vector<string> ConsoleGameDisplay::createPrompt(size_t padding) const {
  vector<string> prompt(padding == 0 ? 0 : padding - 1, "");

  switch (_board->gameState()) {
    using enum GameModel::GameStage;
    case FACTIONSELECT:
      prompt.emplace_back(">> SELECT FACTION <<");
      break;
    case SELECTION:
      prompt.emplace_back(">>  SELECT BOAT  <<");
      break;
    case PLACEMENT:
      prompt.emplace_back(">>   PLACE BOAT  <<");
      break;
    case WAITING:
      break;
    case ATTACKSELECT:
      if (_board->getGamemode() == GameModel::GameMode::COMMANDERS)
        prompt.emplace_back("Energy Left : " + std::to_string(_board->getEnergy()));
      prompt.emplace_back(">> SELECT ATTACK <<");
      break;
    case COMBAT:
      if (_board->getGamemode() == GameModel::GameMode::COMMANDERS)
        prompt.emplace_back("Energy Left : " + std::to_string(_board->getEnergy()));
      prompt.emplace_back(">> SELECT TARGET <<");
      break;
    default:
      throw NotImplementedError("Game stage not implemented");
  }
  
  prompt.emplace_back(">> ");
  return prompt;
}

void ConsoleGameDisplay::printSideBySide(vector<string> left, vector<string> right) {
  size_t left_width = std::max(
      grid_width,
      ranges::max(left, {}, [](string_view s) noexcept { return length(s); }).size()
  );
  size_t idx = 0;
  size_t last_line = std::max(left.size(), right.size());
  string space(left_width, ' ');

  for (idx = 0; idx < last_line; ++idx) {
    // Left
    if (idx < left.size()) {
      output << std::left << left.at(idx);
      if (length(left.at(idx)) < left_width) {
        output << string(left_width - length(left.at(idx)), ' ');
      }
    } else {
      output << space;
    }
    // Right (and grid_gap)
    if (idx < right.size()) {
      output << grid_gap << right.at(idx);
    }
    // New line
    if (idx < last_line - 1) {
      output << '\n';
    }
  }
}

ConsoleGameDisplay::ConsoleGameDisplay(std::ostream& out, std::istream& in,
                                       std::shared_ptr<ClientView const> board,
                                       std::shared_ptr<ClientControl>    control,
                                       ClientTimer* timer) 
  : GameDisplay{std::move(board), std::move(control), timer},
    ConsoleDisplay{out, in},
    string_width{static_cast<uint8_t>(length(
        BoardCoordinates(_board->width() - 1, _board->height() - 1).xToString()))},
    number_width{static_cast<uint8_t>(length(
        BoardCoordinates(_board->width() - 1, _board->height() - 1).yToString()))},
    grid_gap{"   "},
    grid_width{number_width + 1 + (1 + string_width) * _board->width() + 1},
    total_width{grid_width * 2 + grid_gap.size()},
    cell_legend{createMapLegend()},
    factions_legend{createFactionsLegend()},
    controls_legend{createControlsLegend()},
    abilities_legend{createAbilitiesLegend()},
    selection_legend{createAttackSelectionLegend()} {}

Message ConsoleGameDisplay::handleInput() {
  string line;
  std::getline(std::cin, line);


#ifndef GUI
  first_refresh = true;
#endif
  if (std::cin.eof()) {
    output << std::endl;
    _control->quit();
    return {};
  }

  if (line.size() == 0) return {};

  if (_board->isFinished()) {
      return _control->goToMenu(line);
  };

  switch (_board->gameState()) {
    using enum GameModel::GameStage;
    case FACTIONSELECT:
      return _control->factionSelect(line);
    case SELECTION:
      return _control->select(line);
    case PLACEMENT:
      return _control->move(line.at(0));
    case WAITING:
      return {};
    case ATTACKSELECT:
      return _control->selectAbility(line);
    case SPECTATING:
    case OTHERTURN:
      return {};
    case COMBAT:
      return _control->fire(line.at(0));
    default:
      break;
  }

  throw std::runtime_error("Board has an invalid state");
}

void ConsoleGameDisplay::update() {
#ifndef GUI
  if (!first_refresh && timer) {
    output << "\033[s";
    output << '\r';
    output << "\033[29C";
    output << "\033[1J";
    output << "\033[H";
  } else {
    std::system("clear");  // Don't touch this either
  }
#endif

  if (_board->gameState() != GameModel::GameStage::SPECTATING)
    output << "Time remaining: " << timer->elapsed() << "\n\n";
  
  output << createHeader();
  if (_board->gameState() != GameModel::GameStage::FACTIONSELECT){
    printSideBySide({createGridLabel(true)}, {createGridLabel(false)});
    output << '\n';
    printSideBySide(createGrid(true), createGrid(false));
    output << '\n';
  }
  
  if (_board->leftTurn() && !_board->isFinished())
    switch (_board->gameState()) {
      using enum GameModel::GameStage;
      case FACTIONSELECT:
        // output << ">> " << _board->inputResult() << '\n';
        printSideBySide(factions_legend, createPrompt(factions_legend.size()));
        break;
      case SELECTION:
        // output << ">> " << _board->inputResult() << '\n';
        printSideBySide(formatInventory(_board->getInventory()), createPrompt(formatInventory(_board->getInventory()).size()));  // Meh
        break;
      case PLACEMENT:
        // output << ">> " << _board->inputResult();
        printSideBySide(controls_legend, createPrompt(controls_legend.size()));
        break;
      case WAITING:
        output << "\nWaiting for opponent...\n";
        break;
      case SPECTATING:
        output << "\nSpectating\n";
        break;
      case ATTACKSELECT:
        printSideBySide(abilities_legend, createPrompt(abilities_legend.size()));
        break;
      case COMBAT:
        // output << ">> " << _board->inputResult() << '\n';
        printSideBySide(selection_legend, createPrompt(selection_legend.size())); 
        // printSideBySide(cell_legend, createPrompt(cell_legend.size()));
        break;
      default:
        throw NotImplementedError("Game stage not implemented: 312 csb");
    }
#ifndef GUI
  if (!first_refresh && timer)
    output << "\033[u";

  if (_board->isFinished())
    output << "\n> Leave: '/q'\n";
  output << std::flush;

  first_refresh = false;
#endif
}
