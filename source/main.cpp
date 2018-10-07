#include <iostream>
#include <cstdlib>
#include "reversi/engine/Traverse.h"

using namespace Reversi;

int main(int argc, char **argv) {
  Board board;
  board.putDisc(Position('E', 4), Player::White);
  board.putDisc(Position('D', 5), Player::White);
  board.putDisc(Position('D', 4), Player::Black);
  board.putDisc(Position('E', 5), Player::Black);
  State state(board, Player::Black);

  auto eval = [](CellState state, Position pos) {
    if (state == CellState::Black) {
      return -1;
    } else if (state == CellState::White) {
      return 1;
    } else {
      return 0;
    }
  };

  std::string line;
  while (true) {
    std::cout << state.getBoard().getPlainMetric() << std::endl;
    std::cout << state.getBoard() << std::endl;
    getline(std::cin, line);
    std::pair<int32_t, std::optional<Position>> move = traverse(state, eval, 5, state.getPlayer() == Player::Black ? 1 : -1, INT16_MIN, INT16_MAX);
    if (move.second) {
      Position next_move = move.second.value();
      std::cout << next_move << std::endl;
      state.apply(next_move);
    } else {
      state.next();
    }
  }

  return EXIT_SUCCESS;
}