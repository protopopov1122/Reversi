#ifndef REVERSI_FRONTEND_REVERSI_BOARD_H_
#define REVERSI_FRONTEND_REVERSI_BOARD_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/BoardController.h"
#include "reversi/frontend/BoardModel.h"

namespace Reversi::Frontend {

  class ReversiBoard : public wxWindow {
   public:
    ReversiBoard(wxWindow *, wxWindowID, BoardController &, BoardModel &);
    void update();
   private:
    void OnPaintEvent(wxPaintEvent &);
    void OnMouseClick(wxMouseEvent &);
    void render(wxDC &);

    BoardController &controller;
    BoardModel &model;
  };
}

#endif