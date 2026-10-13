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
