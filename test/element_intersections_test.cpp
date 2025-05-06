#include "shape/element_intersections.hpp"

#include <gtest/gtest.h>

using namespace shape;


struct ComputeIntersectionsTestParams
{
    ShapeElement element_1;
    ShapeElement element_2;
    bool strict;
    std::vector<Point> expected_intersections;
};

class ComputeIntersectionsTest: public testing::TestWithParam<ComputeIntersectionsTestParams> { };

TEST_P(ComputeIntersectionsTest, ComputeIntersections)
{
    ComputeIntersectionsTestParams test_params = GetParam();
    std::cout << "element_1 " << test_params.element_1.to_string() << std::endl;
    std::cout << "element_2 " << test_params.element_2.to_string() << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_intersections" << std::endl;
    for (const Point& point: test_params.expected_intersections)
        std::cout << "- " << point.to_string() << std::endl;

    std::vector<Point> intersections = compute_intersections(
            test_params.element_1,
            test_params.element_2,
            test_params.strict);
    std::cout << "intersections" << std::endl;
    for (const Point& point: intersections) {
        std::cout << "- " << std::setprecision(15) << point.x
            << " " << std::setprecision(15) << point.y
            << std::endl;
    }

    ASSERT_EQ(intersections.size(), test_params.expected_intersections.size());
    for (const Point& expected_intersection: test_params.expected_intersections) {
        EXPECT_NE(std::find_if(
                    intersections.begin(),
                    intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsTestParams>{
            {  // Non-intersecting line segments
                build_shape({{0, 0}, {0, 1}}, true).elements.front(),
                build_shape({{1, 0}, {1, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Simple line segment intersection.
                build_shape({{1, 0}, {1, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                false,
                {{1, 1}},
            }, {  // One line segment touching another.
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // One line segment touching another (strict).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                true,
                {},
            }, {  // Two identical line segments.
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 0}, {0, 2}},
            }, {  // Two identical line segments (reversed).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 2}, {0, 0}}, true).elements.front(),
                false,
                {{0, 0}, {0, 2}},
            }, {  // Two identical line segments (reversed) (strict).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                true,
                {},
            }, {  // Two overlapping line segments.
                build_shape({{0, 0}, {0, 3}}, true).elements.front(),
                build_shape({{0, 1}, {0, 4}}, true).elements.front(),
                false,
                {{0, 1}, {0, 3}},
            }, {  // Two overlapping line segments (strict).
                build_shape({{0, 0}, {0, 3}}, true).elements.front(),
                build_shape({{0, 1}, {0, 4}}, true).elements.front(),
                true,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                build_shape({{2, 0}, {2, 2}}, true).elements.front(),
                false,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                build_shape({{1, 1}, {2, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{2, 0}, {0, 0, 1}, {0, 2}}, true).elements.front(),
                build_shape({{2, 1}, {3, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Intersecting line segment and circular arc.
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Intersecting line segment and circular arc at two points.
                build_shape({{-1, -2}, {-1, 2}}, true).elements.front(),
                build_shape({{1, 1}, {0, 0, 1}, {1, -1}}, true).elements.front(),
                false,
                {{-1, -1}, {-1, 1}},
            }, {
                build_shape({{39.2075327238964, 921.938482687602}, {39.2004011663701, 921.949097066976}}, true).elements.front(),
                build_shape({{39.2075268512914, 921.938491537799}, {39.2066965548415, 921.937934215805, -1}, {39.2075327238964, 921.938482687602}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{398683.828041, 9213001.274628}, {398845.790901, 9213041.800811}}, true).elements.front(),
                build_shape({{398645.799546, 9213137.752427}, {398972.799200, 9213516.000000, 1}, {398972.799200, 9213016.000000}}, true).elements.front(),
                false,
                {{398828.138764737, 9213037.38391307}},
            }, {  // Touching line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 1}, {1, 1}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Touching line segment and circular arc (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 1}, {1, 1}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc at two points.
                build_shape({{2, 0}, {-2, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Touching line segment and circular arc at two points (strict).
                build_shape({{2, 0}, {-2, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc.
                build_shape({{5, -1}, {7, -1, 1}, {7, -3}}, true).elements.front(),
                build_shape({{6, 5}, {7, -3}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc.
                build_shape({{5, -1}, {7, -1, 1}, {7, -3}}, true).elements.front(),
                build_shape({{6, 5}, {7, -3}}, true).elements.front(),
                false,
                {{7, -3}},
            }, {  // Non-intersecting circular arcs.
                build_shape({{2, 0}, {0, 0, 1}, {0, 2}}, true).elements.front(),
                build_shape({{3, 0}, {1, 0, 1}, {1, 2}}, true).elements.front(),
                false,
                {},
            }, {  // Intersecting circular arcs.
                build_shape({{3, 0}, {1, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {-1, 0, 1}, {-3, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs.
                build_shape({{3, 0}, {1, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-3, 0}, {-1, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs.
                build_shape({{-1, 0}, {1, 0, -1}, {3, 0}}, true).elements.front(),
                build_shape({{-3, 0}, {-1, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs at two points.
                build_shape({{-2, 1}, {-1, 0, -1}, {-2, -1}}, true).elements.front(),
                build_shape({{+2, 1}, {+1, 0, +1}, {+2, -1}}, true).elements.front(),
                false,
                {{0, -1}, {0, 1}},
            }, {  // Touching circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, 1}, {1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Touching circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, 1}, {1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Identical circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Identical circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Identical circular arcs (reversed).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Identical circular arcs (reversed) (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Overlapping circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 1}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                false,
                {{0, 1}, {-1, 0}},
            }, {  // Overlapping circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 1}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{1.96721311, -0.78740157}, {60.98360656, -0.78740157}}, true).elements.front(),
                build_shape({{1.941450017182601, -0.7869799841717368}, {1.96721311, -0, 1}, {1.96721311, -0.78740157}}, true).elements.front(),
                false,
                {{1.96721311, -0.78740157}},
            }}));


struct MergeIntersectingShapesTestParams
{
    std::vector<Shape> shapes;
    std::vector<Shape> expected_merged_shapes;
};

class MergeIntersectingShapesTest: public testing::TestWithParam<MergeIntersectingShapesTestParams> { };

TEST_P(MergeIntersectingShapesTest, MergeIntersectingShapes)
{
    MergeIntersectingShapesTestParams test_params = GetParam();
    std::cout << "shapes" << std::endl;
    for (const Shape& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_merged_shapes" << std::endl;
    for (const Shape& shape: test_params.expected_merged_shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;

    std::vector<Shape> merged_shapes = merge_intersecting_shapes(
            test_params.shapes);
    std::cout << "merged_shapes" << std::endl;
    for (const Shape& shape: merged_shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;

    ASSERT_EQ(merged_shapes.size(), test_params.expected_merged_shapes.size());
    for (const Shape& expected_shape: test_params.expected_merged_shapes) {
        EXPECT_NE(std::find_if(
                    merged_shapes.begin(),
                    merged_shapes.end(),
                    [&expected_shape](const Shape& shape) { return equal(shape, expected_shape); }),
                merged_shapes.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        MergeIntersectingShapesTest,
        testing::ValuesIn(std::vector<MergeIntersectingShapesTestParams>{
            {
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
            }, {
                {
                    build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                    build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}),
                },
                {build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}})},
            }, {
                {
                    build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                    build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}),
                    build_shape({{5, 0}, {7, 0}, {7, 2}, {5, 2}}),
                },
                {
                    build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}),
                    build_shape({{5, 0}, {7, 0}, {7, 2}, {5, 2}}),
                },
            }}));
