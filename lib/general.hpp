/**
 * @file general.hpp
 * @author tarakanov.2004@mail.ru
 * @brief Header file for general entities
 */
#pragma once

#include "graphio.h"

namespace SP
{
    // Edge comparator for setting priority queue
    struct compareEdges
    {
        bool operator()(const edge &e1, const edge &e2)
        {
            return e1.val > e2.val;
        }
    };
} // namespace SP
