#ifndef REVERSI_FRONTEND_REVERSI_APP_H_
#define REVERSI_FRONTEND_REVERSI_APP_H_

#include "reversi/frontend/base.h"

namespace Reversi::Frontend {

  class ReversiApp : public wxApp {
   public:
    bool OnInit() override;
  };
}

wxDECLARE_APP(Reversi::Frontend::ReversiApp);

#endif