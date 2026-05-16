#pragma once

#include <chrono>

namespace chrono = std::chrono;

class Timer {
 public:
  enum class Type {
    GLOBAL,
    TURN
  };

  enum class Who {
    RIGHT,
    LEFT,
    NONE
  };

  static constexpr auto DEFAULTLIMIT = chrono::seconds(1800);
  static constexpr auto DEFAULTTURN  = chrono::seconds(60);

 protected:
  using time_point = chrono::time_point<chrono::system_clock>;

  bool running;
  Type t;
  Who w;

  Timer(Type _t) : running{false}, t{_t}, w{Who::NONE} {}

public:
  [[nodiscard]] inline Type type() const { return t; }
  [[nodiscard]] inline Who  kind() const { return w; }

  inline void swap(Who _w) { w = _w; }
  inline void stop()       { running = false; }

  [[nodiscard]] inline bool done() const { return !running; }
};
