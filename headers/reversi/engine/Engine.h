#ifndef REVERSI_ENGINE_ENGINE_H_
#define REVERSI_ENGINE_ENGINE_H_

#include "reversi/engine/Event.h"
#include "reversi/engine/AI.h"
#include "reversi/engine/Tree.h"
#include <algorithm>


namespace Reversi {

  using PlayerMove = std::pair<Player, Position>;

  class GameEngine : public EventListener<PlayerMove>, public EventSource<State> {
   public:
    GameEngine();
    GameEngine(const State &);
    virtual ~GameEngine() = default;
    void setState(const State &);
    const State &getState() const;
   protected:
    void stateUpdated();
  
    State state;
  };

  class DefaultGameEngine : public GameEngine {
   public:
    DefaultGameEngine();
    DefaultGameEngine(const State &);
    void triggerEvent();
    void receiveEvent(const PlayerMove &) override;
   protected:
    bool hasMoves(Player) const;
    bool isMovePossible(Player, Position) const;
    bool isGameFinished() const;
  };
}

#endif