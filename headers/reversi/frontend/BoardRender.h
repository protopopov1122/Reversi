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

#ifndef REVERSI_FRONTEND_BOARD_RENDER_H_
#define REVERSI_FRONTEND_BOARD_RENDER_H_

#include "reversi/frontend/base.h"

namespace Reversi::Frontend {

  class ReversiBoardRender {
   public:
    virtual ~ReversiBoardRender() = default;
    virtual wxRect getCell(const wxSize &, Position) = 0;
    virtual Position getPosition(const wxSize &, wxPoint) = 0;
    virtual bool insideBoard(const wxSize &, wxPoint) = 0;
    virtual void render(wxDC &, const wxSize &, const State *) = 0;
    virtual void highlight(wxDC &, const wxSize &, const std::vector<Position> &) = 0;
  };

  class ReversiDefaultBoardRender : public ReversiBoardRender {
   public:
    wxRect getCell(const wxSize &, Position) override;
    Position getPosition(const wxSize &, wxPoint) override;
    bool insideBoard(const wxSize &, wxPoint) override;
    void render(wxDC &, const wxSize &, const State *) override;
    void highlight(wxDC &, const wxSize &, const std::vector<Position> &) override;
   private:
    wxRect getBoard(const wxSize &);
  };
}

#endif