#pragma once

#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN && __x86_64__

#include <type_traits>
#include <vector>
#include <string>
#include <stdexcept>
#include <optional>
#include <span>
#include <tuple>
#include <iostream>

#include "network_io.hh"
#include "board_common.hh"
#include "lobby_common.hh"
#include "boat.hh"
#include "ability.hh"

namespace NM {

  /**
   * Empty struct denoting a serializable object.
   * Any class used in NM::Message inherits this.
   */
  struct serializable_t {};

  template<typename T>
  concept Serializable = std::is_base_of_v<serializable_t, T>;

  /**
   * Exception to be thrown when deserialization
   * encounters a problem.
   * 
   * Currently unused. Throw it in from_bytes functions
   * and catch in extract() for extra safety.
   */
  class MangledBytesError : public std::runtime_error {
   public:
    explicit MangledBytesError(const std::string& what)
      : std::runtime_error("Bytestream is mangled: " + what) {}
  };

  /**
   * Message class intended to be serialized and sent
   * to remotes. Body is optional and can be deserialized 
   * into defined classes.
   */
  class Message {
    using Request = Networkable::Request;

    enum class BodyType : uint64_t {
      NOTHING,
      CREDENTIALS,
      RELATION_UPDATE,
      CHAT_LOG,
      CHAT_UPDATE,
      ACCOUNT,
      MATCHES,
      HOST_MATCH,
      JOIN_MATCH,
      CHANGE_SLOT,
      LOBBY_DETAILS,
      FACTION,
      BOAT_SELECTION,
      CONFIRMATION,
      START_COMBAT,
      ABILITY_SELECTION,
      CLIENT_FIRE,
      SERVER_FIRE,
      GAME_END,
      RECORDING
    };

    bool is_empty;
    Request req;
    BodyType type;
    std::vector<std::byte> body;

    constexpr Message(Request&& req, BodyType&& type, std::vector<std::byte>&& body)
      : is_empty{false}, req{req}, type{type}, body{body} {}

   public:
    using Pair = std::pair<int, Message>;
    using Multi = std::vector<Pair>;
    using Double = std::pair<Message, Message>;
    using Triple = std::tuple<Message,Message,Message>;

    [[nodiscard]] constexpr inline Request request() const { return req; }

    [[nodiscard]] static std::vector<std::byte> serialize(Message&& message);
    [[nodiscard]] static Message              deserialize(std::vector<std::byte>&& data);

    [[nodiscard]] constexpr inline bool empty() const { return is_empty; }

    Message(Request req, Serializable auto&& content) 
                         : is_empty{false}, req{req}, type{content.getType()}, body{content.serialize()} {}
    Message(Request req) : is_empty{false}, req{req}, type{BodyType::NOTHING}, body{} {}
    constexpr Message()  : is_empty{true}, req{}, type{}, body{} {}

    std::span<std::byte const> get_body() const { return body; }

    /**
     * Deserialize contents from message body.
     * Returns std::nullopt if there is a type mismatch.
     * 
     * \return std::optional object of valid type.
     */
    template<Serializable T>
    std::optional<T> extract() const {
      try {
        if (T::getType() == type) {
          uint64_t offset = 0;
          return T::deserialize(body, offset);
        }
      } catch (const MangledBytesError& e) {
        std::cerr << "Deserialization error: " << e.what() << std::endl;
      }
      return std::nullopt;
    }

    //    ╔════════════════════════════╗
    //    ║ Message Object Definitions ║
    //    ╚════════════════════════════╝

    class Credentials : serializable_t {
     public:
      constexpr Credentials(std::string_view name, std::string_view password)
        : name{name}, password{password} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(name, password); }

     private:
      std::string name;
      std::string password;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CREDENTIALS; }
        std::vector<std::byte> serialize() const;
        static Credentials   deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class Relationship : serializable_t {
     public:
      enum class Kind : uint8_t {
        SENDING,
        RECEIVING,
        ACCEPTING,
        REJECTING,
        REMOVING,
        JOINGAME,
        SENDINGGAME
      };

      constexpr Relationship(std::string_view you, std::string_view other, Kind k)
        : you{you}, other{other}, k{k} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(you, other, k); }

     private:
      std::string you;
      std::string other;
      Kind k;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::RELATION_UPDATE; }
        std::vector<std::byte> serialize() const;
        static Relationship  deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class ChatLog : serializable_t {
     public:
       constexpr ChatLog(std::string_view recipient, std::span<std::string const> log) : recipient{recipient}, log{log.begin(), log.end()} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(recipient, log); }

     private:
      std::string recipient;
      std::vector<std::string> log;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CHAT_LOG; }
        std::vector<std::byte> serialize() const;
        static ChatLog       deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class ChatUpdate : serializable_t {
     public:
      constexpr ChatUpdate(std::string_view sender, std::string_view receiver, std::string_view line)
        : sender{sender}, receiver{receiver}, line{line} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(sender, receiver, line); }

     private:
      std::string sender;
      std::string receiver;
      std::string line;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CHAT_UPDATE; }
        std::vector<std::byte> serialize() const;
        static ChatUpdate    deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class Account : serializable_t {
     public:
      constexpr Account(std::string_view username) : username{username} {}

      constexpr
      Account(std::string_view username,
              std::span<std::string const> friends,
              std::span<std::string const> inbound,
              std::span<std::string const> outbound,
              std::span<std::string const> game_requests) // Maybe add construct /w game_request ? 
        : username{username},
          friends{friends.begin(), friends.end()},
          inbound{inbound.begin(), inbound.end()},
          outbound{outbound.begin(), outbound.end()},
          game_requests{game_requests.begin(), game_requests.end()} {}

      void pushRelationships(std::span<std::string const> friends,
                             std::span<std::string const> inbound,
                             std::span<std::string const> outbound,
                             std::span<std::string const> game_requests);

      [[nodiscard]] constexpr inline auto data() const { return std::tie(username, friends, inbound, outbound, game_requests); }

     private:
      std::string username;
      std::vector<std::string> friends;
      std::vector<std::string> inbound;
      std::vector<std::string> outbound;
      std::vector<std::string> game_requests;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::ACCOUNT; }
        std::vector<std::byte> serialize() const;
        static Account       deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class Matches : serializable_t {
     public:
      struct Match {
        std::array<char, Lobby::LOBBY_NAME_MAXSIZE> name;
        uint8_t players;
        bool started;
        bool password;
      };

      constexpr Matches() : matches{} {}

      constexpr Matches(std::span<Match const> matches) : matches{matches.begin(), matches.end()} {}

      constexpr inline void push_back(const auto& match) { matches.push_back(match); }

      constexpr inline void clear() noexcept { matches.clear(); }

      constexpr inline void shrink_to_fit() { matches.shrink_to_fit(); }

      constexpr inline std::vector<Match> getMatch() { return matches; }
     private:
      std::vector<Match> matches;

      friend std::ostream& operator<<(std::ostream& output, const Matches& matches);

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::MATCHES; }
        std::vector<std::byte> serialize() const;
        static Matches       deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class HostLobby : serializable_t {
     public:
      constexpr HostLobby() = default;

      constexpr HostLobby(std::string_view _name, std::string_view password) : name{}, password{password} { ranges::copy(_name, name.data()); }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(name, password); }

      constexpr inline void clear() noexcept { password.clear(); }

      constexpr inline void shrink_to_fit() { password.shrink_to_fit(); }

     private:
      std::array<char, Lobby::LOBBY_NAME_MAXSIZE> name;
      std::string password;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::HOST_MATCH; }
        std::vector<std::byte> serialize() const;
        static HostLobby     deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class SlotLobby : serializable_t {
     public:
      enum class Slot {
        SPECTATOR,
        LEFT,
        RIGHT,
        QUITTING
      };

      constexpr SlotLobby(std::string_view name, Slot s = Slot::SPECTATOR) : name{name}, s{s} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(name, s); }

     private:
      std::string name;
      Slot s;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CHANGE_SLOT; }
        std::vector<std::byte> serialize() const;
        static SlotLobby     deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class LobbyParameters : serializable_t {
     public:
      constexpr LobbyParameters() = default;

      constexpr LobbyParameters(Lobby::parameter_t data)
        : game_time{std::get<0>(data)}, turn_time{std::get<1>(data)}, tt{std::get<2>(data)}, gt{std::get<3>(data)} { }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(game_time, turn_time, tt, gt); }

     private:
      chrono::seconds game_time;
      chrono::seconds turn_time;
      Timer::Type tt;
      GameModel::GameMode gt;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::LOBBY_DETAILS; }
        std::vector<std::byte>   serialize() const;
        static LobbyParameters deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class JoinLobby : serializable_t {
     public:
      constexpr JoinLobby(HostLobby lobby, LobbyParameters params, std::span<SlotLobby const> clients = {})
        : lobby{lobby}, params{params}, clients{clients.begin(), clients.end()} { }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(lobby, params, clients); }

      constexpr inline void push_back(const SlotLobby& client) { clients.push_back(client); }

     private:
      HostLobby lobby;
      LobbyParameters params;
      std::vector<SlotLobby> clients;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::JOIN_MATCH; }
        std::vector<std::byte> serialize() const;
        static JoinLobby     deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    //    ╔═══════════════════════╗
    //    ║ Game-specific Classes ║
    //    ╚═══════════════════════╝

    class Faction : serializable_t {
     public:
      constexpr Faction(GameModel::Faction fac) : fac{fac} {}

      [[nodiscard]] constexpr inline auto data() const { return fac; }

     private:
      GameModel::Faction fac;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::FACTION; }
        std::vector<std::byte> serialize() const;
        static Faction       deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class BoatSelection : serializable_t {
     public:
      constexpr BoatSelection(Boat::Type type) : type{type} {}

      [[nodiscard]] constexpr inline auto data() const { return type; }

     private:
      Boat::Type type;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::BOAT_SELECTION; }
        std::vector<std::byte> serialize() const;
        static BoatSelection deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class Confirmation : serializable_t {
     public:
      constexpr Confirmation(std::span<BoardCoordinates const> coordinates, int boat_id, Boat::Type type)
        : coordinates{coordinates.begin(), coordinates.end()}, boat_id{boat_id}, type{type} { }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(coordinates, boat_id, type); }

     private:
      std::vector<BoardCoordinates> coordinates;
      int boat_id;
      Boat::Type type;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CONFIRMATION; }
        std::vector<std::byte> serialize() const;
        static Confirmation  deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class StartCombat : serializable_t {
     public:
      constexpr StartCombat(bool your_turn) : your_turn{your_turn} {}

      [[nodiscard]] constexpr inline auto data() const { return your_turn; }

     private:
      bool your_turn;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::START_COMBAT; }
        std::vector<std::byte> serialize() const;
        static StartCombat   deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class AbilitySelection : serializable_t {
     public:
      constexpr AbilitySelection(Ability::Type type) : type{type} {}

      [[nodiscard]] constexpr inline auto data() const { return type; }

     private:
      Ability::Type type;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::ABILITY_SELECTION; }
        std::vector<std::byte>    serialize() const;
        static AbilitySelection deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class ClientFire : serializable_t {
     public:
      constexpr ClientFire(std::span<BoardCoordinates const> coordinates, Ability::Type type)
        : coordinates{coordinates.begin(), coordinates.end()}, type{type} {}

      [[nodiscard]] constexpr inline auto data() const { return std::tie(coordinates, type); }

     private:
      std::vector<BoardCoordinates> coordinates;
      Ability::Type type;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::CLIENT_FIRE; }
        std::vector<std::byte> serialize() const;
        static ClientFire    deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class ServerFire : serializable_t {
     public:
      struct Cell {
        using State = GameModel::CellType;
        BoardCoordinates c;
        int id;
        State new_state;
      };
      constexpr ServerFire(std::span<Cell const> cells, bool board, bool turn, int new_energy)
        : cells{cells.begin(), cells.end()}, your_board{board}, your_turn{turn}, new_energy{new_energy} { }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(cells, your_board, your_turn, new_energy); }

     private:
      std::vector<Cell> cells;
      bool your_board;
      bool your_turn;
      int new_energy;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::SERVER_FIRE; }
        std::vector<std::byte> serialize() const;
        static ServerFire    deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class GameEnd : serializable_t {
     public:
      constexpr GameEnd(GameModel::Victor victor) : victor{victor} {}

      [[nodiscard]] constexpr inline auto data() const { return victor; }

     private:
       GameModel::Victor victor;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::GAME_END; }
        std::vector<std::byte> serialize() const;
        static GameEnd       deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };

    class Recording : serializable_t {
     public:
      constexpr Recording() = default;
      constexpr Recording(std::string_view left, std::string_view right, std::span<ServerFire const> moves)
        : left{left}, right{right}, moves{moves.begin(), moves.end()} {}

      constexpr inline void push_back(const auto& move) { moves.push_back(move); }

      [[nodiscard]] constexpr inline auto data() const { return std::tie(left, right, moves); }

      // Emergency
      static Recording from_bytes(std::span<std::byte const> bytes) {
        uint64_t offset = 0;
        return deserialize(bytes, offset);
      }

     private:
      std::string left;
      std::string right;
      std::vector<ServerFire> moves;

      friend Message;
        constexpr static inline BodyType getType() { return BodyType::RECORDING; }
        std::vector<std::byte> serialize() const;
        static Recording     deserialize(std::span<std::byte const> bytes, uint64_t& offset);
    };
  };
}
#endif
