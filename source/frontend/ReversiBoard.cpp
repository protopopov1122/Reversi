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
    if (this->session && this->renderer.insideBoard(size, point)) {
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
          move.getMove()
        };
        this->renderer.highlight(dc, this->GetSize(), position);
      }
    }
  }
}