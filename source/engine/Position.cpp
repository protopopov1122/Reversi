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

#include <iostream>
#include "reversi/engine/Position.h"
#include "reversi/engine/Error.h"

namespace Reversi {

	Position::Position(char column, unsigned int row) {
		if (column < 'A' || column > 'H') {
			throw ReversiError("Row must be between A and H");	
		}
		if (row == 0 || row > 8) {
			throw ReversiError("Column must be between 1 and 8");
		}
		this->column = column - 'A';
		this->row = row - 1;
	}

	char Position::getColumn() const {
		return this->column + 'A';
	}

	unsigned int Position::getRow() const {
		return this->row + 1;
	}

	bool Position::operator==(Position other) const {
		return this->row == other.row && this->column == other.column;
	}
	 
	bool Position::isPossible(char column, unsigned int row) {
		return column >= 'A' && column <= 'H' &&
			row >= 1 && row <= 8;
	}

	std::ostream &operator<<(std::ostream &os, Position pos) {
		os << pos.getColumn() << pos.getRow();
		return os;
	}
}
