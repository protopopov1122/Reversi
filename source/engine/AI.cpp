#include "reversi/engine/Engine.h"

namespace Reversi {

  static const std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  AIPlayer::AIPlayer(Player player, unsigned int difficulty, GameEngine &engine, bool wait)
    : player(player), difficulty(difficulty), threads(THREAD_POOL_CAPACITY), engine(engine), active(false), wait(wait), randomized(false) {
    this->engine.addEventListener(*this);
  }

  AIPlayer::~AIPlayer() {
    this->engine.removeEventListener(*this);
  }

  Player AIPlayer::getPlayer() const {
    return this->player;
  }

  unsigned int AIPlayer::getDifficulty() const {
    return this->difficulty;
  }

  void AIPlayer::setDifficulty(unsigned int diff) {
    this->difficulty = diff;
  }

  bool AIPlayer::isActive() const {
    return this->active.load();
  }

  void AIPlayer::setRandomized(bool rand) {
    this->randomized = rand;
  }

  void AIPlayer::makeMove() {
    const State &state = this->engine.getState();
    if (state.getPlayer() == this->player) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::receiveEvent(const State &state) {
    if (state.getPlayer() == this->player && !wait) {
      this->aiTurn(state);
    }
  }

  void AIPlayer::aiTurn(const State &state) {
    this->active = true;
    std::thread thread([&]() {
      std::function<int32_t (const State &)> reduce = StateHelpers::assessState;
      Strategy strat = {reduce, reduce};
      Node root(state);
      auto move = root.build(this->difficulty, strat, this->threads, this->randomized);
      this->active = false;
      if (move && move.value().move) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().move.value()));
      } else {
        std::vector<Position> moves;
        state.getBoard().getMoves(moves, this->player);
      }
    });
    thread.detach();
  }
}