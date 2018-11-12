#include "reversi/frontend/ReversiFrame.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Reversi::Frontend {

  wxDEFINE_EVENT(ReversiFrameUpdateEvent, wxThreadEvent);

  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(DISPLAY_MOVE_METRIC ? 750 : 650, 550)),
       session(nullptr), sessionSettings(nullptr) {
    this->SetMinSize(this->GetSize());
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(frameSizer);
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    this->boardWindow = new ReversiBoard(panel, wxID_ANY);
    sizer->Add(this->boardWindow, 1, wxALL | wxEXPAND);

    this->initSettings(frameSizer);
    this->initMenu();
    this->initMoveList();
    
    this->CreateStatusBar(2);
    this->Bind(ReversiFrameUpdateEvent, &ReversiFrame::OnUpdate, this);
    this->Bind(wxEVT_SIZE, &ReversiFrame::OnResize, this);
    this->Bind(wxEVT_MAXIMIZE, &ReversiFrame::OnMaximize, this);
  
    this->updateListener.setCallback([this](const State &state) {
      wxPostEvent(this, wxThreadEvent(ReversiFrameUpdateEvent));
    });
  }

  void ReversiFrame::newSession(ReversiSessionFactory &factory, const State &state) {
    if (this->sessionSettings) {
      this->settingsPanel->GetSizer()->Detach(this->sessionSettings);
      this->Layout();
      this->sessionSettings->Destroy();
      this->sessionSettings = nullptr;
    }
    this->moveList->DeleteAllItems();
    this->session = factory.createSession(state);
    if (this->session) {
      this->session->getEngine().addEventListener(this->updateListener);
      this->boardWindow->setSession(this->session.get());
      this->sessionSettings = this->session->getSettings(this->settingsPanel, wxID_ANY);
      if (this->sessionSettings) {
        this->settingsPanel->GetSizer()->Insert(2, this->sessionSettings, 0, wxALL | wxEXPAND);
        this->Layout();
      }
      this->boardWindow->update();
      this->Refresh();
      this->updateStatistics(this->session->getState());
    } else {
      this->boardWindow->setSession(nullptr);
      this->SetStatusText("", 0);
      this->SetStatusText("", 1);
    }
  }

  void ReversiFrame::newSession(ReversiSessionFactory &factory) {
    this->newSession(factory, StateHelpers::getDefaultInitialState());
  }

  void ReversiFrame::initMenu() {
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
  }

  void ReversiFrame::initSettings(wxSizer *frameSizer) {
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
  }

  void ReversiFrame::initMoveList() {
    this->moveList = new wxListCtrl(this->settingsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    this->settingsPanel->GetSizer()->Add(this->moveList, 1, wxALL | wxEXPAND);
    this->moveList->InsertColumn(0, "#");
    this->moveList->InsertColumn(1, "Player");
    this->moveList->InsertColumn(2, "Move");
    if constexpr (DISPLAY_MOVE_METRIC) {
      this->moveList->InsertColumn(3, "Metric");
    }
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
    this->moveList->DeleteAllItems();
    if (this->session) {
      this->Enable(!this->session->isCurrentlyProcessing());
      this->showMoves(this->session->getMoves());
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
  
  void ReversiFrame::OnResize(wxSizeEvent &evt) {
	 this->Layout();
	 this->Refresh();
	 this->moveList->ClearBackground();
	 this->moveList->Update();
  }
 
  void ReversiFrame::OnMaximize(wxMaximizeEvent &evt) {
	 this->Layout();
	 this->Refresh();
	 this->moveList->ClearBackground();
	 this->moveList->Update();
  }

  void ReversiFrame::updateStatistics(const State &state) {
    int32_t whiteScore = state.getBoard().getMetric([](int32_t sum, CellState state, Position pos) {
      return state == CellState::White ? sum + 1 : sum;
    });
    int32_t blackScore = state.getBoard().getMetric([](int32_t sum, CellState state, Position pos) {
      return state == CellState::Black ? sum + 1 : sum;
    });
    this->SetStatusText(std::to_string(whiteScore) + "x" + std::to_string(blackScore), 0);
    this->SetStatusText("", 1);
    if (this->session->isClosing()) {
      if (whiteScore > blackScore) {
        wxMessageBox("White won!", "", wxOK | wxICON_INFORMATION);
        this->SetStatusText("White won!", 1);
      } else if (whiteScore == blackScore) {
        wxMessageBox("Draw", "", wxOK | wxICON_INFORMATION);
        this->SetStatusText("Draw", 1);
      } else {
        wxMessageBox("Black won!", "", wxOK | wxICON_INFORMATION);
        this->SetStatusText("Black won!", 1);
      }
    }
  }

  void ReversiFrame::showMoves(const std::vector<PlayerMoveDiff> &moves) {
    for (std::size_t i = 0; i < moves.size(); i++) {
      wxListItem item;
      this->moveList->InsertItem(i, item);
      this->moveList->SetItem(i, 0, std::to_string(i + 1));
      this->moveList->SetItem(i, 1, moves.at(i).getPlayer() == Player::White ? "White" : "Black");
      Position move = moves.at(i).getMove();
      this->moveList->SetItem(i, 2, std::string(1, move.getColumn()) + std::to_string(move.getRow()));
      if constexpr (DISPLAY_MOVE_METRIC) {
        if (!isinf(moves.at(i).getMetric())) {
          std::stringstream stream;
          stream << std::fixed << std::setprecision(2) << moves.at(i).getMetric();
          this->moveList->SetItem(i, 3, stream.str() + "%");
        }
      }
    }
  }
}