//
// Created by nikita on 24.08.23.
//

#ifndef CASINO_CONCURRENT_QUEUE_H
#define CASINO_CONCURRENT_QUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template <typename T>
struct concurrent_queue {
  bool push(T value) {
    {
      std::unique_lock<std::mutex> ul(m);
      if (open) {
        deq.push_back(std::move(value));
      } else {
        return false;
      }
    }
    update.notify_one();
    return true;
  }

  bool empty() const noexcept {
    std::lock_guard<std::mutex> lg(m);
    return deq.empty();
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> ulk(m);
    update.wait(ulk, [&] { return !deq.empty() || !open; });
    if (!open) {
      return std::nullopt;
    }
    T result = deq.front();
    deq.pop_front();
    return result;
  }

  std::optional<T> try_pop(int sleep_) {
    std::unique_lock<std::mutex> ulk(m);
    update.wait_for(ulk, std::chrono::seconds(sleep_), [&] { return !deq.empty() || !open; });
    if (deq.empty()) {
      return std::nullopt;
    }
    T result = deq.front();
    deq.pop_front();
    return result;
  }

  void close() noexcept {
    std::lock_guard lg(m);
    open = false;
    update.notify_all();
  }

  bool is_closed() const noexcept {
    std::lock_guard lg(m);
    return !open;
  }

private:
  mutable std::mutex m;
  bool open{true};
  std::deque<T> deq;
  std::condition_variable update;
};

#endif // CASINO_CONCURRENT_QUEUE_H