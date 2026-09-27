/**
 * @file alt.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Alt algorithm class source file
 */

#include "alt.hpp"

namespace SP
{
ReturnCode ALTAlgo::preProcessImpl()
{
    ReturnCode rc = AbstractAStarAlgo::preProcessImpl();
    return rc != ReturnCode::OK ? rc : landmarks.build(graph);
}

double ALTAlgo::heuristic(int vertex)
{
    return landmarks.lowerBound(vertex, this->destination);
}

} // namespace SP
