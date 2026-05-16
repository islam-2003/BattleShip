#include "serializer.hh"

#include <cstring>
#include <iomanip>

#include "utils.hh"

using std::string, std::string_view, std::vector, std::array, std::byte, std::span;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-promo"
namespace NM {

  vector<byte> Message::serialize(Message&& message) {
    if (message.type == BodyType::NOTHING) {
      uint64_t request = static_cast<uint64_t>(message.req);
      return vector<byte>((byte*)&request, (byte*)&request + sizeof(uint64_t));
    } else {
      auto ptr = reinterpret_cast<byte*>(&message.type);
      message.body.insert(message.body.begin(), ptr, ptr + sizeof(uint64_t));
      uint64_t request = static_cast<uint64_t>(message.req);
      message.body.insert(message.body.begin(), (byte*)&request, (byte*)&request + sizeof(uint64_t));
      return message.body;
    }
  }

  Message Message::deserialize(vector<byte>&& data) {
    if (data.size() == sizeof(uint64_t))
      return Message(static_cast<Request>(*data.data()));

    if (data.size() > sizeof(uint64_t) * 2)
      return Message(static_cast<Request>(*data.data()),
                     static_cast<BodyType>(*(data.data() + sizeof(uint64_t))),
                     {data.begin() + (sizeof(uint64_t) * 2), data.end()});
    return Message();
  }

  //    ╔══════════════════╗
  //    ║ Helper Functions ║
  //    ╚══════════════════╝

  /**
   * Round up given value to multiple of 8.
   * 
   * \param Any integer at or under 64 bits.
   * \return Multiple of 8.
   */
  static inline uint64_t pad(uint64_t value) {
    return value + (8 - value % 8) % 8;
  }

  static void append_bytes(vector<byte>& dst, span<byte const> src) {
    dst.insert(dst.end(), src.begin(), src.end());
  }

  // --------- To Bytes ---------

  template<typename T>
    requires std::is_integral_v<T> || std::is_integral_v<std::underlying_type_t<T>>
  static vector<byte> to_bytes(T value) {
    return {(byte*)&value, (byte*)&value + sizeof(T)};
  }

  template<std::ranges::contiguous_range R>
    requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
             && std::negation_v<std::is_convertible<R, string_view>>
             && std::negation_v<std::is_same<R, std::vector<std::byte>>>
  static vector<byte> to_bytes(const R& vec) {
    vector<byte> bytes = to_bytes(vec.size());
    if (vec.empty())
      return bytes;
    bytes.insert(bytes.end(), (byte*)vec.data(),
                              (byte*)vec.data() + (vec.size() * sizeof(std::ranges::range_value_t<R>)));
    return bytes;
  }

  template<typename T, size_t S>
    requires std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>
  static vector<byte> to_bytes(const std::array<T, S>& vec) {
    return vector<byte>{(byte*)vec.data(), (byte*)vec.data() + (S * sizeof(T))};
  }

  // Padded
  static vector<byte> to_bytes(string_view str) {
    vector<byte> bytes = to_bytes(str.size());
    if (str.empty())
     return bytes;
    bytes.insert(bytes.end(), (byte*)str.data(), (byte*)str.data() + str.size());
    bytes.resize(pad(bytes.size()));
    return bytes;
  }

  // Implicitly padded
  static vector<byte> to_bytes(span<string const> vec) {
    vector<byte> bytes = to_bytes(vec.size());
    if (vec.empty())
      return bytes;
    for (string_view str : vec)
      append_bytes(bytes, to_bytes(str));
    return bytes;
  }

  // --------- From Bytes ---------

  template<std::integral T>
  static T to_integral(span<byte const> bytes, uint64_t& offset) {
    if (bytes.size() < offset + sizeof(T))
      throw MangledBytesError("Out of range integral conversion");
    span<byte const> data = bytes.subspan(offset, sizeof(T));
    offset += sizeof(T);
    return *reinterpret_cast<const T*>(data.data());
  }

  template<typename T>
    requires std::is_enum_v<T>
  static T to_enum(span<byte const> bytes, uint64_t& offset) {
    return static_cast<T>(to_integral<std::underlying_type_t<T>>(bytes, offset));
  }

  template<typename T>
    requires std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>
  static vector<T> to_vector(span<byte const> bytes, uint64_t& offset) {
    auto size = to_integral<size_t>(bytes, offset);
    vector<T> vec(size);
    if (size == 0)
      return vec;
    if (bytes.size() < offset + (size * sizeof(T)))
      throw MangledBytesError("Out of range vector conversion");
    auto subspan = bytes.subspan(offset, size * sizeof(T));
    std::memcpy(vec.data(), subspan.data(), subspan.size());
    offset += subspan.size();

    return vec;
  }

  template<typename T, size_t S>
    requires std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>
  static array<T, S> to_array(span<byte const> bytes, uint64_t& offset) {
    array<T, S> arr;

    if (bytes.size() < offset + (S * sizeof(T)))
      throw MangledBytesError("Out of range array conversion");
    auto subspan = bytes.subspan(offset, S * sizeof(T));
    std::memcpy(arr.data(), subspan.data(), subspan.size());
    offset += S;

    return arr;
  }

  // Endian-agnostic
  // Assumes padded
  static uint64_t to_u64(span<byte const> bytes, uint64_t& offset) {
    offset = pad(offset);
    if (bytes.size() < offset + sizeof(8))
      throw MangledBytesError("Out of range u64 conversion");
    span<byte const> data = bytes.subspan(offset, 8);
    offset += sizeof(uint64_t);

    return ((uint64_t)data[0] << 0)  |
           ((uint64_t)data[1] << 8)  |
           ((uint64_t)data[2] << 16) |
           ((uint64_t)data[3] << 24) |
           ((uint64_t)data[4] << 32) |
           ((uint64_t)data[5] << 40) |
           ((uint64_t)data[6] << 48) |
           ((uint64_t)data[7] << 56);
  }

  // Assumes padded
  static string to_string(span<byte const> bytes, uint64_t& offset) {
    offset = pad(offset);
    uint64_t size = to_u64(bytes, offset);

    if (size == 0)
      return "";
     if (bytes.size() < offset + size)
      throw MangledBytesError("Out of range string conversion");

    string str{(char*)bytes.data() + offset, (char*)bytes.data() + offset + size};
    offset = pad(offset + str.size());

    return str;
  }

  // Assumes padded
  static vector<string> to_vector(span<byte const> bytes, uint64_t& offset) {
    offset = pad(offset);
    vector<string> vec(to_u64(bytes, offset));

    for (string& str : vec)
      str = to_string(bytes, offset);

    return vec;
  }

  //    ╔═══════════════════════════════╗
  //    ║ Credentials Class Definitions ║
  //    ╚═══════════════════════════════╝

  vector<byte> Message::Credentials::serialize() const {
    vector<byte> bytes = to_bytes(name);
    append_bytes(bytes, to_bytes(password));

    return bytes;
  }

  Message::Credentials Message::Credentials::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto name     = to_string(bytes, offset);
    auto password = to_string(bytes, offset);
    return Credentials(name, password);
  }

  //    ╔════════════════════════════════╗
  //    ║ Relationship Class Definitions ║
  //    ╚════════════════════════════════╝

  vector<byte> Message::Relationship::serialize() const {
    vector<byte> bytes = to_bytes(you);
    append_bytes(bytes, to_bytes(other));
    bytes.push_back(byte(k));

    return bytes;
  }

  Message::Relationship Message::Relationship::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto you   = to_string(bytes, offset);
    auto other = to_string(bytes, offset);
    auto k     = static_cast<Kind>(bytes[pad(offset)]);
    return Relationship(you, other, k);
  }

  //    ╔═══════════════════════════╗
  //    ║ ChatLog Class Definitions ║
  //    ╚═══════════════════════════╝

  vector<byte> Message::ChatLog::serialize() const {
    vector<byte> bytes = to_bytes(recipient);
    append_bytes(bytes, to_bytes(log));
    return bytes;
  }

  Message::ChatLog Message::ChatLog::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto recipient = to_string(bytes, offset);
    auto log       = to_vector(bytes, offset);
    return ChatLog(recipient, log);
  }

  //    ╔══════════════════════════════╗
  //    ║ ChatUpdate Class Definitions ║
  //    ╚══════════════════════════════╝

  vector<byte> Message::ChatUpdate::serialize() const {
    vector<byte> bytes = to_bytes(sender);

    append_bytes(bytes, to_bytes(receiver));
    append_bytes(bytes, to_bytes(line));

    return bytes;
  }

  Message::ChatUpdate Message::ChatUpdate::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto sender   = to_string(bytes, offset);
    auto receiver = to_string(bytes, offset);
    auto line     = to_string(bytes, offset);
    return ChatUpdate(sender, receiver, line);
  }

  //    ╔═══════════════════════════╗
  //    ║ Account Class Definitions ║
  //    ╚═══════════════════════════╝

  void Message::Account::pushRelationships(std::span<std::string const> _friends,
                                           std::span<std::string const> _inbound,
                                           std::span<std::string const> _outbound,
                                           std::span<std::string const> _game_requests){
    friends.assign(_friends.begin(), _friends.end());
    inbound.assign(_inbound.begin(), _inbound.end());
    outbound.assign(_outbound.begin(), _outbound.end());
    game_requests.assign(_game_requests.begin(), _game_requests.end());
  }

  std::vector<std::byte> Message::Account::serialize() const {
    vector<byte> bytes = to_bytes(username);

    append_bytes(bytes, to_bytes(friends));
    append_bytes(bytes, to_bytes(inbound));
    append_bytes(bytes, to_bytes(outbound));
    append_bytes(bytes, to_bytes(game_requests));

    return bytes;
  }

  Message::Account Message::Account::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto username = to_string(bytes, offset);
    auto friends  = to_vector(bytes, offset);
    auto inbound  = to_vector(bytes, offset);
    auto outbound = to_vector(bytes, offset);
    auto game_requests = to_vector(bytes, offset);
    return Account(username, friends, inbound, outbound, game_requests);
  }

  //    ╔═══════════════════════════╗
  //    ║ Matches Class Definitions ║
  //    ╚═══════════════════════════╝

  std::ostream& operator<<(std::ostream& output, const Message::Matches& matches) {
    using enum setc::Color;
    constexpr static uint8_t MAXSIZE = Lobby::LOBBY_NAME_MAXSIZE;

    size_t id = 1;
    for (auto&& match : matches.matches) {
      output << std::right << std::setw(4)  << id << ". | "
             << std::left  << std::setw(MAXSIZE - 1) << match.name.data() << " | "
             << std::right << std::setw(20)  << color_string{std::to_string(match.players), YELLOW} << "    | "
             << (match.started  ? "  Yes  " : "   No  ") << " | "
             << (match.password ? color_string{"  Yes  ", RED} : color_string{"   No  ", GREEN});
      
      if (id < matches.matches.size())
        output << "\n      | "
               << std::left << std::setw(MAXSIZE - 1) << " "
               << " |         |         |          ";
      output << "\n";
      ++id;
    }
    return output;
  }

  vector<byte> Message::Matches::serialize() const {
    return to_bytes(matches);
  }

  Message::Matches Message::Matches::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto matches = to_vector<Match>(bytes, offset);
    return Matches(matches);
  }

  //    ╔═════════════════════════════╗
  //    ║ HostLobby Class Definitions ║
  //    ╚═════════════════════════════╝

  vector<byte> Message::HostLobby::serialize() const {
    vector<byte> bytes = to_bytes(name);
    bytes.resize(pad(bytes.size()));
    append_bytes(bytes,  to_bytes(password));
    return bytes;
  }

  Message::HostLobby Message::HostLobby::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto name     = to_array<char, Lobby::LOBBY_NAME_MAXSIZE>(bytes, offset);
         offset   = pad(offset);
    auto password = to_string(bytes, offset);
    return HostLobby(name.data(), password);
  }

  //    ╔═════════════════════════════╗
  //    ║ SlotLobby Class Definitions ║
  //    ╚═════════════════════════════╝

  vector<byte> Message::SlotLobby::serialize() const {
    vector<byte> bytes = to_bytes(name);
    append_bytes(bytes, to_bytes(s));
    return bytes;
  }

  Message::SlotLobby Message::SlotLobby::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto name = to_string(bytes, offset);
    auto s    = to_enum<Slot>(bytes, offset);
    return SlotLobby(name, s);
  }

  //    ╔═══════════════════════════════════╗
  //    ║ LobbyParameters Class Definitions ║
  //    ╚═══════════════════════════════════╝

  vector<byte> Message::LobbyParameters::serialize() const {
    vector<byte> bytes = to_bytes(game_time.count());
    append_bytes(bytes,  to_bytes(turn_time.count()));
    append_bytes(bytes,  to_bytes(tt));
    append_bytes(bytes,  to_bytes(gt));
    return bytes;
  }

  Message::LobbyParameters Message::LobbyParameters::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto game_time = chrono::seconds(to_integral<uint64_t>(bytes, offset));
    auto turn_time = chrono::seconds(to_integral<uint64_t>(bytes, offset));
    auto tt = to_enum<Timer::Type>        (bytes, offset);
    auto gt = to_enum<GameModel::GameMode>(bytes, offset);
    return LobbyParameters({game_time, turn_time, tt, gt});
  }

  //    ╔═════════════════════════════╗
  //    ║ JoinLobby Class Definitions ║
  //    ╚═════════════════════════════╝

  vector<byte> Message::JoinLobby::serialize() const {
    vector<byte> bytes = lobby.serialize();
    append_bytes(bytes, params.serialize());
    bytes.resize(pad(bytes.size()));  // Padding
    append_bytes(bytes, to_bytes(clients.size()));
    for (auto&& client : clients) {
      append_bytes(bytes, client.serialize());
      bytes.resize(pad(bytes.size()));  // Padding
    }
    return bytes;
  }

  Message::JoinLobby Message::JoinLobby::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto lobby  = HostLobby::deserialize(bytes, offset);
    auto params = LobbyParameters::deserialize(bytes, offset);
    offset = pad(offset);  // Padding
    uint64_t size = to_integral<uint64_t>(bytes, offset);

    vector<Message::SlotLobby> clients;
    for (uint64_t i = 0; i < size; ++i)
      clients.emplace_back(SlotLobby::deserialize(bytes, offset));
    return JoinLobby(lobby, params, clients);
  }

  //    ╔═══════════════════════════╗
  //    ║ Faction Class Definitions ║
  //    ╚═══════════════════════════╝

  vector<byte> Message::Faction::serialize() const {
    return to_bytes(fac);
  }

  Message::Faction Message::Faction::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto fac = static_cast<GameModel::Faction>(bytes[offset]);
    return Faction(fac);
  }

  //    ╔═════════════════════════════════╗
  //    ║ BoatSelection Class Definitions ║
  //    ╚═════════════════════════════════╝

  vector<byte> Message::BoatSelection::serialize() const {
    return to_bytes(type);
  }

  Message::BoatSelection Message::BoatSelection::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto type = static_cast<Boat::Type>(bytes[offset]);
    return BoatSelection(type);
  }

  //    ╔════════════════════════════════╗
  //    ║ Confirmation Class Definitions ║
  //    ╚════════════════════════════════╝

  vector<byte> Message::Confirmation::serialize() const {
    vector<byte> bytes = to_bytes(coordinates);

    append_bytes(bytes, to_bytes(boat_id));
    append_bytes(bytes, to_bytes(type));

    return bytes;
  }

  Message::Confirmation Message::Confirmation::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto coordinates = to_vector<BoardCoordinates>(bytes, offset);
    auto boat_id     = to_integral<int>(bytes, offset);
    auto type        = static_cast<Boat::Type>(bytes[offset]);
    return Confirmation(coordinates, boat_id, type);
  }

  //    ╔═══════════════════════════════╗
  //    ║ StartCombat Class Definitions ║
  //    ╚═══════════════════════════════╝

  std::vector<std::byte> Message::StartCombat::serialize() const {
    return to_bytes(your_turn);
  }

  Message::StartCombat Message::StartCombat::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto your_turn = to_integral<bool>(bytes, offset);
    return StartCombat(your_turn);
  }

  //    ╔════════════════════════════════════╗
  //    ║ AbilitySelection Class Definitions ║
  //    ╚════════════════════════════════════╝

  vector<byte> Message::AbilitySelection::serialize() const {
    return to_bytes(type);
  }

  Message::AbilitySelection Message::AbilitySelection::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto type = static_cast<Ability::Type>(bytes[offset]);
    return AbilitySelection(type);
  }

  //    ╔══════════════════════════════╗
  //    ║ ClientFire Class Definitions ║
  //    ╚══════════════════════════════╝

  vector<byte> Message::ClientFire::serialize() const {
    vector<byte> bytes = to_bytes(coordinates);
    append_bytes(bytes, to_bytes(type));
    return bytes;
  }

  Message::ClientFire Message::ClientFire::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto coordinates = to_vector<BoardCoordinates>(bytes, offset);
    auto type        = static_cast<Ability::Type>(bytes[offset]);
    return ClientFire(coordinates, type);
  }

  //    ╔══════════════════════════════╗
  //    ║ ServerFire Class Definitions ║
  //    ╚══════════════════════════════╝

  vector<byte> Message::ServerFire::serialize() const {
    vector<byte> bytes = to_bytes(cells);
    append_bytes(bytes, to_bytes(your_board));
    append_bytes(bytes, to_bytes(your_turn));
    bytes.resize(pad(bytes.size()));  // Padding
    append_bytes(bytes, to_bytes(new_energy));

    return bytes;
  }

  Message::ServerFire Message::ServerFire::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto cells      = to_vector<Cell>  (bytes, offset);
    auto your_board = to_integral<bool>(bytes, offset);
    auto your_turn  = to_integral<bool>(bytes, offset);
    offset = pad(offset);
    auto new_energy = to_integral<int> (bytes, offset);
    return ServerFire(cells, your_board, your_turn, new_energy);
  }

  //    ╔═══════════════════════════╗
  //    ║ GameEnd Class Definitions ║
  //    ╚═══════════════════════════╝

  std::vector<std::byte> Message::GameEnd::serialize() const {
    return to_bytes(victor);
  }

  Message::GameEnd Message::GameEnd::deserialize(span<byte const> bytes, uint64_t& offset) {
    auto victor = static_cast<GameModel::Victor>(bytes[offset]);
    return GameEnd(victor);
  }

  //    ╔═════════════════════════════╗
  //    ║ Recording Class Definitions ║
  //    ╚═════════════════════════════╝

  std::vector<std::byte> Message::Recording::serialize() const {
    vector<byte> bytes = to_bytes(left);
    append_bytes(bytes, to_bytes(right));
    append_bytes(bytes, to_bytes(moves.size()));
    bytes.resize(pad(bytes.size()));
    for (auto && move : moves) {
      append_bytes(bytes, move.serialize());
      bytes.resize(pad(bytes.size()));
    }
    return bytes;
  }

  Message::Recording Message::Recording::deserialize(std::span<std::byte const> bytes, uint64_t & offset) {
    auto left = to_string(bytes, offset);
    auto right = to_string(bytes, offset);
    auto size = to_integral<uint64_t>(bytes, offset);
    Recording recording;
    recording.left = left;
    recording.right = right;
    offset = pad(offset);
    for (uint64_t i = 0; i < size; ++i) {
      auto move  = ServerFire::deserialize(bytes, offset);
      offset = pad(offset);
      recording.moves.push_back(std::move(move));
    }
    return recording;
  }

}
#pragma GCC diagnostic pop
