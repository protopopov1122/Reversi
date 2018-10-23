#include "reversi/frontend/BoardRender.h"

namespace Reversi::Frontend {

  wxRect ReversiDefaultBoardRender::getCell(const wxSize &size, Position position) {
    const int ROW_COUNT = 8;
    const int COL_COUNT = 8;
    wxSize cellSize(size.GetWidth() / COL_COUNT, size.GetHeight() / ROW_COUNT);
    return wxRect((position.getColumn() - 'A') * cellSize.GetWidth(),
      (position.getRow() - 1) * cellSize.GetHeight(),
      cellSize.GetWidth(), cellSize.GetHeight());
  }

  Position ReversiDefaultBoardRender::getPosition(const wxSize &size, wxPoint coord) {
    char row = static_cast<char>(8.0f * coord.x / size.GetWidth()) + 'A';
    unsigned int col = static_cast<unsigned int>(8.0f * coord.y / size.GetHeight()) + 1;
    return Position(row, col);
  }

  void ReversiDefaultBoardRender::render(wxDC &dc, const wxSize &size, const State *state) {
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

    // Draw discs and moves
    if (state) {
      dc.SetPen(discBorderPen);
      const Board &board = state->getBoard();
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

  void ReversiDefaultBoardRender::highlight(wxDC &dc, const wxSize &size, const std::vector<Position> &positions) {
    for (Position position : positions) {
      wxRect rect = this->getCell(size, position);
      dc.DrawRectangle(rect);
    }
  }
}