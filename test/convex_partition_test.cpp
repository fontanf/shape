#include "shape/convex_partition.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/intersection_tree.hpp"

#include <gtest/gtest.h>

using namespace shape;


struct ConvexPartitionTestParams
{
    ShapeWithHoles shape;
};

class ConvexPartitionTest: public testing::TestWithParam<ConvexPartitionTestParams> { };

TEST_P(ConvexPartitionTest, ConvexPartition)
{
    ConvexPartitionTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;

    std::vector<Shape> parts = compute_convex_partition(test_params.shape);

    std::cout << "parts (" << parts.size() << ")" << std::endl;
    for (ShapePos part_pos = 0;
            part_pos < (ShapePos)parts.size();
            ++part_pos) {
        std::cout << "- part " << part_pos
            << ": " << parts[part_pos].to_string(0) << std::endl;
    }

    // Check 1: all parts are convex.
    for (const Shape& part: parts)
        EXPECT_TRUE(part.is_convex());

    // Convert parts to ShapeWithHoles for the remaining checks.
    std::vector<ShapeWithHoles> parts_as_shapes_with_holes;
    for (const Shape& part: parts)
        parts_as_shapes_with_holes.push_back({part, {}});

    // Check 2: no two parts strictly intersect.
    IntersectionTree intersection_tree(
            parts_as_shapes_with_holes,
            {},
            {});
    std::vector<std::pair<ShapePos, ShapePos>> intersecting_pairs =
        intersection_tree.compute_intersecting_shapes(true);
    std::cout << "intersecting pairs: " << intersecting_pairs.size() << std::endl;
    EXPECT_TRUE(intersecting_pairs.empty());

    // Check 3: the union of all parts equals the input shape.
    std::vector<ShapeWithHoles> union_output =
        compute_union(parts_as_shapes_with_holes);
    std::cout << "union contains " << union_output.size() << " shape(s)" << std::endl;
    if (!union_output.empty())
        std::cout << "union: " << union_output[0].to_string(0) << std::endl;
    ASSERT_EQ(union_output.size(), 1);
    EXPECT_TRUE(equal(union_output[0], test_params.shape));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ConvexPartitionTest,
        testing::ValuesIn(std::vector<ConvexPartitionTestParams>{
            // Convex inputs: should produce a single convex part.
            {  // Triangle.
                {build_shape({{0, 0}, {3, 0}, {1, 3}})},
            }, {  // Rectangle.
                {build_rectangle(4, 3)},
            }, {  // Convex trapezoid.
                {build_shape({{0, 0}, {4, 0}, {3, 2}, {1, 2}})},
            }, {  // Convex pentagon.
                {build_shape({{0, 0}, {4, 0}, {5, 2}, {3, 4}, {1, 4}})},
            },
            // Non-convex inputs: should produce multiple convex parts.
            {  // L-shape.
                {build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {0, 2}})},
            }, {  // Cross/plus shape.
                {build_shape({
                    {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}})},
            }, {  // U-shape.
                {build_shape({
                    {0, 0}, {3, 0}, {3, 3}, {2, 3},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
            }, {  // W-shape.
                {build_shape({
                    {0, 0}, {5, 0}, {5, 3}, {4, 3},
                    {4, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
            }, {  // T-shape.
                {build_shape({
                    {0, 2}, {1, 2}, {1, 0}, {2, 0},
                    {2, 2}, {3, 2}, {3, 3}, {0, 3}})},
            }, {  // Staircase shape.
                {build_shape({
                    {0, 0}, {4, 0}, {4, 2}, {3, 2},
                    {3, 1}, {1, 1}, {1, 2}, {0, 2}})},
            },
            // Shapes with holes.
            {  // Square ring: square with a square hole.
                {
                    build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                    {build_shape({{1, 1}, {3, 1}, {3, 3}, {1, 3}})}
                },
            }, {  // Octagon with a diamond hole.
                {
                    build_shape({{1, 0}, {3, 0}, {4, 1}, {4, 3}, {3, 4}, {1, 4}, {0, 3}, {0, 1}}),
                    {build_shape({{2, 1}, {3, 2}, {2, 3}, {1, 2}})}
                },
            },
        }));
