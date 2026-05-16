#include "network_io.hh"

#include <iostream>
#include <ranges>

#include "serializer.hh"

//                   ╔═══════════════╗
//                   ║ I/O Functions ║
//                   ╚═══════════════╝

void Networkable::write_message(int recipient, NM::Message&& message)  {
  vector<std::byte> data = NM::Message::serialize(std::move(message));

  if (data.size() > MAXSHORT) return;

  uint16_t size = htons(data.size());
  send(recipient, &size, sizeof(uint16_t), MSG_DONTWAIT);
  send(recipient, data.data(), data.size(), MSG_DONTWAIT);
}

NM::Message Networkable::read_message(int sender) {
  vector<std::byte> data;

  uint16_t size;
  recv(sender, &size, sizeof(uint16_t), MSG_WAITALL);
  size = ntohs(size);

  data.resize(size);
  recv(sender, data.data(), size, MSG_WAITALL);

  return NM::Message::deserialize(std::move(data));
}
