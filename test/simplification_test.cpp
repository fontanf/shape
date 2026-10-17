#include "shape/simplification.hpp"
#include "shape/boolean_operations.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <cmath>
#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct TryExtendToIntersectionTestParams
{
    std::string name;
    ShapeElement element_prev;
    ShapeElement element_next;
    bool expected_feasible;
    /** Meaningful only when expected_feasible is true. */
    Point expected_intersection;
};

void PrintTo(const TryExtendToIntersectionTestParams& params, std::ostream* os)
{
    *os << "element_prev " << params.element_prev.to_string() << "\n";
    *os << "element_next " << params.element_next.to_string() << "\n";
}

class TryExtendToIntersectionTest:
    public testing::TestWithParam<TryExtendToIntersectionTestParams> { };

TEST_P(TryExtendToIntersectionTest, TryExtendToIntersection)
{
    TryExtendToIntersectionTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    ExtendToIntersectionOutput output = try_extend_to_intersection(
            test_params.element_prev,
            test_params.element_next);

    std::cout << "feasible " << output.feasible << std::endl;
    if (output.feasible) {
        std::cout << "new_element_prev.end "
            << output.new_element_prev.end.to_string() << std::endl;
        std::cout << "new_element_next.start "
            << output.new_element_next.start.to_string() << std::endl;
    }

    EXPECT_EQ(output.feasible, test_params.expected_feasible);
    if (test_params.expected_feasible && output.feasible) {
        EXPECT_TRUE(equal(output.new_element_prev.end, test_params.expected_intersection));
        EXPECT_TRUE(equal(output.new_element_next.start, test_params.expected_intersection));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        TryExtendToIntersectionTest,
        testing::ValuesIn(std::vector<TryExtendToIntersectionTestParams>{
            {  // Line + Line, feasible: horizontal then vertical.
                "LineLineFeasible",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({3, 1}, {3, 3}),
                true,
                {3, 0},
            }, {  // Line + Line, infeasible: intersection lies behind prev's end.
                "LineLineInfeasibleBehind",
                build_line_segment({0, 0}, {5, 0}),
                build_line_segment({3, 1}, {3, 3}),
                false,
                {0, 0},
            }, {  // Line + Line, infeasible: intersection lies inside next.
                "LineLineInfeasibleInsideNext",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({3, -1}, {3, 3}),
                false,
                {0, 0},
            }, {  // Line + Line, infeasible: parallel lines never meet.
                "LineLineParallel",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({0, 1}, {1, 1}),
                false,
                {0, 0},
            }, {  // CCW Arc + Line, feasible.
                "ArcLineFeasible",
                build_circular_arc({2, 0}, {1, sqrt(3.0)}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 3}, {0, 5}),
                true,
                {0, 2},
            }, {  // Line + CCW Arc, feasible.
                "LineArcFeasible",
                build_line_segment({-3, 0}, {-1.5, 0}),
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                true,
                {-1, 0},
            }, {  // CCW Arc + CCW Arc, feasible.
                "ArcArcFeasible",
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({0, 2}, {-1, 2}, {0, 1}, ShapeElementOrientation::Anticlockwise),
                true,
                {-sqrt(3.0) / 2, 0.5},
            }, {  // CCW Arc + CCW Arc, infeasible: circles too far apart.
                "ArcArcTooFarApart",
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({6, 0}, {5, 1}, {5, 0}, ShapeElementOrientation::Anticlockwise),
                false,
                {0, 0},
            },
        }),
        [](const testing::TestParamInfo<TryExtendToIntersectionTest::ParamType>& info) {
            return info.param.name;
        });


////////////////////////////////////////////////////////////////////////////////
// try_round_corner
////////////////////////////////////////////////////////////////////////////////

struct TryRoundCornerTestParams
{
    std::string name;
    ShapeElement element_prev;
    ShapeElement element_next;
    LengthDbl radius;
    bool expected_feasible;
    /** Only meaningful when expected_feasible is true. */
    std::vector<ShapeElement> expected_elements;
};

void PrintTo(const TryRoundCornerTestParams& params, std::ostream* os)
{
    *os << "element_prev " << params.element_prev.to_string() << "\n";
    *os << "element_next " << params.element_next.to_string() << "\n";
    *os << "radius " << params.radius << "\n";
}

class TryRoundCornerTest:
    public testing::TestWithParam<TryRoundCornerTestParams> { };

TEST_P(TryRoundCornerTest, TryRoundCorner)
{
    TryRoundCornerTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    RoundCornerOutput output = try_round_corner(
            test_params.element_prev,
            test_params.element_next,
            test_params.radius);

    std::cout << "feasible " << output.feasible << std::endl;
    for (const ShapeElement& element: output.elements)
        std::cout << "  " << element.to_string() << std::endl;

    EXPECT_EQ(output.feasible, test_params.expected_feasible);
    if (test_params.expected_feasible && output.feasible) {
        ASSERT_EQ(output.elements.size(), test_params.expected_elements.size());
        for (ElementPos pos = 0;
                pos < (ElementPos)output.elements.size();
                ++pos) {
            EXPECT_TRUE(equal(output.elements[pos], test_params.expected_elements[pos]));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// try_smooth_arc_to_line
////////////////////////////////////////////////////////////////////////////////

struct TrySmoothArcToLineTestParams
{
    std::string name;
    ShapeElement element_prev;
    ShapeElement element_next;
    bool expected_feasible;
    /** Only meaningful when expected_feasible is true. */
    ShapeElement expected_new_element_prev;
    ShapeElement expected_new_element_next;
};

void PrintTo(const TrySmoothArcToLineTestParams& params, std::ostream* os)
{
    *os << "element_prev " << params.element_prev.to_string() << "\n";
    *os << "element_next " << params.element_next.to_string() << "\n";
}

class TrySmoothArcToLineTest:
    public testing::TestWithParam<TrySmoothArcToLineTestParams> { };

TEST_P(TrySmoothArcToLineTest, TrySmoothArcToLine)
{
    TrySmoothArcToLineTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    SmoothArcToLineOutput output = try_smooth_arc_to_line(
            test_params.element_prev,
            test_params.element_next);

    std::cout << "feasible " << output.feasible << std::endl;
    if (output.feasible) {
        std::cout << "new_element_prev " << output.new_element_prev.to_string() << std::endl;
        std::cout << "new_element_next " << output.new_element_next.to_string() << std::endl;
    }

    EXPECT_EQ(output.feasible, test_params.expected_feasible);
    if (test_params.expected_feasible && output.feasible) {
        EXPECT_TRUE(equal(output.new_element_prev, test_params.expected_new_element_prev));
        EXPECT_TRUE(equal(output.new_element_next, test_params.expected_new_element_next));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        TrySmoothArcToLineTest,
        testing::ValuesIn(std::vector<TrySmoothArcToLineTestParams>{
            {  // CCW arc + line, feasible.
                "CCWArcLineFeasible",
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 2}),
                true,
                build_circular_arc({1, 0}, {sqrt(3.0) / 2, 0.5}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({sqrt(3.0) / 2, 0.5}, {0, 2}),
            }, {  // CW arc + line, feasible.
                "CWArcLineFeasible",
                build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({1, 0}, {2, 0}),
                true,
                build_circular_arc({0, 1}, {0.5, sqrt(3.0) / 2}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({0.5, sqrt(3.0) / 2}, {2, 0}),
            }, {  // Line + CCW arc, feasible.
                "LineCCWArcFeasible",
                build_line_segment({0, 2}, {0, 1}),
                build_circular_arc({0, 1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                true,
                build_line_segment({0, 2}, {-sqrt(3.0) / 2, 0.5}),
                build_circular_arc({-sqrt(3.0) / 2, 0.5}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
            }, {  // Line + CW arc, feasible.
                "LineCWArcFeasible",
                build_line_segment({-1, sqrt(3.0)}, {0, 1}),
                build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                true,
                build_line_segment({-1, sqrt(3.0)}, {0.5, sqrt(3.0) / 2}),
                build_circular_arc({0.5, sqrt(3.0) / 2}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
            }, {  // Infeasible: neither (arc, line) nor (line, arc).
                "LineLineInfeasible",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {1, 2}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            }, {  // Infeasible: element_next.end is inside the circle.
                "EndInsideCircle",
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 0.5}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            }, {  // Infeasible: tangent point lies on arc.end.
                "TangentPointOnArcEnd",
                build_circular_arc({1, 0}, {sqrt(3.0) / 2, 0.5}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({sqrt(3.0) / 2, 0.5}, {0, 2}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            },
        }),
        [](const testing::TestParamInfo<TrySmoothArcToLineTest::ParamType>& info) {
            return info.param.name;
        });


INSTANTIATE_TEST_SUITE_P(
        Shape,
        TryRoundCornerTest,
        testing::ValuesIn(std::vector<TryRoundCornerTestParams>{
            {  // 90° CCW (left) turn: horizontal then up, r=1.
               // tangent_length = r*tan(45°) = 1.
               // T1=(1,0), T2=(2,1), center=(1,1), arc CCW.
                "CCWTurn90",
                build_line_segment({0, 0}, {2, 0}),
                build_line_segment({2, 0}, {2, 2}),
                1.0,
                true,
                {
                    build_line_segment({0, 0}, {1, 0}),
                    build_circular_arc({1, 0}, {2, 1}, {1, 1}, ShapeElementOrientation::Anticlockwise),
                    build_line_segment({2, 1}, {2, 2}),
                },
            }, {  // 90° CW (right) turn: horizontal then down, r=1.
               // tangent_length = r*tan(45°) = 1.
               // T1=(1,0), T2=(2,-1), center=(1,-1), arc CW.
                "CWTurn90",
                build_line_segment({0, 0}, {2, 0}),
                build_line_segment({2, 0}, {2, -2}),
                1.0,
                true,
                {
                    build_line_segment({0, 0}, {1, 0}),
                    build_circular_arc({1, 0}, {2, -1}, {1, -1}, ShapeElementOrientation::Clockwise),
                    build_line_segment({2, -1}, {2, -2}),
                },
            }, {  // 90° CCW turn with tangent point at segment start: prev segment
                  // is exactly r long, so the trimmed prev has zero length and is
                  // omitted.  T1=(0,0)=element_prev.start, T2=(1,1).
                "CCWTurnTangentAtStart",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {1, 2}),
                1.0,
                true,
                {
                    build_circular_arc({0, 0}, {1, 1}, {0, 1}, ShapeElementOrientation::Anticlockwise),
                    build_line_segment({1, 1}, {1, 2}),
                },
            }, {  // Infeasible: element_prev is a circular arc, not a line segment.
                "InfeasibleArcPrev",
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 3}),
                1.0,
                false,
                {},
            }, {  // Infeasible: collinear segments, no corner to round.
                "InfeasibleCollinear",
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {3, 0}),
                1.0,
                false,
                {},
            }, {  // Infeasible: radius too large (tangent_length = 2 > len_prev = 0.5).
                "InfeasibleRadiusTooLarge",
                build_line_segment({0, 0}, {0.5, 0}),
                build_line_segment({0.5, 0}, {0.5, 2}),
                2.0,
                false,
                {},
            },
        }),
        [](const testing::TestParamInfo<TryRoundCornerTest::ParamType>& info) {
            return info.param.name;
        });


struct SimplifyParametersTestParams
{
    std::string name;
    Shape shape;
    AreaDbl maximum_approximation_area;
    ElementPos minimum_number_of_vertices;
    ElementPos minimum_average_number_of_elements;
    bool expected_invalid_argument;
    ElementPos expected_number_of_elements;
};

void PrintTo(const SimplifyParametersTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "maximum_approximation_area " << params.maximum_approximation_area << "\n";
    *os << "minimum_number_of_vertices " << params.minimum_number_of_vertices << "\n";
    *os << "minimum_average_number_of_elements " << params.minimum_average_number_of_elements << "\n";
}

class SimplifyParametersTest: public testing::TestWithParam<SimplifyParametersTestParams> { };

TEST_P(SimplifyParametersTest, SimplifyParameters)
{
    SimplifyParametersTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    if (test_params.expected_invalid_argument) {
        EXPECT_THROW(
                simplify(
                    {{{test_params.shape}, 1}},
                    test_params.maximum_approximation_area,
                    test_params.minimum_number_of_vertices,
                    test_params.minimum_average_number_of_elements),
                std::invalid_argument);
        return;
    }
    std::vector<ShapeWithHoles> output = simplify(
            {{{test_params.shape}, 1}},
            test_params.maximum_approximation_area,
            test_params.minimum_number_of_vertices,
            test_params.minimum_average_number_of_elements);
    ASSERT_EQ(output.size(), 1);
    std::cout << "output " << output.front().to_string(1) << std::endl;
    EXPECT_EQ((ElementPos)output.front().shape.elements.size(), test_params.expected_number_of_elements);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        SimplifyParametersTest,
        testing::ValuesIn(std::vector<SimplifyParametersTestParams>{
            {
                // The shape has less than 8 elements: it is not simplified
                // with the default minimum average number of elements.
                "NotchDefaultMinimumAverageNumberOfElements",
                build_shape({{0, 0}, {10, 0}, {10, 10}, {6, 10}, {5, 9}, {4, 10}, {0, 10}}),
                1,
                4,
                8,
                false,
                7,
            }, {
                // Without the minimum average number of elements, the notch
                // is filled.
                "NotchNoMinimumAverageNumberOfElements",
                build_shape({{0, 0}, {10, 0}, {10, 10}, {6, 10}, {5, 9}, {4, 10}, {0, 10}}),
                1,
                4,
                0,
                false,
                4,
            }, {
                "MinimumNumberOfVerticesTooSmall",
                build_shape({{0, 0}, {10, 0}, {10, 10}, {6, 10}, {5, 9}, {4, 10}, {0, 10}}),
                1,
                2,
                0,
                true,
                0,
            },
        }),
        [](const testing::TestParamInfo<SimplifyParametersTest::ParamType>& info) {
            return info.param.name;
        });


struct SimplifyInvalidInputTestParams
{
    std::string name;
    std::vector<SimplifyInputShape> shapes;
};

void PrintTo(const SimplifyInvalidInputTestParams& params, std::ostream* os)
{
    *os << "shapes (" << params.shapes.size() << ")\n";
    for (const SimplifyInputShape& shape: params.shapes)
        *os << "- " << shape.shape.to_string(2) << "\n";
}

class SimplifyInvalidInputTest: public testing::TestWithParam<SimplifyInvalidInputTestParams> { };

TEST_P(SimplifyInvalidInputTest, SimplifyInvalidInput)
{
    SimplifyInvalidInputTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    EXPECT_THROW(simplify(test_params.shapes, 10), std::invalid_argument);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        SimplifyInvalidInputTest,
        testing::ValuesIn(std::vector<SimplifyInvalidInputTestParams>{
            {
                "CircularArcInShape",
                {{{build_shape({{0, 0}, {20, 0}, {20, 20}, {10, 20}, {10, 10, 1}, {0, 20}}), {}}, 1}},
            }, {
                "CircularArcInHole",
                {{{build_rectangle(0, 20, 0, 20), {build_shape({{4, 4}, {7, 4}, {7, 5}, {6, 5}, {5, 5, 1}, {4, 5}})}}, 1}},
            },
        }),
        [](const testing::TestParamInfo<SimplifyInvalidInputTest::ParamType>& info) {
            return info.param.name;
        });


struct SimplifyTestParams
{
    std::string name;
    std::vector<SimplifyInputShape> shapes;
    AreaDbl maximum_approximation_area;
    ElementPos minimum_number_of_vertices = 4;
    ElementPos minimum_average_number_of_elements = 8;
    // Number of elements of the outline of each simplified shape. Not
    // checked if empty.
    std::vector<ElementPos> expected_numbers_of_elements;


    /**
     * Read the inputs written by simplify_export_inputs, and optionally the
     * "expected_numbers_of_elements".
     */
    static SimplifyTestParams read_json(
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
        SimplifyTestParams test_params;
        test_params.name = fs::path(file_path).stem().string();
        for (auto& json_shape: json["shapes"]) {
            SimplifyInputShape shape;
            shape.shape = ShapeWithHoles::from_json(json_shape["shape"]);
            shape.copies = json_shape["copies"];
            test_params.shapes.push_back(shape);
        }
        test_params.maximum_approximation_area = json["maximum_approximation_area"];
        test_params.minimum_number_of_vertices = json["minimum_number_of_vertices"];
        test_params.minimum_average_number_of_elements = json["minimum_average_number_of_elements"];
        if (json.contains("expected_numbers_of_elements")) {
            for (auto& json_number: json["expected_numbers_of_elements"])
                test_params.expected_numbers_of_elements.push_back(json_number);
        }
        return test_params;
    }
};

void PrintTo(const SimplifyTestParams& params, std::ostream* os)
{
    *os << "shapes (" << params.shapes.size() << ")\n";
    for (const SimplifyInputShape& shape: params.shapes)
        *os << "- copies " << shape.copies << " " << shape.shape.to_string(1) << "\n";
    *os << "maximum_approximation_area " << params.maximum_approximation_area << "\n";
    *os << "minimum_number_of_vertices " << params.minimum_number_of_vertices << "\n";
    *os << "minimum_average_number_of_elements " << params.minimum_average_number_of_elements << "\n";
}

class SimplifyTest: public testing::TestWithParam<SimplifyTestParams> { };

TEST_P(SimplifyTest, Simplify)
{
    SimplifyTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    std::vector<ShapeWithHoles> output = simplify(
            test_params.shapes,
            test_params.maximum_approximation_area,
            test_params.minimum_number_of_vertices,
            test_params.minimum_average_number_of_elements);
    ASSERT_EQ(output.size(), test_params.shapes.size());

    // Each simplified shape contains its shape, and the areas added, counted
    // once for each copy, sum up to at most the maximum approximation area.
    AreaDbl approximation_area = 0.0;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)output.size();
            ++shape_pos) {
        const SimplifyInputShape& input_shape = test_params.shapes[shape_pos];
        const ShapeWithHoles& output_shape = output[shape_pos];
        std::cout << "output " << shape_pos << " " << output_shape.to_string(1) << std::endl;
        if (!test_params.expected_numbers_of_elements.empty()) {
            EXPECT_EQ(
                    (ElementPos)output_shape.shape.elements.size(),
                    test_params.expected_numbers_of_elements[shape_pos]);
        }
        AreaDbl area_outside = 0.0;
        for (const ShapeWithHoles& part: compute_difference(input_shape.shape, output_shape).shapes_with_holes)
            area_outside += part.compute_area();
        EXPECT_TRUE(equal(area_outside, 0.0)) << "shape " << shape_pos;
        approximation_area += input_shape.copies
            * (output_shape.compute_area() - input_shape.shape.compute_area());
    }
    EXPECT_FALSE(strictly_greater(
                approximation_area,
                test_params.maximum_approximation_area));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        SimplifyTest,
        testing::ValuesIn(std::vector<SimplifyTestParams>{
            {
                // Notches of area 1 are filled within the maximum
                // approximation area (simplify stops once the shape has at
                // most 8 elements).
                "Notches",
                {{{build_shape({
                        {0, 0}, {20, 0}, {20, 10},
                        {17, 10}, {16, 9}, {15, 10},
                        {11, 10}, {10, 9}, {9, 10},
                        {5, 10}, {4, 9}, {3, 10},
                        {0, 10}})}, 1}},
                2.5,
                4,
                8,
                {8},
            }, {
                // The vertices (500, 0) and (600, 100.0001) are almost
                // aligned with their neighbors, but removing them would move
                // the boundary inwards by much more than the tolerance along
                // these long elements. Only the vertices exactly aligned with
                // their neighbors are removed.
                "AlmostAlignedVertices",
                {{{build_shape({{0, 0}, {500, 0}, {1000, 0.0004}, {1000, 100}, {800, 100}, {600, 100.0001}, {400, 100}, {200, 100}, {100, 100}, {0, 100}})}, 1}},
                0,
                4,
                8,
                {8},
            },
            SimplifyTestParams::read_json(
                    (fs::path("data") / "tests" / "simplification" / "simplify" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<SimplifyTest::ParamType>& info) {
            return info.param.name;
        });
