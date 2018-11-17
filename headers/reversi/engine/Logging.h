#ifndef REVERSI_ENGINE_LOGGIGNG_H_
#define REVERSI_ENGINE_LOGGIGNG_H_

#include <functional>
#include <chrono>
#include <string>
#include "reversi/config.h"

namespace Reversi {

  class Logger {
   public:
    static void log(const std::string &, std::function<void (std::ostream &)>);
    template <typename T = std::chrono::microseconds, typename C = std::chrono::high_resolution_clock>
    static auto measureTime() {
      std::chrono::time_point start_time_point = C::now();
      return [start_time_point] {
        std::chrono::time_point end_time_point = C::now();
        return std::chrono::duration_cast<T>(end_time_point - start_time_point);
      };
    };
  };
}

#endif