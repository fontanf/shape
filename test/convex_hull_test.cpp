#include "shape/convex_hull.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct ConvexHullTestParams
{
    Shape shape;
    bool expect_throw = false;
    // Only meaningful if !expect_throw.
    Shape expected_output;
};

void PrintTo(const ConvexHullTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expect_throw " << params.expect_throw << "\n";
    if (!params.expect_throw)
        *os << "expected_output " << params.expected_output.to_string(0) << "\n";
}

class ConvexHullTest: public testing::TestWithParam<ConvexHullTestParams> { };

TEST_P(ConvexHullTest, ConvexHull)
{
    ConvexHullTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    if (test_params.expect_throw) {
        EXPECT_THROW(shape::convex_hull(test_params.shape), std::invalid_argument);
        return;
    }

    Shape output = shape::convex_hull(test_params.shape);
    std::cout << "output " << output.to_string(0) << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ConvexHullTest,
        testing::ValuesIn(std::vector<ConvexHullTestParams>{
            {
                // Already-convex triangle: hull is the shape itself.
                build_shape({{0, 0}, {3, 0}, {1, 3}}),
                false,
                build_shape({{0, 0}, {3, 0}, {1, 3}}),
            }, {
                // Concave pentagon: reflex angle at (2, 1) is dropped from
                // the hull.
                build_shape({{0, 0}, {4, 0}, {2, 1}, {4, 4}, {0, 4}}),
                false,
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
            }, {
                // convex_hull only looks at each element's start point, so a
                // shape with a circular arc bulging past the chord between
                // its endpoints used to silently produce a hull that
                // doesn't actually contain the input shape (found via
                // fuzzing). Convex hulls of arc-containing shapes aren't
                // supported yet; reject them clearly instead.
                build_shape({{0, 0}, {1, 0}, {0, 0, 1}, {0, 1}}),
                true,
                {},
            }, {
                // (-1, 1e-9) is collinear, within the tolerance, with the
                // lowest point (0, 0) and (10, 0), but on the left of
                // (0, 0). With (0, 0) as the pivot, it was sorted as if it
                // were on its right, and, depending on the implementation of
                // std::sort, dropped from the hull. The pivot is now the
                // leftmost of the lowest points within the tolerance (and
                // (0, 0) is kept: it is, exactly, a left turn).
                build_shape({{0, 0}, {10, 0}, {10, 10}, {-1, 10}, {-1, 1e-9}}),
                false,
                build_shape({{-1, 1e-9}, {0, 0}, {10, 0}, {10, 10}, {-1, 10}}),
            },
        }),
        [](const testing::TestParamInfo<ConvexHullTest::ParamType>& info) {
            return std::to_string(info.index);
        });
