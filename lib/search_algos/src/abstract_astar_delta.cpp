/**
 * @file abstract_astar_delta.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar delta-stepping algorithm class source file
 */

#include "abstract_astar_delta.hpp"
#include <cmath>

namespace SP
{

ReturnCode AbstractAStarDeltaAlgo::buildResult()
{
    ReturnCode rc = AbstractDeltaSteppingAlgo::buildResult();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    // The label is the reduced distance d(v) + h(v) - h(source)
    if (!std::isinf(outData.shortestDistance))
    {
        outData.shortestDistance +=
            potential(this->source) - potential(this->destination);
    }

    return rc;
}

} // namespace SP
