#include "shape/trapezoid.hpp"

#include <gtest/gtest.h>

using namespace shape;

TEST(Trapezoid, Intersection1)
{
    GeneralizedTrapezoid trapezoid_1(-25, 175, 375, 775, 575, 975);
    GeneralizedTrapezoid trapezoid_2(0, 200, 0, 400, 200, 600);
    EXPECT_EQ(trapezoid_1.compute_right_shift(trapezoid_2), 0);
    EXPECT_EQ(trapezoid_1.compute_right_shift_if_intersects(trapezoid_2), 0);
}

TEST(Trapezoid, Intersection2)
{
    GeneralizedTrapezoid trapezoid_1(-25, 175, 350, 750, 550, 950);
    GeneralizedTrapezoid trapezoid_2(0, 200, 0, 400, 200, 600);
    EXPECT_EQ(trapezoid_1.compute_right_shift(trapezoid_2), 25);
    EXPECT_EQ(trapezoid_1.compute_right_shift_if_intersects(trapezoid_2), 25);
}

TEST(Trapezoid, Intersection3)
{
    GeneralizedTrapezoid trapezoid_1(-25, 175, -425, -25, -225, 175);
    GeneralizedTrapezoid trapezoid_2(0, 200, 0, 400, 200, 600);
    EXPECT_EQ(trapezoid_1.compute_right_shift(trapezoid_2), 800);
    EXPECT_EQ(trapezoid_1.compute_right_shift_if_intersects(trapezoid_2), 0);
}


struct GeneralizedTrapezoidComputeTopRightShiftTestParams
{
    GeneralizedTrapezoid trapezoid_1;
    GeneralizedTrapezoid trapezoid_2;
    double a;
    LengthDbl expected_shift;
};

void PrintTo(const GeneralizedTrapezoidComputeTopRightShiftTestParams& params, std::ostream* os)
{
    *os << "trapezoid_1 " << params.trapezoid_1 << "\n";
    *os << "trapezoid_2 " << params.trapezoid_2 << "\n";
    *os << "a " << params.a << "\n";
    *os << "expected_shift " << params.expected_shift << "\n";
}

class GeneralizedTrapezoidComputeTopRightShiftTest: public testing::TestWithParam<GeneralizedTrapezoidComputeTopRightShiftTestParams> { };

TEST_P(GeneralizedTrapezoidComputeTopRightShiftTest, GeneralizedTrapezoidComputeTopRightShift)
{
    GeneralizedTrapezoidComputeTopRightShiftTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    LengthDbl shift = test_params.trapezoid_1.compute_top_right_shift(
            test_params.trapezoid_2,
            test_params.a);
    std::cout << "shift " << shift << std::endl;
    EXPECT_TRUE(equal(shift, test_params.expected_shift));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        GeneralizedTrapezoidComputeTopRightShiftTest,
        testing::ValuesIn(std::vector<GeneralizedTrapezoidComputeTopRightShiftTestParams>{
            {  // 0. Rectangles.
                GeneralizedTrapezoid(0, 2, 0, 1, 0, 1),
                GeneralizedTrapezoid(0, 2, 3, 4, 3, 4),
                1.0,
                2,
            }, {  // 1. Direction parallel to the sides of the second trapezoid.
                GeneralizedTrapezoid(0, 1, -10, -9, -10, -9),
                GeneralizedTrapezoid(0, 1, 100, 101, 101, 102),
                1.0,
                0,
            }, {  // 2. Direction parallel to the sides of the first trapezoid.
                GeneralizedTrapezoid(0, 1, 100, 101, 101, 102),
                GeneralizedTrapezoid(0, 1, 200, 201, 200, 201),
                1.0,
                0,
            },
        }),
        [](const testing::TestParamInfo<GeneralizedTrapezoidComputeTopRightShiftTest::ParamType>& info) {
            return std::to_string(info.index);
        });


struct GeneralizedTrapezoidExtendLeftTestParams
{
    std::string name;
    GeneralizedTrapezoid trapezoid;
    LengthDbl l;
    // Expected trapezoid, built directly with its left side at x = l.
    GeneralizedTrapezoid expected_trapezoid;
};

void PrintTo(const GeneralizedTrapezoidExtendLeftTestParams& params, std::ostream* os)
{
    *os << "trapezoid " << params.trapezoid << "\n";
    *os << "l " << params.l << "\n";
    *os << "expected_trapezoid " << params.expected_trapezoid << "\n";
}

class GeneralizedTrapezoidExtendLeftTest: public testing::TestWithParam<GeneralizedTrapezoidExtendLeftTestParams> { };

TEST_P(GeneralizedTrapezoidExtendLeftTest, GeneralizedTrapezoidExtendLeft)
{
    GeneralizedTrapezoidExtendLeftTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    GeneralizedTrapezoid trapezoid = test_params.trapezoid;
    trapezoid.extend_left(test_params.l);
    std::cout << "trapezoid " << trapezoid << std::endl;

    // The extended trapezoid is the same as the one built directly, including
    // its computed attributes.
    const GeneralizedTrapezoid& expected = test_params.expected_trapezoid;
    EXPECT_EQ(trapezoid, expected);
    EXPECT_TRUE(equal(trapezoid.width_bottom(), expected.width_bottom()));
    EXPECT_TRUE(equal(trapezoid.width_top(), expected.width_top()));
    EXPECT_TRUE(equal(trapezoid.height(), expected.height()));
    EXPECT_TRUE(equal(trapezoid.area(), expected.area()));
    EXPECT_TRUE(equal(trapezoid.x_min(), expected.x_min()));
    EXPECT_TRUE(equal(trapezoid.x_max(), expected.x_max()));
    EXPECT_TRUE(equal(trapezoid.a_left(), expected.a_left()));
    EXPECT_EQ(trapezoid.left_side_increasing_not_vertical(), expected.left_side_increasing_not_vertical());
    EXPECT_EQ(trapezoid.left_side_decreasing_not_vertical(), expected.left_side_decreasing_not_vertical());
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        GeneralizedTrapezoidExtendLeftTest,
        testing::ValuesIn(std::vector<GeneralizedTrapezoidExtendLeftTestParams>{
            {
                "LeftSideGoingRight",
                GeneralizedTrapezoid(0, 2, 2, 6, 3, 5),
                0,
                GeneralizedTrapezoid(0, 2, 0, 6, 0, 5),
            }, {
                "LeftSideGoingLeft",
                GeneralizedTrapezoid(0, 2, 3, 6, 1, 5),
                -1,
                GeneralizedTrapezoid(0, 2, -1, 6, -1, 5),
            }, {
                "VerticalLeftSide",
                GeneralizedTrapezoid(1, 3, 2, 4, 2, 4),
                0,
                GeneralizedTrapezoid(1, 3, 0, 4, 0, 4),
            }, {
                "Triangle",
                GeneralizedTrapezoid(0, 2, 2, 6, 4, 4),
                1,
                GeneralizedTrapezoid(0, 2, 1, 6, 1, 4),
            },
        }),
        [](const testing::TestParamInfo<GeneralizedTrapezoidExtendLeftTest::ParamType>& info) {
            return info.param.name;
        });
