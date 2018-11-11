#include "reversi/frontend/ReversiSession.h"
#include <algorithm>
#include <wx/spinctrl.h>

namespace Reversi::Frontend {

  const unsigned int DefaultReversiSession::DEFAULT_AI_DIFFICULTY = 5;

  DefaultReversiSession::DefaultReversiSession() : closed(false) {}

  DefaultReversiSession::DefaultReversiSession(const State &state)
    : engine(state), closed(false) {}
  
  GameEngine &DefaultReversiSession::getEngine() {
    return this->engine;
  }

  const State &DefaultReversiSession::getState() const {
    return this->engine.getState();
  }

  const std::vector<PlayerMove> &DefaultReversiSession::getMoves() const {
    return this->engine.getMoves();
  }

  bool DefaultReversiSession::isClosing() {
    if (!this->closed && StateHelpers::isGameFinished(this->getState())) {
      this->closed = true;
      return true;
    } else {
      return false;
    }
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

    wxWindow *getSettings(wxWindow *parent, wxWindowID id) override {
      wxPanel *settingsPanel = new wxPanel(parent, id);
      wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
      settingsPanel->SetSizer(sizer);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Human-AI"), 0, wxALIGN_CENTER | wxALIGN_LEFT);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Difficulty: "), 0, wxALIGN_CENTER | wxALIGN_RIGHT);
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

    bool isCurrentlyProcessing() override {
      return (this->getState().getPlayer() == Player::White && this->aiWhite.isActive()) ||
        (this->getState().getPlayer() == Player::Black && this->aiBlack.isActive());
    }

    wxWindow *getSettings(wxWindow *parent, wxWindowID id) override {
      wxPanel *settingsPanel = new wxPanel(parent, id);
      wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
      settingsPanel->SetSizer(sizer);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "AI-AI"), 0, wxALIGN_CENTER | wxALIGN_LEFT);
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, ""));
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "White difficulty: "), 0, wxALIGN_CENTER | wxALIGN_RIGHT);
      wxSpinCtrl *whiteDifficulty = new wxSpinCtrl(settingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, 10, this->aiWhite.getDifficulty());
      sizer->Add(whiteDifficulty, 0, wxALIGN_CENTER);
      whiteDifficulty->Bind(wxEVT_SPINCTRL, [this, whiteDifficulty](wxCommandEvent &evt) {
        this->aiWhite.setDifficulty(whiteDifficulty->GetValue());
      });
      sizer->Add(new wxStaticText(settingsPanel, wxID_ANY, "Black difficulty: "), 0, wxALIGN_CENTER | wxALIGN_RIGHT);
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
    return std::make_unique<ReversiHumanHumanSession>(state);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::Human_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiHumanAISession>(state.getPlayer(), state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, false);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::AI_Human = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiHumanAISession>(invertPlayer(state.getPlayer()), state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, false);
  });

  std::unique_ptr<ReversiSessionFactory> ReversiSessionFactory::AI_AI = std::make_unique<LambdaReversiSessionFactory>([](const State &state) {
    return std::make_unique<ReversiAIAISession>(state, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, DefaultReversiSession::DEFAULT_AI_DIFFICULTY, true);
  });

}