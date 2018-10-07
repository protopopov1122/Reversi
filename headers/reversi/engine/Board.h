#ifndef REVERSI_ENGINE_BOARD_H_
#define REVERSI_ENGINE_BOARD_H_

#include <functional>
#include <vector>
#include "reversi/engine/Position.h"
#include "reversi/engine/Player.h"

namespace Reversi {

  enum class CellState {
    Empty = 0,
    White = 1,
    Black = -1
  };

  using BoardReduceFunction = std::function<int32_t(int32_t, CellState, Position)>;

  class Board {
    public:
      Board();
      Board(const Board &);
      Board &operator=(const Board &);

      CellState getCellState(Position) const;
      bool putDisc(Position, Player);
      void getMoves(std::vector<Position> &, Player) const;
      int32_t getMetric(BoardReduceFunction) const;
      int32_t getPlainMetric() const;

      friend std::ostream &operator<<(std::ostream &, const Board &);
    private:
      std::size_t position_to_index(Position) const;
      void addMovement(Position, std::vector<Position> &, CellState) const;
      void addMovementPosition(char, unsigned int, std::vector<Position> &, CellState) const;
      bool check_line(Position, CellState) const;
      bool check_diagonal(unsigned int, char, int, int, CellState) const;
      void fill_diagonal(unsigned int, char, int, int, Player);

      uint64_t white;
      uint64_t black;
  };
}

#endif