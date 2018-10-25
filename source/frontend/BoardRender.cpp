#include "reversi/frontend/BoardRender.h"

namespace Reversi::Frontend {

  wxRect ReversiDefaultBoardRender::getCell(const wxSize &size, Position position) {
    const int ROW_COUNT = 8;
    const int COL_COUNT = 8;
    wxRect boardDim = this->getBoard(size);
    wxSize cellSize(boardDim.GetWidth() / COL_COUNT, boardDim.GetHeight() / ROW_COUNT);
    return wxRect(boardDim.GetX() + (position.getColumn() - 'A') * cellSize.GetWidth(),
      boardDim.GetY() + (position.getRow() - 1) * cellSize.GetHeight(),
      cellSize.GetWidth(), cellSize.GetHeight());
  }

  Position ReversiDefaultBoardRender::getPosition(const wxSize &size, wxPoint coord) {
    wxRect boardDim = this->getBoard(size);
    char row = static_cast<char>(8.0f * (coord.x - boardDim.GetX()) / boardDim.GetWidth()) + 'A';
    unsigned int col = static_cast<unsigned int>(8.0f * (coord.y - boardDim.GetY()) / boardDim.GetHeight()) + 1;
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

    wxRect boardDim = this->getBoard(size);

    // Fill background
    dc.SetBrush(backgroundBrush);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    // Draw board
    const int ROW_COUNT = 8;
    const int COL_COUNT = 8;
    wxSize cellSize(boardDim.GetWidth() / COL_COUNT, boardDim.GetHeight() / ROW_COUNT);
    dc.SetPen(cellBoardPen);
    for (wxCoord x = boardDim.GetX(); x <= boardDim.GetWidth(); x += cellSize.GetWidth()) {
      dc.DrawLine(x, boardDim.GetY(), x, boardDim.GetY() + boardDim.GetHeight());
    }
    for (wxCoord y = boardDim.GetY(); y <= boardDim.GetHeight(); y += cellSize.GetHeight()) {
      dc.DrawLine(boardDim.GetX(), y, boardDim.GetX() + boardDim.GetWidth(), y);
    }
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(boardDim);

    // Draw board labels
    wxFont font(wxFontInfo(10));
    dc.SetFont(font);
    char colnum = 'A';
    for (wxCoord x = boardDim.GetX(); x <= boardDim.GetWidth(); x += cellSize.GetWidth()) {
      dc.DrawText(std::string(1, colnum++), x + cellSize.GetWidth() / 2, 0);
    }
    unsigned int rownum = 1;
    for (wxCoord y = boardDim.GetY(); y <= boardDim.GetHeight(); y += cellSize.GetHeight()) {
      dc.DrawText(std::to_string(rownum++), boardDim.GetX() / 5, y + cellSize.GetHeight() / 2);
    }

    // Draw discs and moves
    const unsigned int DISC_DIV = 20;
    const unsigned int DISC_MUL = 1;
    if (state) {
      dc.SetPen(discBorderPen);
      const Board &board = state->getBoard();
      for (char row = 'A'; row <= 'H'; row++) {
        for (unsigned int col = 1; col <= 8; col++) {
          CellState cell = board.getCellState(Position(row, col));
          if (cell != CellState::Empty) {
            wxCoord x = boardDim.GetX() + (row - 'A') * cellSize.GetWidth() + cellSize.GetWidth() * DISC_MUL / DISC_DIV;
            wxCoord y = boardDim.GetY() + (col - 1) * cellSize.GetHeight() + cellSize.GetHeight() * DISC_MUL / DISC_DIV;
            if (cell == CellState::White) {
              dc.SetBrush(discWhiteBrush);
            } else {
              dc.SetBrush(discBlackBrush);
            }
            dc.DrawEllipse(x, y, cellSize.GetWidth() * (DISC_DIV - 2 * DISC_MUL) / DISC_DIV, cellSize.GetHeight() * (DISC_DIV - 2 * DISC_MUL) / DISC_DIV);
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

  wxRect ReversiDefaultBoardRender::getBoard(const wxSize &size) {
    const unsigned int BOARD_DIV = 30;
    const unsigned int BOARD_MUL = 1;
    return wxRect(size.GetWidth() * BOARD_MUL / BOARD_DIV,
      size.GetHeight() * BOARD_MUL / BOARD_DIV,
      size.GetWidth() * (BOARD_DIV - 2 * BOARD_MUL) / BOARD_DIV,
      size.GetHeight() * (BOARD_DIV - 2 * BOARD_MUL) / BOARD_DIV);
  }
}