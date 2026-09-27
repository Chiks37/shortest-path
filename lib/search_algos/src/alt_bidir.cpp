/**
 * @file alt_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional ALT algorithm class source file
 */

#include "alt_bidir.hpp"

namespace SP
{
ReturnCode ALTBiDirAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarBiDirAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc : landmarks.build(graph);
}

double ALTBiDirAlgo::heuristic(int vertex, int target)
{
    return landmarks.lowerBound(vertex, target);
}

} // namespace SP
