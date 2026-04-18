/**
 * @file astarg.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Astar with geometical heuristic algorithm class header file
 */
#include "astar.hpp"
#include <utility>
#include <vector>

namespace SP
{

class AStarGAlgo : public AStarAlgo
{
  public:
    AStarGAlgo(std::string graphFileName, std::string nodesMappingFileName)
        : AStarAlgo(graphFileName), nodesMappingFileName(nodesMappingFileName)
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