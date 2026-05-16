#include "client_timer.hh"

#include <stdexcept>
#include <iomanip>

ClientTimer::~ClientTimer() {
  if (clock_fd != -1) close(clock_fd);
}

void ClientTimer::make(Type _t, chrono::seconds _limit, chrono::seconds _turn_limit) {
  t = _t;
  seconds_game = _limit;
  seconds_turn = _turn_limit;
  seconds_turn_limit = _turn_limit;
}

string ClientTimer::elapsed() const {
  std::time_t time = seconds_game.count();
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&time), "%T");
  if (t == Timer::Type::TURN) {
    oss << "\nThis turn: ";
    std::time_t turn_time = seconds_turn.count();
    oss << std::put_time(std::gmtime(&turn_time), "%M:%S");
  }
  return oss.str();
}

void ClientTimer::start() {
  running = true;
  turn_done = false;
  seconds_turn = seconds_turn_limit;
  setTime({ {INTERVAL}, {INTERVAL} });
}

void ClientTimer::swap(bool on) {
  if (t == Type::GLOBAL)
    return;
  on ? start() : stop();
}

void ClientTimer::stop() {
  running = false;
  setTime({ {0}, {0} });
}

void ClientTimer::update() {
  int64_t intervals;
  read(clock_fd, &intervals, sizeof(int64_t));
  seconds_game -= chrono::seconds(intervals);
  if (t == Timer::Type::TURN)
    seconds_turn -= chrono::seconds(intervals);
  if (0 >= seconds_turn.count())
    turn_done = true;
  if (0 >= seconds_game.count())
    running = false;
}

void ClientTimer::setTime(const itimerspec& time) {
  if (timerfd_settime(clock_fd, 0, &time, NULL) == -1) {
    close(clock_fd);
    clock_fd = -1;
  }
}