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
