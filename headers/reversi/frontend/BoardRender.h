#ifndef REVERSI_FRONTEND_BOARD_RENDER_H_
#define REVERSI_FRONTEND_BOARD_RENDER_H_

#include "reversi/frontend/base.h"

namespace Reversi::Frontend {

  class ReversiBoardRender {
   public:
    virtual ~ReversiBoardRender() = default;
    virtual wxRect getCell(const wxSize &, Position) = 0;
    virtual Position getPosition(const wxSize &, wxPoint) = 0;
    virtual void render(wxDC &, const wxSize &, const State *) = 0;
    virtual void highlight(wxDC &, const wxSize &, const std::vector<Position> &) = 0;
  };

  class ReversiDefaultBoardRender : public ReversiBoardRender {
   public:
    wxRect getCell(const wxSize &, Position) override;
    Position getPosition(const wxSize &, wxPoint) override;
    void render(wxDC &, const wxSize &, const State *) override;
    void highlight(wxDC &, const wxSize &, const std::vector<Position> &) override;
   private:
    wxRect getBoard(const wxSize &);
  };
}

#endif