#ifndef REVERSI_FRONTEND_FRAME_H_
#define REVERSI_FRONTEND_FRAME_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/ReversiSession.h"
#include "reversi/frontend/ReversiBoard.h"
#include <functional>
#include <wx/spinctrl.h>

namespace Reversi::Frontend {

  class ReversiFrame : public wxFrame {
   public:
    ReversiFrame(std::string);
    void newSession(ReversiSessionFactory &, const State &);
    void newSession(ReversiSessionFactory &);
   private:
    void OnQuit(wxCommandEvent &);
    void OnHumanHumanGame(wxCommandEvent &);
    void OnHumanAIGame(wxCommandEvent &);
    void OnAIAIGame(wxCommandEvent &);
    void OnWhiteSpin(wxCommandEvent &);
    void OnBlackSpin(wxCommandEvent &);

    std::unique_ptr<DefaultReversiSession> session;
    FunctionEventListener<State> updateListener;
    ReversiBoard *boardWindow;
    wxSpinCtrl *whiteDifficulty;
    wxSpinCtrl *blackDifficulty;
  };
}

#endif