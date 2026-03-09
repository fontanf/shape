#include "shape/rasterization.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/shapes_intersections.hpp"
#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <algorithm>

using namespace shape;


struct RasterizationTestParams
{
    ShapeWithHoles shape;
    LengthDbl cell_width;
    LengthDbl cell_height;
};

bool operator==(const Cell& a, const Cell& b)
{
    return a.column == b.column && a.row == b.row;
}

class RasterizationTest: public testing::TestWithParam<RasterizationTestParams> { };

TEST_P(RasterizationTest, Rasterization)
{
    RasterizationTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "cell_width " << test_params.cell_width
        << " cell_height " << test_params.cell_height << std::endl;

    Writer writer;
    writer.add_shape_with_holes(test_params.shape).write_json("rasterization_input.json");
    std::vector<IntersectedCell> cells = rasterization(
            test_params.shape,
            test_params.cell_width,
            test_params.cell_height);
    for (const IntersectedCell& cell: cells)
        writer.add_shape(cell_to_shape(cell.cell, test_params.cell_width, test_params.cell_height));
    writer.write_json("rasterization_output.json");

    std::cout << "cells (" << cells.size() << ")" << std::endl;
    for (const IntersectedCell& ic: cells) {
        std::cout << "  col=" << ic.cell.column
            << " row=" << ic.cell.row
            << " full=" << ic.full << std::endl;
    }

    // Property 1: no duplicate cells.
    std::vector<Cell> cell_keys;
    for (const IntersectedCell& ic: cells)
        cell_keys.push_back(ic.cell);
    std::sort(
            cell_keys.begin(),
            cell_keys.end(),
            [](const Cell& a, const Cell& b) {
                if (a.column != b.column)
                    return a.column < b.column;
                return a.row < b.row;
            });
    for (size_t i = 1; i < cell_keys.size(); ++i) {
        EXPECT_FALSE(cell_keys[i] == cell_keys[i - 1])
            << "duplicate cell col=" << cell_keys[i].column
            << " row=" << cell_keys[i].row;
    }

    // Property 2: all full=true cells have their center strictly inside the shape.
    for (const IntersectedCell& ic: cells) {
        if (!ic.full)
            continue;
        Point center = {
            (ic.cell.column + 0.5) * test_params.cell_width,
            (ic.cell.row + 0.5) * test_params.cell_height};
        EXPECT_TRUE(test_params.shape.contains(center, true))
            << "full cell col=" << ic.cell.column
            << " row=" << ic.cell.row
            << " center (" << center.x << "," << center.y << ")"
            << " is not strictly inside the shape";
    }

    // Property 3: all returned cells intersect the original shape.
    for (const IntersectedCell& cell: cells) {
        Shape cell_shape = cell_to_shape({cell.cell}, test_params.cell_width, test_params.cell_height);
        EXPECT_TRUE(intersect(test_params.shape, cell_shape, true))
            << "cell col=" << cell.cell.column
            << " row=" << cell.cell.row
            << " does not intersect the shape";
    }

    std::vector<Cell> all_cells;
    for (const IntersectedCell& cell: cells)
        all_cells.push_back(cell.cell);
    ShapeWithHoles cells_union = cells_to_shapes(all_cells, test_params.cell_width, test_params.cell_height).front();
    std::vector<ShapeWithHoles> union_output = compute_union({cells_union, test_params.shape});
    EXPECT_EQ(union_output.size(), 1);
    EXPECT_TRUE(equal(union_output.front(), cells_union));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        RasterizationTest,
        testing::ValuesIn(std::vector<RasterizationTestParams>{
            // Shape contained within a single cell.
            {  // Tiny triangle inside cell (0,0).
                {{build_shape({{0.1, 0.1}, {0.9, 0.1}, {0.5, 0.8}})}, {}},
                1.0, 1.0,
            },
            {  // Small rectangle inside cell (2,3).
                {{build_shape({{2.2, 3.2}, {2.8, 3.2}, {2.8, 3.8}, {2.2, 3.8}})}, {}},
                1.0, 1.0,
            },
            // Single-column cases (col_min == col_max).
            {  // Tall thin strip spanning 3 rows.
                {{build_shape({{0.4, 0.1}, {0.6, 0.1}, {0.6, 2.9}, {0.4, 2.9}})}, {}},
                1.0, 1.0,
            },
            {  // Thin strip in column 1, spanning 4 rows.
                {{build_shape({{1.3, 0.5}, {1.7, 0.5}, {1.7, 3.5}, {1.3, 3.5}})}, {}},
                1.0, 1.0,
            },
            // Multi-column rectangles.
            {  // 2x1 rectangle.
                {{build_shape({{0.2, 0.2}, {1.8, 0.2}, {1.8, 0.8}, {0.2, 0.8}})}, {}},
                1.0, 1.0,
            },
            {  // 3x3 rectangle with a fully interior cell.
                {{build_shape({{0.1, 0.1}, {2.9, 0.1}, {2.9, 2.9}, {0.1, 2.9}})}, {}},
                1.0, 1.0,
            },
            {  // 5x5 rectangle with many fully interior cells.
                {{build_shape({{0.1, 0.1}, {4.9, 0.1}, {4.9, 4.9}, {0.1, 4.9}})}, {}},
                1.0, 1.0,
            },
            // Non-rectangular shapes.
            {  // Right triangle fitting in a 2x2 grid.
                {{build_shape({{0.1, 0.1}, {1.9, 0.1}, {0.1, 1.9}})}, {}},
                1.0, 1.0,
            },
            {  // L-shape: 2 cells wide at bottom, 1 cell wide at top.
                {{build_shape({{0.1, 0.1}, {1.9, 0.1}, {1.9, 0.9},
                               {0.9, 0.9}, {0.9, 1.9}, {0.1, 1.9}})}, {}},
                1.0, 1.0,
            },
            // Shape with a hole.
            {  // Square ring: 5x5 outer with 3x3 inner hole.
                {
                    build_shape({{0.1, 0.1}, {4.9, 0.1}, {4.9, 4.9}, {0.1, 4.9}}),
                    {build_shape({{1.1, 1.1}, {3.9, 1.1}, {3.9, 3.9}, {1.1, 3.9}})},
                },
                1.0, 1.0,
            },
            // Large cell: entire shape fits within a single cell.
            {  // Rectangle much smaller than the cell.
                {{build_shape({{1.0, 1.0}, {4.0, 1.0}, {4.0, 4.0}, {1.0, 4.0}})}, {}},
                10.0, 10.0,
            },
            // Non-unit cell dimensions.
            {  // Rectangle with 2x3 cells.
                {{build_shape({{0.5, 0.5}, {5.5, 0.5}, {5.5, 5.5}, {0.5, 5.5}})}, {}},
                2.0, 3.0,
            },
            {  // Rectangle with 2x3 cells.
                {shape::build_rectangle(100, 50).rotate(30)},
                10,
                10,
            },
        }));
