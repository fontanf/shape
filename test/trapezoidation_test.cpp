#include "shape/trapezoidation.hpp"

#include <gtest/gtest.h>

#include <fstream>

using namespace shape;

struct TrapezoidationTestParams
{
    ShapeWithHoles shape;
    std::vector<GeneralizedTrapezoid> expected_result;

    template <class basic_json>
    static TrapezoidationTestParams from_json(
            basic_json& json_item)
    {
        TrapezoidationTestParams test_params;
        test_params.shape = ShapeWithHoles::from_json(json_item["shape"]);
        for (auto& json_trapezoid: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(GeneralizedTrapezoid::from_json(json_trapezoid.value()));
        return test_params;
    }

    static TrapezoidationTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::TrapezoidationTestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class TrapezoidationTest: public testing::TestWithParam<TrapezoidationTestParams> { };

TEST_P(TrapezoidationTest, Trapezoidation)
{
    TrapezoidationTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const GeneralizedTrapezoid& trapezoid: test_params.expected_result)
        std::cout << "- " << trapezoid << std::endl;

    std::vector<GeneralizedTrapezoid> result = trapezoidation(
            test_params.shape);
    std::cout << "result" << std::endl;
    for (const GeneralizedTrapezoid& trapezoid: result)
        std::cout << "- " << trapezoid << std::endl;

    ASSERT_EQ(result.size(), test_params.expected_result.size());
    for (const GeneralizedTrapezoid& trapezoid: test_params.expected_result) {
        EXPECT_NE(std::find(result.begin(), result.end(), trapezoid), result.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        TrapezoidationTest,
        testing::ValuesIn(std::vector<TrapezoidationTestParams>{
            {  // Triangle1
                {build_shape({{0, 0}, {3, 0}, {1, 3}})},
                {GeneralizedTrapezoid(0, 3, 0, 3, 1, 1)},
            }, {  // Triangle2
                {build_shape({{2, 0}, {3, 3}, {0, 3}})},
                {GeneralizedTrapezoid(0, 3, 2, 2, 0, 3)},
            }, {  // Square
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                {GeneralizedTrapezoid(0, 1, 0, 1, 0, 1)},
            }, {  // Trapezoid1
                {build_shape({{0, 0}, {3, 0}, {2, 3}, {1, 3}})},
                {GeneralizedTrapezoid(0, 3, 0, 3, 1, 2)},
            }, {  // Trapezoid2
                {build_shape({{1, 0}, {2, 0}, {3, 3}, {0, 3}})},
                {GeneralizedTrapezoid(0, 3, 1, 2, 0, 3)},
            }, {  // Triangle3
                {build_shape({{4, 0}, {1, 3}, {0, 1}})},
                {
                    GeneralizedTrapezoid(1, 3, 0, 3, 1, 1),
                    GeneralizedTrapezoid(0, 1, 4, 4, 0, 3),
                },
            }, {  // Trapezoid3
                {build_shape({{5, 0}, {2, 3}, {1, 3}, {0, 1}})},
                {
                    GeneralizedTrapezoid(1, 3, 0, 4, 1, 2),
                    GeneralizedTrapezoid(0, 1, 5, 5, 0, 4),
                },
            }, {  // DoubleTrapezoid1
                {build_shape({{0, 0}, {4, 0}, {2, 2}, {4, 4}, {0, 4}, {1, 2}})},
                {
                    GeneralizedTrapezoid(2, 4, 1, 2, 0, 4),
                    GeneralizedTrapezoid(0, 2, 0, 4, 1, 2),
                },
            }, {  // DoubleTrapezoid2
                {build_shape({{1, 0}, {2, 0}, {4, 2}, {2, 4}, {1, 4}, {0, 2}})},
                {
                    GeneralizedTrapezoid(2, 4, 0, 4, 1, 2),
                    GeneralizedTrapezoid(0, 2, 1, 2, 0, 4),
                },
            }, {  // ReversedH
                {build_shape({
                    {0, 0}, {3, 0}, {3, 1}, {2, 1}, {2, 2}, {3, 2},
                    {3, 3}, {0, 3}, {0, 2}, {1, 2}, {1, 1}, {0, 1}})},
                {
                    GeneralizedTrapezoid(2, 3, 0, 3, 0, 3),
                    GeneralizedTrapezoid(1, 2, 1, 2, 1, 2),
                    GeneralizedTrapezoid(0, 1, 0, 3, 0, 3),
                },
            }, {  // Cross
                {build_shape({
                    {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}})},
                {
                    GeneralizedTrapezoid(2, 3, 1, 2, 1, 2),
                    GeneralizedTrapezoid(1, 2, 0, 3, 0, 3),
                    GeneralizedTrapezoid(0, 1, 1, 2, 1, 2),
                },
            }, {  // U
                {build_shape({
                    {0, 0}, {3, 0}, {3, 3}, {2, 3},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
                {
                    GeneralizedTrapezoid(1, 3, 0, 1, 0, 1),
                    GeneralizedTrapezoid(1, 3, 2, 3, 2, 3),
                    GeneralizedTrapezoid(0, 1, 0, 3, 0, 3),
                },
            }, {  // W
                {build_shape({
                    {0, 0}, {5, 0}, {5, 3}, {4, 3},
                    {4, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
                {
                    GeneralizedTrapezoid(1, 3, 0, 1, 0, 1),
                    GeneralizedTrapezoid(1, 3, 4, 5, 4, 5),
                    GeneralizedTrapezoid(1, 2, 2, 3, 2, 3),
                    GeneralizedTrapezoid(0, 1, 0, 5, 0, 5),
                },
            }, {  // Shape1
                {build_shape({
                    {185.355, 114.645},
                    {150.0, 79.289},
                    {79.289, 150.0},
                    {114.645, 185.355},
                    {0.0, 300.0},
                    {0.0, 0.0},
                    {300.0, 0.0}})},
                {
                    GeneralizedTrapezoid(185.355, 300, 0, 114.645, 0, 0),
                    GeneralizedTrapezoid(150, 185.355, 0, 79.289, 0, 114.645),
                    GeneralizedTrapezoid(79.289, 150, 0, 150, 0, 79.289),
                    GeneralizedTrapezoid(79.289, 114.645, 150, 220.711, 185.355, 185.355),
                    GeneralizedTrapezoid(0, 79.289, 0, 300, 0, 220.711),
                },
            }, {  // SquareRing
                {
                    build_shape({{0, 0}, {3, 0}, {3, 3}, {0, 3}}),
                    {build_shape({{1, 1}, {2, 1}, {2, 2}, {1, 2}})}
                }, {
                    GeneralizedTrapezoid(2, 3, 0, 3, 0, 3),
                    GeneralizedTrapezoid(1, 2, 0, 1, 0, 1),
                    GeneralizedTrapezoid(1, 2, 2, 3, 2, 3),
                    GeneralizedTrapezoid(0, 1, 0, 3, 0, 3),
                },
            }, {  // DiamondHole
                {
                    build_shape({{1, 0}, {3, 0}, {4, 1}, {4, 3}, {3, 4}, {1, 4}, {0, 3}, {0, 1}}),
                    {build_shape({{2, 1}, {3, 2}, {2, 3}, {1, 2}})}
                }, {
                    GeneralizedTrapezoid(3, 4, 0, 4, 1, 3),
                    GeneralizedTrapezoid(2, 3, 0, 1, 0, 2),
                    GeneralizedTrapezoid(2, 3, 3, 4, 2, 4),
                    GeneralizedTrapezoid(1, 2, 0, 2, 0, 1),
                    GeneralizedTrapezoid(1, 2, 2, 4, 3, 4),
                    GeneralizedTrapezoid(0, 1, 1, 3, 0, 4),
                },
            }, {  // ButterflyHole
                {
                    build_shape({{1, 0}, {3, 0}, {4, 1}, {4, 3}, {3, 4}, {1, 4}, {0, 3}, {0, 1}}),
                    {build_shape({{1, 1}, {2, 1.5}, {3, 1}, {3, 3}, {2, 2.5}, {1, 3}})}
                }, {
                    GeneralizedTrapezoid(3, 4, 0, 4, 1, 3),
                    GeneralizedTrapezoid(2.5, 3, 2, 2, 1, 3),
                    GeneralizedTrapezoid(1, 3, 0, 1, 0, 1),
                    GeneralizedTrapezoid(1, 1.5, 1, 3, 2, 2),
                    GeneralizedTrapezoid(1, 3, 3, 4, 3, 4),
                    GeneralizedTrapezoid(0, 1, 1, 3, 0, 4),
                },
            }, {
                {
                    build_shape({
                            {0, 0}, {5, 0}, {5, 2}, {3, 2}, {4, 1},
                            {1, 1}, {2, 2}, {0, 2}}),
                }, {
                    GeneralizedTrapezoid(1, 2, 0, 1, 0, 2),
                    GeneralizedTrapezoid(1, 2, 4, 5, 3, 5),
                    GeneralizedTrapezoid(0, 1, 0, 5, 0, 5),
                },
            }, {  // Touching hole top flat
                {
                    build_shape({{0, 0}, {12, 0}, {12, 12}, {0, 12}}),
                    {build_shape({{3, 6}, {9, 6}, {6, 12}})}
                }, {
                    GeneralizedTrapezoid(6, 12, 0, 3, 0, 6),
                    GeneralizedTrapezoid(6, 12, 9, 12, 6, 12),
                    GeneralizedTrapezoid(0, 6, 0, 12, 0, 12),
                },
            }, {  // Touching hole top increasing
                {
                    build_shape({{0, 0}, {12, 0}, {12, 13}, {0, 11}}),
                    {build_shape({{3, 9}, {9, 9}, {6, 12}})}
                }, {
                    GeneralizedTrapezoid(12, 13, 6, 12, 12, 12),
                    GeneralizedTrapezoid(11, 12, 0, 5, 6, 6),
                    GeneralizedTrapezoid(9, 11, 0, 3, 0, 5),
                    GeneralizedTrapezoid(9, 12, 9, 12, 6, 12),
                    GeneralizedTrapezoid(0, 9, 0, 12, 0, 12),
                },
            }, {  // Touching hole top decreasing
                {
                    build_shape({{0, 0}, {12, 0}, {12, 11}, {0, 13}}),
                    {build_shape({{3, 9}, {9, 9}, {6, 12}})}
                }, {
                    GeneralizedTrapezoid(12, 13, 0, 6, 0, 0),
                    GeneralizedTrapezoid(11, 12, 7, 12, 6, 6),
                    GeneralizedTrapezoid(9, 11, 9, 12, 7, 12),
                    GeneralizedTrapezoid(9, 12, 0, 3, 0, 6),
                    GeneralizedTrapezoid(0, 9, 0, 12, 0, 12),
                },
            }, {  // Touching hole bottom flat
                {
                    build_shape({{0, 0}, {12, 0}, {12, 12}, {0, 12}}),
                    {build_shape({{3, 3}, {6, 0}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(3, 12, 0, 12, 0, 12),
                    GeneralizedTrapezoid(0, 3, 0, 6, 0, 3),
                    GeneralizedTrapezoid(0, 3, 6, 12, 9, 12),
                },
            }, {  // Touching hole bottom increasing
                {
                    build_shape({{0, -1}, {12, 1}, {12, 12}, {0, 12}}),
                    {build_shape({{3, 3}, {6, 0}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(3, 12, 0, 12, 0, 12),
                    GeneralizedTrapezoid(1, 3, 7, 12, 9, 12),
                    GeneralizedTrapezoid(0, 3, 0, 6, 0, 3),
                    GeneralizedTrapezoid(-1, 0, 0, 0, 0, 6),
                    GeneralizedTrapezoid(0, 1, 6, 6, 7, 12),
                },
            }, {  // Touching hole bottom decreasing
                {
                    build_shape({{0, 1}, {12, -1}, {12, 12}, {0, 12}}),
                    {build_shape({{3, 3}, {6, 0}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(3, 12, 0, 12, 0, 12),
                    GeneralizedTrapezoid(1, 3, 0, 5, 0, 3),
                    GeneralizedTrapezoid(0, 1, 6, 6, 0, 5),
                    GeneralizedTrapezoid(0, 3, 6, 12, 9, 12),
                    GeneralizedTrapezoid(-1, 0, 12, 12, 6, 12),
                },
            }, {  // Touching hole left flat
                {
                    build_shape({{0, 0}, {12, 0}, {12, 12}, {0, 12}}),
                    {build_shape({{0, 6}, {3, 3}, {3, 9}})}
                }, {
                    GeneralizedTrapezoid(9, 12, 0, 12, 0, 12),
                    GeneralizedTrapezoid(6, 9, 0, 0, 0, 3),
                    GeneralizedTrapezoid(3, 9, 3, 12, 3, 12),
                    GeneralizedTrapezoid(3, 6, 0, 3, 0, 0),
                    GeneralizedTrapezoid(0, 3, 0, 12, 0, 12),
                },
            }, {  // Touching hole left increasing
                {
                    build_shape({{-1, 0}, {12, 0}, {12, 12}, {1, 12}}),
                    {build_shape({{0, 6}, {3, 3}, {3, 9}})}
                }, {
                    GeneralizedTrapezoid(9, 12, 0.5, 12, 1, 12),
                    GeneralizedTrapezoid(6, 9, 0, 0, 0.5, 3),
                    GeneralizedTrapezoid(3, 6, -0.5, 3, 0, 0),
                    GeneralizedTrapezoid(3, 9, 3, 12, 3, 12),
                    GeneralizedTrapezoid(0, 3, -1, 12, -0.5, 12),
                },
            }, {  // Touching hole left decreasing
                {
                    build_shape({{1, 0}, {12, 0}, {12, 12}, {-1, 12}}),
                    {build_shape({{0, 6}, {3, 3}, {3, 9}})}
                }, {
                    GeneralizedTrapezoid(9, 12, -0.5, 12, -1, 12),
                    GeneralizedTrapezoid(6, 9, 0, 0, -0.5, 3),
                    GeneralizedTrapezoid(3, 6, 0.5, 3, 0, 0),
                    GeneralizedTrapezoid(3, 9, 3, 12, 3, 12),
                    GeneralizedTrapezoid(0, 3, 1, 12, 0.5, 12),
                },
            }, {  // Touching hole right flat
                {
                    build_shape({{0, 0}, {12, 0}, {12, 12}, {0, 12}}),
                    {build_shape({{12, 6}, {9, 9}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(9, 12, 0, 12, 0, 12),
                    GeneralizedTrapezoid(6, 9, 12, 12, 9, 12),
                    GeneralizedTrapezoid(3, 9, 0, 9, 0, 9),
                    GeneralizedTrapezoid(3, 6, 9, 12, 12, 12),
                    GeneralizedTrapezoid(0, 3, 0, 12, 0, 12),
                },
            }, {  // Touching hole right increasing
                {
                    build_shape({{0, 0}, {11, 0}, {13, 12}, {0, 12}}),
                    {build_shape({{12, 6}, {9, 9}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(9, 12, 0, 12.5, 0, 13),
                    GeneralizedTrapezoid(6, 9, 12, 12, 9, 12.5),
                    GeneralizedTrapezoid(3, 9, 0, 9, 0, 9),
                    GeneralizedTrapezoid(3, 6, 9, 11.5, 12, 12),
                    GeneralizedTrapezoid(0, 3, 0, 11, 0, 11.5),
                },
            }, {  // Touching hole right decreasing
                {
                    build_shape({{0, 0}, {13, 0}, {11, 12}, {0, 12}}),
                    {build_shape({{12, 6}, {9, 9}, {9, 3}})}
                }, {
                    GeneralizedTrapezoid(9, 12, 0, 11.5, 0, 11),
                    GeneralizedTrapezoid(6, 9, 12, 12, 9, 11.5),
                    GeneralizedTrapezoid(3, 9, 0, 9, 0, 9),
                    GeneralizedTrapezoid(3, 6, 9, 12.5, 12, 12),
                    GeneralizedTrapezoid(0, 3, 0, 13, 0, 12.5),
                },
            },
            }));
