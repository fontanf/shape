#include "shape/shape.hpp"

#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include "test_params.hpp"

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


struct ShapeElementPointTestParams
{
    ShapeElement element;
    LengthDbl length;
    Point expected_output;
};

class ShapeElementPointTest: public testing::TestWithParam<ShapeElementPointTestParams> { };

TEST_P(ShapeElementPointTest, ShapeElementPoint)
{
    ShapeElementPointTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "length " << test_params.length << std::endl;
    std::cout << "expected_output " << test_params.expected_output.to_string() << std::endl;
    Point output = test_params.element.point(test_params.length);
    EXPECT_TRUE(equal(output, test_params.expected_output));
    std::cout << "output " << output.to_string() << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementPointTest,
        testing::ValuesIn(std::vector<ShapeElementPointTestParams>{
            {build_line_segment({0, 0}, {0, 2}), 1, {0, 1}},
            {build_line_segment({0, 0}, {2, 0}), 1, {1, 0}},
            {build_circular_arc({1, 0}, {-1, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise), M_PI / 2, {0, 1}},
            {build_circular_arc({1, 0}, {-1, 1}, {0, 0}, ShapeElementOrientation::Clockwise), M_PI / 2, {0, -1}},
            }));


struct ShapeElementFindPointBetweenTestParams
{
    ShapeElement element;
    Point point_1;
    Point point_2;
    Point expected_output;
};

class ShapeElementFindPointBetweenTest: public testing::TestWithParam<ShapeElementFindPointBetweenTestParams> { };

TEST_P(ShapeElementFindPointBetweenTest, ShapeElementFindPointBetween)
{
    ShapeElementFindPointBetweenTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "point_1 " << test_params.point_1.to_string() << std::endl;
    std::cout << "point_2 " << test_params.point_2.to_string() << std::endl;
    std::cout << "expected_output " << test_params.expected_output.to_string() << std::endl;

    Point output = test_params.element.find_point_between(
            test_params.point_1,
            test_params.point_2);
    std::cout << "output " << output.to_string() << std::endl;

    EXPECT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementFindPointBetweenTest,
        testing::ValuesIn(std::vector<ShapeElementFindPointBetweenTestParams>{
            {
                build_line_segment({0, 0}, {0, 2}),
                {0, 0},
                {0, 2},
                {0, 1},
            }, {
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, {ShapeElementOrientation::Anticlockwise}),
                {1, 0},
                {-1, 0},
                {0, 1},
            }, {
                build_circular_arc({25.09217340838399, 562.1293338082015}, {25.12432681320623, 562.1171997938548}, {25.14000053474341, 562.2074008893288}, {ShapeElementOrientation::Anticlockwise}),
                {25.12314661878088, 562.1174128418544},
                {25.12432681320623, 562.1171997938548},
                {25.12373636714434, 562.1173043853798},
            }}));


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
    EXPECT_TRUE(jet == test_params.expected_jet);
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


struct ShapeElementContainsTestParams
{
    ShapeElement element;
    Point point;
    bool expected_output;
};

class ShapeElementContainsTest: public testing::TestWithParam<ShapeElementContainsTestParams> { };

TEST_P(ShapeElementContainsTest, ShapeElementContains)
{
    ShapeElementContainsTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "point " << test_params.point.to_string() << std::endl;
    std::cout << "expceted output " << test_params.expected_output << std::endl;
    bool output = test_params.element.contains(test_params.point);
    std::cout << "output " << output << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        ShapeElement,
        ShapeElementContainsTest,
        testing::ValuesIn(std::vector<ShapeElementContainsTestParams>{
            {
                build_line_segment({173.76745440585034, 708.07662627064951}, {175.56776284529906, 711.01702947644321}),
                {174.91570697722955, 709.95140487030301},
                false,
            },
            }));


struct ShapeElementRecomputeCenterTestParams
{
    ShapeElement element;
};

class ShapeElementRecomputeCenterTest: public testing::TestWithParam<ShapeElementRecomputeCenterTestParams> { };

TEST_P(ShapeElementRecomputeCenterTest, ShapeElementRecomputeCenter)
{
    ShapeElementRecomputeCenterTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    ShapeElement element = test_params.element;
    element.recompute_center();
    std::cout << "output " << element.to_string() << std::endl;
    EXPECT_TRUE(equal(element, test_params.element));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementRecomputeCenterTest,
        testing::ValuesIn(std::vector<ShapeElementRecomputeCenterTestParams>{
            {build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)},
            {build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise)},
            {build_circular_arc({1, 2}, {2, 1}, {1, 1}, ShapeElementOrientation::Anticlockwise)},
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
    bool expected_output;
};

class ShapeContainsTest: public testing::TestWithParam<ShapeContainsTestParams> { };

TEST_P(ShapeContainsTest, ShapeContains)
{
    ShapeContainsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "point " << test_params.point.to_string() << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expceted output " << test_params.expected_output << std::endl;
    bool output = test_params.shape.contains(
            test_params.point,
            test_params.strict);
    std::cout << "output " << output << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
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


struct ShapeFindPointStrictlyInsideTestParams
{
    Shape shape;
};

class ShapeFindPointStrictlyInsideTest: public testing::TestWithParam<ShapeFindPointStrictlyInsideTestParams> { };

TEST_P(ShapeFindPointStrictlyInsideTest, ShapeFindPointStrictlyInside)
{
    ShapeFindPointStrictlyInsideTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    Point output = test_params.shape.find_point_strictly_inside();
    std::cout << "output " << output.to_string() << std::endl;
    EXPECT_TRUE(test_params.shape.contains(output, true));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeFindPointStrictlyInsideTest,
        testing::ValuesIn(std::vector<ShapeFindPointStrictlyInsideTestParams>{
            {
                build_rectangle(2, 4),
            }, {
                build_circle(2),
            }, {
                build_shape({{0, 100}, {100, 100}, {100, 0}, {200, 0}, {200, 200}, {0, 200}}),
            }, {
                build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}}),
            }}));


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
