/**
 * @file astarg_delta.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic delta-stepping algorithm class header
 * file
 */
#include "astar_delta.hpp"
#include <utility>
#include <vector>

namespace SP
{

class AStarGDeltaAlgo : public AStarDeltaAlgo
{
  public:
    AStarGDeltaAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AStarDeltaAlgo(graphFileName),
          nodesMappingFileName(nodesMappingFileName)
    {
    }

  protected:
    std::string nodesMappingFileName;
    std::vector<std::pair<double, double>> vertexCoordinates;

    virtual ReturnCode preProcessImpl() override;
    virtual double heuristic(int vertex) override;
    ReturnCode loadNodeMapping();
};

} // namespace SP
