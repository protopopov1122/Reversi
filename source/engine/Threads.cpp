#include "reversi/engine/Threads.h"

namespace Reversi {

  FixedThreadPool::FixedThreadPool(std::size_t thread_count) {
    this->spawnThreads(thread_count);
  }

  FixedThreadPool::~FixedThreadPool() {
    this->working = false;
    std::unique_lock<std::mutex> lock(this->threads_mutex);
    this->queue_cond.notify_all();
    this->queue_cond.wait(lock, [&]() {
      return this->threads.empty();
    });
  }

  bool FixedThreadPool::isActive() const {
    return this->working;
  }

  std::size_t FixedThreadPool::getThreadCount() const {
    return this->threads.size();
  }

  void FixedThreadPool::spawnThreads(std::size_t thread_count) {
    this->working = true;
    for (unsigned int i = 0; i < thread_count; i++) {
      std::unique_ptr<std::thread> thread = std::make_unique<std::thread>([&, i]() {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        lock.unlock();
        while (true) {
          lock.lock();
          while (this->queue.empty() && this->working) {
            this->queue_cond.wait(lock);
          }
          if (!this->working) {
            std::unique_lock<std::mutex> lock(this->threads_mutex);
            this->threads.erase(this->threads.begin() + i);
            this->queue_cond.notify_all();
            break;
          } else {
            std::function<void()> fn = this->queue.at(0);
            this->queue.erase(this->queue.begin());
            lock.unlock();
            fn();
          }
        }
      });
      thread->detach();
      this->threads.push_back(std::move(thread));
    }
  }
}