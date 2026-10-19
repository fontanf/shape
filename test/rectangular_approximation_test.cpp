#include "shape/rectangular_approximation.hpp"

#include "shape/boolean_operations.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct OuterApproximationByRectanglesTestParams
{
    std::string name;
    std::vector<OuterApproximationByRectanglesInputShape> shapes;
    AreaDbl maximum_waste;
    ElementPos maximum_number_of_rectangles = 256;
    // Number of rectangles of each shape. Not checked if empty.
    std::vector<ElementPos> expected_numbers_of_rectangles;
    // False if the maximum waste can't be reached within the maximum number
    // of rectangles.
    bool maximum_waste_reachable = true;
};

void PrintTo(const OuterApproximationByRectanglesTestParams& params, std::ostream* os)
{
    *os << "shapes (" << params.shapes.size() << ")\n";
    for (const OuterApproximationByRectanglesInputShape& shape: params.shapes)
        *os << "- copies " << shape.copies << " " << shape.shape.to_string(1) << "\n";
    *os << "maximum_waste " << params.maximum_waste << "\n";
    *os << "maximum_number_of_rectangles " << params.maximum_number_of_rectangles << "\n";
}

class OuterApproximationByRectanglesTest: public testing::TestWithParam<OuterApproximationByRectanglesTestParams> { };

TEST_P(OuterApproximationByRectanglesTest, OuterApproximationByRectangles)
{
    OuterApproximationByRectanglesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    std::vector<std::vector<AxisAlignedBoundingBox>> output = outer_approximation_by_rectangles(
            test_params.shapes,
            test_params.maximum_waste,
            test_params.maximum_number_of_rectangles);
    ASSERT_EQ(output.size(), test_params.shapes.size());

    // The rectangles of each shape contain it, and their waste, counted once
    // for each copy, sums up to at most the maximum waste.
    AreaDbl waste = 0.0;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)output.size();
            ++shape_pos) {
        const OuterApproximationByRectanglesInputShape& input_shape = test_params.shapes[shape_pos];
        std::cout << "shape " << shape_pos << ": " << output[shape_pos].size() << " rectangles" << std::endl;
        if (!test_params.expected_numbers_of_rectangles.empty()) {
            EXPECT_EQ(
                    (ElementPos)output[shape_pos].size(),
                    test_params.expected_numbers_of_rectangles[shape_pos]);
        }
        EXPECT_LE((ElementPos)output[shape_pos].size(), test_params.maximum_number_of_rectangles);
        MultiShapeWithHoles rectangles;
        AreaDbl rectangles_area = 0.0;
        for (const AxisAlignedBoundingBox& rectangle: output[shape_pos]) {
            std::cout << "- " << rectangle.x_min << " " << rectangle.x_max
                << " " << rectangle.y_min << " " << rectangle.y_max << std::endl;
            rectangles.shapes_with_holes.push_back({build_rectangle(rectangle)});
            rectangles_area += (rectangle.x_max - rectangle.x_min) * (rectangle.y_max - rectangle.y_min);
        }
        MultiShapeWithHoles shape;
        shape.shapes_with_holes.push_back(input_shape.shape);
        AreaDbl area_outside = 0.0;
        for (const ShapeWithHoles& part: compute_difference(shape, rectangles).shapes_with_holes)
            area_outside += part.compute_area();
        EXPECT_TRUE(equal(area_outside, 0.0)) << "shape " << shape_pos << " area outside " << area_outside;
        waste += input_shape.copies * (rectangles_area - input_shape.shape.compute_area());
    }
    std::cout << "waste " << waste << std::endl;
    if (test_params.maximum_waste_reachable) {
        EXPECT_FALSE(strictly_greater(waste, test_params.maximum_waste));
    } else {
        EXPECT_TRUE(strictly_greater(waste, test_params.maximum_waste));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        OuterApproximationByRectanglesTest,
        testing::ValuesIn(std::vector<OuterApproximationByRectanglesTestParams>{
            {
                "Rectangle",
                {{{build_rectangle(10, 5)}, 1}},
                0,
                256,
                {1},
            }, {
                // A rectangular frame (a 400 x 300 rectangle with a 320 x 220
                // hole) is covered exactly by 4 rectangles: its hole is left
                // uncovered.
                "Frame",
                {{{build_rectangle(400, 300), {build_rectangle(40, 360, 40, 260)}}, 1}},
                0,
                256,
                {4},
            }, {
                // An L shape: its 2 rectangles are merged into its 20 x 20
                // bounding box if the maximum waste covers the missing 10 x 10
                // corner.
                "LShapeMerged",
                {{{build_shape({{0, 0}, {20, 0}, {20, 10}, {10, 10}, {10, 20}, {0, 20}})}, 1}},
                100,
                256,
                {1},
            }, {
                "LShapeNotMerged",
                {{{build_shape({{0, 0}, {20, 0}, {20, 10}, {10, 10}, {10, 20}, {0, 20}})}, 1}},
                99,
                256,
                {2},
            }, {
                // A right triangle of area 50: its bounding box wastes 50,
                // so its rectangle is split until the waste is at most 10.
                "Triangle",
                {{{build_shape({{0, 0}, {10, 0}, {0, 10}})}, 1}},
                10,
            }, {
                // With no waste allowed, a shape with an oblique edge is
                // split until it has the maximum number of rectangles, and
                // the waste stays positive.
                "TriangleMaximumNumberOfRectangles",
                {{{build_shape({{0, 0}, {10, 0}, {0, 10}})}, 1}},
                0,
                16,
                {16},
                false,
            }, {
                // A V-shaped notch: the bounding boxes of the trapezoids on
                // both sides of the notch overlap.
                "VNotch",
                {{{build_shape({{0, 0}, {20, 0}, {20, 20}, {15, 20}, {10, 5}, {5, 20}, {0, 20}})}, 1}},
                20,
            }, {
                // A circle, whose arcs are replaced by line segments outside
                // of them.
                "Circle",
                {{{build_circle(10)}, 1}},
                20,
            }, {
                // A square with a circular hole, whose arcs are replaced by
                // line segments inside the hole: the rectangles leave most
                // of the hole uncovered.
                "SquareWithCircularHole",
                {{{build_rectangle(40, 40), {build_circle(10).shift(20, 20)}}, 1}},
                100,
            }, {
                // Two triangles sharing the maximum waste: the waste of the
                // first one counts 10 times.
                "Copies",
                {
                    {{build_shape({{0, 0}, {10, 0}, {0, 10}})}, 10},
                    {{build_shape({{0, 0}, {10, 0}, {0, 10}})}, 1},
                },
                100,
            }}),
        [](const testing::TestParamInfo<OuterApproximationByRectanglesTest::ParamType>& info) {
            return info.param.name;
        });
