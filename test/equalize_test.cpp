#include "shape/equalize.hpp"

#include <gtest/gtest.h>

using namespace shape;


struct EqualizePointsTestParams
{
    std::string name;
    std::vector<Point> points;
    std::vector<Point> expected_points;
};

void PrintTo(const EqualizePointsTestParams& params, std::ostream* os)
{
    *os << "points\n";
    for (const Point& point: params.points)
        *os << "- " << point.to_string() << "\n";
    *os << "expected_points\n";
    for (const Point& point: params.expected_points)
        *os << "- " << point.to_string() << "\n";
}

class EqualizePointsTest: public testing::TestWithParam<EqualizePointsTestParams> { };

TEST_P(EqualizePointsTest, EqualizePoints)
{
    EqualizePointsTestParams test_params = GetParam();
    std::vector<Point> equalized_points = equalize_points(test_params.points);
    for (const Point& point: equalized_points)
        std::cout << point.to_string() << std::endl;

    ASSERT_EQ(equalized_points.size(), test_params.points.size());
    for (ElementPos pos = 0; pos < (ElementPos)equalized_points.size(); ++pos) {
        EXPECT_EQ(equalized_points[pos], test_params.expected_points[pos]);
        // Each point is moved to a point equal to it.
        EXPECT_TRUE(equal(equalized_points[pos], test_params.points[pos]));
    }
    // The equalized points are either identical or not equal.
    for (ElementPos pos_1 = 0; pos_1 < (ElementPos)equalized_points.size(); ++pos_1) {
        for (ElementPos pos_2 = pos_1 + 1; pos_2 < (ElementPos)equalized_points.size(); ++pos_2) {
            const Point& point_1 = equalized_points[pos_1];
            const Point& point_2 = equalized_points[pos_2];
            EXPECT_TRUE(point_1 == point_2 || !equal(point_1, point_2));
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        ,
        EqualizePointsTest,
        testing::ValuesIn(std::vector<EqualizePointsTestParams>{
            {
                "DistinctPoints",
                {{0, 0}, {1, 0}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}},
            }, {
                "DuplicatePoints",
                {{1, 1}, {2, 2}, {1, 1}},
                {{1, 1}, {2, 2}, {1, 1}},
            }, {
                // Consecutive points are equal, but the first and the last
                // ones are 2.7e-6 apart. Moving all of them to the same
                // point would move some of them by more than the tolerance.
                "Chain",
                {{0, 0}, {0.9e-6, 0}, {1.8e-6, 0}, {2.7e-6, 0}},
                {{0.9e-6, 0}, {0.9e-6, 0}, {0.9e-6, 0}, {2.7e-6, 0}},
            }, {
                // The middle point is equal to the 4 others, which are
                // pairwise not equal.
                "Star",
                {{-0.8e-6, 0}, {0, -0.8e-6}, {0, 0}, {0.8e-6, 0}, {0, 0.8e-6}},
                {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            }}),
        [](const testing::TestParamInfo<EqualizePointsTest::ParamType>& info) {
            return info.param.name;
        });
