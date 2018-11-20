#include "reversi/engine/Library.h"
#include "reversi/engine/Engine.h"
#include <random>
#include <algorithm>

namespace Reversi {

  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());

  StateBuilder::StateBuilder()
    : state(StateHelpers::getDefaultInitialState()) {}
  
  StateBuilder::StateBuilder(const State &state)
    : state(state) {}

  StateBuilder & StateBuilder::apply(Position pos) {
    this->state.apply(pos);
    return *this;
  }

  StateBuilder &StateBuilder::apply(char column, unsigned int row) {
    this->state.apply(Position(column, row));
    return *this;
  }

  void MoveLibrary::addMoves(const State &state, std::initializer_list<Position> chain, bool mirroring) {
    State base(state);
    State mirror(state);
    State diagonalMirror(state);
    State diagonalMirror2(state);
    for (Position move : chain) {
      this->addMove(base, move);
      base.apply(move);

      if (mirroring) {
        char column = move.getColumn();
        unsigned int row = move.getRow();
        if (column > 'D') {
          column = 'D' - (move.getColumn() - 'E');
        } else {
          column = 'H' - (move.getColumn() - 'A');
        }
        if (row > 4) {
          row = 4 - (row - 5);
        } else {
          row = 8 - (row - 1);
        }
        Position mirrorMove(column, row);
        this->addMove(mirror, mirrorMove);
        mirror.apply(mirrorMove);


        column = 'A' + move.getRow() - 1;
        row = 1 + move.getColumn() - 'A';
        Position diagonalMirrorMove(column, row);
        this->addMove(diagonalMirror, diagonalMirrorMove);
        diagonalMirror.apply(diagonalMirrorMove);

        column = 'A' + mirrorMove.getRow() - 1;
        row = 1 + mirrorMove.getColumn() - 'A';
        Position diagonalMirrorMove2(column, row);
        this->addMove(diagonalMirror2, diagonalMirrorMove2);
        diagonalMirror2.apply(diagonalMirrorMove2);
      }
    }
  }

  void MoveLibrary::addMove(const State &state, Position position) {
    if (this->library.count(state)) {
      std::vector<Position> &vec = this->library[state];
      if (std::find(vec.begin(), vec.end(), position) == vec.end()) {
        vec.push_back(position);
      }
    } else {
      this->library[state] = std::vector<Position>({ position });
    }
  }

  bool MoveLibrary::hasMove(const State &state) const {
    return this->library.count(state) != 0;
  }

  std::optional<Position> MoveLibrary::getMove(const State &state) const {
    if (this->library.count(state)) {
      const std::vector<Position> &moves = this->library.at(state);
      std::uniform_int_distribution<> distribution(0, moves.size() - 1);
      return moves.at(distribution(random_generator));
    } else {
      return std::optional<Position>();
    }
  }
}