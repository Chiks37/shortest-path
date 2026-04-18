/**
 * @file priority_queue.hpp
 * @author tarakanov.2004@mail.ru
 * @brief TBB concurrent priority queue class-wrapper header file. Wrapper has
 * additional functionality for tracking of in-progress items in the queue.
 */
#pragma once
#include "general.hpp"
#include <tbb/concurrent_priority_queue.h>

namespace SP
{
class TrackedPriorityQueue
{
  public:
    TrackedPriorityQueue() = default;
    ~TrackedPriorityQueue() = default;

    void push(const edge &item) { queue.push(item); }

    bool try_pop(edge &item)
    {
        auto rc = queue.try_pop(item);
        in_progress_count += rc ? 1 : 0;
        return rc;
    }

    void mark_as_done() { in_progress_count--; }

    bool is_drained() const
    {
        return queue.empty() && 0 == in_progress_count.load();
    }

    void clear()
    {
        queue.clear();
        in_progress_count.store(0);
    }

  protected:
    tbb::concurrent_priority_queue<edge, compareEdges> queue;
    std::atomic<int> in_progress_count{0};
};
} // namespace SP