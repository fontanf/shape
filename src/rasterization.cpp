#include "shape/rasterization.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/elements_intersections.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

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
    LengthDbl y_min;
    LengthDbl y_max;
    bool is_above_inside;
};

void fill_columns_intersections(
        LengthDbl cell_width,
        LengthDbl cell_height,
        std::vector<std::vector<ColumnIntersection>>& column_intersections,
        ColumnId column_intersections_offset,
        const Shape& shape,
        bool is_hole)
{
    std::vector<ShapePoint> intersection_points;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        auto mm = element.min_max();
        // Retrieve all intersections between the element and the grid.
        ColumnId column_low = find_cell(cell_width, cell_height, mm.first).column;
        ColumnId column_high = find_cell(cell_width, cell_height, mm.second).column;
        for (ColumnId column = column_low; column <= column_high + 1; ++column) {
            ShapeElement line_segment = build_line_segment(
                    {column * cell_width, mm.first.y - 1},
                    {column * cell_width, mm.second.y + 1});
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
    //std::cout << "intersection_points" << std::endl;
    //for (const ShapePoint& point: intersection_points)
    //    std::cout << point.element_pos << " " << point.point.to_string() << std::endl;
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
            auto mm = shape.compute_min_max(point_prev, point);
            ColumnIntersection column_intersection;
            column_intersection.y_min = mm.first.y;
            column_intersection.y_max = mm.second.y;
            bool in_left = (equal(point_prev.point.x, cell.column * cell_width));
            bool out_left = (equal(point.point.x, cell.column * cell_width));
            if (in_left) {
                if (out_left) {
                    if (point_prev.point.y < point.point.y) {
                        column_intersection.is_above_inside = false;
                    } else {
                        column_intersection.is_above_inside = true;
                    }
                } else {
                    column_intersection.is_above_inside = true;
                }
            } else {
                if (out_left) {
                    column_intersection.is_above_inside = false;
                } else {
                    if (point_prev.point.y < point.point.y) {
                        column_intersection.is_above_inside = true;
                    } else {
                        column_intersection.is_above_inside = false;
                    }
                }
            }
            if (is_hole)
                column_intersection.is_above_inside = !column_intersection.is_above_inside;
            column_intersections[cell.column - column_intersections_offset].push_back(column_intersection);
            //std::cout << "column " << cell.column
            //    << " y_min " << column_intersection.y_min
            //    << " y_max " << column_intersection.y_max
            //    << std::endl;
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
    ColumnId column_min = find_cell(cell_width, cell_height, mm.first).column;
    ColumnId column_max = find_cell(cell_width, cell_height, mm.second).column;
    RowId row_min = find_cell(cell_width, cell_height, mm.first).row;
    RowId row_max = find_cell(cell_width, cell_height, mm.second).row;
    //std::cout << "column min " << column_min << " max " << column_max << std::endl;
    //std::cout << "row min " << row_min << " max " << row_max << std::endl;

    // General case: column_inters is guaranteed non-empty for each column.
    std::vector<std::vector<ColumnIntersection>> column_intersections(column_max - column_min + 1);
    fill_columns_intersections(
            cell_width,
            cell_height,
            column_intersections,
            column_min,
            shape.shape,
            false);
    for (const Shape& hole: shape.holes) {
        fill_columns_intersections(
                cell_width,
                cell_height,
                column_intersections,
                column_min,
                hole,
                true);
    }
    ElementPos number_of_intersections = 0;
    for (ColumnId column = column_min; column <= column_max; ++column)
        number_of_intersections += column_intersections[column - column_min].size();
    //std::cout << "number_of_intersections " << number_of_intersections << std::endl;

    // Single-column or single-row case: fill_columns_intersections produces no
    // records because the shape crosses no horizontal grid lines.
    if (number_of_intersections == 0) {
        for (ColumnId column = column_min; column <= column_max; ++column) {
            for (RowId row = row_min; row <= row_max; ++row) {
                IntersectedCell cell;
                cell.cell = {column, row};
                cell.full = false;
                cells.push_back(cell);
            }
        }
        //std::cout << "cells.size() " << cells.size() << std::endl;
        return cells;
    }

    for (ColumnId column = column_min; column <= column_max; ++column) {
        //std::cout << "column " << column << std::endl;
        const std::vector<ColumnIntersection>& column_inters =
                column_intersections[column - column_min];

        // Sort intersections by ascending min(y_in, y_out).
        std::vector<ColumnIntersection> sorted_intersections = column_inters;
        std::sort(
                sorted_intersections.begin(),
                sorted_intersections.end(),
                [](
                    const ColumnIntersection& column_intersection_1,
                    const ColumnIntersection& column_intersection_2)
                {
                    return column_intersection_1.y_min < column_intersection_2.y_min;
                });

        LengthDbl y_max_prev = -std::numeric_limits<LengthDbl>::infinity();
        bool is_above_inside_prev = false;
        for (ElementPos intersection_pos = 0;
                intersection_pos < sorted_intersections.size();
                ++intersection_pos) {
            const ColumnIntersection& column_intersection = sorted_intersections[intersection_pos];
            RowId row_lo = (RowId)std::floor(column_intersection.y_min / cell_height);
            if (equal((row_lo + 1) * cell_height, column_intersection.y_min))
                row_lo++;
            RowId row_hi = (RowId)std::floor(column_intersection.y_max / cell_height);
            if (equal(row_hi * cell_height, column_intersection.y_max))
                row_hi--;
            RowId row_hi_prev = (RowId)std::floor(y_max_prev / cell_height);
            //std::cout << "i " << intersection_pos
            //    << " row_lo " << row_lo
            //    << " row_hi " << row_hi
            //    << std::endl;
            if (is_above_inside_prev) {
                for (RowId row = row_hi_prev + 1; row < row_lo; ++row) {
                    IntersectedCell cell;
                    cell.cell = {column, row};
                    cell.full = true;
                    cells.push_back(cell);
                }
            }
            for (RowId row = (std::max)(row_lo, row_hi_prev + 1); row <= row_hi; ++row) {
                IntersectedCell cell;
                cell.cell = {column, row};
                cell.full = false;
                cells.push_back(cell);
            }
            row_hi_prev = row_hi;
            if (column_intersection.y_max > y_max_prev) {
                is_above_inside_prev = column_intersection.is_above_inside;
                y_max_prev = column_intersection.y_max;
            } else {
                is_above_inside_prev = is_above_inside_prev & column_intersection.is_above_inside;
            }
        }
    }

    return cells;
}

Shape shape::cell_to_shape(
        const Cell& cell,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    return build_rectangle(
            {cell.column * cell_width, cell.row * cell_height},
            {(cell.column + 1) * cell_width, (cell.row + 1) * cell_height});
}

std::vector<ShapeWithHoles> shape::cells_to_shapes(
        const std::vector<Cell>& cells,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    std::vector<ShapeWithHoles> union_input;
    for (const Cell& cell: cells)
        union_input.push_back({cell_to_shape(cell, cell_width, cell_height)});
    return compute_union(union_input);
}
