#include "reversi/frontend/ReversiFrame.h"
#include "reversi/frontend/ReversiBoard.h"
#include <wx/sizer.h>
#include <iostream>
#include <algorithm>

namespace Reversi::Frontend {


  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(600, 600)),
       session(Player::Black) {
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(frameSizer);
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(sizer);

    ReversiBoard *boardWindow = new ReversiBoard(panel, wxID_ANY, this->session);
    sizer->Add(boardWindow, 1, wxALL | wxEXPAND);

    Board board;
    board.putDisc(Position('E', 4), Player::White);
    board.putDisc(Position('D', 5), Player::White);
    board.putDisc(Position('D', 4), Player::Black);
    board.putDisc(Position('E', 5), Player::Black);
    State state(board, Player::Black);
    this->session.getEngine().setState(state);
    this->session.getEngine().addEventListener(this->updateListener);
    this->updateListener.setCallback([boardWindow](const State &state) {
      boardWindow->update();
    });
  }
}