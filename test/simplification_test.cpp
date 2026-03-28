#include "shape/simplification.hpp"

#include <gtest/gtest.h>

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
