#ifndef REVERSI_ENGINE_ENGINE_H_
#define REVERSI_ENGINE_ENGINE_H_

#include "reversi/engine/Event.h"
#include "reversi/engine/AI.h"
#include "reversi/engine/Tree.h"
#include <algorithm>
#include <vector>


namespace Reversi {

  using PlayerMove = std::pair<Player, Position>;

  class GameEngine : public EventListener<PlayerMove>, public EventSource<State> {
   public:
    GameEngine();
    GameEngine(const State &);
    virtual ~GameEngine() = default;
    void setState(const State &);
    const State &getState() const;
    const std::vector<PlayerMove> &getMoves() const;
   protected:
    void stateUpdated();
  
    State state;
    std::vector<PlayerMove> moves;
  };
  
  class StateHelpers {
   public:
    static bool hasMoves(const State &, Player);
    static bool isMovePossible(const State &, Player, Position);
    static bool isGameFinished(const State &);
    static State getDefaultInitialState();
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