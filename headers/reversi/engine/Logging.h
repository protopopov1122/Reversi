#ifndef REVERSI_ENGINE_LOGGIGNG_H_
#define REVERSI_ENGINE_LOGGIGNG_H_

#include <functional>
#include <string>

namespace Reversi {

  class Logger {
   public:
    static void log(const std::string &, std::function<void (std::ostream &)>);
  };
}

#endif