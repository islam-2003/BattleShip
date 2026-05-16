#pragma once

#include <sys/timerfd.h>
#include <cstdint>
#include <unistd.h>
#include <chrono>

#include "../common/timer.hh"

namespace chrono = std::chrono;
using std::string;

class ClientTimer : public Timer {
 public:
  ClientTimer() : Timer{Timer::Type::GLOBAL}, turn_done{false} {
    if ((clock_fd = timerfd_create(CLOCK_REALTIME, 0)) == -1) {
      return;
    }
  }
  ~ClientTimer();

  void make(Type _t, chrono::seconds _limit, chrono::seconds _turn_limit);

  [[nodiscard]] inline int  fd() const { return clock_fd; }
  [[nodiscard]] inline bool on() const { return running; }
  [[nodiscard]] inline bool turnDone() const { return turn_done; }

  [[nodiscard]] string elapsed() const;

  void start();
  void swap(bool on);
  void stop();
  void update();

 private:
  static constexpr int INTERVAL = 1;
  
  int clock_fd;
  bool turn_done;

  chrono::seconds seconds_game;
  chrono::seconds seconds_turn;
  chrono::seconds seconds_turn_limit;

  void setTime(const itimerspec& time);
};