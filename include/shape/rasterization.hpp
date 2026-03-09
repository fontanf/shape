#pragma once

#include "shape/shape.hpp"

namespace shape
{

using RowId = int64_t;
using ColumnId = int64_t;

struct Cell
{
    ColumnId column = 0;
    RowId row = 0;
};

struct IntersectedCell
{
    Cell cell;
    bool full = true;
};

/**
 * Shape rasterization.
 *
 * Return all the cells fully inside or intersecting the border of the given
 * shape.
 */
std::vector<IntersectedCell> rasterization(
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height);

Shape cell_to_shape(
        const Cell& cell,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * Convert a list of cells into shapes with holes.
 */
std::vector<ShapeWithHoles> cells_to_shapes(
        const std::vector<Cell>& cells,
        LengthDbl cell_width,
        LengthDbl cell_height);

}
