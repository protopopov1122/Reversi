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

#include "reversi/frontend/ReversiSession.h"
#include "reversi/engine/Logging.h"
#include <algorithm>
#include <iostream>
#include <wx/spinctrl.h>

namespace Reversi::Frontend {

  const unsigned int DefaultReversiSession::DEFAULT_AI_DIFFICULTY = 5;

  DefaultReversiSession::DefaultReversiSession() : closed(false) {
    this->started = std::chrono::system_clock::now();
  }

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state), closed(false) {
    this->started = std::chrono::system_clock::now();
  }
  
  GameEngine &DefaultReversiSession::getEngine() {
    return this->engine;
  }

  const State &DefaultReversiSession::getState() const {
    return this->engine.getState();
  }

  const std::vector<PlayerMoveDiff> &DefaultReversiSession::getMoves() const {
    return this->engine.getMoves();
  }

  bool DefaultReversiSession::isClosing() {
    if (!this->closed && StateHelpers::isGameFinished(this->getState())) {
      this->closed = true;
      this->finished = std::chrono::system_clock::now();
      return true;
    } else {
      return false;
    }
  }

  std::chrono::milliseconds DefaultReversiSession::getDuration() {
    std::chrono::time_point<std::chrono::system_clock> end;
    if (this->finished.has_value()) {
      end = this->finished.value();
    } else {
      end = std::chrono::system_clock::now();
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - this->started);
  }

  class ReversiHumanHumanSession : public DefaultReversiSession {
   public:
    ReversiHumanHumanSession() {}

    ReversiHumanHumanSession(const State & state)
      : DefaultReversiSession::DefaultReversiSession(state) {}

    void onClick(Position position) override {
      if (!StateHelpers::isGameFinished(this->getState())) {
        this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
      }
    }

    bool isCurrentlyProcessing() override {
      return false;
    }

    void setReversedMode(bool rev) override {}

    wxWindow *getSettings(wxWindow *parent, wxWindowID id) override {
      wxPanel *settingsPanel = new wxPanel(parent, id);
      wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
      settingsPanel->SetSizer(sizer);
      wxButton *undoButton = new wxButton(settingsPanel, wxID_ANY, "Undo move");
      sizer->Add(undoButton);
      undoButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &evt) {
        this->engine.undoMove(1);
      });
      return settingsPanel;
    }
  };

  class ReversiHumanAISession : public DefaultReversiSession {
   public:
    ReversiHumanAISession(Player human, unsigned int difficulty, bool randomizeAi)
      : human(human), ai(invertPlayer(human), difficulty, this->engine), randomizeAi(randomizeAi) {
      this->ai.setRandomized(this->randomizeAi);
    }

    ReversiHumanAISession(Player human, const State &state, unsigned int difficulty, bool randomizeAi)
      : DefaultReversiSession::DefaultReversiSession(state), human(human), ai(invertPlayer(human), difficulty, this->engine), randomizeAi(randomizeAi) {
      this->ai.setRandomized(this->randomizeAi);
    }

    void onClick(Position position) override {
      if (!StateHelpers::isGameFinished(this->getState())) {
        if (this->getState().getPlayer() == this->human) {
          this->engine.receiveEvent(PlayerMove(this->getState().getPlayer(), position));
        } else {
          this->engine.triggerEvent();
        }
      }
    }

    bool isCurrentlyProcessing() override {
      return this->getState().getPlayer() != human && this->ai.isActive();
    }

    void setReversedMode(bool rev) override {
      this->ai.setReversed(rev);
    }

    wxWindow *getSettings(wxWindow *parent, wxWindowID id) override {
      wxPanel *settingsPanel = new wxPanel(parent, id);
      wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
      settingsPanel->SetSizer(sizer);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Human-AI"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Difficulty: "), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
      wxSpinCtrl *difficulty = new wxSpinCtrl(settingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, 10, this->ai.getDifficulty());
      sizer->Add(difficulty, 0, wxALIGN_CENTER);
      difficulty->Bind(wxEVT_SPINCTRL, [this, difficulty](wxCommandEvent &evt) {
        this->ai.setDifficulty(difficulty->GetValue());
      });
      wxCheckBox *randomizeCheckbox = new wxCheckBox(settingsPanel, wxID_ANY, "Randomize AI");
      sizer->Add(randomizeCheckbox);
      randomizeCheckbox->Bind(wxEVT_CHECKBOX, [this, randomizeCheckbox](wxCommandEvent &evt) {
        bool rand = randomizeCheckbox->GetValue();
        this->ai.setRandomized(rand);
      });
      randomizeCheckbox->SetValue(this->randomizeAi);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      wxButton *undoButton = new wxButton(settingsPanel, wxID_ANY, "Undo move");
      sizer->Add(undoButton);
      undoButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &evt) {
        this->engine.undoMove(2);
      });
      return settingsPanel;
    }
   private:
    Player human;
    AIPlayer ai;
    bool randomizeAi;
  };

  class ReversiAIAISession : public DefaultReversiSession {
   public:
    ReversiAIAISession(unsigned int whiteDifficulty, unsigned int blackDifficulty, bool randomizeAi)
      : aiWhite(Player::White, whiteDifficulty, this->engine, true), aiBlack(Player::Black, blackDifficulty, this->engine, true),
        randomizeAi(randomizeAi) {
      this->aiWhite.setRandomized(this->randomizeAi);
      this->aiBlack.setRandomized(this->randomizeAi);
    }
   
    ReversiAIAISession(const State &state, unsigned int whiteDifficulty, unsigned int blackDifficulty, bool randomizeAi)
      : DefaultReversiSession::DefaultReversiSession(state),
        aiWhite(Player::White, whiteDifficulty, this->engine, true), aiBlack(Player::Black, blackDifficulty, this->engine, true),
        randomizeAi(randomizeAi) {
      this->aiWhite.setRandomized(this->randomizeAi);
      this->aiBlack.setRandomized(this->randomizeAi);
    }
   
    void onClick(Position position) override {
      if (!StateHelpers::isGameFinished(this->getState())) {
        if (!this->aiWhite.isActive() && !this->aiBlack.isActive()) {
          if (this->engine.getState().getPlayer() == Player::White) {
            this->aiWhite.makeMove();
          } else {
            this->aiBlack.makeMove();
          }
          this->engine.triggerEvent();
        }
      }
    }

    void setReversedMode(bool rev) override {
      this->aiWhite.setReversed(rev);
      this->aiBlack.setReversed(rev);
    }

    bool isCurrentlyProcessing() override {
      return (this->getState().getPlayer() == Player::White && this->aiWhite.isActive()) ||
        (this->getState().getPlayer() == Player::Black && this->aiBlack.isActive());
    }

    wxWindow *getSettings(wxWindow *parent, wxWindowID id) override {
      wxPanel *settingsPanel = new wxPanel(parent, id);
      wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
      settingsPanel->SetSizer(sizer);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "AI-AI"), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "White difficulty: "), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
      wxSpinCtrl *whiteDifficulty = new wxSpinCtrl(settingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, 10, this->aiWhite.getDifficulty());
      sizer->Add(whiteDifficulty, 0, wxALIGN_CENTER);
      whiteDifficulty->Bind(wxEVT_SPINCTRL, [this, whiteDifficulty](wxCommandEvent &evt) {
        this->aiWhite.setDifficulty(whiteDifficulty->GetValue());
      });
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Black difficulty: "), 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
      wxSpinCtrl *blackDifficulty = new wxSpinCtrl(settingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, 10, this->aiBlack.getDifficulty());
      sizer->Add(blackDifficulty, 0, wxALIGN_CENTER);
      blackDifficulty->Bind(wxEVT_SPINCTRL, [this, blackDifficulty](wxCommandEvent &evt) {
        this->aiBlack.setDifficulty(blackDifficulty->GetValue());
      });
      wxCheckBox *randomizeCheckbox = new wxCheckBox(settingsPanel, wxID_ANY, "Randomize AI");
      sizer->Add(randomizeCheckbox);
      randomizeCheckbox->Bind(wxEVT_CHECKBOX, [this, randomizeCheckbox](wxCommandEvent &evt) {
        bool rand = randomizeCheckbox->GetValue();
        this->aiWhite.setRandomized(rand);
        this->aiBlack.setRandomized(rand);
      });
      randomizeCheckbox->SetValue(this->randomizeAi);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      wxButton *undoButton = new wxButton(settingsPanel, wxID_ANY, "Undo move");
      sizer->Add(undoButton);
      undoButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &evt) {
        this->engine.undoMove(1);
      });
      return settingsPanel;
    }
   private:
    AIPlayer aiWhite;
    AIPlayer aiBlack;
    bool randomizeAi;
  };

  class LambdaReversiSessionFactory : public ReversiSessionFactory {
   public:
    LambdaReversiSessionFactory(std::function<std::unique_ptr<DefaultReversiSession> (const State &)> build)
      : build(build) {}
    
    std::unique_ptr<DefaultReversiSession> createSession(const State &state) override {
      return this->build(state);
    }
   private:
    std::function<std::unique_ptr<DefaultReversiSession> (const State &)> build;
  };

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::Human_Human = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    Logger::log("Session", [](auto &out) {
      out << "Human-human game session";
    });
    return std::make_unique<ReversiHumanHumanSession>(state);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::Human_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    Logger::log("Session", [](auto &out) {
      out << "Human-AI game session";
    });
    return std::make_unique<ReversiHumanAISession>(state.getPlayer(), state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, true);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::AI_Human = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    Logger::log("Session", [](auto &out) {
      out << "AI-human game session";
    });
    return std::make_unique<ReversiHumanAISession>(invertPlayer(state.getPlayer()), state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, true);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::AI_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    Logger::log("Session", [](auto &out) {
      out << "AI-AI game session";
    });
    return std::make_unique<ReversiAIAISession>(state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, true);
  });

}