/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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