#include "reversi/engine/Logging.h"
#include <iostream>

namespace Reversi {

  void Logger::log(const std::string &tag, std::function<void (std::ostream &)> log) {
    if constexpr (ENABLE_LOGGING) {
      const int PADDING = 10;
      std::string fullTag("[" + tag + "]");
      if (fullTag.size() < PADDING) {
        fullTag.insert(fullTag.size(), PADDING - fullTag.size(), ' ');
      }
      std::cout << fullTag << '\t';
      log(std::cout);
      std::cout << std::endl;
    }
  }
}