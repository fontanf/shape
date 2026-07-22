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

enum class CellState
{
    Empty,
    Full,
    Border,
};

/**
 * Dense rasterization grid.
 *
 * Cell (column, row) is stored at
 * cells[(column - column_offset) * number_of_rows + (row - row_offset)],
 * i.e. column-major, matching the order in which rasterization() computes
 * it.
 */
struct RasterizedGrid
{
    ColumnId column_offset = 0;
    RowId row_offset = 0;
    ColumnId number_of_columns = 0;
    RowId number_of_rows = 0;

    std::vector<CellState> cells;

    CellState& at(
            ColumnId column,
            RowId row)
    {
        return cells[(column - column_offset) * number_of_rows + (row - row_offset)];
    }

    CellState at(
            ColumnId column,
            RowId row) const
    {
        return cells[(column - column_offset) * number_of_rows + (row - row_offset)];
    }
};

/**
 * Shape rasterization.
 *
 * Return a dense grid covering the bounding box of the given shape, with
 * each cell marked Empty, Full (entirely inside the shape) or Border
 * (intersecting the shape's boundary).
 */
RasterizedGrid rasterization(
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height);

void rasterization_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * Convert a cell to a shape.
 */
Shape cell_to_shape(
        const Cell& cell,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * Convert a list of cells into shapes with holes.
 */
MultiShapeWithHoles cells_to_shapes(
        const std::vector<Cell>& cells,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * Convert a rasterized grid into shapes with holes.
 */
MultiShapeWithHoles cells_to_shapes(
        const RasterizedGrid& grid,
        LengthDbl cell_width,
        LengthDbl cell_height,
        bool only_full = false);

}
