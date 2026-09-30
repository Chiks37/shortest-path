/**
 * @file vertex_cache.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Per-query cache of vertex values header file
 */
#pragma once

#include <algorithm>
#include <atomic>
#include <vector>

namespace SP
{
// Value of each vertex computed at most once per query. A value is valid while
// its stamp equals the stamp of the current query, so a new query invalidates
// all of them at once. Threads missing the same value compute and store the
// same number, the stamp is published after the value.
class VertexCache
{
  public:
    void resize(int vertexCount)
    {
        values.assign(vertexCount, 0.0);
        stamps.assign(vertexCount, 0);
        stamp = 0;
    }

    void nextQuery()
    {
        if (++stamp == 0)
        {
            std::fill(stamps.begin(), stamps.end(), 0);
            stamp = 1;
        }
    }

    template <typename Compute> double get(int vertex, Compute compute)
    {
        std::atomic_ref<unsigned> vertexStamp(stamps[vertex]);
        std::atomic_ref<double> value(values[vertex]);
        if (vertexStamp.load(std::memory_order_acquire) == stamp)
        {
            return value.load(std::memory_order_relaxed);
        }

        double computed = compute();
        value.store(computed, std::memory_order_relaxed);
        vertexStamp.store(stamp, std::memory_order_release);
        return computed;
    }

  private:
    std::vector<double> values;
    std::vector<unsigned> stamps;
    unsigned stamp{0};
};
} // namespace SP
