#ifndef REVERSI_FRONTEND_REVERSI_BOARD_H_
#define REVERSI_FRONTEND_REVERSI_BOARD_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/ReversiSession.h"

namespace Reversi::Frontend {

  class ReversiBoard : public wxWindow {
   public:
    ReversiBoard(wxWindow *, wxWindowID, ReversiSession * = nullptr);
    void update();

    void setSession(ReversiSession *);
   private:
    void OnPaintEvent(wxPaintEvent &);
    void OnMouseClick(wxMouseEvent &);
    void render(wxDC &);

    ReversiSession *session;
  };
}

#endif