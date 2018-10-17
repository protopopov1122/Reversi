#include "reversi/frontend/ReversiFrame.h"
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <iostream>
#include <algorithm>

namespace Reversi::Frontend {


  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(600, 600)),
       session(nullptr) {
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(frameSizer);
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    this->boardWindow = new ReversiBoard(panel, wxID_ANY);
    sizer->Add(this->boardWindow, 1, wxALL | wxEXPAND);

    wxPanel *difficultyPanel = new wxPanel(panel, wxID_ANY);
    sizer->Add(difficultyPanel, 0, wxALL | wxEXPAND);
    wxBoxSizer *difficultyPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    difficultyPanel->SetSizer(difficultyPanelSizer);

    const unsigned int MAX_DIFFICULTY = 10;

    difficultyPanelSizer->Add(new wxStaticText(difficultyPanel, wxID_ANY, "White: "), 0, wxALIGN_CENTER);
    this->whiteDifficulty = new wxSpinCtrl(difficultyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, MAX_DIFFICULTY, DefaultReversiSession::DEFAULT_AI_DIFFICULTY);
    difficultyPanelSizer->Add(this->whiteDifficulty, 0, wxRIGHT, 20);
    difficultyPanelSizer->Add(new wxStaticText(difficultyPanel, wxID_ANY, "Black: "), 0, wxALIGN_CENTER);
    this->blackDifficulty = new wxSpinCtrl(difficultyPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 1, MAX_DIFFICULTY, DefaultReversiSession::DEFAULT_AI_DIFFICULTY);
    difficultyPanelSizer->Add(this->blackDifficulty, 0, wxRIGHT, 20);
    this->whiteDifficulty->Enable(false);
    this->blackDifficulty->Enable(false);

    this->whiteDifficulty->Bind(wxEVT_SPINCTRL, &ReversiFrame::OnWhiteSpin, this);
    this->blackDifficulty->Bind(wxEVT_SPINCTRL, &ReversiFrame::OnBlackSpin, this);

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
    this->session = factory.createSession(state);
    if (this->session) {
      this->session->getEngine().addEventListener(this->updateListener);
      this->boardWindow->setSession(this->session.get());

      this->whiteDifficulty->Enable(this->session->isAI(Player::White));
      this->blackDifficulty->Enable(this->session->isAI(Player::Black));
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

  void ReversiFrame::OnWhiteSpin(wxCommandEvent &evt) {
    if (this->session) {
      this->session->setAIDifficulty(Player::White, this->whiteDifficulty->GetValue());
    }
  }

  void ReversiFrame::OnBlackSpin(wxCommandEvent &evt) {
    if (this->session) {
      this->session->setAIDifficulty(Player::Black, this->blackDifficulty->GetValue());
    }
  }
}