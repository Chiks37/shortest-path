/**
 * @file graph_transpose.cpp
 * @author tarakanov.2004@mail.ru
 * @brief Graph transposition source file
 */

#include "graph_transpose.hpp"

namespace SP
{

void transposeGraph(const crsGraph &graph, std::vector<int> &xadj,
                    std::vector<int> &adjncy, std::vector<double> &eweights,
                    crsGraph &transposed)
{
    int V = graph.V;
    int nz = graph.Xadj[V];

    xadj.assign(V + 1, 0);
    adjncy.resize(nz);
    eweights.resize(nz);

    for (int i = 0; i < nz; ++i)
    {
        xadj[graph.Adjncy[i] + 1]++;
    }
    for (int v = 0; v < V; ++v)
    {
        xadj[v + 1] += xadj[v];
    }

    std::vector<int> cursor(xadj.begin(), xadj.end() - 1);
    for (int u = 0; u < V; ++u)
    {
        for (int i = graph.Xadj[u]; i < graph.Xadj[u + 1]; ++i)
        {
            int pos = cursor[graph.Adjncy[i]]++;
            adjncy[pos] = u;
            eweights[pos] = graph.Eweights[i];
        }
    }

    transposed.Xadj = xadj.data();
    transposed.Adjncy = adjncy.data();
    transposed.Eweights = eweights.data();
    transposed.V = V;
    transposed.nz = nz;
}

} // namespace SP
