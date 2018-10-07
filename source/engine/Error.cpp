#include "reversi/engine/Error.h"

namespace Reversi {

	ReversiError::ReversiError(std::string description)
	 : description(std::move(description)) {}

	const char *ReversiError::what() const noexcept {
		return this->description.c_str();
	}
}
