#ifndef REVERSI_ENGINE_TIME_H_
#define REVERSI_ENGINE_TIME_H_

#include <chrono>
#include <functional>

namespace Reversi {

  class TimeUtils {
   public:
    template <typename T = std::chrono::microseconds, typename C = std::chrono::high_resolution_clock>
    static auto stopwatches() {
      std::chrono::time_point start_time_point = C::now();
      return [start_time_point] {
        std::chrono::time_point end_time_point = C::now();
        return std::chrono::duration_cast<T>(end_time_point - start_time_point);
      };
    };
  };
}

#endif