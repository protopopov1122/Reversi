#ifndef REVERSI_ENGINE_ENGINE_H_
#define REVERSI_ENGINE_ENGINE_H_

#include "reversi/config.h"
#include "reversi/engine/Event.h"
#include "reversi/engine/AI.h"
#include "reversi/engine/Tree.h"
#include <algorithm>
#include <vector>


namespace Reversi {

  using PlayerMove = std::pair<Player, Position>;

  template <bool M, typename Enable = void>
  struct PlayerMoveDiffImpl {};

  template <bool M>
  struct PlayerMoveDiffImpl<M, typename std::enable_if<M>::type> {
    PlayerMoveDiffImpl(Player player, Position move, float metric = 0.0f)
      : player(player), move(move), metric(metric) {}

    float getMetric() const {
      return this->metric;
    }

    Player player;
    Position move;
    float metric;
  };

  template <bool M>
  struct PlayerMoveDiffImpl<M, typename std::enable_if<!M>::type> {
    PlayerMoveDiffImpl(Player player, Position move, float metric = 0.0f)
      : player(player), move(move) {}

    float getMetric() const {
      return 0;
    }

    Player player;
    Position move;
  };

  using PlayerMoveDiff = PlayerMoveDiffImpl<DISPLAY_MOVE_METRIC>;

  class GameEngine : public EventListener<PlayerMove>, public EventSource<State> {
   public:
    GameEngine();
    GameEngine(const State &);
    virtual ~GameEngine() = default;
    const State &getState() const;
    const std::vector<PlayerMoveDiff> &getMoves() const;
    void undoMove(std::size_t = 0);
   protected:
    void stateUpdated();

    State baseState;
    State state;
    std::vector<PlayerMoveDiff> moves;
  };
  
  class StateHelpers {
   public:
    static bool hasMoves(const State &, Player);
    static bool isMovePossible(const State &, Player, Position);
    static bool isGameFinished(const State &);
    static State getDefaultInitialState();
    static int32_t assessState(const State &);
   private:
    static int32_t calculateEmptyDiscsAround(const State &, Position);
  };

  class DefaultGameEngine : public GameEngine {
   public:
    DefaultGameEngine();
    DefaultGameEngine(const State &);
    void triggerEvent();
    void receiveEvent(const PlayerMove &) override;
  };
}

#endif