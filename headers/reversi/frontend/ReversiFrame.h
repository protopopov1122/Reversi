#ifndef REVERSI_FRONTEND_FRAME_H_
#define REVERSI_FRONTEND_FRAME_H_

#include "reversi/frontend/base.h"
#include "reversi/frontend/ReversiSession.h"
#include "reversi/frontend/ReversiBoard.h"
#include <wx/listctrl.h>
#include <functional>

namespace Reversi::Frontend {

  wxDECLARE_EVENT(ReversiFrameUpdateEvent, wxThreadEvent);

  class ReversiFrame : public wxFrame {
   public:
    ReversiFrame(std::string);
    void newSession(ReversiSessionFactory &, const State &);
    void newSession(ReversiSessionFactory &);
   private:
    void initMenu();
    void initSettings(wxSizer *);
    void initMoveList();

    void OnQuit(wxCommandEvent &);
    void OnHumanHumanGame(wxCommandEvent &);
    void OnHumanAIGame(wxCommandEvent &);
    void OnAIHumanGame(wxCommandEvent &);
    void OnAIAIGame(wxCommandEvent &);
    void OnUpdate(wxThreadEvent &);
    void OnShowLastMoveChange(wxCommandEvent &);
    void OnShowPossibleMovesChange(wxCommandEvent &);
    void OnResize(wxSizeEvent &);
    void OnMaximize(wxMaximizeEvent &);
    
    void updateStatistics(const State &);
    void showMoves(const std::vector<PlayerMove> &);

    std::unique_ptr<DefaultReversiSession> session;
    FunctionEventListener<State> updateListener;
    ReversiBoard *boardWindow;
    wxPanel *settingsPanel;
    wxWindow *sessionSettings;
    wxCheckBox *showLastMove;
    wxCheckBox *showPossibleMoves;
    wxListCtrl *moveList;
  };
}

#endif