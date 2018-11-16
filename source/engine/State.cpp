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

#include "reversi/engine/State.h"

namespace Reversi {

  State::State()
    : board(), player(Player::Black) {}

  State::State(const Board &board, Player player)
    : board(board), player(player) {}

  State::State(const State &state)
    : board(state.board), player(state.player) {}

  State &State::operator=(const State &state) {
    this->board = state.board;
    this->player = state.player;
    return *this;
  }

  const Board &State::getBoard() const {
    return this->board;
  }

  Player State::getPlayer() const {
    return this->player;
  }

  bool State::apply(Position pos) {
    bool res = this->board.putDisc(pos, this->player);
    if (res) {
      this->player = invertPlayer(this->player);
    }
    return res;
  }

  void State::next() {
    this->player = invertPlayer(this->player);
  }

  bool State::operator<(const State &other) const {
    if (this->board < other.board) {
      return true;
    } else if (this->board == other.board) {
      return static_cast<int>(this->player) < static_cast<int>(other.player);
    } else {
      return false;
    }
  }

  bool State::operator==(const State &other) const {
    return this->board == other.board && this->player == other.player;
  }
}