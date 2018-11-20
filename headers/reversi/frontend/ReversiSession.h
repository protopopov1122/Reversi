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

#ifndef REVERSI_FRONTEND_REVERSI_SESSION_H_
#define REVERSI_FRONTEND_REVERSI_SESSION_H_

#include "reversi/frontend/base.h"
#include <atomic>
#include <chrono>
#include <optional>

namespace Reversi::Frontend {

  class ReversiSession {
   public:
    virtual ~ReversiSession() = default;
    virtual const State &getState() const = 0;
    virtual const std::vector<PlayerMoveDiff> &getMoves() const = 0;
    virtual void onClick(Position) = 0;
  };

  class DefaultReversiSession : public ReversiSession {
   public:
    DefaultReversiSession();
    DefaultReversiSession(const State &);

    GameEngine &getEngine();
    const State &getState() const override;
    const std::vector<PlayerMoveDiff> &getMoves() const override;
    bool isClosing();
    std::chrono::milliseconds getDuration();
    virtual void setReversedMode(bool) = 0;
    virtual wxWindow *getSettings(wxWindow *, wxWindowID) = 0;
    virtual bool isCurrentlyProcessing() = 0;

    static const unsigned int DEFAULT_AI_DIFFICULTY;
   protected:
    DefaultGameEngine engine;
    bool closed;
    std::chrono::time_point<std::chrono::system_clock> started;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> finished;
  };

  class ReversiSessionFactory {
   public:
    virtual ~ReversiSessionFactory() = default;
    virtual std::unique_ptr<DefaultReversiSession> createSession(const State &) = 0;

    static std::unique_ptr<ReversiSessionFactory> Human_Human;
    static std::unique_ptr<ReversiSessionFactory> Human_AI;
    static std::unique_ptr<ReversiSessionFactory> AI_Human;
    static std::unique_ptr<ReversiSessionFactory> AI_AI;
  };
}

#endif