#include "shape/supports.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct ComputeShapeSupportsTestParams
{
    ShapeWithHoles shape;
    std::vector<Shape> expected_supporting_parts;
    std::vector<Shape> expected_supported_parts;
    std::string name;
};

void PrintTo(const ComputeShapeSupportsTestParams& params, std::ostream* os)
{
    *os << "shape:\n";
    *os << "- " << params.shape.to_string(0) << "\n";
    *os << "expected_supporting_parts:\n";
    for (const auto& support: params.expected_supporting_parts) {
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            const ShapeElement& element = support.elements[element_pos];
            if (element_pos == 0) {
                *os << "- " << element.to_string() << "\n";
            } else {
                *os << "  " << element.to_string() << "\n";
            }
        }
    }
    *os << "expected_supported_parts:\n";
    for (const auto& support: params.expected_supported_parts) {
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            const ShapeElement& element = support.elements[element_pos];
            if (element_pos == 0) {
                *os << "- " << element.to_string() << "\n";
            } else {
                *os << "  " << element.to_string() << "\n";
            }
        }
    }
}

class IrregularComputeShapeSupportsTest: public testing::TestWithParam<ComputeShapeSupportsTestParams> { };

TEST_P(IrregularComputeShapeSupportsTest, ComputeShapeSupports)
{
    ComputeShapeSupportsTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    ShapeSupports supports = compute_shape_supports(test_params.shape);
    std::cout << "supporting_parts:" << std::endl;
    for (const auto& support: supports.supporting_parts) {
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            const ShapeElement& element = support.elements[element_pos];
            if (element_pos == 0) {
                std::cout << "- " << element.to_string() << std::endl;
            } else {
                std::cout << "  " << element.to_string() << std::endl;
            }
        }
    }
    std::cout << "supported_parts:" << std::endl;
    for (const auto& support: supports.supported_parts) {
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            const ShapeElement& element = support.elements[element_pos];
            if (element_pos == 0) {
                std::cout << "- " << element.to_string() << std::endl;
            } else {
                std::cout << "  " << element.to_string() << std::endl;
            }
        }
    }

    ASSERT_EQ(supports.supporting_parts.size(), test_params.expected_supporting_parts.size());
    ASSERT_EQ(supports.supported_parts.size(), test_params.expected_supported_parts.size());
    for (ElementPos pos = 0;
            pos < (ElementPos)supports.supporting_parts.size();
            ++pos) {
        Shape support = supports.supporting_parts[pos];
        Shape expected_support = test_params.expected_supporting_parts[pos];
        ASSERT_EQ(supports.supported_parts.size(), test_params.expected_supported_parts.size());
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            EXPECT_EQ(support.elements[element_pos], expected_support.elements[element_pos]);
        }
    }
    for (ElementPos pos = 0;
            pos < (ElementPos)supports.supported_parts.size();
            ++pos) {
        Shape support = supports.supported_parts[pos];
        Shape expected_support = test_params.expected_supported_parts[pos];
        ASSERT_EQ(supports.supported_parts.size(), test_params.expected_supported_parts.size());
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)support.elements.size();
                ++element_pos) {
            EXPECT_EQ(support.elements[element_pos], expected_support.elements[element_pos]);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        Irregular,
        IrregularComputeShapeSupportsTest,
        testing::ValuesIn(std::vector<ComputeShapeSupportsTestParams>{
            {
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}), {}},
                {build_shape({{0, 1}, {1, 1}}, true)},
                {build_shape({{1, 0}, {0, 0}}, true)},
                "Square",
            }, {
                {build_shape({{0, 0}, {1, 0}, {2, 1}, {1, 1}}), {}},
                {build_shape({{0, 0}, {1, 1}, {2, 1}}, true)},
                {build_shape({{2, 1}, {1, 0}, {0, 0}}, true)},
                "Parallelogram",
            }, {
                {build_shape({{0, 0}, {7, 0}, {6, 2}, {5, 2}, {4, 1}, {3, 1}, {2, 2}, {1, 2}}), {}},
                {
                    build_shape({{4, 1}, {5, 2}, {6, 2}, {7, 0}}, true),
                    build_shape({{3, 1}, {4, 1}}, true),
                    build_shape({{0, 0}, {1, 2}, {2, 2}, {3, 1}}, true),
                },
                {build_shape({{7, 0}, {0, 0}}, true)},
                "MShape",
            }, {
                {build_shape({{0, 0}, {2, 0}, {2, 1}, {1, 1}, {1, 2}, {2, 2}, {2, 3}, {0, 3}}), {}},
                {
                    build_shape({{1, 1}, {2, 1}}, true),
                    build_shape({{0, 3}, {2, 3}}, true),
                }, {
                    build_shape({{2, 0}, {0, 0}}, true),
                    build_shape({{2, 2}, {1, 2}}, true),
                },
                "UShape",
            }, {
                {
                    build_shape({{0, 0}, {3, 0}, {3, 3}, {0, 3}}),
                    {build_shape({{1, 1}, {2, 1}, {2, 2}, {1, 2}})}
                }, {
                    build_shape({{0, 3}, {3, 3}}, true),
                    build_shape({{1, 1}, {2, 1}}, true),
                }, {
                    build_shape({{3, 0}, {0, 0}}, true),
                    build_shape({{2, 2}, {1, 2}}, true),
                },
                "SquareWithHole",
            }, {
                {build_shape({{15, 0}, {30, 10}, {0, 10}}), {}},
                {
                    build_shape({{0, 10}, {30, 10}}, true),
                }, {
                    build_shape({{30, 10}, {15, 0}, {0, 10}}, true),
                },
                "Triangle",
            }, {
                {
                    build_shape({{-10, -10}, {50, -10}, {50, 50}, {-10, 50}}),
                    {build_shape({{0, 0}, {40, 20}, {40, 40}, {10, 40}})},
                }, {
                    build_shape({{-10, 50}, {50, 50}}, true),
                    build_shape({{0, 0}, {40, 20}}, true),
                }, {
                    build_shape({{50, -10}, {-10, -10}}, true),
                    build_shape({{40, 40}, {10, 40}, {0, 0}}, true),
                },
                "SquareWithTiltedHole",
            }}),
        [](const testing::TestParamInfo<IrregularComputeShapeSupportsTest::ParamType>& info) {
            return info.param.name;
        });
