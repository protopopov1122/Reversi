#ifndef REVERSI_ENGINE_THREADS_H_
#define REVERSI_ENGINE_THREADS_H_

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <future>
#include <memory>
#include <condition_variable>
#include <iostream>

namespace Reversi {

  class FixedThreadPool {
   public:
    FixedThreadPool(std::size_t);
    ~FixedThreadPool();

    bool isActive() const;
    std::size_t getThreadCount() const;

    template <typename F>
    auto submit(F &&f) -> std::future<decltype(f())> {
      auto task = std::make_shared<std::packaged_task<decltype(f())()>>(std::forward<F>(f));
      auto function = [task]() {
        (*task)();
      };
      std::unique_lock<std::mutex> lock(this->queue_mutex);
      this->queue.push_back(function);
      this->queue_cond.notify_one();
      return task->get_future();
    }
   private:
    void spawnThreads(std::size_t);

    std::mutex queue_mutex;
    std::mutex threads_mutex;
    std::condition_variable queue_cond;
    std::vector<std::function<void()>> queue;
    std::vector<std::unique_ptr<std::thread>> threads;
    bool working;
  };
}

#endif