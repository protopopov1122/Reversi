#include "reversi/frontend/ReversiFrame.h"
#include "reversi/frontend/ReversiBoard.h"
#include <wx/sizer.h>
#include <iostream>
#include <algorithm>

namespace Reversi::Frontend {

  ReversiModelController::ReversiModelController()
    : update([]() {}) {}
  
  void ReversiModelController::onUpdate(std::function<void()> update) {
    this->update = update;
  }

  State &ReversiModelController::getState() {
    return this->state;
  }

  void ReversiModelController::setState(const State &state) {
    this->state = state;
  }

  void ReversiModelController::onClick(Position pos) {
    if (state.getBoard().getCellState(pos) != CellState::Empty) {
      return;
    }
    std::vector<Position> moves;
    this->state.getBoard().getMoves(moves, this->state.getPlayer());
    if (std::count(moves.begin(), moves.end(), pos) > 0 && state.apply(pos)) {
      Node root(state);
      auto move = root.build(4);
      std::cout << root << std::endl;
      if (move) {
        this->state.apply(move.value().first);
      } else {
        this->state.next();
      }
      this->update();
    }
  }

  const Board &ReversiModelController::getBoard() {
    return this->state.getBoard();
  }


  ReversiFrame::ReversiFrame(std::string title)
     : wxFrame::wxFrame(nullptr, wxID_DEFAULT, title, wxDefaultPosition, wxSize(600, 600)) {
    wxBoxSizer *frameSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(frameSizer);
    wxPanel *panel = new wxPanel(this, wxID_ANY);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    panel->SetSizer(sizer);

    ReversiBoard *boardWindow = new ReversiBoard(panel, wxID_ANY, this->model, this->model);
    sizer->Add(boardWindow, 1, wxALL | wxEXPAND);

    Board board;
    board.putDisc(Position('E', 4), Player::White);
    board.putDisc(Position('D', 5), Player::White);
    board.putDisc(Position('D', 4), Player::Black);
    board.putDisc(Position('E', 5), Player::Black);
    State state(board, Player::Black);
    this->model.setState(state);
    this->model.onUpdate([boardWindow]() {
      boardWindow->update();
    });
  }
}