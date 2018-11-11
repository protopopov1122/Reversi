#ifndef REVERSI_ENGINE_AI_H_
#define REVERSI_ENGINE_AI_H_

#include "reversi/engine/Event.h"
#include "reversi/engine/Threads.h"
#include "reversi/engine/Tree.h"
#include <atomic>

namespace Reversi {

  class GameEngine; // Forward referencing

  class AIPlayer : public EventListener<State> {
   public:
    AIPlayer(Player, unsigned int, GameEngine &, bool = false);
    virtual ~AIPlayer();

    Player getPlayer() const;
    unsigned int getDifficulty() const;
    void setDifficulty(unsigned int);
    bool isActive() const;
    void setRandomized(bool);

    void makeMove();
    void receiveEvent(const State &) override;
   private:
    void aiTurn(const State &);

    Player player;
    unsigned int difficulty;
    FixedThreadPool threads;
    GameEngine &engine;
    std::atomic<bool> active;
    bool wait;
    bool randomized;
  };
}

#endif