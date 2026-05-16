#include "display_common.hh"

#include "../common/utils.hh"

void MenuDisplay::handleServer(const NM::Message& message) {
  if (NM::to_underlying(message.request()) >= NM::to_underlying(Networkable::Request::R_SENTINEL))
    throw std::runtime_error("Server sent invalid response to menu");

  switch (message.request()) {
    using enum Networkable::Request;
    case LOGIN:
    case REGISTER:
      control->authentify(message);
      break;
    case LOGOUT:
      throw std::runtime_error("Bizarre Logout request from server");
    case DISCONNECT:
      throw std::runtime_error("DISCONNECT is handled in client.cc");
    case ACCEPT_GAME:
      break;
    case REJECT_GAME:
      break;
    case UPDATE_RELATIONSHIPS:
      control->updateRelation(message);
      break;
    case CHAT_MESSAGE:
      control->appendChat(message);
      break;
    case LOAD_CHAT:
      control->loadChat(message);
      break;
    case HOST:
      control->startLobby(message);
      break;
    case JOIN:
      control->joinLobby(message);
      break;
    case GET_MATCHES:
      control->loadMatches(message);
      break;
    case QUIT_LOBBY:
      control->quitLobby();
      break;
    case UPDATE_LOBBY:
      control->updateLobby(message);
      break;
    case UPDATE_LOBBY_MEMBER:
      control->updateLobbyMember(message);
      break;
    case START_GAME:
      throw std::runtime_error("START_GAME is currently handled in client.cc");
    case GAME:
      throw std::runtime_error("GAME is handled by the game");
    case GAMEOVER:
      throw std::runtime_error("GAMEOVER is handled by the game");
    case OUT_OF_TIME:
      throw std::runtime_error("OUT_OF_TIME should not be sent by the server");
    default:
      throw std::runtime_error("Unreachable branch in handleServer menu");
  }
}

void GameDisplay::handleServer(const NM::Message& message) {
#ifndef GUI
  first_refresh = true;
#endif

  if (message.request() == Networkable::Request::GAMEOVER) {
      _control->endGame(message);
  }

  else
    switch (_board->gameState()) {
      using enum GameModel::GameStage;
      case FACTIONSELECT:
        _control->acceptFaction(message);
        break;
      case SELECTION:
        _control->acceptSelect(message);
        break;
      case PLACEMENT:
        _control->acceptPlace(message);
        break;
      case WAITING:
        _control->acceptStart(message);
        break;
      case ATTACKSELECT:
        _control->acceptAbility(message);
        break;
      case SPECTATING:
      case OTHERTURN:
      case COMBAT:
        _control->acceptFire(message);
        break;
      default:
        throw NotImplementedError("Game stage not implemented: 270 csb");
    }
}