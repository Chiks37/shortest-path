/**
 * @file alt_delta_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional ALT delta-stepping algorithm class source file
 */

#include "alt_delta_bidir.hpp"

namespace SP
{
ReturnCode ALTDeltaBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarDeltaBiDirAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc : landmarks.build(graph);
}

double ALTDeltaBiDirAlgo::heuristic(int vertex, int target)
{
    return landmarks.lowerBound(vertex, target);
}

} // namespace SP
