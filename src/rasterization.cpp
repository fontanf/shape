//#define RASTERIZATION_ENABLE_DEBUG

#include "shape/rasterization.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/elements_intersections.hpp"
#ifdef RASTERIZATION_ENABLE_DEBUG
#include "shape/writer.hpp"
#endif

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#ifdef RASTERIZATION_ENABLE_DEBUG
#include <iostream>
#endif

using namespace shape;

CellId find_cell(
        LengthDbl cell_width,
        LengthDbl cell_height,
        const Point& point)
{
    CellId cell;
    cell.column = (ColumnId)std::floor(point.x / cell_width);
    cell.row = (RowId)std::floor(point.y / cell_height);
    return cell;
}

bool contains(
        LengthDbl cell_width,
        LengthDbl cell_height,
        const CellId& cell,
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
        const CellId& cell,
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
        AxisAlignedBoundingBox aabb = element.min_max();
        // Retrieve all intersections between the element and the grid.
        ColumnId column_low = find_cell(cell_width, cell_height, {aabb.x_min, aabb.y_min}).column;
        ColumnId column_high = find_cell(cell_width, cell_height, {aabb.x_max, aabb.y_max}).column;
        for (ColumnId column = column_low; column <= column_high + 1; ++column) {
            ShapeElement line_segment = build_line_segment(
                    {column * cell_width, aabb.y_min - 1},
                    {column * cell_width, aabb.y_max + 1});
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
#ifdef RASTERIZATION_ENABLE_DEBUG
    std::cout << "intersection_points" << std::endl;
    for (const ShapePoint& point: intersection_points)
        std::cout << point.element_pos << " " << point.point.to_string() << std::endl;
#endif
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
    // A vertex on a column line is found as the end of an element and as the
    // start of the next one; keep it once.
    std::vector<ShapePoint> unique_points;
    for (const ShapePoint& point: intersection_points)
        if (unique_points.empty() || !equal(point.point, unique_points.back().point))
            unique_points.push_back(point);
    if (unique_points.size() > 1
            && equal(unique_points.front().point, unique_points.back().point)) {
        unique_points.pop_back();
    }
    // Remove the points where the shape touches a column line without
    // crossing it, i.e. where the parts of the boundary before and after
    // the point are in the same column. Otherwise, these two parts would end
    // at the same height, and the region above them couldn't be deduced from
    // both.
    std::vector<ColumnId> part_columns(unique_points.size());
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)unique_points.size();
            ++point_pos) {
        const ShapePoint& point_prev = unique_points[(point_pos + unique_points.size() - 1) % unique_points.size()];
        const ShapePoint& point = unique_points[point_pos];
        Point point_between = shape.find_point_between(point_prev, point).point;
        CellId cell = find_cell(cell_width, cell_height, point_between);
        if (equal(point_between.x, cell.column * cell_width)
                || equal(point_between.x, (cell.column + 1) * cell_width)) {
            // This part is on a column line.
            part_columns[point_pos] = std::numeric_limits<ColumnId>::min();
        } else {
            part_columns[point_pos] = cell.column;
        }
    }
    intersection_points.clear();
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)unique_points.size();
            ++point_pos) {
        ColumnId column_before = part_columns[point_pos];
        ColumnId column_after = part_columns[(point_pos + 1) % unique_points.size()];
        if (column_before != std::numeric_limits<ColumnId>::min()
                && column_before == column_after) {
            continue;
        }
        intersection_points.push_back(unique_points[point_pos]);
    }
    if (intersection_points.empty()) {
        // The shape is between two consecutive column lines. The region
        // around it is outside the shape, or inside the outer shape for a
        // hole.
        AxisAlignedBoundingBox aabb = shape.compute_min_max();
        ColumnId column = find_cell(
                cell_width,
                cell_height,
                {(aabb.x_min + aabb.x_max) / 2, aabb.y_min}).column;
        ColumnIntersection column_intersection;
        column_intersection.y_min = aabb.y_min;
        column_intersection.y_max = aabb.y_max;
        column_intersection.is_above_inside = is_hole;
        column_intersections[column - column_intersections_offset].push_back(column_intersection);
        return;
    }
    // For each pair of consecutive intersection points, add the cells to which
    // the point inbetween belongs to.
    ShapePoint point_prev = intersection_points.back();
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)intersection_points.size();
            ++point_pos) {
        const ShapePoint& point = intersection_points[point_pos];
        ShapePoint point_between = shape.find_point_between(point_prev, point);
        CellId cell = find_cell(cell_width, cell_height, point_between.point);
        if (!equal(point_between.point.x, cell.column * cell_width)
                && !equal(point_between.point.x, (cell.column + 1) * cell_width)) {
            AxisAlignedBoundingBox aabb = shape.compute_min_max(point_prev, point);
            ColumnIntersection column_intersection;
            column_intersection.y_min = aabb.y_min;
            column_intersection.y_max = aabb.y_max;
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
#ifdef RASTERIZATION_ENABLE_DEBUG
            std::cout << "column " << cell.column
                << " y_min " << column_intersection.y_min
                << " y_max " << column_intersection.y_max
                << std::endl;
#endif
        }
        point_prev = point;
    }
}

RasterizedGrid shape::rasterization(
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    AxisAlignedBoundingBox aabb = shape.compute_min_max();
    ColumnId column_min = find_cell(cell_width, cell_height, {aabb.x_min, aabb.y_min}).column;
    ColumnId column_max = find_cell(cell_width, cell_height, {aabb.x_max, aabb.y_max}).column;
    RowId row_min = find_cell(cell_width, cell_height, {aabb.x_min, aabb.y_min}).row;
    RowId row_max = find_cell(cell_width, cell_height, {aabb.x_max, aabb.y_max}).row;
#ifdef RASTERIZATION_ENABLE_DEBUG
    std::cout << "column min " << column_min << " max " << column_max << std::endl;
    std::cout << "row min " << row_min << " max " << row_max << std::endl;
#endif

    RasterizedGrid grid;
    grid.column_offset = column_min;
    grid.row_offset = row_min;
    grid.number_of_columns = column_max - column_min + 1;
    grid.number_of_rows = row_max - row_min + 1;
    grid.cells = std::vector<Cell>(grid.number_of_columns * grid.number_of_rows);

    // Cells fully inside the shape: known immediately, no need for the
    // intersection computation below; their shape is simply their whole
    // rectangle, so there is no need to store it (see cell_to_shape).
    auto mark_full = [&](ColumnId column, RowId row) {
        grid.at(column, row).coverage = 1.0;
    };
    // Cells intersecting the shape's boundary: their exact shape part and
    // coverage are computed once all of them are known, below.
    std::vector<CellId> border_cells;
    auto mark_border = [&](ColumnId column, RowId row) {
        border_cells.push_back({column, row});
    };

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
#ifdef RASTERIZATION_ENABLE_DEBUG
    std::cout << "number_of_intersections " << number_of_intersections << std::endl;
#endif

    // Single-column or single-row case: fill_columns_intersections produces no
    // records because the shape crosses no horizontal grid lines.
    if (number_of_intersections == 0) {
        for (ColumnId column = column_min; column <= column_max; ++column)
            for (RowId row = row_min; row <= row_max; ++row)
                mark_border(column, row);
    } else {
        for (ColumnId column = column_min; column <= column_max; ++column) {
#ifdef RASTERIZATION_ENABLE_DEBUG
            std::cout << "column " << column << std::endl;
#endif
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
            RowId row_hi_prev = row_min - 1;
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
#ifdef RASTERIZATION_ENABLE_DEBUG
                std::cout << "i " << intersection_pos
                    << " is_above_inside_prev " << is_above_inside_prev
                    << " row_hi_prev " << row_hi_prev
                    << " y_min " << column_intersection.y_min
                    << " y_max " << column_intersection.y_max
                    << " row_lo " << row_lo
                    << " row_hi " << row_hi
                    << std::endl;
#endif
                if (is_above_inside_prev) {
                    for (RowId row = row_hi_prev + 1; row < row_lo; ++row)
                        mark_full(column, row);
                }
                for (RowId row = (std::max)(row_lo, row_hi_prev + 1); row <= row_hi; ++row)
                    mark_border(column, row);
                row_hi_prev = (std::max)(row_hi_prev, row_hi);
                // A part of the boundary ending lower than the highest one
                // so far doesn't change the region above the highest one.
                // Two parts ending at the same height either don't touch,
                // and the region above them is the same, or touch and are
                // parts of the outer shape and of a hole, or of two holes
                // (the parts of a shape touching a column line are merged in
                // fill_columns_intersections). The region between them is
                // then inside the shape, so the region above them is inside
                // the shape only if it is for both.
                if (strictly_greater(column_intersection.y_max, y_max_prev)) {
                    is_above_inside_prev = column_intersection.is_above_inside;
                    y_max_prev = column_intersection.y_max;
                } else if (equal(column_intersection.y_max, y_max_prev)) {
                    is_above_inside_prev = is_above_inside_prev && column_intersection.is_above_inside;
                }
            }
        }
    }

    // Compute the exact shape part and coverage of each border cell by
    // intersecting the original shape with the union of the border cells'
    // rectangles, without merging the resulting faces together: since cell
    // boundaries are part of the arrangement, no face can straddle two
    // cells, so each face's interior point identifies exactly one cell.
    if (!border_cells.empty()) {
        MultiShapeWithHoles shape_group;
        shape_group.shapes_with_holes.push_back(shape);

        MultiShapeWithHoles border_cells_group;
        for (const CellId& cell: border_cells) {
            border_cells_group.shapes_with_holes.push_back(
                    {cell_to_shape(cell, cell_width, cell_height)});
        }

        std::vector<ShapeWithHoles> faces = compute_intersection_faces(
                {shape_group, border_cells_group});
        for (const ShapeWithHoles& face: faces) {
            Point point = face.find_point_strictly_inside();
            CellId cell = find_cell(cell_width, cell_height, point);
            grid.at(cell.column, cell.row).shape.shapes_with_holes.push_back(face);
        }

        for (const CellId& cell_id: border_cells) {
            Cell& cell = grid.at(cell_id.column, cell_id.row);
            AreaDbl area = 0.0;
            for (const ShapeWithHoles& face: cell.shape.shapes_with_holes)
                area += face.compute_area();
            cell.coverage = area / (cell_width * cell_height);
        }
    }

    return grid;
}

Shape shape::cell_to_shape(
        const CellId& cell,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    return build_rectangle(
            {cell.column * cell_width, cell.row * cell_height},
            {(cell.column + 1) * cell_width, (cell.row + 1) * cell_height});
}

MultiShapeWithHoles shape::cells_to_shapes(
        const std::vector<CellId>& cells,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    std::vector<ShapeWithHoles> union_input;
    for (const CellId& cell: cells)
        union_input.push_back({cell_to_shape(cell, cell_width, cell_height)});
    return compute_union(union_input);
}

MultiShapeWithHoles shape::cells_to_shapes(
        const RasterizedGrid& grid,
        LengthDbl cell_width,
        LengthDbl cell_height,
        CellsToShapesMode mode)
{
    std::vector<ShapeWithHoles> union_input;
    for (ColumnId column = grid.column_offset;
            column < grid.column_offset + grid.number_of_columns;
            ++column) {
        for (RowId row = grid.row_offset;
                row < grid.row_offset + grid.number_of_rows;
                ++row) {
            const Cell& cell = grid.at(column, row);
            if (equal(cell.coverage, 1.0)) {
                union_input.push_back({cell_to_shape({column, row}, cell_width, cell_height)});
            } else if (cell.coverage > 0.0) {
                switch (mode) {
                case CellsToShapesMode::Outer: {
                    union_input.push_back({cell_to_shape({column, row}, cell_width, cell_height)});
                    break;
                } case CellsToShapesMode::Exact: {
                    for (const ShapeWithHoles& part: cell.shape.shapes_with_holes)
                        union_input.push_back(part);
                    break;
                } case CellsToShapesMode::Inner: {
                    break;
                }
                }
            }
        }
    }
    return compute_union(union_input);
}

void shape::rasterization_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl cell_width,
        LengthDbl cell_height)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape"] = shape.to_json();
    json["cell_width"] = cell_width;
    json["cell_height"] = cell_height;
    file << std::setw(4) << json << std::endl;
}
