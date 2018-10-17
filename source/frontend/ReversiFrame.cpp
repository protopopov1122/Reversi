#include "reversi/frontend/ReversiFrame.h"
#include <wx/sizer.h>
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
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(sizer);

    this->boardWindow = new ReversiBoard(panel, wxID_ANY);
    sizer->Add(this->boardWindow, 1, wxALL | wxEXPAND);

    this->updateListener.setCallback([&](const State &state) {
      this->boardWindow->update();
    });
  }

  void ReversiFrame::newSession(ReversiSessionFactory &factory, const State &state) {
    this->session = factory.createSession(state);
    if (this->session) {
      this->session->getEngine().addEventListener(this->updateListener);
      this->boardWindow->setSession(this->session.get());
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
}