/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "reversi/engine/Library.h"
#include "reversi/engine/Engine.h"
#include <algorithm>

namespace Reversi {

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

  void MoveLibrary::getMove(const State &state, std::vector<Position> &moves) const {
    if (this->library.count(state)) {
      const std::vector<Position> &savedMoves = this->library.at(state);
      moves = savedMoves;
    }
  }
}