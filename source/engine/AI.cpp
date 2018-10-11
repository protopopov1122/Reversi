#include "reversi/engine/Engine.h"

namespace Reversi {

  static const std::size_t THREAD_POOL_CAPACITY = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;

  AIPlayer::AIPlayer(Player player, unsigned int difficulty, GameEngine &engine, bool wait)
    : player(player), difficulty(difficulty), threads(THREAD_POOL_CAPACITY), engine(engine), active(false), wait(wait) {
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
      BoardReduceFunction reduce = [](int32_t sum, CellState state, Position position) {
        return sum + static_cast<int>(state);
      };
      Strategy strat = {reduce, reduce};
      Node root(state);
      auto move = root.build(this->difficulty, strat, this->threads);
      if (move && move.value().first) {
        this->engine.receiveEvent(PlayerMove(state.getPlayer(), move.value().first.value()));
      }
      this->active = false;
    });
    thread.detach();
  }
}