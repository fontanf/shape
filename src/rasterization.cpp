#include "shape/rasterization.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/elements_intersections.hpp"

#include <algorithm>
#include <cmath>

using namespace shape;

Cell find_cell(
        LengthDbl cell_width,
        LengthDbl cell_height,
        const Point& point)
{
    Cell cell;
    cell.column = (ColumnId)std::floor(point.x / cell_width);
    cell.row = (RowId)std::floor(point.y / cell_height);
    return cell;
}

bool contains(
        LengthDbl cell_width,
        LengthDbl cell_height,
        const Cell& cell,
        const Point& point)
{
    if (strictly_lesser(point.x, cell.column * cell_width))
        return false;
    if (strictly_greater(point.x, (cell.column + 1) * cell_width))
        return false;
    if (strictly_lesser(point.y, cell.row * cell_height))
        return false;
    if (strictly_greater(point.y, (cell.row + 1) * cell_height))
        return false;
    return true;
}

bool strictly_contains(
        LengthDbl cell_width,
        LengthDbl cell_height,
        const Cell& cell,
        const Point& point)
{
    if (!strictly_greater(point.x, cell.column * cell_width))
        return false;
    if (!strictly_lesser(point.x, (cell.column + 1) * cell_width))
        return false;
    if (!strictly_greater(point.y, cell.row * cell_height))
        return false;
    if (!strictly_lesser(point.y, (cell.row + 1) * cell_height))
        return false;
    return true;
}

struct ColumnIntersection
{
    LengthDbl y_in;
    LengthDbl y_out;
};

void fill_columns_intersections(
        LengthDbl cell_width,
        LengthDbl cell_height,
        std::vector<std::vector<ColumnIntersection>>& column_intersections,
        ColumnId column_intersections_offset,
        const Shape& shape)
{
    std::vector<ShapePoint> intersection_points;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        auto mm = element.min_max();
        // Retrieve all intersections between the element and the grid.
        RowId row_low = find_cell(cell_width, cell_height, mm.first).row;
        RowId row_high = find_cell(cell_width, cell_height, mm.second).row;
        for (RowId row = row_low; row <= row_high + 1; ++row) {
            ShapeElement line_segment = build_line_segment(
                    {mm.first.x - 1, row * cell_height},
                    {mm.second.x + 1, row * cell_height});
            ShapeElementIntersectionsOutput intersections = compute_intersections(element, line_segment);
            for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
                intersection_points.push_back({element_pos, overlapping_part.start});
                intersection_points.push_back({element_pos, overlapping_part.end});
            }
            for (const Point& point: intersections.improper_intersections)
                intersection_points.push_back({element_pos, point});
            for (const Point& point: intersections.proper_intersections)
                intersection_points.push_back({element_pos, point});
        }
    }
    if (intersection_points.empty())
        return;
    // Sort intersection points by their length on the element.
    std::sort(
            intersection_points.begin(),
            intersection_points.end(),
            [&shape](
                const ShapePoint& point_1,
                const ShapePoint& point_2)
            {
                return shape.is_strictly_closer_to_path_start(point_1, point_2);
            });
    // For each pair of consecutive intersection points, add the cells to which
    // the point inbetween belongs to.
    ShapePoint point_prev = intersection_points.back();
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)intersection_points.size();
            ++point_pos) {
        const ShapePoint& point = intersection_points[point_pos];
        ShapePoint point_between = shape.find_point_between(point_prev, point);
        Cell cell = find_cell(cell_width, cell_height, point_between.point);
        if (!equal(point_between.point.x, cell.column * cell_width)
                && !equal(point_between.point.x, (cell.column + 1) * cell_width)) {
            ColumnIntersection column_intersection;
            column_intersection.y_in = point_prev.point.y;
            column_intersection.y_out = point.point.y;
            column_intersections[cell.column - column_intersections_offset].push_back(column_intersection);
        }
        point_prev = point;
    }
}

std::vector<IntersectedCell> shape::rasterization(
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    std::vector<IntersectedCell> cells;

    std::pair<Point, Point> mm = shape.compute_min_max();
    ColumnId col_min = find_cell(cell_width, cell_height, mm.first).column;
    ColumnId col_max = find_cell(cell_width, cell_height, mm.second).column;
    RowId row_min = find_cell(cell_width, cell_height, mm.first).row;
    RowId row_max = find_cell(cell_width, cell_height, mm.second).row;

    // Single-column or single-row case: fill_columns_intersections produces no
    // records because the shape crosses no horizontal grid lines.
    if (col_min == col_max || row_min == row_max) {
        for (ColumnId col = col_min; col <= col_max; ++col) {
            for (RowId row = row_min; row <= row_max; ++row) {
                Point center = {(col + 0.5) * cell_width, (row + 0.5) * cell_height};
                if (shape.contains(center)) {
                    IntersectedCell ic;
                    ic.cell = {col, row};
                    ic.full = false;
                    cells.push_back(ic);
                }
            }
        }
        return cells;
    }

    // General case: col_inters is guaranteed non-empty for each column.
    std::vector<std::vector<ColumnIntersection>> column_intersections(col_max - col_min + 1);
    fill_columns_intersections(cell_width, cell_height, column_intersections, col_min, shape.shape);
    for (const Shape& hole: shape.holes)
        fill_columns_intersections(cell_width, cell_height, column_intersections, col_min, hole);

    for (ColumnId col = col_min; col <= col_max; ++col) {
        const std::vector<ColumnIntersection>& col_inters =
                column_intersections[col - col_min];

        // Sort intersections by ascending min(y_in, y_out).
        std::vector<ColumnIntersection> sorted_inters = col_inters;
        std::sort(
                sorted_inters.begin(),
                sorted_inters.end(),
                [](const ColumnIntersection& a, const ColumnIntersection& b) {
                    return std::min(a.y_in, a.y_out) < std::min(b.y_in, b.y_out);
                });

        for (size_t i = 0; i < sorted_inters.size(); ++i) {
            const ColumnIntersection& ci = sorted_inters[i];
            RowId row_lo = (RowId)std::floor(std::min(ci.y_in, ci.y_out) / cell_height);
            RowId row_hi = (RowId)std::floor(std::max(ci.y_in, ci.y_out) / cell_height) - 1;
            // Emit border cells for this intersection span.
            for (RowId row = row_lo; row <= row_hi; ++row) {
                IntersectedCell ic;
                ic.cell = {col, row};
                ic.full = false;
                cells.push_back(ic);
            }
            // Emit full cells between this intersection and the next.
            if (i + 1 < sorted_inters.size()) {
                const ColumnIntersection& next = sorted_inters[i + 1];
                RowId full_lo = (RowId)std::floor(std::max(ci.y_in, ci.y_out) / cell_height);
                RowId full_hi = (RowId)std::floor(std::min(next.y_in, next.y_out) / cell_height) - 1;
                for (RowId row = full_lo; row <= full_hi; ++row) {
                    IntersectedCell ic2;
                    ic2.cell = {col, row};
                    ic2.full = true;
                    cells.push_back(ic2);
                }
            }
        }
    }

    return cells;
}

std::vector<ShapeWithHoles> shape::cells_to_shapes(
        std::vector<Cell> cells,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    std::vector<ShapeWithHoles> shapes;
    for (const Cell& cell: cells) {
        LengthDbl x0 = cell.column * cell_width;
        LengthDbl y0 = cell.row * cell_height;
        LengthDbl x1 = (cell.column + 1) * cell_width;
        LengthDbl y1 = (cell.row + 1) * cell_height;
        ShapeWithHoles shape;
        shape.shape = build_shape({{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}});
        shapes.push_back(shape);
    }
    return compute_union(shapes);
}
