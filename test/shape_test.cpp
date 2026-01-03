#include "shape/shape.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct ShapeElementLengthTestParams
{
    ShapeElement element;
    LengthDbl expected_length;
};

class ShapeElementLengthTest: public testing::TestWithParam<ShapeElementLengthTestParams> { };

TEST_P(ShapeElementLengthTest, ShapeElementLength)
{
    ShapeElementLengthTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "expected_length " << test_params.expected_length << std::endl;
    LengthDbl length = test_params.element.length();
    EXPECT_TRUE(equal(length, test_params.expected_length));
    std::cout << "length " << length << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementLengthTest,
        testing::ValuesIn(std::vector<ShapeElementLengthTestParams>{
            {build_line_segment({0, 0}, {0, 1}), 1},
            {build_line_segment({0, 0}, {1, 0}), 1},
            {build_line_segment({0, 0}, {1, 1}), std::sqrt(2.0)},
            {build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise), M_PI / 2},
            {build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Clockwise), 3 * M_PI / 2},
            {build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise), 3 * M_PI / 2},
            }));


struct ShapeElementJetTestParams
{
    ShapeElement element;
    Point point;
    bool reverse;
    Jet expected_jet;
};

class ShapeElementJetTest: public testing::TestWithParam<ShapeElementJetTestParams> { };

TEST_P(ShapeElementJetTest, ShapeElementJet)
{
    ShapeElementJetTestParams test_params = GetParam();
    Jet jet = test_params.element.jet(
            test_params.point,
            test_params.reverse);
    EXPECT_TRUE(equal(jet, test_params.expected_jet));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementJetTest,
        testing::ValuesIn(std::vector<ShapeElementJetTestParams>{
            {
                build_line_segment({0, 0}, {1, 0}),
                {0, 0},
                false,
                {0, 0},
            }, {
                build_line_segment({0, 0}, {1, 0}),
                {0, 0},
                true,
                {M_PI, 0},
            }, {
                build_line_segment({0, 0}, {0, 1}),
                {0, 0},
                false,
                {M_PI / 2, 0}
            }, {
                build_line_segment({0, 0}, {0, 1}),
                {0, 0},
                true,
                {3 * M_PI / 2, 0}
            }, {
                build_line_segment({0, 0}, {-1, 0}),
                {0, 0},
                false,
                {M_PI, 0},
            }, {
                build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {1, 0},
                false,
                {M_PI / 2, 1},
            }, {
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Clockwise),
                {1, 0},
                false,
                {3 * M_PI / 2, -1},
            } }));


struct ShapeElementMiddleTestParams
{
    ShapeElement circular_arc;
    Point expected_middle;
};

class ShapeElementMiddleTest: public testing::TestWithParam<ShapeElementMiddleTestParams> { };

TEST_P(ShapeElementMiddleTest, ShapeElementMiddle)
{
    ShapeElementMiddleTestParams test_params = GetParam();
    std::cout << "circular_arc" << std::endl;
    std::cout << test_params.circular_arc.to_string() << std::endl;
    std::cout << "expected_middle" << std::endl;
    std::cout << test_params.expected_middle.to_string() << std::endl;

    Point middle = test_params.circular_arc.middle();
    std::cout << "computed_middle" << std::endl;
    std::cout << middle.to_string() << std::endl;

    EXPECT_TRUE(equal(middle, test_params.expected_middle));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementMiddleTest,
        testing::ValuesIn(std::vector<ShapeElementMiddleTestParams>{
            {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                {sqrt(2) / 2, sqrt(2) / 2}
            }, {
                build_shape({{0, 1}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                {sqrt(2) / 2, sqrt(2) / 2}
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                {0, 1}
            }, {
                build_shape({{1, 0}, {0, 0, -1}, {-1, 0}}, true).elements.front(),
                {0, -1}
            }, {
                build_shape({{-1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                {sqrt(2) / 2, - sqrt(2) / 2}
            }, {
                build_shape({{-1, 0}, {0, 0, -1}, {0, 1}}, true).elements.front(),
                {- sqrt(2) / 2, sqrt(2) / 2}
            }
        }));


struct ShapeElementMinMaxTestParams
{
    ShapeElement element;
    LengthDbl expected_x_min;
    LengthDbl expected_y_min;
    LengthDbl expected_x_max;
    LengthDbl expected_y_max;
};

class ShapeElementMinMaxTest: public testing::TestWithParam<ShapeElementMinMaxTestParams> { };

TEST_P(ShapeElementMinMaxTest, ShapeElementMinMax)
{
    ShapeElementMinMaxTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "expected x_min " << test_params.expected_x_min
        << " y_min " << test_params.expected_y_min
        << " x_max " << test_params.expected_x_max
        << " y_max " << test_params.expected_y_max << std::endl;
    auto mm = test_params.element.min_max();
    std::cout << "x_min " << mm.first.x
        << " y_min " << mm.first.y
        << " x_max " << mm.second.x
        << " y_max " << mm.second.y << std::endl;
    EXPECT_TRUE(equal(mm.first.x, test_params.expected_x_min));
    EXPECT_TRUE(equal(mm.second.x, test_params.expected_x_max));
    EXPECT_TRUE(equal(mm.first.y, test_params.expected_y_min));
    EXPECT_TRUE(equal(mm.second.y, test_params.expected_y_max));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementMinMaxTest,
        testing::ValuesIn(std::vector<ShapeElementMinMaxTestParams>{
            {build_shape({{0, 1}, {2, 3}}, true).elements.front(), 0, 1, 2, 3 },
            {build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(), -1, 0, 1, 1 },
            {build_shape({{1, 0}, {0, 0, -1}, {-1, 0}}, true).elements.front(), -1, -1, 1, 0 },

            {build_shape({{0, 1}, {0, 0, 1}, {1, 0}}, true).elements.front(), -1, -1, 1, 1 },
            {build_shape({{-1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(), -1, -1, 1, 1 },
            {build_shape({{0, -1}, {0, 0, 1}, {-1, 0}}, true).elements.front(), -1, -1, 1, 1 },
            {build_shape({{1, 0}, {0, 0, 1}, {0, -1}}, true).elements.front(), -1, -1, 1, 1 },
            }));


struct ShapeComputeAreaTestParams
{
    Shape shape;
    AreaDbl expected_area;
};

class ShapeComputeAreaTest: public testing::TestWithParam<ShapeComputeAreaTestParams> { };

TEST_P(ShapeComputeAreaTest, ShapeComputeArea)
{
    ShapeComputeAreaTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "expected area " << to_string(test_params.expected_area) << std::endl;
    AreaDbl area = test_params.shape.compute_area();
    std::cout << "area " << to_string(area) << std::endl;
    EXPECT_TRUE(equal(area, test_params.expected_area));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeComputeAreaTest,
        testing::ValuesIn(std::vector<ShapeComputeAreaTestParams>{
            {
                build_rectangle(1, 1),
                1,
            }, {
                build_rectangle(2, 3),
                6,
            }, {
                build_circle(1),
                M_PI,
            }, {
                build_circle(4),
                M_PI * 16,
            }, {
                build_shape({
                    build_line_segment({41.39894441055727, 194.5414289923167}, {46.75574653250537, 184.749743605213}),
                    build_circular_arc({46.75574653250537, 184.749743605213}, {47.33576625598295, 188.9848689097349}, {65.64848776416608, 184.3195773641661}, ShapeElementOrientation::Clockwise),
                    build_line_segment({47.33576625598295, 188.9848689097349}, {43.62858633055728, 195.7612139523166}),
                    build_line_segment({43.62858633055728, 195.7612139523166}, {41.39894441055727, 194.5414289923167}),
                }),
                23.6526695078428,
            }}));


struct ShapeComputeFurthestPointsTestParams
{
    Shape shape;
    Angle angle;
    Shape::FurthestPoint expected_point_min;
    Shape::FurthestPoint expected_point_max;
};

class ShapeComputeFurthestPointsTest: public testing::TestWithParam<ShapeComputeFurthestPointsTestParams> { };

TEST_P(ShapeComputeFurthestPointsTest, ShapeComputeFurthestPoints)
{
    ShapeComputeFurthestPointsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "angle " << test_params.angle << std::endl;
    std::cout << "expected point_min " << test_params.expected_point_min.point.to_string()
        << " pos " << test_params.expected_point_min.element_pos
        << " point_max " << test_params.expected_point_max.point.to_string()
        << " pos " << test_params.expected_point_max.element_pos
        << std::endl;
    auto p = test_params.shape.compute_furthest_points(test_params.angle);
    std::cout << "point_min " << p.first.point.to_string()
        << " pos " << p.first.element_pos
        << " point_max " << p.second.point.to_string()
        << " pos " << p.second.element_pos
        << std::endl;
    EXPECT_TRUE(equal(p.first.point, test_params.expected_point_min.point));
    EXPECT_TRUE(equal(p.first.element_pos, test_params.expected_point_min.element_pos));
    EXPECT_TRUE(equal(p.second.point, test_params.expected_point_max.point));
    EXPECT_TRUE(equal(p.second.element_pos, test_params.expected_point_max.element_pos));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeComputeFurthestPointsTest,
        testing::ValuesIn(std::vector<ShapeComputeFurthestPointsTestParams>{
            {
                build_shape({{1, 0}, {2, 1}, {1, 2}, {0, 1}}),
                0,
                {Point{1, 0}, 0},
                {Point{1, 2}, 1},
            }, {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                45,
                {Point{2, 0}, 0},
                {Point{0, 2}, 2},
            }, {
                build_shape({{0, 0}, {2, 0}, {4, 2}, {3, 3, 1}, {2, 4}, {0, 2}}),
                90 + 45,
                {Point{4, 4}, 2},
                {Point{0, 0}, 0},
            },
            }));


struct ShapeContainsTestParams
{
    Shape shape;
    Point point;
    bool strict;
    bool expected_result;
};

class ShapeContainsTest: public testing::TestWithParam<ShapeContainsTestParams> { };

TEST_P(ShapeContainsTest, ShapeContains)
{
    ShapeContainsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "point " << test_params.point.to_string() << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expceted result " << test_params.expected_result << std::endl;
    bool result = test_params.shape.contains(
            test_params.point,
            test_params.strict);
    std::cout << "result " << result << std::endl;
    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeContainsTest,
        testing::ValuesIn(std::vector<ShapeContainsTestParams>{
            {
                build_shape({{2.5, -9.682458365518542}, {5, 0, -1}, {2.5, 9.682458365518542}, {0, 0, 1}}),
                {15, 0},
                true,
                false,
            },
            }));


struct ShapeSplitTestParams
{
    Shape shape;
    std::vector<ShapePoint> points;
    std::vector<Shape> expected_paths;
};

class ShapeSplitTest: public testing::TestWithParam<ShapeSplitTestParams> { };

TEST_P(ShapeSplitTest, ShapeSplit)
{
    ShapeSplitTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "points" << std::endl;
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)test_params.points.size();
            ++point_pos) {
        const ShapePoint& point = test_params.points[point_pos];
        std::cout << "- " << point_pos << " element_pos " << point.element_pos << " point " << point.point.to_string() << std::endl;
    }
    std::cout << "expceted paths" << std::endl;
    for (ShapePos path_pos = 0;
            path_pos < (ShapePos)test_params.expected_paths.size();
            ++path_pos) {
        const Shape& path = test_params.expected_paths[path_pos];
        std::cout << "- " << path_pos << " path " << path.to_string(1) << std::endl;
    }
    auto paths = test_params.shape.split(test_params.points);
    std::cout << "paths" << std::endl;
    for (ShapePos path_pos = 0;
            path_pos < (ShapePos)paths.size();
            ++path_pos) {
        const Shape& path = paths[path_pos];
        std::cout << "- " << path_pos << " path " << path.to_string(1) << std::endl;
    }
    ASSERT_EQ(paths.size(), test_params.expected_paths.size());
    for (ShapePos path_pos = 0;
            path_pos < (ShapePos)paths.size();
            ++path_pos) {
        const Shape& path = paths[path_pos];
        const Shape& expected_path = test_params.expected_paths[path_pos];
        EXPECT_TRUE(equal(path, expected_path));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeSplitTest,
        testing::ValuesIn(std::vector<ShapeSplitTestParams>{
            {
                build_path({{0, 0}, {0, 2}}),
                {{0, {0, 1}}},
                {
                    build_path({{0, 0}, {0, 1}}),
                    build_path({{0, 1}, {0, 2}}),
                },
            }, {
                build_shape({{0, 0}, {0, 4}, {2, 4}, {2, 0}}),
                {{0, {0, 1}}},
                {
                    build_path({{0, 1}, {0, 4}, {2, 4}, {2, 0}, {0, 0}, {0, 1}}),
                },
            }, {
                build_shape({{0, 0}, {0, 4}, {2, 4}, {2, 0}}),
                {{0, {0, 1}}, {2, {2, 3}}},
                {
                    build_path({{2, 3}, {2, 0}, {0, 0}, {0, 1}}),
                    build_path({{0, 1}, {0, 4}, {2, 4}, {2, 3}}),
                },
            },
            }));


struct ApproximateCircularArcByLineSegmentsTestParams
{
    ShapeElement circular_arc;
    LengthDbl number_of_line_segments;
    bool outer;
    std::vector<ShapeElement> expected_line_segments;
};

class ApproximateCircularArcByLineSegmentsTest: public testing::TestWithParam<ApproximateCircularArcByLineSegmentsTestParams> { };

TEST_P(ApproximateCircularArcByLineSegmentsTest, ApproximateCircularArcByLineSegments)
{
    ApproximateCircularArcByLineSegmentsTestParams test_params = GetParam();
    std::cout << "circular_arc" << std::endl;
    std::cout << test_params.circular_arc.to_string() << std::endl;
    std::cout << "expected_line_segments" << std::endl;
    for (const ShapeElement& line_segment: test_params.expected_line_segments)
        std::cout << line_segment.to_string() << std::endl;
    LengthDbl segment_length = test_params.circular_arc.length() / test_params.number_of_line_segments;
    std::vector<ShapeElement> line_segments = approximate_circular_arc_by_line_segments(
            test_params.circular_arc,
            segment_length,
            test_params.outer);
    std::cout << "line_segments" << std::endl;
    for (const ShapeElement& line_segment: line_segments)
        std::cout << line_segment.to_string() << std::endl;

    ASSERT_EQ(line_segments.size(), test_params.number_of_line_segments);
    for (ElementPos pos = 0; pos < test_params.number_of_line_segments; ++pos) {
        //std::cout << std::setprecision (15) << line_segments[pos].start.x << std::endl;
        EXPECT_TRUE(equal(line_segments[pos], test_params.expected_line_segments[pos]));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ApproximateCircularArcByLineSegmentsTest,
        testing::ValuesIn(std::vector<ApproximateCircularArcByLineSegmentsTestParams>{
            {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                1,
                false,
                build_shape({{1, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                1,
                true,
                build_shape({{1, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                2,
                true,
                build_shape({{1, 0}, {1, 1}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                2,
                false,
                build_shape({{1, 0}, {0, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                3,
                true,
                build_shape({{1, 0}, {1, 0.414213562373095}, {0.414213562373095, 1}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                3,
                false,
                build_shape({{1, 0}, {0.585786437626905, 0}, {0, 0.585786437626905}, {0, 1}}, true).elements
            }}));


struct ShapeWithHolesBridgeTouchingHolesTestParams
{
    ShapeWithHoles shape;
    ShapeWithHoles expected_shape;
};

class ShapeWithHolesBridgeTouchingHolesTest: public testing::TestWithParam<ShapeWithHolesBridgeTouchingHolesTestParams> { };

TEST_P(ShapeWithHolesBridgeTouchingHolesTest, ShapeWithHolesBridgeTouchingHoles)
{
    ShapeWithHolesBridgeTouchingHolesTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected shape " << test_params.expected_shape.to_string(2) << std::endl;
    ShapeWithHoles res = test_params.shape.bridge_touching_holes();
    std::cout << "result " << res.to_string(2) << std::endl;
    EXPECT_TRUE(equal(res, test_params.expected_shape));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeWithHolesBridgeTouchingHolesTest,
        testing::ValuesIn(std::vector<ShapeWithHolesBridgeTouchingHolesTestParams>{
            {  // Shape without hole.
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
            }, {  // Shape with one hole not touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                }, {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                },
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {20, 5}, {19, 6}, {18, 5}}),
                    }
                },
                {build_shape({
                        {0, 0}, {20, 0}, {20, 5},
                        {19, 4}, {18, 5}, {19, 6},
                        {20, 5}, {20, 10}, {0, 10}})},
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {19, 5}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {19, 6}, {17, 6}, {17, 4}}),
                    }
                },
                {build_shape({
                        {0, 0}, {20, 0}, {19, 5},
                        {19, 4}, {17, 4}, {17, 6}, {19, 6},
                        {19, 5}, {20, 10}, {0, 10}})},
            }, {  // Shape with one hole touching its outline and another hole touching the first hole.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{10, 1}, {11, 2}, {9, 2}}),
                        build_shape({{10, 0}, {11, 1}, {9, 1}}),
                    }
                },
                {build_shape({
                        {0, 0},
                        {10, 0}, {9, 1},
                        {10, 1}, {9, 2}, {11, 2}, {10, 1},
                        {11, 1}, {10, 0},
                        {20, 0}, {20, 50}, {0, 50}})},
            }, {  // Shape with 3 holes.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{10, 1}, {11, 2}, {9, 2}}),
                        build_shape({{10, 0}, {11, 1}, {9, 1}}),
                        build_shape({{10, 2}, {11, 3}, {9, 3}}),
                    }
                },
                {build_shape({
                        {0, 0},
                        {10, 0}, {9, 1},
                        {10, 1}, {9, 2},
                        {10, 2}, {9, 3}, {11, 3}, {10, 2},
                        {11, 2}, {10, 1},
                        {11, 1}, {10, 0},
                        {20, 0}, {20, 50}, {0, 50}})},
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                    }
                },
                {build_shape({
                        {0, 0},
                        {10, 1}, {9, 1}, {10, 2}, {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})},
            }, {  // Shape with one hole touching its outline and another hole touching the first hole.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 2}, {11, 2}, {10, 3}}),
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                    }
                },
                {build_shape({
                        {0, 0},
                        {10, 1}, {9, 1},
                        {10, 2}, {9, 2}, {10, 3}, {11, 2}, {10, 2},
                        {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})},
            }, {  // Shape with 3 holes.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 2}, {11, 2}, {10, 3}}),
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                        build_shape({{9, 3}, {11, 3}, {10, 4}}),
                    }
                },
                {build_shape({
                        {0, 0},
                        {10, 1}, {9, 1},
                        {10, 2}, {9, 2},
                        {10, 3}, {9, 3}, {10, 4}, {11, 3}, {10, 3},
                        {11, 2}, {10, 2},
                        {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})},
            },
            }));


struct ShapeWithHolesBridgeHolesTestParams
{
    ShapeWithHoles shape;
    Shape expected_shape;
};

class ShapeWithHolesBridgeHolesTest: public testing::TestWithParam<ShapeWithHolesBridgeHolesTestParams> { };

TEST_P(ShapeWithHolesBridgeHolesTest, ShapeWithHolesBridgeHoles)
{
    ShapeWithHolesBridgeHolesTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected shape " << test_params.expected_shape.to_string(2) << std::endl;
    Shape res = test_params.shape.bridge_holes();
    std::cout << "res " << res.to_string(2) << std::endl;
    EXPECT_TRUE(equal(res, test_params.expected_shape));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeWithHolesBridgeHolesTest,
        testing::ValuesIn(std::vector<ShapeWithHolesBridgeHolesTestParams>{
            {  // Shape without hole.
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {20, 5}, {19, 6}, {18, 5}}),
                    }
                },
                build_shape({
                        {0, 0}, {20, 0}, {20, 5},
                        {19, 4}, {18, 5}, {19, 6},
                        {20, 5}, {20, 10}, {0, 10}}),
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {19, 5}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {19, 6}, {17, 6}, {17, 4}}),
                    }
                },
                build_shape({
                        {0, 0}, {20, 0}, {19, 5},
                        {19, 4}, {17, 4}, {17, 6}, {19, 6},
                        {19, 5}, {20, 10}, {0, 10}}),
            }, {  // Shape with one hole not touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}}),
                    }
                },
                build_shape({
                        {0, 0}, {20, 0}, {20, 10}, {0, 10}, {0, 5},
                        {14, 5}, {15, 6}, {16, 5}, {15, 4}, {14, 5},
                        {0, 5}}),
            }, {  // Shape with two holes.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}}),
                        build_shape({{10, 4}, {11, 5}, {10, 6}, {9, 5}}),
                    }
                },
                build_shape({
                        {0, 0}, {20, 0}, {20, 10}, {0, 10}, {0, 5},
                        {9, 5}, {10, 6}, {11, 5},
                        {14, 5}, {15, 6}, {16, 5}, {15, 4}, {14, 5},
                        {11, 5}, {10, 4}, {9, 5},
                        {0, 5}}),
            },
            }));
