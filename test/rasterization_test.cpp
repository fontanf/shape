//#define RASTERIZATION_TEST_DEBUG

#include "shape/rasterization.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/shapes_intersections.hpp"
#ifdef RASTERIZATION_TEST_DEBUG
#include "shape/writer.hpp"
#endif

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct RasterizationTestParams
{
    std::string name;
    ShapeWithHoles shape;
    LengthDbl cell_width;
    LengthDbl cell_height;


    static RasterizationTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        RasterizationTestParams test_params;
        test_params.name = file_path;
        test_params.shape = ShapeWithHoles::from_json(json["shape"]);
        test_params.cell_width = json["cell_width"];
        test_params.cell_height = json["cell_height"];
        return test_params;
    }
};

void PrintTo(const RasterizationTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "cell_width " << params.cell_width
        << " cell_height " << params.cell_height << "\n";
}

class RasterizationTest: public testing::TestWithParam<RasterizationTestParams> { };

TEST_P(RasterizationTest, Rasterization)
{
    RasterizationTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef RASTERIZATION_TEST_DEBUG
    Writer writer;
    writer.add_shape_with_holes(test_params.shape).write_json("rasterization_input.json");
#endif
    RasterizedGrid grid = rasterization(
            test_params.shape,
            test_params.cell_width,
            test_params.cell_height);
#ifdef RASTERIZATION_TEST_DEBUG
    for (ColumnId column = grid.column_offset;
            column < grid.column_offset + grid.number_of_columns;
            ++column) {
        for (RowId row = grid.row_offset;
                row < grid.row_offset + grid.number_of_rows;
                ++row) {
            if (grid.at(column, row).coverage > 0.0)
                writer.add_shape(cell_to_shape({column, row}, test_params.cell_width, test_params.cell_height));
        }
    }
    writer.write_json("rasterization_output.json");
#endif

    std::cout << "grid " << grid.number_of_columns << "x" << grid.number_of_rows
        << " offset (" << grid.column_offset << "," << grid.row_offset << ")" << std::endl;
    for (ColumnId column = grid.column_offset;
            column < grid.column_offset + grid.number_of_columns;
            ++column) {
        for (RowId row = grid.row_offset;
                row < grid.row_offset + grid.number_of_rows;
                ++row) {
            const Cell& cell = grid.at(column, row);
            if (cell.coverage <= 0.0)
                continue;
            std::cout << "  col=" << column
                << " row=" << row
                << " coverage=" << cell.coverage << std::endl;
        }
    }

    LengthDbl cell_area = test_params.cell_width * test_params.cell_height;
    for (ColumnId column = grid.column_offset;
            column < grid.column_offset + grid.number_of_columns;
            ++column) {
        for (RowId row = grid.row_offset;
                row < grid.row_offset + grid.number_of_rows;
                ++row) {
            const Cell& cell = grid.at(column, row);

            // Property 1: coverage stays within [0, 1].
            EXPECT_GE(cell.coverage, 0.0)
                << "col=" << column << " row=" << row;
            EXPECT_LE(cell.coverage, 1.0 + 1e-9)
                << "col=" << column << " row=" << row;

            // Property 2: fully covered cells have their center strictly
            // inside the shape.
            if (equal(cell.coverage, 1.0)) {
                Point center = {
                    (column + 0.5) * test_params.cell_width,
                    (row + 0.5) * test_params.cell_height};
                EXPECT_TRUE(test_params.shape.contains(center, true))
                    << "full cell col=" << column
                    << " row=" << row
                    << " center (" << center.x << "," << center.y << ")"
                    << " is not strictly inside the shape";
            }

            Shape cell_shape = cell_to_shape({column, row}, test_params.cell_width, test_params.cell_height);
            bool cell_intersects = intersect(test_params.shape, cell_shape, true);

            // Property 3: covered cells (coverage > 0) intersect the shape;
            // uncovered cells (coverage == 0) do not.
            if (cell.coverage > 0.0) {
                EXPECT_TRUE(cell_intersects)
                    << "cell col=" << column
                    << " row=" << row
                    << " does not intersect the shape";
            } else {
                EXPECT_FALSE(cell_intersects)
                    << "empty cell col=" << column
                    << " row=" << row
                    << " intersects the shape";
            }

            // Property 4: a partially covered cell's stored shape area
            // matches its coverage ratio.
            if (cell.coverage > 0.0 && !equal(cell.coverage, 1.0)) {
                AreaDbl shape_area = 0.0;
                for (const ShapeWithHoles& part: cell.shape.shapes_with_holes)
                    shape_area += part.compute_area();
                EXPECT_TRUE(equal(shape_area / cell_area, cell.coverage))
                    << "cell col=" << column
                    << " row=" << row
                    << " stored shape area / cell area (" << (shape_area / cell_area) << ")"
                    << " does not match coverage (" << cell.coverage << ")";
            }
        }
    }

    // The union of all cells' exact shape parts must reconstruct the
    // original shape.
    MultiShapeWithHoles cells_union = cells_to_shapes(
            grid, test_params.cell_width, test_params.cell_height);
    ASSERT_EQ(cells_union.shapes_with_holes.size(), 1);
    EXPECT_TRUE(equal(cells_union.shapes_with_holes.front(), test_params.shape));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        RasterizationTest,
        testing::ValuesIn(std::vector<RasterizationTestParams>{
            // Shape contained within a single cell.
            {  // Tiny triangle inside cell (0,0).
                "TinyTriangle",
                {{build_shape({{0.1, 0.1}, {0.9, 0.1}, {0.5, 0.8}})}, {}},
                1.0, 1.0,
            },
            {  // Small rectangle inside cell (2,3).
                "SmallRectangle",
                {{build_shape({{2.2, 3.2}, {2.8, 3.2}, {2.8, 3.8}, {2.2, 3.8}})}, {}},
                1.0, 1.0,
            },
            // Single-column cases (col_min == col_max).
            {  // Tall thin strip spanning 3 rows.
                "TallThinStrip3Rows",
                {{build_shape({{0.4, 0.1}, {0.6, 0.1}, {0.6, 2.9}, {0.4, 2.9}})}, {}},
                1.0, 1.0,
            },
            {  // Thin strip in column 1, spanning 4 rows.
                "ThinStrip4Rows",
                {{build_shape({{1.3, 0.5}, {1.7, 0.5}, {1.7, 3.5}, {1.3, 3.5}})}, {}},
                1.0, 1.0,
            },
            // Multi-column rectangles.
            {  // 2x1 rectangle.
                "Rectangle2x1",
                {{build_shape({{0.2, 0.2}, {1.8, 0.2}, {1.8, 0.8}, {0.2, 0.8}})}, {}},
                1.0, 1.0,
            },
            {  // 3x3 rectangle with a fully interior cell.
                "Rectangle3x3",
                {{build_shape({{0.1, 0.1}, {2.9, 0.1}, {2.9, 2.9}, {0.1, 2.9}})}, {}},
                1.0, 1.0,
            },
            {  // 5x5 rectangle with many fully interior cells.
                "Rectangle5x5",
                {{build_shape({{0.1, 0.1}, {4.9, 0.1}, {4.9, 4.9}, {0.1, 4.9}})}, {}},
                1.0, 1.0,
            },
            // Non-rectangular shapes.
            {  // Right triangle fitting in a 2x2 grid.
                "RightTriangle",
                {{build_shape({{0.1, 0.1}, {1.9, 0.1}, {0.1, 1.9}})}, {}},
                1.0, 1.0,
            },
            {  // L-shape: 2 cells wide at bottom, 1 cell wide at top.
                "LShape",
                {{build_shape({{0.1, 0.1}, {1.9, 0.1}, {1.9, 0.9},
                               {0.9, 0.9}, {0.9, 1.9}, {0.1, 1.9}})}, {}},
                1.0, 1.0,
            },
            // Shape with a hole.
            {  // Square ring: 5x5 outer with 3x3 inner hole.
                "SquareRing",
                {
                    build_shape({{0.1, 0.1}, {4.9, 0.1}, {4.9, 4.9}, {0.1, 4.9}}),
                    {build_shape({{1.1, 1.1}, {3.9, 1.1}, {3.9, 3.9}, {1.1, 3.9}})},
                },
                1.0, 1.0,
            },
            // Large cell: entire shape fits within a single cell.
            {  // Rectangle much smaller than the cell.
                "LargeCell",
                {{build_shape({{1.0, 1.0}, {4.0, 1.0}, {4.0, 4.0}, {1.0, 4.0}})}, {}},
                10.0, 10.0,
            },
            // Non-unit cell dimensions.
            {  // Rectangle with 2x3 cells.
                "Rectangle2x3Cells",
                {{build_shape({{0.5, 0.5}, {5.5, 0.5}, {5.5, 5.5}, {0.5, 5.5}})}, {}},
                2.0, 3.0,
            },
            {  // Rectangle with 2x3 cells.
                "RotatedRectangle",
                {shape::build_rectangle(100, 50).rotate(30)},
                10,
                10,
            },
            RasterizationTestParams::read_json(
                    (fs::path("data") / "tests" / "rasterization" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<RasterizationTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });
