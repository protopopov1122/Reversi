#include "reversi/frontend/ReversiFrame.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <iostream>
#include <algorithm>

namespace Reversi::Frontend {

  wxDEFINE_EVENT(ReversiFrameUpdateEvent, wxThreadEvent);

  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(700, 600)),
       session(nullptr), sessionSettings(nullptr) {
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(frameSizer);
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    this->boardWindow = new ReversiBoard(panel, wxID_ANY);
    sizer->Add(this->boardWindow, 1, wxALL | wxEXPAND);

    this->settingsPanel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(this->settingsPanel, 0, wxALL | wxEXPAND);
    wxBoxSizer *settingsSizer = new wxBoxSizer(wxVERTICAL);
    this->settingsPanel->SetSizer(settingsSizer);

    this->showLastMove = new wxCheckBox(this->settingsPanel, wxID_ANY, "Show last move");
    settingsSizer->Add(this->showLastMove);
    this->showLastMove->Bind(wxEVT_CHECKBOX, &ReversiFrame::OnShowLastMoveChange, this);
    this->showLastMove->SetValue(false);
    this->boardWindow->showLastMove(false);

    this->showPossibleMoves = new wxCheckBox(this->settingsPanel, wxID_ANY, "Show possible moves");
    settingsSizer->Add(this->showPossibleMoves);
    this->showPossibleMoves->Bind(wxEVT_CHECKBOX, &ReversiFrame::OnShowPossibleMovesChange, this);
    this->showPossibleMoves->SetValue(false);
    this->boardWindow->showPossibleMoves(false);

    wxMenuBar *menuBar = new wxMenuBar();
    wxMenu *gameMenu = new wxMenu();
    menuBar->Append(gameMenu, "Game");

    wxWindowID idHumanHuman = wxNewId();
    wxMenuItem *humanHumanItem = gameMenu->Append(idHumanHuman, "Human-Human");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnHumanHumanGame, this, idHumanHuman);
    wxWindowID idHumanAI = wxNewId();
    wxMenuItem *humanAIItem = gameMenu->Append(idHumanAI, "Human-AI");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnHumanAIGame, this, idHumanAI);
    wxWindowID idAIHuman = wxNewId();
    wxMenuItem *AIHumanItem = gameMenu->Append(idAIHuman, "AI-Human");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnAIHumanGame, this, idAIHuman);
    wxWindowID idAIAI = wxNewId();
    wxMenuItem *AIAIIem = gameMenu->Append(idAIAI, "AI-AI");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnAIAIGame, this, idAIAI);

    gameMenu->AppendSeparator();
    wxMenuItem *quitItem = gameMenu->Append(wxID_EXIT, "Quit");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnQuit, this, wxID_EXIT);
    this->SetMenuBar(menuBar);
    
    this->CreateStatusBar(1);
    this->Bind(ReversiFrameUpdateEvent, &ReversiFrame::OnUpdate, this);
  
    this->updateListener.setCallback([&](const State &state) {
      wxThreadEvent evt(ReversiFrameUpdateEvent);
      wxPostEvent(this, evt);
    });
  }

  void ReversiFrame::newSession(ReversiSessionFactory &factory, const State &state) {
    if (this->sessionSettings) {
      this->settingsPanel->GetSizer()->Detach(this->sessionSettings);
      this->Layout();
      this->sessionSettings->Destroy();
      this->sessionSettings = nullptr;
    }
    this->session = factory.createSession(state);
    if (this->session) {
      this->session->getEngine().addEventListener(this->updateListener);
      this->boardWindow->setSession(this->session.get());
      this->sessionSettings = this->session->getSettings(this->settingsPanel, wxID_ANY);
      if (this->sessionSettings) {
        this->settingsPanel->GetSizer()->Add(this->sessionSettings, 0, wxALL | wxEXPAND);
        this->Layout();
      }
      this->updateStatistics(this->session->getState());
    } else {
      this->boardWindow->setSession(nullptr);
      this->SetStatusText("", 0);
    }
  }

  void ReversiFrame::newSession(ReversiSessionFactory &factory) {
    Board board;
    board.putDisc(Position('E', 4), Player::White);
    board.putDisc(Position('D', 5), Player::White);
    board.putDisc(Position('D', 4), Player::Black);
    board.putDisc(Position('E', 5), Player::Black);
    State state(board, Player::Black);
    this->newSession(factory, state);
  }

  void ReversiFrame::OnHumanHumanGame(wxCommandEvent &evt) {
    this->newSession(*ReversiSessionFactory::Human_Human);
  }

  void ReversiFrame::OnHumanAIGame(wxCommandEvent &evt) {
    this->newSession(*ReversiSessionFactory::Human_AI);
  }

  void ReversiFrame::OnAIHumanGame(wxCommandEvent &evt) {
    this->newSession(*ReversiSessionFactory::AI_Human);
  }

  void ReversiFrame::OnAIAIGame(wxCommandEvent &evt) {
    this->newSession(*ReversiSessionFactory::AI_AI);
  }

  void ReversiFrame::OnQuit(wxCommandEvent &evt) {
    this->Destroy();
  }

  void ReversiFrame::OnUpdate(wxThreadEvent &evt) {
    this->boardWindow->update();
    if (this->session) {
      this->Enable(!this->session->isCurrentlyProcessing());
      this->updateStatistics(this->session->getState());
    }
    this->Refresh();
  }

  void ReversiFrame::OnShowLastMoveChange(wxCommandEvent &evt) {
    this->boardWindow->showLastMove(this->showLastMove->GetValue());
    this->Refresh();
  }

  void ReversiFrame::OnShowPossibleMovesChange(wxCommandEvent &evt) {
    this->boardWindow->showPossibleMoves(this->showPossibleMoves->GetValue());
    this->Refresh();
  }

  void ReversiFrame::updateStatistics(const State &state) {
    int32_t whiteScore = state.getBoard().getMetric([](int32_t sum, CellState state, Position pos) {
      return state == CellState::White ? sum + 1 : sum;
    });
    int32_t blackScore = state.getBoard().getMetric([](int32_t sum, CellState state, Position pos) {
      return state == CellState::Black ? sum + 1 : sum;
    });
    this->SetStatusText(std::to_string(whiteScore) + "x" + std::to_string(blackScore), 0);
  }
}