#include "shape/extract_borders.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct ExtractBordersTestParams
{
    std::string name;
    Shape shape;
    std::vector<Shape> expected_borders;
};

void PrintTo(const ExtractBordersTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_borders\n";
    for (const Shape& border: params.expected_borders)
        *os << "- " << border.to_string(0) << "\n";
}

class ExtractBordersTest: public testing::TestWithParam<ExtractBordersTestParams> { };

TEST_P(ExtractBordersTest, ExtractBorders)
{
    ExtractBordersTestParams test_params = GetParam();
    std::vector<Shape> shape_borders = extract_borders(test_params.shape);
    for (const Shape& border: shape_borders)
        std::cout << border.to_string(0) << std::endl;
    EXPECT_EQ(shape_borders.size(), test_params.expected_borders.size());
    for (const Shape& expected_border: test_params.expected_borders) {
        EXPECT_NE(std::find(
                    shape_borders.begin(),
                    shape_borders.end(),
                    expected_border),
                shape_borders.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ExtractBordersTest,
        testing::ValuesIn(std::vector<ExtractBordersTestParams>{
            {
                "Square",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {},
            }, {
                "Triangle",
                build_shape({{2, 0}, {3, 1}, {0, 1}}),
                {
                    build_shape({{3, 0}, {3, 1}, {2, 0}}),
                    build_shape({{0, 0}, {2, 0}, {0, 1}}),
                },
            }, {
                "FlatTriangle",
                build_shape({{0, 0}, {3, 1}, {0, 1}}),
                {
                    build_shape({{3, 0}, {3, 1}, {0, 0}}),
                },
            }, {
                "LargeTriangle",
                build_shape({{0, 0}, {50, 0}, {30, 30}}),
                {
                    build_shape({{0, 0}, {30, 30}, {0, 30}}),
                    build_shape({{30, 30}, {50, 0}, {50, 30}}),
                },
            }
        }),
        [](const testing::TestParamInfo<ExtractBordersTest::ParamType>& info) {
            return info.param.name;
        });
