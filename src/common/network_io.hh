#pragma once

#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <span>

constexpr uint16_t MAXSHORT = static_cast<uint16_t>(-1);  // 65535
constexpr uint16_t PORT = 28772;

inline bool is_interrupted = false;

//                   ╔═══════════════╗
//                   ║ I/O Functions ║
//                   ╚═══════════════╝

namespace NM {
  class Message;  // Forward declaration from serializer.hh
}

class Networkable {
 public:
  enum class Request : uint8_t {
    LOGIN,
    REGISTER,
    LOGOUT,
    DISCONNECT,
    ACCEPT_GAME,
    REJECT_GAME,

    // Friend-specific
    UPDATE_RELATIONSHIPS,
    CHAT_MESSAGE,
    LOAD_CHAT,
    JOINONINVITE,

    // Lobby-specific
    HOST,
    JOIN,
    GET_MATCHES,
    QUIT_LOBBY,
    UPDATE_LOBBY,
    UPDATE_LOBBY_MEMBER,
    START_GAME,
    START_SPECTATING,
    INVITE,

    // Game-specific
    GAME,
    GAMEOVER,
    OUT_OF_TIME,
    BACK_TO_LOBBY,
    RECORDING,

    R_SENTINEL
  };

  struct ILLEGALCHARACTERS {
    static constexpr char SPACE = ' ';
  };

 protected:
  // Errno is set in case of errors

  void write_message(int recipient, NM::Message&& message);
  NM::Message read_message(int sender);
};

//                   ╔═════════════════╗
//                   ║ Signal Handling ║
//                   ╚═════════════════╝

inline void signal_handler(int sig) {
  if (sig == SIGINT || SIGHUP || SIGTERM) is_interrupted = true;
}

inline int set_sigaction() {  // We might need to eventually make two of these
  struct sigaction action {
    .__sigaction_handler{signal_handler}
  };

  if (sigemptyset(&action.sa_mask) == -1) return -1;
  if (sigaction(SIGINT, &action, NULL) == -1) return -1;
  if (sigaction(SIGHUP, &action, NULL) == -1) return -1;
  if (sigaction(SIGTERM, &action, NULL) == -1) return -1;

  return 0;
}