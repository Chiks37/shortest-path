/**
 * @file astarg.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic algorithm class header file
 */
#include "abstract_astar.hpp"
#include <utility>
#include <vector>

namespace SP
{

class AStarGAlgo final : public AbstractAStarAlgo
{
  public:
    AStarGAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AbstractAStarAlgo(graphFileName),
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