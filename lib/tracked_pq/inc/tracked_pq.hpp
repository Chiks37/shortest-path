/**
 * @file tracked_pq.hpp
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

    // An item is counted from push until mark_as_done, so the queue is never
    // seen drained while an item is passed from the queue to a worker
    void push(const edge &item)
    {
        unfinished_count++;
        queue.push(item);
    }

    bool try_pop(edge &item) { return queue.try_pop(item); }

    void mark_as_done() { unfinished_count--; }

    bool is_drained() const { return 0 == unfinished_count.load(); }

    void clear()
    {
        queue.clear();
        unfinished_count.store(0);
    }

  protected:
    tbb::concurrent_priority_queue<edge, compareEdges> queue;
    std::atomic<int> unfinished_count{0};
};
} // namespace SP