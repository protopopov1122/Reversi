/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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