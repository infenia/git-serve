/*
 * Copyright 2025 Infenia Private Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <atomic>
#include <cstdint>

template <typename T, int size>
class LockFreeQueue {
  T data[size];
  std::atomic_uint32_t head;
  std::atomic_uint32_t head_pending;
  std::atomic_uint32_t tail;
  std::atomic_uint32_t tail_pending;

  uint32_t mask = size - 1;

 public:
  LockFreeQueue() {
    head.store(0);
    head_pending.store(0);
    tail.store(0);
    tail_pending.store(0);
  }

  bool is_empty() { return head == tail; }

  bool push(T value) {
    uint32_t current_head, current_pending_tail, new_tail;

    current_pending_tail = tail_pending.load(std::memory_order_relaxed);
    do {
      current_head = head.load(std::memory_order_relaxed);
      new_tail     = current_pending_tail + 1;
      // Check if queue is full
      if ((new_tail & mask) == (current_head & mask)) {
        return false;
      }
    } while (!tail_pending.compare_exchange_strong(
        current_pending_tail, new_tail, std::memory_order_relaxed));

    data[current_pending_tail & mask] = value;

    uint32_t expected = 0;
    do {
      expected = current_pending_tail;
    } while (!tail.compare_exchange_strong(expected, new_tail,
                                           std::memory_order_seq_cst,
                                           std::memory_order_relaxed));

    tail.notify_one();

    return true;
  }

  bool pop(T *value) {
    uint32_t current_pending_head, current_tail, new_head;

    current_pending_head = head_pending.load(std::memory_order_relaxed);
    do {
      current_tail = tail.load(std::memory_order_relaxed);
      new_head     = current_pending_head + 1;
      // Check if queue is empty
      if ((current_pending_head & mask) == (current_tail & mask)) {
        return false;
      }
    } while (!head_pending.compare_exchange_strong(
        current_pending_head, new_head, std::memory_order_seq_cst,
        std::memory_order_relaxed));

    *value = data[current_pending_head & mask];

    uint32_t expected = 0;
    do {
      expected = current_pending_head;
    } while (!head.compare_exchange_strong(expected, new_head,
                                           std::memory_order_seq_cst,
                                           std::memory_order_relaxed));
    return true;
  }

  void wait() {
    uint32_t current_tail = tail.load(std::memory_order_relaxed);
    if (current_tail == head.load(std::memory_order_relaxed)) {
      tail.wait(current_tail);
    }
  }
};