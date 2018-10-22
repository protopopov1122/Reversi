#include "reversi/frontend/ReversiFrame.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <iostream>
#include <algorithm>

namespace Reversi::Frontend {


  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(600, 600)),
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

    wxMenuBar *menuBar = new wxMenuBar();
    wxMenu *gameMenu = new wxMenu();
    menuBar->Append(gameMenu, "Game");

    wxWindowID idHumanHuman = wxNewId();
    wxMenuItem *humanHumanItem = gameMenu->Append(idHumanHuman, "Human-Human");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnHumanHumanGame, this, idHumanHuman);
    wxWindowID idHumanAI = wxNewId();
    wxMenuItem *humanAIItem = gameMenu->Append(idHumanAI, "Human-AI");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnHumanAIGame, this, idHumanAI);
    wxWindowID idAIAI = wxNewId();
    wxMenuItem *AIAIIem = gameMenu->Append(idAIAI, "AI-AI");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnAIAIGame, this, idAIAI);

    gameMenu->AppendSeparator();
    wxMenuItem *quitItem = gameMenu->Append(wxID_EXIT, "Quit");
    this->Bind(wxEVT_COMMAND_MENU_SELECTED, &ReversiFrame::OnQuit, this, wxID_EXIT);
    this->SetMenuBar(menuBar);
  
    this->updateListener.setCallback([&](const State &state) {
      this->boardWindow->update();
      if (this->session) {
        this->Enable(!this->session->isCurrentlyProcessing());
      }
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
    } else {
      this->boardWindow->setSession(nullptr);
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

  void ReversiFrame::OnAIAIGame(wxCommandEvent &evt) {
    this->newSession(*ReversiSessionFactory::AI_AI);
  }

  void ReversiFrame::OnQuit(wxCommandEvent &evt) {
    this->Destroy();
  }
}