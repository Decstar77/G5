#pragma once

#include "atto_containers.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <atomic>

namespace atto {
    template<typename _type_, i32 c>
    class FixedQueueThreadSafe {
    public:
        FixedQueueThreadSafe() {
            queue.Clear(true);
        }

        ~FixedQueueThreadSafe() {
        }

        void Enqueue(const _type_ &item) {
            std::unique_lock<std::mutex> lock(mutex);
            queue.Enqueue(item);
        }

        _type_ Dequeue() {
            std::unique_lock<std::mutex> lock(mutex);
            return queue.Dequeue();
        }

        bool IsEmpty() {
            std::unique_lock<std::mutex> lock(mutex);
            return queue.IsEmpty();
        }

    private:
        std::mutex mutex;
        FixedQueue<_type_, c> queue;
    };
}
