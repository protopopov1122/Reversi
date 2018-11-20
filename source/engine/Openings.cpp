#include "reversi/engine/Library.h"
#include "reversi/engine/Engine.h"

namespace Reversi {

  const OpeningLibrary OpeningLibrary::Openings;

  OpeningLibrary::OpeningLibrary() {
    State initialState(StateHelpers::getDefaultInitialState());
    this->addMoves(initialState, { Position('C', 4), Position('C', 3), Position('D', 3), Position('C', 5), Position('B', 4) }, true);
    this->addMoves(initialState, { Position('C', 4), Position('C', 5) }, true);
    this->addMoves(initialState, { Position('C', 4), Position('E', 3), Position('F', 4), Position('C', 5) }, true);
  }
}