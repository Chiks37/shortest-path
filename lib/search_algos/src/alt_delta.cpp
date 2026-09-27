/**
 * @file alt_delta.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Alt delta-stepping algorithm class source file
 */

#include "alt_delta.hpp"

namespace SP
{
ReturnCode ALTDeltaAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarDeltaAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc : landmarks.build(graph);
}

double ALTDeltaAlgo::heuristic(int vertex)
{
    return landmarks.lowerBound(vertex, this->destination);
}

} // namespace SP
