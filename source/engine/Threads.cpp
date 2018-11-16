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