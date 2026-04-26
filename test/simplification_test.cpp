#include "shape/simplification.hpp"

#include <gtest/gtest.h>

#include <cmath>

using namespace shape;


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
