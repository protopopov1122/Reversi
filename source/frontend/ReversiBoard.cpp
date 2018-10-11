#include "reversi/frontend/ReversiBoard.h"
#include <wx/dcbuffer.h>
#include <wx/colour.h>
#include <iostream>

namespace Reversi::Frontend {

  ReversiBoard::ReversiBoard(wxWindow *parent, wxWindowID id, ReversiSession &session)
    : wxWindow::wxWindow(parent, id), session(session) {
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);
    this->Bind(wxEVT_LEFT_DOWN, &ReversiBoard::OnMouseClick, this);
    this->Bind(wxEVT_PAINT, &ReversiBoard::OnPaintEvent, this);
  }

  void ReversiBoard::update() {
    wxPaintEvent evt;
    wxPostEvent(this, evt);
  }

  void ReversiBoard::OnMouseClick(wxMouseEvent &evt) {
    wxPoint point = evt.GetPosition();
    wxSize size = this->GetSize();
    char row = static_cast<char>(8.0f * point.x / size.GetWidth()) + 'A';
    unsigned int col = static_cast<unsigned int>(8.0f * point.y / size.GetHeight()) + 1;
    this->session.onClick(Position(row, col));
  }

  void ReversiBoard::OnPaintEvent(wxPaintEvent &evt) {
    wxAutoBufferedPaintDC dc(this);
    this->render(dc);
  }

  void ReversiBoard::render(wxDC &dc) {
    wxColour backgroundColor(255, 255, 255);
    wxColour cellBorderColor(0, 0, 0);
    wxColour discBorderColor(0, 0, 0);
    wxColour discWhiteColor(255, 255, 255);
    wxColour discBlackColor(0, 0, 0);

    wxBrush backgroundBrush(backgroundColor);
    wxPen cellBoardPen(cellBorderColor);
    wxPen discBorderPen(discBorderColor);
    wxBrush discWhiteBrush(discWhiteColor);
    wxBrush discBlackBrush(discBlackColor);

    // Fill background
    wxSize size = this->GetSize();
    dc.SetBrush(backgroundBrush);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    // Draw cells
    const int ROW_COUNT = 8;
    const int COL_COUNT = 8;
    wxSize cellSize(size.GetWidth() / COL_COUNT, size.GetHeight() / ROW_COUNT);
    dc.SetPen(cellBoardPen);
    for (wxCoord x = 0; x < size.GetWidth(); x += cellSize.GetWidth()) {
      dc.DrawLine(x, 0, x, size.GetHeight());
    }
    for (wxCoord y = 0; y < size.GetHeight(); y += cellSize.GetHeight()) {
      dc.DrawLine(0, y, size.GetWidth(), y);
    }

    // Draw discs
    dc.SetPen(discBorderPen);
    const Board &board = this->session.getState().getBoard();
    for (char row = 'A'; row <= 'H'; row++) {
      for (unsigned int col = 1; col <= 8; col++) {
        CellState cell = board.getCellState(Position(row, col));
        if (cell != CellState::Empty) {
          wxCoord x = (row - 'A') * cellSize.GetWidth();
          wxCoord y = (col - 1) * cellSize.GetHeight();
          if (cell == CellState::White) {
            dc.SetBrush(discWhiteBrush);
          } else {
            dc.SetBrush(discBlackBrush);
          }
          dc.DrawEllipse(x, y, cellSize.GetWidth(), cellSize.GetHeight());
        }
      }
    }
  }
}