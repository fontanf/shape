#pragma once

#include "shape/shape.hpp"

namespace shape
{

using RowId = int64_t;
using ColumnId = int64_t;

struct CellId
{
    ColumnId column = 0;
    RowId row = 0;
};

struct Cell
{
    /** Fraction of the cell's area covered by the shape, in [0.0, 1.0]. */
    double coverage = 0.0;

    /**
     * The part of the shape that lies within this cell.
     *
     * Only populated for cells not entirely covered (coverage < 1.0); a
     * fully covered cell's shape is simply its whole rectangle, so there is
     * no need to store it (see cell_to_shape).
     */
    MultiShapeWithHoles shape;
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

    std::vector<Cell> cells;

    Cell& at(
            ColumnId column,
            RowId row)
    {
        return cells[(column - column_offset) * number_of_rows + (row - row_offset)];
    }

    const Cell& at(
            ColumnId column,
            RowId row) const
    {
        return cells[(column - column_offset) * number_of_rows + (row - row_offset)];
    }
};

/**
 * Shape rasterization.
 *
 * Return a dense grid covering the bounding box of the given shape. Each
 * cell's coverage is 0.0 (outside), 1.0 (entirely inside) or in between
 * (intersecting the shape's boundary), and its shape attribute holds the
 * part of the original shape that lies within that cell.
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
 * Convert a cell position to a shape.
 */
Shape cell_to_shape(
        const CellId& cell,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * Convert a list of cell positions into shapes with holes.
 */
MultiShapeWithHoles cells_to_shapes(
        const std::vector<CellId>& cells,
        LengthDbl cell_width,
        LengthDbl cell_height);

/**
 * How cells_to_shapes(const RasterizedGrid&, ...) should handle partially
 * covered (0.0 < coverage < 1.0) cells.
 */
enum class CellsToShapesMode
{
    /** Include a partial cell as its whole rectangle (over-approximation). */
    Outer,
    /** Include a partial cell as its exact clipped shape. */
    Exact,
    /** Exclude partial cells entirely (under-approximation). */
    Inner,
};

/**
 * Convert a rasterized grid into shapes with holes.
 *
 * Fully covered cells are always included as their whole rectangle; mode
 * controls how partially covered cells are handled.
 */
MultiShapeWithHoles cells_to_shapes(
        const RasterizedGrid& grid,
        LengthDbl cell_width,
        LengthDbl cell_height,
        CellsToShapesMode mode = CellsToShapesMode::Exact);

}
