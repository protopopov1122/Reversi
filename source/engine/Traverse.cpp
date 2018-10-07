#include <iostream>
#include "reversi/engine/Traverse.h"

namespace Reversi {

  std::pair<int32_t, std::optional<Position>> traverse(const State &base, std::function<int32_t(CellState, Position)> eval, unsigned int depth, int color, int32_t alpha, int32_t beta) {
    if (depth == 0) {
      return std::make_pair(color * base.getBoard().getMetric(eval), std::optional<Position>());
    } else {
      std::vector<Position> moves;
      base.getBoard().getMoves(moves, base.getPlayer());
      if (moves.size() == 0) {
        return std::make_pair(color * base.getBoard().getMetric(eval), std::optional<Position>());
      }
      State state(base);
      int32_t best = INT32_MIN;
      std::optional<Position> best_pos;
      for (Position position : moves) {
        state.apply(position);
        int32_t state_metric = -traverse(state, eval, depth - 1,  -color, -beta, -alpha).first;
        state = base;
        if (state_metric > best) {
          best = state_metric;
          best_pos = position;
        }
        if (state_metric > alpha) {
          alpha = state_metric;
        }
        if (state_metric > beta) {
          break;
        }
      }
      return std::make_pair(best, best_pos);
    }
  }
}