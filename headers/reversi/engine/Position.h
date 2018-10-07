#ifndef REVERSI_ENGINE_POSITION_H_
#define REVERSI_ENGINE_POSITION_H_

#include <cinttypes>
#include <iosfwd>

namespace Reversi {
	
	class Position {
	 public:
		Position(char, unsigned int);
		
		char getColumn() const;
		unsigned int getRow() const;

		bool operator==(Position) const;

		static bool isPossible(char, unsigned int);
	 private:
		uint8_t row : 3, column : 3;
	};

	std::ostream &operator<<(std::ostream &, Position);
}

#endif
