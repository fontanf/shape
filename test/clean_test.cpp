#include "shape/clean.hpp"

#include <gtest/gtest.h>

using namespace shape;


struct RemoveRedundantVerticesTestParams
{
    Shape shape;
    Shape expected_shape;
};

class RemoveRedundantVerticesTest: public testing::TestWithParam<RemoveRedundantVerticesTestParams> { };

TEST_P(RemoveRedundantVerticesTest, RemoveRedundantVertices)
{
    RemoveRedundantVerticesTestParams test_params = GetParam();
    Shape cleaned_shape = remove_redundant_vertices(test_params.shape).second;
    std::cout << cleaned_shape.to_string(0) << std::endl;
    EXPECT_EQ(test_params.expected_shape, cleaned_shape);
}

INSTANTIATE_TEST_SUITE_P(
        ,
        RemoveRedundantVerticesTest,
        testing::ValuesIn(std::vector<RemoveRedundantVerticesTestParams>{
            {
                build_shape({{0, 0}, {0, 0}, {100, 0}, {100, 100}}),
                build_shape({{0, 0}, {100, 0}, {100, 100}})
            }}));


struct FixSelfIntersectionsTestParams
{
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> expected_result;
};

class FixSelfIntersectionsTest: public testing::TestWithParam<FixSelfIntersectionsTestParams> { };

TEST_P(FixSelfIntersectionsTest, FixSelfIntersections)
{
    FixSelfIntersectionsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected shapes:" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << shape.to_string(2) << std::endl;
    std::vector<ShapeWithHoles> result = fix_self_intersections(test_params.shape);
    std::cout << "result:" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << shape.to_string(2) << std::endl;

    ASSERT_EQ(result.size(), test_params.expected_result.size());
    for (const ShapeWithHoles& expected_shape: test_params.expected_result) {
        EXPECT_NE(std::find_if(
                      result.begin(),
                      result.end(),
                      [&expected_shape](const ShapeWithHoles& shape) { return equal(shape, expected_shape); }),
                  result.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        FixSelfIntersectionsTest,
        testing::ValuesIn(std::vector<FixSelfIntersectionsTestParams>{
            {
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                },
            }, {
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                    {build_shape({{1, 1}, {2, 1}, {2, 2}, {1, 2}})},
                },
            },
            }));
