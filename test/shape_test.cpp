//#define SHAPE_TEST_DEBUG

#include "shape/shape.hpp"

#ifdef SHAPE_TEST_DEBUG
#include "shape/writer.hpp"
#endif

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>


using namespace shape;
namespace fs = boost::filesystem;


struct StrictlyLesserAngleTestParams
{
    Point vector_1;
    Point vector_2;
    bool expected_output;
};

void PrintTo(const StrictlyLesserAngleTestParams& params, std::ostream* os)
{
    *os << "vector_1 " << params.vector_1.to_string() << "\n";
    *os << "vector_2 " << params.vector_2.to_string() << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class StrictlyLesserAngleTest: public testing::TestWithParam<StrictlyLesserAngleTestParams> { };

TEST_P(StrictlyLesserAngleTest, StrictlyLesserAngle)
{
    StrictlyLesserAngleTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            // A vector is not strictly lesser than itself, even when the
            // cross product is computed with a fused multiply-add.
            {
                {0.80978762335406329, 0.5867231076603151},
                {0.80978762335406329, 0.5867231076603151},
                false,
            },
            {
                {-0.80978762335406329, -0.5867231076603151},
                {-0.80978762335406329, -0.5867231076603151},
                false,
            },
            {
                {0.89070897486398548, -0.4545740006827802},
                {0.89070897486398548, -0.4545740006827802},
                false,
            },
            // Two horizontal vectors in the same direction have the same
            // angle, whatever their lengths.
            {{2, 0}, {1, 0}, false},
            {{1, 0}, {2, 0}, false},
            {{-2, 0}, {-1, 0}, false},
            {{-1, 0}, {-2, 0}, false},
            // A horizontal vector going right has angle 0, and one going left
            // has angle pi.
            {{1, 0}, {-2, 0}, true},
            {{-2, 0}, {1, 0}, false},
        }),
        [](const testing::TestParamInfo<StrictlyLesserAngleTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct PowerOfTwoTestParams
{
    LengthDbl value;
    LengthDbl expected_largest_power_of_two_lesser_or_equal;
    LengthDbl expected_smallest_power_of_two_greater_or_equal;
};

void PrintTo(const PowerOfTwoTestParams& params, std::ostream* os)
{
    *os << "value " << params.value << "\n";
    *os << "expected_largest_power_of_two_lesser_or_equal " << params.expected_largest_power_of_two_lesser_or_equal << "\n";
    *os << "expected_smallest_power_of_two_greater_or_equal " << params.expected_smallest_power_of_two_greater_or_equal << "\n";
}

class PowerOfTwoTest: public testing::TestWithParam<PowerOfTwoTestParams> { };

TEST_P(PowerOfTwoTest, PowerOfTwo)
{
    PowerOfTwoTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    // Powers of two are exact in floating-point arithmetic.
    EXPECT_EQ(
            largest_power_of_two_lesser_or_equal(test_params.value),
            test_params.expected_largest_power_of_two_lesser_or_equal);
    EXPECT_EQ(
            smallest_power_of_two_greater_or_equal(test_params.value),
            test_params.expected_smallest_power_of_two_greater_or_equal);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        PowerOfTwoTest,
        testing::ValuesIn(std::vector<PowerOfTwoTestParams>{
            {0.25, 0.25, 0.25},
            {0.3, 0.25, 0.5},
            {0.5, 0.5, 0.5},
            {0.75, 0.5, 1},
            {1, 1, 1},
            {3, 2, 4},
            {4, 4, 4},
            {5, 4, 8},
            {1000, 512, 1024},
            {1024, 1024, 1024},
        }),
        [](const testing::TestParamInfo<PowerOfTwoTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementLengthTestParams
{
    ShapeElement element;
    LengthDbl expected_length;
};

void PrintTo(const ShapeElementLengthTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "expected_length " << params.expected_length << "\n";
}

class ShapeElementLengthTest: public testing::TestWithParam<ShapeElementLengthTestParams> { };

TEST_P(ShapeElementLengthTest, ShapeElementLength)
{
    ShapeElementLengthTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeElementLengthTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementLengthPointTestParams
{
    ShapeElement element;
    Point point;
    LengthDbl expected_length;
};

void PrintTo(const ShapeElementLengthPointTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "point " << params.point.to_string() << "\n";
    *os << "expected_length " << params.expected_length << "\n";
}

class ShapeElementLengthPointTest: public testing::TestWithParam<ShapeElementLengthPointTestParams> { };

TEST_P(ShapeElementLengthPointTest, ShapeElementLengthPoint)
{
    ShapeElementLengthPointTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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

        }),
        [](const testing::TestParamInfo<ShapeElementLengthPointTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementPointTestParams
{
    ShapeElement element;
    LengthDbl length;
    Point expected_output;
};

void PrintTo(const ShapeElementPointTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "length " << params.length << "\n";
    *os << "expected_output " << params.expected_output.to_string() << "\n";
}

class ShapeElementPointTest: public testing::TestWithParam<ShapeElementPointTestParams> { };

TEST_P(ShapeElementPointTest, ShapeElementPoint)
{
    ShapeElementPointTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeElementPointTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementFindPointBetweenTestParams
{
    ShapeElement element;
    Point point_1;
    Point point_2;
    Point expected_output;
};

void PrintTo(const ShapeElementFindPointBetweenTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "point_1 " << params.point_1.to_string() << "\n";
    *os << "point_2 " << params.point_2.to_string() << "\n";
    *os << "expected_output " << params.expected_output.to_string() << "\n";
}

class ShapeElementFindPointBetweenTest: public testing::TestWithParam<ShapeElementFindPointBetweenTestParams> { };

TEST_P(ShapeElementFindPointBetweenTest, ShapeElementFindPointBetween)
{
    ShapeElementFindPointBetweenTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

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
            }
        }),
        [](const testing::TestParamInfo<ShapeElementFindPointBetweenTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeFindPointBetweenTestParams
{
    Shape shape;
    ShapePoint point_1;
    ShapePoint point_2;
    ShapePoint expected_output;
};

void PrintTo(const ShapeFindPointBetweenTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "point_1 element_pos " << params.point_1.element_pos << " point " << params.point_1.point.to_string() << "\n";
    *os << "point_2 element_pos " << params.point_2.element_pos << " point " << params.point_2.point.to_string() << "\n";
    *os << "expected_output element_pos " << params.expected_output.element_pos << " point " << params.expected_output.point.to_string() << "\n";
}

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
            }, {  // point_2 at the start of the element following the element
                // of point_1, which is the end of the element of point_1: the
                // point between them is on the element of point_1, not its end.
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {0, {0.5, 0}},
                {1, {1, 0}},
                {0, {0.75, 0}},
            },
        }),
        [](const testing::TestParamInfo<ShapeFindPointBetweenTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementMiddleTestParams
{
    ShapeElement circular_arc;
    Point expected_middle;

    /** Points between which the middle is computed; the start and the end of
     * the element if empty. */
    std::vector<Point> points = {};
};

void PrintTo(const ShapeElementMiddleTestParams& params, std::ostream* os)
{
    *os << "circular_arc\n" << params.circular_arc.to_string() << "\n";
    for (const Point& point: params.points)
        *os << "point " << point.to_string() << "\n";
    *os << "expected_middle\n" << params.expected_middle.to_string() << "\n";
}

class ShapeElementMiddleTest: public testing::TestWithParam<ShapeElementMiddleTestParams> { };

TEST_P(ShapeElementMiddleTest, ShapeElementMiddle)
{
    ShapeElementMiddleTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    Point middle = (test_params.points.empty())?
        test_params.circular_arc.middle():
        test_params.circular_arc.middle(test_params.points[0], test_params.points[1]);
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
            }, {
                build_shape({{0, 1}, {0, 0, 1}, {1, 0}}, true).elements.front(),
                {-1, 0},
                {{0, 1}, {0, -1}},
            }, {
                build_shape({{1, 0}, {0, 0, -1}, {0, 1}}, true).elements.front(),
                {0, -1},
                {{1, 0}, {-1, 0}},
            }, {
                build_shape({{1, 0}, {0, 0, -1}, {0, 1}}, true).elements.front(),
                {-sqrt(2) / 2, -sqrt(2) / 2},
                {{0, -1}, {-1, 0}},
            }, {
                build_circle(1).elements.front(),
                {-1, 0},
            }, {
                build_circle(1).elements.front(),
                {0, -1},
                {{-1, 0}, {1, 0}},
            }
        }),
        [](const testing::TestParamInfo<ShapeElementMiddleTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementMinMaxTestParams
{
    ShapeElement element;
    LengthDbl expected_x_min;
    LengthDbl expected_y_min;
    LengthDbl expected_x_max;
    LengthDbl expected_y_max;
};

void PrintTo(const ShapeElementMinMaxTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "expected x_min " << params.expected_x_min
        << " y_min " << params.expected_y_min
        << " x_max " << params.expected_x_max
        << " y_max " << params.expected_y_max << "\n";
}

class ShapeElementMinMaxTest: public testing::TestWithParam<ShapeElementMinMaxTestParams> { };

TEST_P(ShapeElementMinMaxTest, ShapeElementMinMax)
{
    ShapeElementMinMaxTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeElementMinMaxTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementContainsTestParams
{
    ShapeElement element;
    Point point;
    bool expected_output;
};

void PrintTo(const ShapeElementContainsTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "point " << params.point.to_string() << "\n";
    *os << "expceted output " << params.expected_output << "\n";
}

class ShapeElementContainsTest: public testing::TestWithParam<ShapeElementContainsTestParams> { };

TEST_P(ShapeElementContainsTest, ShapeElementContains)
{
    ShapeElementContainsTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            // Point equal (within tolerance) to the segment's end point, but
            // narrowly failing the betweenness check.
            {
                build_line_segment({136.32056802281824, -8.3472129636816392e-07}, {136.32058090281825, 94.48824084527871}),
                {136.32058090281831, 94.488241703944908},
                true,
            },
            // The arc's own end point, stored slightly off the circle
            // defined by its center and start point (radially off by
            // ~1.27e-6, i.e. more than the on-circle tolerance, even though
            // each coordinate is within tolerance of the true point).
            {
                build_circular_arc({10, 0}, {7.0710687118654755, 7.0710687118654755}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {7.0710687118654755, 7.0710687118654755},
                true,
            },
            // General guarantee: a point equal (within tolerance) to
            // element.point(l) for some interior l (not an endpoint) must be
            // contained.
            {
                build_line_segment({0, 0}, {10, 0}),
                {5.0000009, 0.0000009},
                true,
            },
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {9.238796225112868, 3.826833423650898},
                true,
            },
            // Negative control: same arc, but off the circle (radius off by
            // ~1e-3) by far more than tolerance.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {9.239719204645379, 3.827217007083263},
                false,
            },
        }),
        [](const testing::TestParamInfo<ShapeElementContainsTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementInCircularArcConeTestParams
{
    ShapeElement element;
    Point point;
    bool expected_output;
};

void PrintTo(const ShapeElementInCircularArcConeTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "point " << params.point.to_string() << "\n";
    *os << "expceted output " << params.expected_output << "\n";
}

class ShapeElementInCircularArcConeTest: public testing::TestWithParam<ShapeElementInCircularArcConeTestParams> { };

TEST_P(ShapeElementInCircularArcConeTest, ShapeElementInCircularArcCone)
{
    ShapeElementInCircularArcConeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    bool output = test_params.element.in_circular_arc_cone(test_params.point);
    std::cout << "output " << output << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        ShapeElement,
        ShapeElementInCircularArcConeTest,
        testing::ValuesIn(std::vector<ShapeElementInCircularArcConeTestParams>{
            // The center, apex of the cone.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {0, 0},
                true,
            },
            // The center, apex of the cone, of a clockwise arc.
            {
                build_circular_arc({10, 0}, {0, -10}, {0, 0}, ShapeElementOrientation::Clockwise),
                {0, 0},
                true,
            },
            // The arc's own start point.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {10, 0},
                true,
            },
            // The arc's own end point.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {0, 10},
                true,
            },
            // On-circle point strictly inside the cone (45 degrees, mid-arc).
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {7.0710678118654755, 7.071067811865475},
                true,
            },
            // On-circle point strictly outside the cone (135 degrees, past the end).
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {-7.071067811865475, 7.0710678118654755},
                false,
            },
            // Clockwise arc: on-circle point inside the cone.
            {
                build_circular_arc({10, 0}, {0, -10}, {0, 0}, ShapeElementOrientation::Clockwise),
                {8.660254037844387, -4.999999999999999},
                true,
            },
            // Clockwise arc: on-circle point outside the cone.
            {
                build_circular_arc({10, 0}, {0, -10}, {0, 0}, ShapeElementOrientation::Clockwise),
                {-4.999999999999998, -8.660254037844387},
                false,
            },
            // Full circle: every on-circle point is in the cone.
            {
                build_circular_arc({5, 0}, {5, 0}, {0, 0}, ShapeElementOrientation::Full),
                {-4.698463103929543, -1.7101007166283433},
                true,
            },
            // Tolerance: a point ~5e-6 degrees past the end, off the true
            // end point by less than the 'equal' tolerance, must still be
            // considered in the cone (clamped l lands on 'end', which is
            // 'equal' to this point).
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {-8.726646266149131e-07, 9.999999999999963},
                true,
            },
            // Negative control: a point ~1e-5 degrees past the end, off the
            // true end point by more than tolerance.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {-1.7453292516216967e-06, 9.999999999999847},
                false,
            },
            // The cone test is direction-only, unlike contains(): a point at
            // the same angle as the inside-cone case (45 degrees), but off
            // the circle (radius off by ~1e-3), is still in the cone.
            {
                build_circular_arc({10, 0}, {0, 10}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {7.071774918646662, 7.0717749186466605},
                true,
            },
            // Same, with a much larger radius deviation (5x): quarter-arc
            // from (1, 0) to (0, 1), point at 45 degrees but radius 5.
            {
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {3.5355339059327378, 3.5355339059327373},
                true,
            },
            // Same quarter-arc, point at the correct radius (1x), 45 degrees.
            {
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {0.7071067811865476, 0.7071067811865475},
                true,
            },
            // Same quarter-arc, on-circle point outside the cone (135 degrees).
            {
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {-0.7071067811865475, 0.7071067811865476},
                false,
            },
        }),
        [](const testing::TestParamInfo<ShapeElementInCircularArcConeTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeElementRecomputeCenterTestParams
{
    ShapeElement element;
};

void PrintTo(const ShapeElementRecomputeCenterTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
}

class ShapeElementRecomputeCenterTest: public testing::TestWithParam<ShapeElementRecomputeCenterTestParams> { };

TEST_P(ShapeElementRecomputeCenterTest, ShapeElementRecomputeCenter)
{
    ShapeElementRecomputeCenterTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeElementRecomputeCenterTest::ParamType>& info) {
            return std::to_string(info.index);
        });


enum class AxialSymmetry
{
    IdentityLine,
    XAxis,
    YAxis,
};

struct ShapeElementAxialSymmetryTestParams
{
    ShapeElement element;
    AxialSymmetry axial_symmetry;
    ShapeElement expected_output;
};

void PrintTo(const ShapeElementAxialSymmetryTestParams& params, std::ostream* os)
{
    *os << "element " << params.element.to_string() << "\n";
    *os << "axial_symmetry " << (int)params.axial_symmetry << "\n";
    *os << "expected_output " << params.expected_output.to_string() << "\n";
}

class ShapeElementAxialSymmetryTest: public testing::TestWithParam<ShapeElementAxialSymmetryTestParams> { };

TEST_P(ShapeElementAxialSymmetryTest, ShapeElementAxialSymmetry)
{
    ShapeElementAxialSymmetryTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    ShapeElement output;
    switch (test_params.axial_symmetry) {
    case AxialSymmetry::IdentityLine:
        output = test_params.element.axial_symmetry_identity_line();
        break;
    case AxialSymmetry::XAxis:
        output = test_params.element.axial_symmetry_x_axis();
        break;
    case AxialSymmetry::YAxis:
        output = test_params.element.axial_symmetry_y_axis();
        break;
    }
    std::cout << "output " << output.to_string() << std::endl;
    EXPECT_TRUE(equal(output, test_params.expected_output));
}

// The image of an element is reversed (its start is the image of the end of
// the element), so that the image of a shape keeps its orientation. The image
// of a circular arc must be the same portion of the image of its circle, not
// its complement.
INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeElementAxialSymmetryTest,
        testing::ValuesIn(std::vector<ShapeElementAxialSymmetryTestParams>{
            {   // Line segment, y axis.
                build_line_segment({0, 0}, {2, 1}),
                AxialSymmetry::YAxis,
                build_line_segment({-2, 1}, {0, 0}),
            }, {  // Line segment, x axis.
                build_line_segment({0, 0}, {2, 1}),
                AxialSymmetry::XAxis,
                build_line_segment({2, -1}, {0, 0}),
            }, {  // Line segment, identity line.
                build_line_segment({0, 0}, {2, 1}),
                AxialSymmetry::IdentityLine,
                build_line_segment({1, 2}, {0, 0}),
            }, {  // Anticlockwise quarter arc (0° to 90°), y axis: 90° to 180°.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                AxialSymmetry::YAxis,
                build_circular_arc({0, 1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
            }, {  // Anticlockwise quarter arc (0° to 90°), x axis: -90° to 0°.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                AxialSymmetry::XAxis,
                build_circular_arc({0, -1}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
            }, {  // Anticlockwise quarter arc (0° to 90°), identity line: itself.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                AxialSymmetry::IdentityLine,
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
            }, {  // Clockwise arc (90° to 0°) not centered on the origin, y axis.
                build_circular_arc({1, 3}, {2, 2}, {1, 2}, ShapeElementOrientation::Clockwise),
                AxialSymmetry::YAxis,
                build_circular_arc({-2, 2}, {-1, 3}, {-1, 2}, ShapeElementOrientation::Clockwise),
            }, {  // Clockwise arc (90° to 0°) not centered on the origin, x axis.
                build_circular_arc({1, 3}, {2, 2}, {1, 2}, ShapeElementOrientation::Clockwise),
                AxialSymmetry::XAxis,
                build_circular_arc({2, -2}, {1, -3}, {1, -2}, ShapeElementOrientation::Clockwise),
            }, {  // Clockwise arc (90° to 0°) not centered on the origin, identity line.
                build_circular_arc({1, 3}, {2, 2}, {1, 2}, ShapeElementOrientation::Clockwise),
                AxialSymmetry::IdentityLine,
                build_circular_arc({2, 2}, {3, 1}, {2, 1}, ShapeElementOrientation::Clockwise),
            },
        }),
        [](const testing::TestParamInfo<ShapeElementAxialSymmetryTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeIsConvexTestParams
{
    Shape shape;
    bool expected_output;
};

void PrintTo(const ShapeIsConvexTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class ShapeIsConvexTest: public testing::TestWithParam<ShapeIsConvexTestParams> { };

TEST_P(ShapeIsConvexTest, ShapeIsConvex)
{
    ShapeIsConvexTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeIsConvexTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeComputeAreaTestParams
{
    Shape shape;
    AreaDbl expected_area;
};

void PrintTo(const ShapeComputeAreaTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected area " << to_string(params.expected_area) << "\n";
}

class ShapeComputeAreaTest: public testing::TestWithParam<ShapeComputeAreaTestParams> { };

TEST_P(ShapeComputeAreaTest, ShapeComputeArea)
{
    ShapeComputeAreaTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            }
        }),
        [](const testing::TestParamInfo<ShapeComputeAreaTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeComputeFurthestPointsTestParams
{
    Shape shape;
    Angle angle;
    Shape::FurthestPoint expected_point_min;
    Shape::FurthestPoint expected_point_max;
};

void PrintTo(const ShapeComputeFurthestPointsTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "angle " << params.angle << "\n";
    *os << "expected point_min " << params.expected_point_min.point.to_string()
        << " pos " << params.expected_point_min.element_pos
        << " point_max " << params.expected_point_max.point.to_string()
        << " pos " << params.expected_point_max.element_pos
        << "\n";
}

class ShapeComputeFurthestPointsTest: public testing::TestWithParam<ShapeComputeFurthestPointsTestParams> { };

TEST_P(ShapeComputeFurthestPointsTest, ShapeComputeFurthestPoints)
{
    ShapeComputeFurthestPointsTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
        }),
        [](const testing::TestParamInfo<ShapeComputeFurthestPointsTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeContainsTestParams
{
    Shape shape;
    Point point;
    bool strict;
    bool expected_output;


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
        ShapeContainsTestParams test_params;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.point = Point::from_json(json["point"]);
        test_params.strict = json["strict"];
        if (json.contains("expected_output"))
            test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const ShapeContainsTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "point " << params.point.to_string() << "\n";
    *os << "strict " << params.strict << "\n";
    *os << "expceted output " << params.expected_output << "\n";
}

class ShapeContainsTest: public testing::TestWithParam<ShapeContainsTestParams> { };

TEST_P(ShapeContainsTest, ShapeContains)
{
    ShapeContainsTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            {
                // A CircularArc whose *end* is exactly its own circle's
                // topmost point (its tangent there is exactly horizontal),
                // closed by a chord back to the arc's start. The horizontal
                // ray used by the ray-casting algorithm for this query point
                // is tangent to the arc exactly at that endpoint. The point
                // queried here (-3, 1) is well outside this small shape
                // (whose points all satisfy x in [0, 1], y in [0, 1]) but is
                // currently reported as contained regardless of 'strict':
                // CircularArc elements only exclude an intersection at their
                // own start/end via a proper vs. improper distinction,
                // unlike LineSegment elements (see the "Horizontal edges are
                // excluded" special case in Shape::contains), so a ray
                // tangent to an arc at exactly its topmost/bottommost point
                // is miscounted.
                build_shape({
                        build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                        build_line_segment({0, 1}, {1, 0}),
                    }),
                {-3, 1},
                true,
                false,
            },
        }),
        [](const testing::TestParamInfo<ShapeContainsTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeFindPointStrictlyInsideTestParams
{
    std::string name;
    Shape shape;


    static ShapeFindPointStrictlyInsideTestParams read_json(
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
        ShapeFindPointStrictlyInsideTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        return test_params;
    }
};

void PrintTo(const ShapeFindPointStrictlyInsideTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
}

class ShapeFindPointStrictlyInsideTest: public testing::TestWithParam<ShapeFindPointStrictlyInsideTestParams> { };

TEST_P(ShapeFindPointStrictlyInsideTest, ShapeFindPointStrictlyInside)
{
    ShapeFindPointStrictlyInsideTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    Point output = test_params.shape.find_point_strictly_inside();
    std::cout << "output " << output.to_string() << std::endl;
    EXPECT_TRUE(test_params.shape.contains(output, true));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeFindPointStrictlyInsideTest,
        testing::ValuesIn(std::vector<ShapeFindPointStrictlyInsideTestParams>{
            {
                "Rectangle",
                build_rectangle(2, 4),
            }, {
                "Circle",
                build_circle(2),
            }, {
                "ConcavePolygon",
                build_shape({{0, 100}, {100, 100}, {100, 0}, {200, 0}, {200, 200}, {0, 200}}),
            }, {
                "Diamond",
                build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}}),
            },
            // Regression test for fontanf/packingsolver issue #563. This
            // concave shape has a corner-fillet CircularArc (radius 5,
            // center (3.5, 38)) whose bottommost point is tangent to y = 33
            // strictly between the arc's own start and end (not at either
            // endpoint, so the tangent-at-endpoint handling in
            // Shape::contains() does not apply here). find_point_strictly_
            // inside() samples a horizontal ray at y = 33, and its "two
            // smallest x-intersections" heuristic pairs that tangent touch
            // with the next real crossing as if both were real crossings,
            // returning a midpoint that is not actually inside the shape.
            ShapeFindPointStrictlyInsideTestParams::read_json(
                    (fs::path("data") / "tests" / "shape" / "find_point_strictly_inside" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<ShapeFindPointStrictlyInsideTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeWithHolesFindPointStrictlyInsideTestParams
{
    std::string name;
    ShapeWithHoles shape;
};

void PrintTo(const ShapeWithHolesFindPointStrictlyInsideTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
}

class ShapeWithHolesFindPointStrictlyInsideTest: public testing::TestWithParam<ShapeWithHolesFindPointStrictlyInsideTestParams> { };

TEST_P(ShapeWithHolesFindPointStrictlyInsideTest, ShapeWithHolesFindPointStrictlyInside)
{
    ShapeWithHolesFindPointStrictlyInsideTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    Point output = test_params.shape.find_point_strictly_inside();
    std::cout << "output " << output.to_string() << std::endl;
    EXPECT_TRUE(test_params.shape.contains(output, true));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeWithHolesFindPointStrictlyInsideTest,
        testing::ValuesIn(std::vector<ShapeWithHolesFindPointStrictlyInsideTestParams>{
            {
                "Rectangle",
                {build_rectangle(2, 4), {}},
            }, {
                "RectangleWithHole",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(2, 8, 2, 8)}},
            }, {
                "CircleWithCircularHole",
                {build_circle(2), {build_circle(1)}},
            }, {
                // The ray at y = 1 is tangent to the bottom of the circular
                // arc, at (2, 1).
                "TangentArc",
                {build_shape({{8, 0}, {10, 0}, {10, 2}, {0, 2}, {0, 1.5}, {1.5, 1.5}, {2, 1.5, 1}, {2.5, 1.5}, {8, 1.5}}), {}},
            }, {
                // The ray at y = 5 is tangent to the circular hole, at (5, 5).
                "TangentHole",
                {build_rectangle(0, 10, 0, 10), {build_shape({{6, 6}, {5, 6, 1}, {4, 6}, {5, 6, 1}})}},
            }, {
                // The ray at y = 5 runs along the bottom edge of the hole.
                "RayAlongHoleEdge",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(2, 8, 5, 8)}},
            },
        }),
        [](const testing::TestParamInfo<ShapeWithHolesFindPointStrictlyInsideTest::ParamType>& info) {
            return info.param.name;
        });


struct ShapeExtractPathTestParams
{
    Shape shape;
    ShapePoint point_start;
    ShapePoint point_end;
    Shape expected_path;
};

void PrintTo(const ShapeExtractPathTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "point_start element_pos " << params.point_start.element_pos << " point " << params.point_start.point.to_string() << "\n";
    *os << "point_end element_pos " << params.point_end.element_pos << " point " << params.point_end.point.to_string() << "\n";
    *os << "expected path " << params.expected_path.to_string(1) << "\n";
}

class ShapeExtractPathTest: public testing::TestWithParam<ShapeExtractPathTestParams> { };

TEST_P(ShapeExtractPathTest, ShapeExtractPath)
{
    ShapeExtractPathTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    Shape path = test_params.shape.extract_path(test_params.point_start, test_params.point_end);
    std::cout << "path " << path.to_string(1) << std::endl;
    EXPECT_TRUE(equal(path, test_params.expected_path));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeExtractPathTest,
        testing::ValuesIn(std::vector<ShapeExtractPathTestParams>{
            {
                build_path({{0, 0}, {0, 2}}),
                {0, {0, 0}},
                {0, {0, 1}},
                build_path({{0, 0}, {0, 1}}),
            }, {
                build_path({{0, 0}, {0, 2}}),
                {0, {0, 1}},
                {0, {0, 2}},
                build_path({{0, 1}, {0, 2}}),
            }, {
                build_shape({{0, 0}, {0, 4}, {2, 4}, {2, 0}}),
                {0, {0, 1}},
                {2, {2, 3}},
                build_path({{0, 1}, {0, 4}, {2, 4}, {2, 3}}),
            }, {
                build_shape({{0, 0}, {0, 4}, {2, 4}, {2, 0}}),
                {2, {2, 3}},
                {0, {0, 1}},
                build_path({{2, 3}, {2, 0}, {0, 0}, {0, 1}}),
            }, {
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {1, {4, 1}},
                {1, {4, 3}},
                build_path({{4, 1}, {4, 3}}),
            }, {
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                {1, {4, 3}},
                {1, {4, 1}},
                build_path({{4, 3}, {4, 4}, {0, 4}, {0, 0}, {4, 0}, {4, 1}}),
            }, {
                build_circle(1),
                {0, {1, 0}},
                {0, {0, 1}},
                build_path({{1, 0}, {0, 0, 1}, {0, 1}}),
            }, {
                build_circle(1),
                {0, {0, 1}},
                {0, {1, 0}},
                build_path({{0, 1}, {0, 0, 1}, {1, 0}}),
            },
        }),
        [](const testing::TestParamInfo<ShapeExtractPathTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct ShapeReplaceTestParams
{
    Shape shape;
    std::vector<Shape::PathReplacement> paths;
    Shape expected_output;
};

void PrintTo(const ShapeReplaceTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    for (const Shape::PathReplacement& path: params.paths) {
        *os << "start " << path.start.element_pos << " " << path.start.point.to_string() << "\n";
        *os << "end " << path.end.element_pos << " " << path.end.point.to_string() << "\n";
    }
    *os << "expceted output " << params.expected_output.to_string(0) << "\n";
}

class ShapeReplaceTest: public testing::TestWithParam<ShapeReplaceTestParams> { };

TEST_P(ShapeReplaceTest, ShapeReplace)
{
    ShapeReplaceTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            }
        }),
        [](const testing::TestParamInfo<ShapeReplaceTest::ParamType>& info) {
            return std::to_string(info.index);
        });


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

void PrintTo(const ShapeComputeMinMaxTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "point_1 element_pos " << params.point_1.element_pos
        << " point " << params.point_1.point.to_string() << "\n";
    *os << "point_2 element_pos " << params.point_2.element_pos
        << " point " << params.point_2.point.to_string() << "\n";
    *os << "expected x_min " << params.expected_x_min
        << " y_min " << params.expected_y_min
        << " x_max " << params.expected_x_max
        << " y_max " << params.expected_y_max << "\n";
}

class ShapeComputeMinMaxTest: public testing::TestWithParam<ShapeComputeMinMaxTestParams> { };

TEST_P(ShapeComputeMinMaxTest, ShapeComputeMinMax)
{
    ShapeComputeMinMaxTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
            }, {  // 13. Quarter of a full circle.
                build_circle(1),
                {0, {1, 0}}, {0, {0, 1}},
                0, 0, 1, 1,
            },
        }),
        [](const testing::TestParamInfo<ShapeComputeMinMaxTest::ParamType>& info) {
            return std::to_string(info.index);
        });

struct ShapeToSvgPathTestParams
{
    Shape shape;
    std::string expected_svg_path;
};

void PrintTo(const ShapeToSvgPathTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected SVG path " << params.expected_svg_path << "\n";
}

class ShapeToSvgPathTest: public testing::TestWithParam<ShapeToSvgPathTestParams> { };

TEST_P(ShapeToSvgPathTest, ShapeToSvgPath)
{
    ShapeToSvgPathTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    std::string svg_path = test_params.shape.to_svg_path();
    std::cout << "SVG path " << svg_path << std::endl;
    EXPECT_EQ(svg_path, test_params.expected_svg_path);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeToSvgPathTest,
        testing::ValuesIn(std::vector<ShapeToSvgPathTestParams>{
            {  // 0. Closed shape.
                build_shape({{0, 0}, {1, 0}, {1, 1}}),
                "M0.000000,-0.000000L1.000000,-0.000000L1.000000,-1.000000L0.000000,-0.000000Z",
            }, {  // 1. Open path.
                build_path({{0, 0}, {1, 0}, {1, 1}}),
                "M0.000000,-0.000000L1.000000,-0.000000L1.000000,-1.000000",
            }, {  // 2. Anticlockwise quarter circle.
                build_path({{1, 0}, {0, 0, 1}, {0, 1}}),
                "M1.000000,-0.000000A1.000000,1.000000,0,0,0,0.000000,-1.000000",
            }, {  // 3. Anticlockwise three quarters of a circle.
                build_path({{0, 1}, {0, 0, 1}, {1, 0}}),
                "M0.000000,-1.000000A1.000000,1.000000,0,1,0,1.000000,-0.000000",
            }, {  // 4. Clockwise quarter circle.
                build_path({{0, 1}, {0, 0, -1}, {1, 0}}),
                "M0.000000,-1.000000A1.000000,1.000000,0,0,1,1.000000,-0.000000",
            }, {  // 5. Clockwise three quarters of a circle.
                build_path({{1, 0}, {0, 0, -1}, {0, 1}}),
                "M1.000000,-0.000000A1.000000,1.000000,0,1,1,0.000000,-1.000000",
            },
        }),
        [](const testing::TestParamInfo<ShapeToSvgPathTest::ParamType>& info) {
            return std::to_string(info.index);
        });

struct ShapeWriteSvgTestParams
{
    Shape shape;
};

void PrintTo(const ShapeWriteSvgTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
}

class ShapeWriteSvgTest: public testing::TestWithParam<ShapeWriteSvgTestParams> { };

/** Count the occurrences of 'pattern' in the file 'file_path'. */
Counter count_occurrences(
        const std::string& file_path,
        const std::string& pattern)
{
    std::ifstream file(file_path);
    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();
    Counter number_of_occurrences = 0;
    for (std::string::size_type pos = content.find(pattern);
            pos != std::string::npos;
            pos = content.find(pattern, pos + 1)) {
        number_of_occurrences++;
    }
    return number_of_occurrences;
}

TEST_P(ShapeWriteSvgTest, ShapeWriteSvg)
{
    ShapeWriteSvgTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    // Each test case uses its own file, since they may run in parallel.
    std::string test_name = testing::UnitTest::GetInstance()->current_test_info()->name();
    std::replace(test_name.begin(), test_name.end(), '/', '_');
    std::string file_path = testing::TempDir() + "shape_write_svg_test_" + test_name + ".svg";

    test_params.shape.write_svg(file_path);
    EXPECT_EQ(count_occurrences(file_path, "<path"), 1);
    EXPECT_EQ(count_occurrences(file_path, "<path d=\"M"), 1);

    for (const ShapeElement& element: test_params.shape.elements) {
        element.write_svg(file_path);
        EXPECT_EQ(count_occurrences(file_path, "<path"), 1);
        EXPECT_EQ(count_occurrences(file_path, "<path d=\"M"), 1);
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeWriteSvgTest,
        testing::ValuesIn(std::vector<ShapeWriteSvgTestParams>{
            {build_shape({{0, 0}, {1, 0}, {1, 1}})},
            {build_shape({{-1, 0}, {1, 0}, {0, 0, 1}})},
            {build_circle(1)},
        }),
        [](const testing::TestParamInfo<ShapeWriteSvgTest::ParamType>& info) {
            return std::to_string(info.index);
        });

struct ShapeWithHolesCheckHolesTestParams
{
    std::string name;
    ShapeWithHoles shape;
    bool expected_output;


    static ShapeWithHolesCheckHolesTestParams read_json(
            const std::string& file_path,
            bool expected_output)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        ShapeWithHolesCheckHolesTestParams test_params;
        test_params.name = fs::path(file_path).stem().string();
        test_params.shape = ShapeWithHoles::from_json(json["shape"]);
        test_params.expected_output = expected_output;
        return test_params;
    }
};

void PrintTo(const ShapeWithHolesCheckHolesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(1) << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class ShapeWithHolesCheckHolesTest: public testing::TestWithParam<ShapeWithHolesCheckHolesTestParams> { };

TEST_P(ShapeWithHolesCheckHolesTest, ShapeWithHolesCheckHoles)
{
    ShapeWithHolesCheckHolesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    EXPECT_EQ(test_params.shape.check(), test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeWithHolesCheckHolesTest,
        testing::ValuesIn(std::vector<ShapeWithHolesCheckHolesTestParams>{
            {
                "HoleInside",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(2, 8, 2, 8)}},
                true,
            }, {
                "HoleTouchingOutlineAtVertex",
                {build_rectangle(0, 10, 0, 10), {build_shape({{0, 5}, {5, 2}, {5, 8}})}},
                true,
            }, {
                "HoleTouchingOutlineAlongEdge",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(0, 2, 2, 4)}},
                true,
            }, {
                // The hole goes out of the outer shape, but the point
                // strictly inside the hole used to check it is inside the
                // outer shape.
                "HoleCrossingOutline",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(2, 11, 2, 8)}},
                false,
            }, {
                "HoleOutside",
                {build_rectangle(0, 10, 0, 10), {build_rectangle(12, 14, 2, 8)}},
                false,
            },
            // About 14% of the area of the hole is outside of the outer shape
            // (formerly a trapezoidation fixture).
            ShapeWithHolesCheckHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "shape" / "check" / "hole_partially_outside.json").string(),
                    false),
        }),
        [](const testing::TestParamInfo<ShapeWithHolesCheckHolesTest::ParamType>& info) {
            return info.param.name;
        });

struct ShapeCheckTestParams
{
    std::string name;
    Shape shape;
    bool expected_output;
};

void PrintTo(const ShapeCheckTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(1) << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class ShapeCheckTest: public testing::TestWithParam<ShapeCheckTestParams> { };

TEST_P(ShapeCheckTest, ShapeCheck)
{
    ShapeCheckTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    EXPECT_EQ(test_params.shape.check(), test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeCheckTest,
        testing::ValuesIn(std::vector<ShapeCheckTestParams>{
            {
                "Square",
                build_square(1),
                true,
            }, {
                "Path",
                build_path({{0, 0}, {1, 0}, {1, 1}}),
                true,
            }, {
                "Circle",
                build_circle(1),
                true,
            }, {
                // The end of the first element is equal to the start of the
                // second one within tolerance.
                "SquareWithGapWithinTolerance",
                build_shape({
                        build_line_segment({0, 0}, {1, 0}),
                        build_line_segment({1, 1e-7}, {1, 1}),
                        build_line_segment({1, 1}, {0, 1}),
                        build_line_segment({0, 1}, {0, 0})}),
                true,
            }, {
                // The end of the first element isn't the start of the second
                // one, although the shape is closed and doesn't intersect
                // itself.
                "DisconnectedSquare",
                build_shape({
                        build_line_segment({0, 0}, {1, 0}),
                        build_line_segment({1, 0.5}, {1, 1}),
                        build_line_segment({1, 1}, {0, 1}),
                        build_line_segment({0, 1}, {0, 0})}),
                false,
            }, {
                "DisconnectedPath",
                build_path({
                        build_line_segment({0, 0}, {1, 0}),
                        build_line_segment({2, 0}, {3, 0})}),
                false,
            }, {
                "NotClosed",
                build_shape({
                        build_line_segment({0, 0}, {1, 0}),
                        build_line_segment({1, 0}, {1, 1}),
                        build_line_segment({1, 1}, {0, 1})}),
                false,
            }, {
                "Clockwise",
                build_square(1).reverse(),
                false,
            }, {
                "SelfIntersecting",
                build_shape({{0, 0}, {1, 1}, {1, 0}, {0, 1}}),
                false,
            }, {
                "Empty",
                Shape(),
                false,
            },
        }),
        [](const testing::TestParamInfo<ShapeCheckTest::ParamType>& info) {
            return info.param.name;
        });

struct ShapeEqualTestParams
{
    std::string name;
    Shape shape_1;
    Shape shape_2;
    bool expected_operator_equal;
    bool expected_equal;
};

void PrintTo(const ShapeEqualTestParams& params, std::ostream* os)
{
    *os << "shape_1 " << params.shape_1.to_string(1) << "\n";
    *os << "shape_2 " << params.shape_2.to_string(1) << "\n";
}

class ShapeEqualTest: public testing::TestWithParam<ShapeEqualTestParams> { };

TEST_P(ShapeEqualTest, ShapeEqual)
{
    ShapeEqualTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    EXPECT_EQ(test_params.shape_1 == test_params.shape_2, test_params.expected_operator_equal);
    EXPECT_EQ(equal(test_params.shape_1, test_params.shape_2), test_params.expected_equal);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ShapeEqualTest,
        testing::ValuesIn(std::vector<ShapeEqualTestParams>{
            {
                "RotatedSquare",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                build_shape({{1, 1}, {0, 1}, {0, 0}, {1, 0}}),
                true,
                true,
            }, {
                "ShapeAndPath",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                build_path({{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}}),
                false,
                false,
            }, {
                "RotatedPath",
                build_path({{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}}),
                build_path({{1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}}),
                false,
                false,
            }, {
                "SquareWithinTolerance",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                build_shape({{0, 0}, {1 + 1e-7, 0}, {1, 1}, {0, 1}}),
                false,
                true,
            },
        }),
        [](const testing::TestParamInfo<ShapeEqualTest::ParamType>& info) {
            return info.param.name;
        });

TEST(ShapeWithHolesCheckTest, RejectsHoleNotContainedInOuterShape)
{
    // Both the outer shape and the hole individually pass Shape::check(),
    // but the hole sits entirely above the rectangle (y in [140, 160] vs.
    // the rectangle's [0, 100]) rather than inside it -- ShapeWithHoles is
    // still expected to reject this as a whole.
    ShapeWithHoles shape = {
        build_rectangle(200, 100),
        {build_shape({{50, 150}, {60, 140}, {70, 150}, {60, 160}})}
    };
    EXPECT_TRUE(shape.shape.check());
    EXPECT_TRUE(shape.holes[0].check());
    EXPECT_FALSE(shape.check());
}
