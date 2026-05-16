#include "utils.hh"

std::optional<int> NM::from_string(std::string_view string) {
    int value;

    if (std::from_chars(string.data(), string.data() + string.size(), value).ec == std::errc{})
        return value;
    else
        return std::nullopt;
}

NM::ReusableThread::ReusableThread() : worker(std::bind_front(&ReusableThread::run, this)) {}

void NM::ReusableThread::run(std::stop_token token) {
  std::stop_callback callback(token, [&] { condition.notify_all(); });
  std::unique_lock   lock(thread_mutex);

  while (!token.stop_requested()) {
    condition.wait(lock, [&] { return !tasks.empty() || token.stop_requested(); });
    if (tasks.empty() && token.stop_requested())
      break;

    tasks.front()();
    tasks.pop();
  }
}
