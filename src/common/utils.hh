#pragma once

#include <charconv>
#include <string_view>
#include <optional>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <iostream>

namespace NM {
  /**
   * Concept of a type with a defined << operator.
   */
  template<typename T>
  concept ostreamable = requires(T t, std::ostream& os) { os << t; };

  /**
   * Templated struct containing range and a separator,
   * to be passed to overloaded ostream << operator.
   * 
   * \param Any iterable range.
   * \param Separator string.
   */
  template<typename R>
    requires std::ranges::range<R> && ostreamable<std::ranges::range_value_t<R>>
  struct print_range {
    const R& rng;
    const std::string_view separator = "\n";

    friend std::ostream& operator<<(std::ostream& output, print_range&& data) {
      for (auto&& it = data.rng.begin(); it != data.rng.end(); ++it) {
        output << *it;
        if (std::next(it) != data.rng.end())
          output << data.separator;
      }
      return output;
    }
  };

  /**
   * Shorthand template for any enum string conversion.
   * 
   * \param Any variable of enum type.
   * \return Stringified underlying type.
   */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-promo"
  template<typename E>
    requires std::is_enum_v<E>
  [[nodiscard]] inline std::string to_string(E request) { return std::to_string(static_cast<std::underlying_type_t<E>>(request)); }
#pragma GCC diagnostic pop

  /**
   * Shorthand to get the underlying object of an enum.
   * 
   * \param Any variable of enum type.
   * \return Object of underlying type.
   */
  template<typename E>
    requires std::is_enum_v<E>
  [[nodiscard]] inline std::underlying_type_t<E> to_underlying(E request) { return static_cast<std::underlying_type_t<E>>(request); }

  /**
   * Returns an integer if string could be converted
   * or nullopt on failure. Return type can be implicitly
   * converted to other numeric types
   * 
   * \param Any numeric string.
   * \return An optional of type int or nullopt.
   */
  [[nodiscard]] std::optional<int> from_string(std::string_view string);

  /**
   * Streamable structure capable of recoloring terminal output
   * using ANSI escape sequences. Works only on ANSI-supported
   * terminals.
   */
  struct setc {
    enum class Color : uint8_t {
      BLACK = 30,
      RED,
      GREEN,
      YELLOW,
      BLUE,
      MAGENTA,
      CYAN,
      WHITE,
      DEFAULT = 39
    };

    Color&& fg = Color::DEFAULT;
    Color&& bg = Color::DEFAULT;

    std::string operator+(std::string_view rhs) {
      return apply_color(fg, bg) + std::string{rhs};
    }

    friend std::string operator+(std::string_view lhs, setc rhs) {
      return std::string{lhs} + rhs.apply_color(rhs.fg, rhs.bg);
    }

   private:
    static constexpr std::string code   = "\033[";
    static constexpr std::string suffix = "m";

    static std::string apply_color(Color fg, Color bg) {
      return code + to_string(fg) + ";" + std::to_string(to_underlying(bg) + 10) + suffix;
    }

    friend std::ostream& operator<<(std::ostream& output, setc colorizer) {
      output << apply_color(colorizer.fg, colorizer.bg);
      return output;
    }
  };

  /**
   * Structure to recolor a given string.
   * Uses NM::setc.
   */
  struct color_string {
    std::string&& str;
    setc::Color&& fg = setc::Color::DEFAULT;
    setc::Color&& bg = setc::Color::DEFAULT;

   private:
    std::string colorize() const {
      return setc{std::move(fg), std::move(bg)} + str + setc{};
    }

    friend std::ostream& operator<<(std::ostream& output, color_string string) {
      output << string.colorize();
      return output;
    }
  };

  /**
   * Concept of a function that does not return anything.
   */
  template<typename F, typename... Args>
  concept VoidReturn = std::is_invocable_v<F&&> && std::same_as<std::invoke_result_t<F&&, Args&&...>, void>;

  /**
   * std::jthread wrapper that sleeps while there are no tasks in internal queue.
   */
  class ReusableThread {
   public:
    /**
     * Initializes a sleeping thread with an empty queue.
     */
    ReusableThread();

    /**
     * Delegates a new task to the underlying thread.
     * Member functions must be called together with owning object.
     * Tasks must return void.
     *
     * \param Function with void return type.
     * \param Sequence of arguments for function.
     */
    template<typename F, typename... Args>
      requires VoidReturn<F, Args...>
    void push(F&& task, Args&&... args) {
      {
        std::scoped_lock lock(thread_mutex);
        tasks.push(std::bind_front(task, args...));
      }
      condition.notify_all();
    }

    /**
     * Acquire scoped_lock for when a critical task may be running.
     * 
     * \return std::scoped_lock of the internal thread_mutex.
     */
    [[nodiscard]] inline std::scoped_lock<std::mutex> scoped_lock() { return std::scoped_lock{thread_mutex}; }

    ReusableThread(ReusableThread&&)      = delete;
    ReusableThread(const ReusableThread&) = delete;

   private:
    std::queue<std::function<void()>> tasks;

    std::condition_variable condition;
    std::mutex thread_mutex;
    std::jthread worker;

    /**
     * Internal function called by the thread on initialization.
     * 
     * \param The std::stop_token of a std::jthread.
     */
    void run(std::stop_token token);
  };
}
