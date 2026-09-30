/**
 * @file abstract_astar_delta_bidir.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Bidirectional Astar delta-stepping algorithm class source file
 */

#include "abstract_astar_delta_bidir.hpp"
#include <cmath>

namespace SP
{

ReturnCode AbstractAStarDeltaBiDirAlgo::buildResult()
{
    ReturnCode rc = AbstractDeltaSteppingBiDirAlgo::buildResult();
    if (ReturnCode::OK != rc)
    {
        return rc;
    }

    // The best path is found in reduced weights, which shift the length of
    // every path by p(destination) - p(source)
    if (!std::isinf(outData.shortestDistance))
    {
        outData.shortestDistance +=
            potential(this->source) - potential(this->destination);
    }

    return rc;
}

} // namespace SP
