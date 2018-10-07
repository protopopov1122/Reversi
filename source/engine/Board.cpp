#include <iostream>
#include <algorithm>
#include "reversi/engine/Board.h"

namespace Reversi {

  Board::Board()
    : white(0), black(0) {}

  Board::Board(const Board &board)
    : white(board.white), black(board.black) {}

  Board &Board::operator=(const Board &board) {
    this->white = board.white;
    this->black = board.black;
    return *this;
  }

  CellState Board::getCellState(Position position) const {
    std::size_t index = this->position_to_index(position);
    if (((this->white >> index) & 1) != 0) {
      return CellState::White;
    } else if (((this->black >> index) & 1) != 0) {
      return CellState::Black;
    } else {
      return CellState::Empty;
    }
  }

  bool Board::putDisc(Position position, Player player) {
    if (this->getCellState(position) != CellState::Empty) {
      return false;
    } else {
      std::size_t index = this->position_to_index(position);
      uint64_t value = 1LL << index;
      if (player == Player::White) {
        this->white |= value;
      } else {
        this->black |= value;
      }
      this->fill_diagonal(position.getRow(), position.getColumn(), -1, 0, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), 1, 0, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), 0, -1, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), 0, 1, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), -1, -1, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), -1, 1, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), 1, -1, player);
      this->fill_diagonal(position.getRow(), position.getColumn(), 1, 1, player);
      return true;
    }
  }

  void Board::getMoves(std::vector<Position> &positions, Player player) const {
    for (std::size_t i = 0; i < 64; i++) {
      Position position(i % 8 + 'A', i / 8 + 1);
      if ((player == Player::White && ((this->black >> i) & 1) != 0) ||
        (player == Player::Black && ((this->white >> i) & 1) != 0)) {
        this->addMovement(position, positions, player == Player::White ? CellState::White : CellState::Black);
      }
    }
    std::sort(positions.begin(), positions.end(), [&](Position p1, Position p2) {
      return this->position_to_index(p1) < this->position_to_index(p2);
    });
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
  }

  int32_t Board::getMetric(BoardReduceFunction eval) const {
    int32_t metric = 0;
    for (std::size_t i = 0; i < 64; i++) {
      unsigned int row = i / 8 + 1;
      char col = i % 8 + 'A';
      Position pos(col, row);
      metric = eval(metric, this->getCellState(pos), pos);
    }
    return metric;
  }

  int32_t Board::getPlainMetric() const {
    int32_t metric = 0;
    for (std::size_t i = 0; i < 64; i++) {
      if (((this->black >> i) & 1) != 0) {
        metric += static_cast<int>(CellState::Black);
      } else if (((this->white >> i) & 1) != 0) {
        metric += static_cast<int>(CellState::White);
      }
    }
    return metric;
  }

  std::size_t Board::position_to_index(Position position) const {
    return (position.getRow() - 1) * 8 + position.getColumn() - 'A';
  }

  void Board::addMovement(Position position, std::vector<Position> &positions, CellState state) const {
    unsigned int row = position.getRow();
    char col = position.getColumn();
    this->addMovementPosition(col - 1, row, positions, state);
    this->addMovementPosition(col + 1, row, positions, state);
    this->addMovementPosition(col, row - 1, positions, state);
    this->addMovementPosition(col, row + 1, positions, state);
    this->addMovementPosition(col - 1, row - 1, positions, state);
    this->addMovementPosition(col + 1, row - 1, positions, state);
    this->addMovementPosition(col - 1, row + 1, positions, state);
    this->addMovementPosition(col + 1, row + 1, positions, state);
  }

  void Board::addMovementPosition(char col, unsigned int row, std::vector<Position> &positions, CellState state) const {
    if (Position::isPossible(col, row) &&
      this->getCellState(Position(col, row)) == CellState::Empty &&
      this->check_line(Position(col, row), state)) {
      positions.push_back(Position(col, row));
    }
  }

  bool Board::check_line(Position position, CellState state) const {
    unsigned int r = position.getRow();
    char c = position.getColumn();
    return this->check_diagonal(r, c, 0, -1, state) ||
      this->check_diagonal(r, c, 0, 1, state) ||
      this->check_diagonal(r, c, -1, 0, state) ||
      this->check_diagonal(r, c, 1, 0, state) ||
      this->check_diagonal(r, c, -1, -1, state) ||
      this->check_diagonal(r, c, 1, -1, state) ||
      this->check_diagonal(r, c, -1, 1, state) ||
      this->check_diagonal(r, c, 1, 1, state);
  }

  bool Board::check_diagonal(unsigned int row, char column, int rdir, int cdir, CellState dest) const {
    unsigned int c;
    unsigned int r;
    int count = 0;
    for (c = column + cdir, r = row + rdir;
      c >= 'A' && c <= 'H' && r >= 1 && r <= 8;
      c += cdir, r += rdir) {
      Position pos(c, r);
      if (this->getCellState(pos) == CellState::Empty) {
        return false;
      } else if (this->getCellState(pos) != dest) {
        count++;
      } else if (this->getCellState(Position(c, r)) == dest) {
        return count > 0;
      }
    }
    return false;
  }

  void Board::fill_diagonal(unsigned int row, char column, int rdir, int cdir, Player player) {
    CellState state = player == Player::White  ? CellState::Black : CellState::White;
    CellState dest = player == Player::White  ? CellState::White : CellState::Black;
    bool found = this->check_diagonal(row, column, rdir, cdir, dest);
    unsigned int c;
    unsigned int r;
    for (c = column + cdir, r = row + rdir;
      c >= 'A' && c <= 'H' && r >= 1 && r <= 8 && found && this->getCellState(Position(c, r)) == state;
      c += cdir, r += rdir) {
      std::size_t index = this->position_to_index(Position(c, r));
      uint64_t value = 1LL << index;
      if (player == Player::White) {
        this->black &= ~value;
        this->white |= value;
      } else {
        this->white &= ~value;
        this->black |= value;
      }
    }
  }

  std::ostream &operator<<(std::ostream &os, const Board &board) {
    for (char r = 1; r <= 8; r++) {
      if (r > 1) {
        os << '|' << std::endl;
      }
      for (char c = 'A'; c <= 'H'; c++) {
        Position position(c, r);
        CellState state = board.getCellState(position);
        if (state == CellState::White) {
          os << "|+";
        } else if (state == CellState::Black) {
          os << "|-";
        } else {
          os << "| ";
        }
      }
    }
    os << '|';
    return os;
  }
}