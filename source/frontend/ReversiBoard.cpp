#include "reversi/frontend/ReversiBoard.h"
#include <wx/dcbuffer.h>
#include <wx/colour.h>
#include <iostream>

namespace Reversi::Frontend {

  ReversiBoard::ReversiBoard(wxWindow *parent, wxWindowID id, ReversiSession *session)
    : wxWindow::wxWindow(parent, id), session(session), outlineLastMove(false), outlinePossibleMoves(false) {
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    this->Bind(wxEVT_LEFT_DOWN, &ReversiBoard::OnMouseClick, this);
    this->Bind(wxEVT_PAINT, &ReversiBoard::OnPaintEvent, this);
  }

  void ReversiBoard::update() {
    wxPaintEvent evt;
    wxPostEvent(this, evt);
  }

  void ReversiBoard::setSession(ReversiSession *session) {
    this->session = session;
    this->update();
  }

  void ReversiBoard::showLastMove(bool show) {
    this->outlineLastMove = show;
    this->update();
  }

  void ReversiBoard::showPossibleMoves(bool show) {
    this->outlinePossibleMoves = show;
    this->update();
  }

  void ReversiBoard::OnMouseClick(wxMouseEvent &evt) {
    wxPoint point = evt.GetPosition();
    wxSize size = this->GetSize();
    if (this->session) {
      this->session->onClick(this->renderer.getPosition(size, point));
    }
  }

  void ReversiBoard::OnPaintEvent(wxPaintEvent &evt) {
    wxAutoBufferedPaintDC dc(this);
    this->render(dc);
  }

  void ReversiBoard::render(wxDC &dc) {
    wxColour lastMoveColor(255, 0, 0);
    wxColour possibleMoveColor(0, 255, 0);
    wxPen lastMovePen(lastMoveColor, 3);
    wxPen possibleMovePen(possibleMoveColor, 3);

    this->renderer.render(dc, this->GetSize(), this->session ? &this->session->getState() : nullptr);

    if (this->session) {
      if (this->outlinePossibleMoves) {
        dc.SetPen(possibleMovePen);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        std::vector<Position> moves;
        this->session->getState().getBoard().getMoves(moves, this->session->getState().getPlayer());
        this->renderer.highlight(dc, this->GetSize(), moves);
      }
      
      if (!this->session->getMoves().empty() && this->outlineLastMove) {
        dc.SetPen(lastMovePen);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        PlayerMoveDiff move = this->session->getMoves().back();
        std::vector<Position> position {
          move.move
        };
        this->renderer.highlight(dc, this->GetSize(), position);
      }
    }
  }
}