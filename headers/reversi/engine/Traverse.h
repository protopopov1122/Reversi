#ifndef REVERSI_ENGINE_TRAVERSE_H_
#define REVERSI_ENGINE_TRAVERSE_H_

#include <optional>
#include "reversi/engine/State.h"

namespace Reversi {

  std::pair<int32_t, std::optional<Position>> traverse(const State &, std::function<int32_t(CellState, Position)>, unsigned int, int, int32_t, int32_t);
}

#endif