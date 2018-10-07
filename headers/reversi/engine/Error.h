#ifndef REVERSI_ENGINE_ERROR_H_
#define REVERSI_ENGINE_ERROR_H_

#include <string>
#include <exception>

namespace Reversi {

	class ReversiError : public std::exception {
	 public:
		ReversiError(std::string);
		const char *what() const noexcept override;
	 private:
		std::string description;
	};
}

#endif
