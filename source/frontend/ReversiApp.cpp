#include "reversi/frontend/ReversiApp.h"
#include "reversi/frontend/ReversiFrame.h"

namespace Reversi::Frontend {

  bool ReversiApp::OnInit() {
    ReversiFrame *frame = new ReversiFrame("Reversi");
    frame->Show(true);
    return true;
  }
}

wxIMPLEMENT_APP(Reversi::Frontend::ReversiApp);