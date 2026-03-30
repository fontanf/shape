#include "shape/simplification.hpp"

#include <gtest/gtest.h>

#include <cmath>

using namespace shape;


struct TryExtendToIntersectionTestParams
{
    ShapeElement element_prev;
    ShapeElement element_next;
    bool expected_feasible;
    /** Meaningful only when expected_feasible is true. */
    Point expected_intersection;
};

class TryExtendToIntersectionTest:
    public testing::TestWithParam<TryExtendToIntersectionTestParams> { };

TEST_P(TryExtendToIntersectionTest, TryExtendToIntersection)
{
    TryExtendToIntersectionTestParams test_params = GetParam();
    std::cout << "element_prev " << test_params.element_prev.to_string() << std::endl;
    std::cout << "element_next " << test_params.element_next.to_string() << std::endl;

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
                // prev goes right; next goes up. Extending both meets at (3, 0).
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({3, 1}, {3, 3}),
                true,
                {3, 0},
            }, {  // Line + Line, infeasible: intersection lies behind prev's end.
                // The lines meet at (3, 0), but (3, 0) is behind end (5, 0) of prev.
                build_line_segment({0, 0}, {5, 0}),
                build_line_segment({3, 1}, {3, 3}),
                false,
                {0, 0},
            }, {  // Line + Line, infeasible: intersection lies inside next.
                // The lines meet at (3, 0), which is strictly between next's endpoints.
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({3, -1}, {3, 3}),
                false,
                {0, 0},
            }, {  // Line + Line, infeasible: parallel lines never meet.
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({0, 1}, {1, 1}),
                false,
                {0, 0},
            }, {  // CCW Arc + Line, feasible: arc extends from 0° to 60° (r=2),
                  // vertical line starts above arc end; both extend to meet at (0, 2).
                build_circular_arc({2, 0}, {1, sqrt(3.0)}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 3}, {0, 5}),
                true,
                {0, 2},
            }, {  // Line + CCW Arc, feasible: horizontal line ending at (-1.5, 0),
                  // arc starts at (1, 0); extending the line forward and the arc
                  // backward meets the circle at (-1, 0), the candidate closest
                  // to the gap midpoint (-0.25, 0).
                build_line_segment({-3, 0}, {-1.5, 0}),
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                true,
                {-1, 0},
            }, {  // CCW Arc + CCW Arc, feasible: two unit-radius arcs on intersecting
                  // circles; extending both meets at (-sqrt(3)/2, 1/2).
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({0, 2}, {-1, 2}, {0, 1}, ShapeElementOrientation::Anticlockwise),
                true,
                {-sqrt(3.0) / 2, 0.5},
            }, {  // CCW Arc + CCW Arc, infeasible: circles too far apart to intersect.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({6, 0}, {5, 1}, {5, 0}, ShapeElementOrientation::Anticlockwise),
                false,
                {0, 0},
            },
        }));


////////////////////////////////////////////////////////////////////////////////
// try_round_corner
////////////////////////////////////////////////////////////////////////////////

struct TryRoundCornerTestParams
{
    ShapeElement element_prev;
    ShapeElement element_next;
    LengthDbl radius;
    bool expected_feasible;
    /** Only meaningful when expected_feasible is true. */
    std::vector<ShapeElement> expected_elements;
};

class TryRoundCornerTest:
    public testing::TestWithParam<TryRoundCornerTestParams> { };

TEST_P(TryRoundCornerTest, TryRoundCorner)
{
    TryRoundCornerTestParams test_params = GetParam();
    std::cout << "element_prev " << test_params.element_prev.to_string() << std::endl;
    std::cout << "element_next " << test_params.element_next.to_string() << std::endl;
    std::cout << "radius " << test_params.radius << std::endl;

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
    ShapeElement element_prev;
    ShapeElement element_next;
    bool expected_feasible;
    /** Only meaningful when expected_feasible is true. */
    ShapeElement expected_new_element_prev;
    ShapeElement expected_new_element_next;
};

class TrySmoothArcToLineTest:
    public testing::TestWithParam<TrySmoothArcToLineTestParams> { };

TEST_P(TrySmoothArcToLineTest, TrySmoothArcToLine)
{
    TrySmoothArcToLineTestParams test_params = GetParam();
    std::cout << "element_prev " << test_params.element_prev.to_string() << std::endl;
    std::cout << "element_next " << test_params.element_next.to_string() << std::endl;

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
               // Arc from (1,0) to (0,1), center (0,0), r=1 (quarter circle in Q1).
               // Line from (0,1) to (0,2), so element_next.end = (0,2).
               // alpha = acos(1/2) = 60°.
               // tangent_point = center + r * rotate((0,1), -60°) = (sin60°, cos60°) = (√3/2, 0.5).
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 2}),
                true,
                build_circular_arc({1, 0}, {sqrt(3.0) / 2, 0.5}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({sqrt(3.0) / 2, 0.5}, {0, 2}),
            }, {  // CW arc + line, feasible.
               // Arc from (0,1) to (1,0), center (0,0), r=1 (quarter circle CW in Q1).
               // Line from (1,0) to (2,0), so element_next.end = (2,0).
               // alpha = acos(1/2) = 60°.
               // tangent_point = center + r * rotate((1,0), +60°) = (cos60°, sin60°) = (0.5, √3/2).
                build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({1, 0}, {2, 0}),
                true,
                build_circular_arc({0, 1}, {0.5, sqrt(3.0) / 2}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({0.5, sqrt(3.0) / 2}, {2, 0}),
            }, {  // Line + CCW arc, feasible.
               // Line from (0,2) to (0,1); CCW arc from (0,1) to (-1,0), center (0,0).
               // external_point = element_prev.start = (0,2); alpha = acos(1/2) = 60°.
               // tangent_point = center + r * rotate((0,1), +60°) = (-√3/2, 0.5).
                build_line_segment({0, 2}, {0, 1}),
                build_circular_arc({0, 1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                true,
                build_line_segment({0, 2}, {-sqrt(3.0) / 2, 0.5}),
                build_circular_arc({-sqrt(3.0) / 2, 0.5}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
            }, {  // Line + CW arc, feasible.
               // Line from (-1,√3) to (0,1); CW arc from (0,1) to (1,0), center (0,0).
               // external_point = element_prev.start = (-1,√3); distance = 2; alpha = 60°.
               // tangent_point = center + r * rotate((-1/2, √3/2), -60°) = (1/2, √3/2).
                build_line_segment({-1, sqrt(3.0)}, {0, 1}),
                build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                true,
                build_line_segment({-1, sqrt(3.0)}, {0.5, sqrt(3.0) / 2}),
                build_circular_arc({0.5, sqrt(3.0) / 2}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
            }, {  // Infeasible: neither (arc, line) nor (line, arc).
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {1, 2}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            }, {  // Infeasible: element_next.end is inside the circle.
               // Arc from (1,0) to (0,1), center (0,0), r=1.
               // Line endpoint (0, 0.5) is at distance 0.5 < 1 from center.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 0.5}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            }, {  // Infeasible: tangent_point lies on arc.end (not strictly interior).
               // Arc from (1,0) to (√3/2, 0.5), center (0,0), r=1 [0° to 30°].
               // Line endpoint (0,2): the computed tangent_point would be (√3/2, 0.5) = arc.end.
                build_circular_arc({1, 0}, {sqrt(3.0) / 2, 0.5}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({sqrt(3.0) / 2, 0.5}, {0, 2}),
                false,
                build_line_segment({0, 0}, {0, 0}),
                build_line_segment({0, 0}, {0, 0}),
            },
        }));


INSTANTIATE_TEST_SUITE_P(
        Shape,
        TryRoundCornerTest,
        testing::ValuesIn(std::vector<TryRoundCornerTestParams>{
            {  // 90° CCW (left) turn: horizontal then up, r=1.
               // tangent_length = r*tan(45°) = 1.
               // T1=(1,0), T2=(2,1), center=(1,1), arc CCW.
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
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {1, 2}),
                1.0,
                true,
                {
                    build_circular_arc({0, 0}, {1, 1}, {0, 1}, ShapeElementOrientation::Anticlockwise),
                    build_line_segment({1, 1}, {1, 2}),
                },
            }, {  // Infeasible: element_prev is a circular arc, not a line segment.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 1}, {0, 3}),
                1.0,
                false,
                {},
            }, {  // Infeasible: collinear segments, no corner to round.
                build_line_segment({0, 0}, {1, 0}),
                build_line_segment({1, 0}, {3, 0}),
                1.0,
                false,
                {},
            }, {  // Infeasible: radius too large (tangent_length = 2 > len_prev = 0.5).
                build_line_segment({0, 0}, {0.5, 0}),
                build_line_segment({0.5, 0}, {0.5, 2}),
                2.0,
                false,
                {},
            },
        }));
