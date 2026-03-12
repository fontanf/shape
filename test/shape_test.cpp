//#define SHAPE_TEST_DEBUG

#include "shape/shape.hpp"

#ifdef SHAPE_TEST_DEBUG
#include "shape/writer.hpp"
#endif

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>


using namespace shape;
namespace fs = boost::filesystem;


struct StrictlyLesserAngleTestParams
{
    Point vector_1;
    Point vector_2;
    bool expected_output;
};

class StrictlyLesserAngleTest: public testing::TestWithParam<StrictlyLesserAngleTestParams> { };

TEST_P(StrictlyLesserAngleTest, StrictlyLesserAngle)
{
    StrictlyLesserAngleTestParams test_params = GetParam();
    std::cout << "vector_1 " << test_params.vector_1.to_string() << std::endl;
    std::cout << "vector_2 " << test_params.vector_2.to_string() << std::endl;
    std::cout << "expected_output " << test_params.expected_output << std::endl;
#ifdef SHAPE_TEST_DEBUG
    Writer()
        .add_element(build_line_segment({0, 0}, test_params.vector_1))
        .add_element(build_line_segment({0, 0}, test_params.vector_2))
        .write_json("strictly_lesser_angle_inputs.json");
#endif
    bool output = strictly_lesser_angle(test_params.vector_1, test_params.vector_2);
    std::cout << "output " << output << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        StrictlyLesserAngleTest,
        testing::ValuesIn(std::vector<StrictlyLesserAngleTestParams>{
            {
                {-0.00058384162763758241, -0.00073391498813180078},
                {-0.00058382111200927511, -0.00073393133686749934},
                true,
            },
            {
                {0.00058761354789993447, 0.000730898704205174},
                {0.00058755222531203799, 0.00073094802962714311},
                true,
            },
            }));


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


struct ShapeElementLengthPointTestParams
{
    ShapeElement element;
    Point point;
    LengthDbl expected_length;
};

class ShapeElementLengthPointTest: public testing::TestWithParam<ShapeElementLengthPointTestParams> { };

TEST_P(ShapeElementLengthPointTest, ShapeElementLengthPoint)
{
    ShapeElementLengthPointTestParams test_params = GetParam();
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "point " << test_params.point.to_string() << std::endl;
    std::cout << "expected_length " << test_params.expected_length << std::endl;
    if (!test_params.element.contains(test_params.point)) {
        throw std::invalid_argument(FUNC_SIGNATURE);
    }
#ifdef SHAPE_TEST_DEBUG
    Writer().add_element(test_params.element).write_json("shape_element_length_inputs.json");
#endif
    LengthDbl length = test_params.element.length(test_params.point);
    EXPECT_TRUE(equal(length, test_params.expected_length));
    std::cout << "length " << length << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementLengthPointTest,
        testing::ValuesIn(std::vector<ShapeElementLengthPointTestParams>{
            {build_line_segment({0, 0}, {0, 1}), {0, 0.2}, 0.2},
            {
                build_circular_arc({24.017319696054429, 562.69162734170129}, {24.033416251762397, 562.66318411783629}, {23.947853645941326, 562.63353891269458}, ShapeElementOrientation::Clockwise),
                {24.017319578247072, 562.69162748260499},
                0,
            },

//path_element 20 CircularArc start (24.001219675214291, 562.70669535207219) end (24.017319696054429, 562.69162734170129) center (23.947853645941475, 562.63353891269435) orientation Clockwise


//path_element_pos 20 shape_element_pos 1 point (24.017319578247072, 562.69162748260499) l 0.0221059
//path_element_pos 20 shape_element_pos 2 point (24.017319578247072, 562.69162748260499) l 0.0221059
//path_element_pos 21 shape_element_pos 0 point (24.023228790389016, 562.6837224289352) l 0.00987429
//path_element_pos 21 shape_element_pos 1 point (24.017319578247072, 562.69162748260499) l 0.568959
//path_element_pos 21 shape_element_pos 2 point (24.017319578247072, 562.69162748260499) l 0.568959

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


struct ShapeFindPointBetweenTestParams
{
    Shape shape;
    ShapePoint point_1;
    ShapePoint point_2;
    ShapePoint expected_output;
};

class ShapeFindPointBetweenTest: public testing::TestWithParam<ShapeFindPointBetweenTestParams> { };

TEST_P(ShapeFindPointBetweenTest, ShapeFindPointBetween)
{
    ShapeFindPointBetweenTestParams test_params = GetParam();
    ShapePoint output = test_params.shape.find_point_between(
            test_params.point_1,
            test_params.point_2);
    EXPECT_EQ(output.element_pos, test_params.expected_output.element_pos);
    EXPECT_TRUE(equal(output.point, test_params.expected_output.point));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeFindPointBetweenTest,
        testing::ValuesIn(std::vector<ShapeFindPointBetweenTestParams>{
            {  // point_1 at the end of the last element: tests wrap-around to element 0.
                // Square: elements 0=(0,0)→(1,0), 1=(1,0)→(1,1), 2=(1,1)→(0,1), 3=(0,1)→(0,0).
                // point_1 is at the end of element 3 (i.e. (0,0)), point_2 is on element 1.
                // Expected: midpoint of element 0.
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {3, {0, 0}},
                {1, {1, 0.5}},
                {0, {0.5, 0}},
            },
        }));


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
    AxisAlignedBoundingBox aabb = test_params.element.min_max();
    std::cout << "x_min " << aabb.x_min
        << " y_min " << aabb.y_min
        << " x_max " << aabb.x_max
        << " y_max " << aabb.y_max << std::endl;
    EXPECT_TRUE(equal(aabb.x_min, test_params.expected_x_min));
    EXPECT_TRUE(equal(aabb.x_max, test_params.expected_x_max));
    EXPECT_TRUE(equal(aabb.y_min, test_params.expected_y_min));
    EXPECT_TRUE(equal(aabb.y_max, test_params.expected_y_max));
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


struct ShapeIsConvexTestParams
{
    Shape shape;
    bool expected_output;
};

class ShapeIsConvexTest: public testing::TestWithParam<ShapeIsConvexTestParams> { };

TEST_P(ShapeIsConvexTest, ShapeIsConvex)
{
    ShapeIsConvexTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "expected_output " << test_params.expected_output << std::endl;
    bool output = test_params.shape.is_convex();
    std::cout << "output " << output << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeIsConvexTest,
        testing::ValuesIn(std::vector<ShapeIsConvexTestParams>{
            // Convex shapes.
            {
                // Triangle.
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                true,
            }, {
                // Rectangle.
                build_rectangle(3, 2),
                true,
            }, {
                // Diamond (rhombus).
                build_shape({{1, 0}, {2, 1}, {1, 2}, {0, 1}}),
                true,
            }, {
                // Convex pentagon.
                build_shape({{0, 0}, {4, 0}, {5, 2}, {3, 4}, {1, 4}}),
                true,
            }, {
                // Full circle.
                build_circle(1),
                true,
            }, {
                // Half-disk: a line segment closed by an anticlockwise arc.
                // Vertices: (-1, 0) and (1, 0); arc goes CCW through (0, 1).
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                true,
            },
            // Non-convex shapes.
            {
                // L-shape: reflex angle at (1, 1).
                build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {0, 2}}),
                false,
            }, {
                // Square with one vertex pushed inward: reflex angle at (2, 2).
                build_shape({{0, 0}, {4, 0}, {4, 4}, {2, 2}, {0, 4}}),
                false,
            }, {
                // Notched triangle: reflex angle at (2, 1).
                build_shape({{0, 0}, {3, 0}, {3, 3}, {2, 1}, {0, 3}}),
                false,
            }, {
                // Concave pentagon: reflex angle at (1, 1).
                build_shape({{0, 0}, {2, 0}, {2, 2}, {1, 1}, {0, 2}}),
                false,
            }, {
                // Boomerang: reflex angle at (1, 0).
                build_shape({{0, 0}, {3, -1}, {1, 0}, {3, 1}}),
                false,
            }, {
                // Rectangle with the top edge replaced by a clockwise arc.
                // Arc goes CW from (2, 1) to (0, 1) around center (1, 1).
                build_shape({{0, 0}, {2, 0}, {2, 1}, {1, 1, -1}, {0, 1}}),
                false,
            },
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


    template <class basic_json>
    static ShapeContainsTestParams from_json(
            basic_json& json_item)
    {
        ShapeContainsTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.point = Point::from_json(json_item["point"]);
        test_params.strict = json_item["strict"];
        if (json_item.contains("expected_output"))
            test_params.expected_output = json_item["expected_output"];
        return test_params;
    }

    static ShapeContainsTestParams read_json(
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
        return from_json(json);
    }
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
            ShapeContainsTestParams::read_json(
                    (fs::path("data") / "tests" / "shape" / "shape_contains" / "0.json").string()),
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
            }, {
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {{1, {4, 1}}, {1, {4, 3}}},
                {
                    build_path({{4, 3}, {4, 4}, {0, 4}, {0, 0}, {4, 0}, {4, 1}}),
                    build_path({{4, 1}, {4, 3}}),
                },
            },
            }));


struct ShapeReplaceTestParams
{
    Shape shape;
    std::vector<Shape::PathReplacement> paths;
    Shape expected_output;
};

class ShapeReplaceTest: public testing::TestWithParam<ShapeReplaceTestParams> { };

TEST_P(ShapeReplaceTest, ShapeReplace)
{
    ShapeReplaceTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    for (const Shape::PathReplacement& path: test_params.paths) {
        std::cout << "start " << path.start.element_pos << " " << path.start.point.to_string() << std::endl;
        std::cout << "end " << path.end.element_pos << " " << path.end.point.to_string() << std::endl;
    }
    std::cout << "expceted output " << test_params.expected_output.to_string(0) << std::endl;
    Shape output = test_params.shape.replace(test_params.paths);
    std::cout << "output " << output.to_string(0) << std::endl;
    ASSERT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeReplaceTest,
        testing::ValuesIn(std::vector<ShapeReplaceTestParams>{
            {
                build_path({{0, 0}, {0, 10}}),
                {
                    {{0, {0, 1}}, {0, {0, 9}}, build_path({{0, 1}, {1, 5}, {0, 9}}).elements},
                },
                build_path({{0, 0}, {0, 1}, {1, 5}, {0, 9}, {0, 10}}),
            }, {
                build_path({{0, 0}, {0, 10}, {10, 10}}),
                {
                    {{0, {0, 5}}, {1, {5, 10}}, build_path({{0, 5}, {5, 10}}).elements},
                },
                build_path({{0, 0}, {0, 5}, {5, 10}, {10, 10}}),
            }, {
                build_path({{0, 0}, {0, 10}, {10, 10}}),
                {
                    {{1, {1, 10}}, {1, {9, 10}}, build_path({{1, 10}, {5, 11}, {9, 10}}).elements},
                    {{0, {0, 1}}, {0, {0, 9}}, build_path({{0, 1}, {1, 5}, {0, 9}}).elements},
                },
                build_path({{0, 0}, {0, 1}, {1, 5}, {0, 9}, {0, 10}, {1, 10}, {5, 11}, {9, 10}, {10, 10}}),
            }}));


struct ShapeComputeMinMaxTestParams
{
    Shape shape;
    ShapePoint point_1;
    ShapePoint point_2;
    LengthDbl expected_x_min;
    LengthDbl expected_y_min;
    LengthDbl expected_x_max;
    LengthDbl expected_y_max;
};

class ShapeComputeMinMaxTest: public testing::TestWithParam<ShapeComputeMinMaxTestParams> { };

TEST_P(ShapeComputeMinMaxTest, ShapeComputeMinMax)
{
    ShapeComputeMinMaxTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "point_1 element_pos " << test_params.point_1.element_pos
        << " point " << test_params.point_1.point.to_string() << std::endl;
    std::cout << "point_2 element_pos " << test_params.point_2.element_pos
        << " point " << test_params.point_2.point.to_string() << std::endl;
    std::cout << "expected x_min " << test_params.expected_x_min
        << " y_min " << test_params.expected_y_min
        << " x_max " << test_params.expected_x_max
        << " y_max " << test_params.expected_y_max << std::endl;
    AxisAlignedBoundingBox aabb = test_params.shape.compute_min_max(test_params.point_1, test_params.point_2);
    std::cout << "x_min " << aabb.x_min
        << " y_min " << aabb.y_min
        << " x_max " << aabb.x_max
        << " y_max " << aabb.y_max << std::endl;
    EXPECT_TRUE(equal(aabb.x_min, test_params.expected_x_min));
    EXPECT_TRUE(equal(aabb.y_min, test_params.expected_y_min));
    EXPECT_TRUE(equal(aabb.x_max, test_params.expected_x_max));
    EXPECT_TRUE(equal(aabb.y_max, test_params.expected_y_max));
}

// Square: el 0 = (0,0)→(4,0), el 1 = (4,0)→(4,4), el 2 = (4,4)→(0,4), el 3 = (0,4)→(0,0).
// Half-disk: el 0 = segment (-1,0)→(1,0), el 1 = arc (1,0)→(-1,0) CCW center (0,0).
INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeComputeMinMaxTest,
        testing::ValuesIn(std::vector<ShapeComputeMinMaxTestParams>{
            {   // 1. Same element, interior sub-segment.
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {0, {1, 0}}, {0, {3, 0}},
                1, 0, 3, 0,
            }, {  // 2. Same element, full element span.
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {1, {4, 0}}, {1, {4, 4}},
                4, 0, 4, 4,
            }, {  // 3. Two adjacent elements.
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {0, {2, 0}}, {1, {4, 2}},
                2, 0, 4, 2,
            }, {  // 4. Three elements.
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {0, {2, 0}}, {2, {2, 4}},
                2, 0, 4, 4,
            }, {  // 5. All four elements.
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {0, {2, 0}}, {3, {0, 2}},
                0, 0, 4, 4,
            }, {  // 6. Same element, wrap-around (point_1 after point_2).
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {0, {3, 0}}, {0, {1, 0}},
                0, 0, 4, 4,
            }, {  // 7. Arc, first quarter (0°→90°), no extremum at 0° since it's the start point.
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {1, {1, 0}}, {1, {0, 1}},
                0, 0, 1, 1,
            }, {  // 8. Arc, second quarter (90°→180°).
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {1, {0, 1}}, {1, {-1, 0}},
                -1, 0, 0, 1,
            }, {  // 9. Arc, full semicircle (0°→180°).
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {1, {1, 0}}, {1, {-1, 0}},
                -1, 0, 1, 1,
            }, {  // 10. Arc portion crossing top extremum (45°→135°).
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {1, {sqrt(2) / 2, sqrt(2) / 2}}, {1, {-sqrt(2) / 2, sqrt(2) / 2}},
                -sqrt(2) / 2, sqrt(2) / 2, sqrt(2) / 2, 1,
            }, {  // 11. Cross-elements: line segment midpoint → arc top.
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {0, {0, 0}}, {1, {0, 1}},
                0, 0, 1, 1,
            }, {  // 12. Cross-elements: arc top → line segment midpoint (via arc end and el 0 start).
                build_shape({{-1, 0}, {1, 0}, {0, 0, 1}}),
                {1, {0, 1}}, {0, {0, 0}},
                -1, 0, 0, 1,
            },
        }));
