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

#include "reversi/frontend/ReversiFrame.h"
#include "reversi/engine/Logging.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <wx/aboutdlg.h>
#include <iostream>
#include <sstream>
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
    
    this->CreateStatusBar(3);
    this->Bind(ReversiFrameUpdateEvent, &ReversiFrame::OnUpdate, this);
    this->Bind(wxEVT_SIZE, &ReversiFrame::OnResize, this);
    this->Bind(wxEVT_MAXIMIZE, &ReversiFrame::OnMaximize, this);
    this->Bind(wxEVT_TIMER, &ReversiFrame::updateDuration, this);
  
    this->updateListener.setCallback([this](const State &state) {
      wxPostEvent(this, wxThreadEvent(ReversiFrameUpdateEvent));
    });

    wxTimer *updateDurationTimer = new wxTimer(this);
    updateDurationTimer->Start(1000);
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
      this->updateStatistics(this->session->getState());
      this->boardWindow->update();
      this->Refresh();
	  this->moveList->ClearBackground();
	  this->moveList->Update();
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
    wxMenuItem *aboutItem = gameMenu->Append(wxID_ABOUT, "About");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnAbout, this, wxID_ABOUT);
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

  void ReversiFrame::OnAbout(wxCommandEvent &evt) {
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Reversi");
    aboutInfo.SetVersion("0.9");
    aboutInfo.SetDescription("Reversi (Othello) game implementation");
    aboutInfo.SetCopyright("(C) 2018");
    aboutInfo.SetWebSite("https://github.com/protopopov1122/Reversi");
    aboutInfo.SetLicense("Copyright 2018 Jevgenijs Protopopovs\n\n"
      "Redistribution and use in source and binary forms, with or without modification,\nare permitted provided that the following conditions are met:\n"
      "1. Redistributions of source code must retain the above copyright notice, this\nlist of conditions and the following disclaimer.\n"
      "2. Redistributions in binary form must reproduce the above copyright notice, this\nlist of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n"
      "3. Neither the name of the copyright holder nor the names of its contributors may be\nused to endorse or promote products derived from this software without specific prior written permission.\n\n"
      "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
      "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\nA PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE\n"
      "COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,\nINCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,\n"
      "BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS\nOF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
      "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\nSTRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING\n"
      "IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\nPOSSIBILITY OF SUCH DAMAGE."
    );
    aboutInfo.AddDeveloper("Jevgenijs Protopopovs");
    wxAboutBox(aboutInfo);
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
      this->sessionSettings->Enable(false);
      Logger::log("Session", [&](auto &out) {
        out << "Session closed with score " << whiteScore << "x" << blackScore;
      });
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

  void ReversiFrame::updateDuration(wxTimerEvent &evt) {
    if (this->session) {
      std::chrono::milliseconds duration = this->session->getDuration();
      auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
      duration -= hours;
      auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
      duration -= minutes;
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
      std::stringstream ss;
      ss << std::setfill('0') << std::setw(2) << hours.count() << ':';
      ss << std::setfill('0') << std::setw(2) << minutes.count() << ':';
      ss << std::setfill('0') << std::setw(2) << seconds.count();
      this->SetStatusText(ss.str(), 2);
    } else {
      this->SetStatusText("", 2);
    }
  }
}