#include "shape/convex_partition.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/intersection_tree.hpp"

#include <gtest/gtest.h>

using namespace shape;


struct ConvexPartitionTestParams
{
    ShapeWithHoles shape;
    std::string name;
};

void PrintTo(const ConvexPartitionTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
}

class ConvexPartitionTest: public testing::TestWithParam<ConvexPartitionTestParams> { };

TEST_P(ConvexPartitionTest, ConvexPartition)
{
    ConvexPartitionTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

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

    // Check 1b: a convex input shape must produce a single convex part.
    // (A greedy pairwise-merge decomposition getting stuck with more pieces
    // than necessary, even though the whole shape is convex, was a real bug:
    // see the ArchPolygon127 case below.)
    if (test_params.shape.holes.empty() && test_params.shape.shape.is_convex())
        EXPECT_EQ(parts.size(), 1);

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
        compute_union(parts_as_shapes_with_holes).shapes_with_holes;
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
                "Triangle",
            }, {  // Rectangle.
                {build_rectangle(4, 3)},
                "Rectangle",
            }, {  // Convex trapezoid.
                {build_shape({{0, 0}, {4, 0}, {3, 2}, {1, 2}})},
                "ConvexTrapezoid",
            }, {  // Convex pentagon.
                {build_shape({{0, 0}, {4, 0}, {5, 2}, {3, 4}, {1, 4}})},
                "ConvexPentagon",
            }, {  // 127-vertex convex "arch" shape (real production data).
                // trapezoidation()'s sweep produced a degenerate near-zero-
                // height sliver trapezoid at this orientation, whose
                // vanishingly short edges made is_convex() report spurious
                // large "reflex" angles at some merge candidates and made
                // the greedy pairwise merge get stuck with 48 parts instead
                // of recognizing the whole (convex) shape as a single part.
                {build_shape({
                    {-1.42109e-14, -230.244}, {9.97228, -230.119}, {19.9384, -229.746}, {29.892, -229.125}, {39.8271, -228.256}, {49.7375, -227.139}, {59.6169, -225.776}, {69.4593, -224.167}, {79.2585, -222.313}, {89.0084, -220.215}, {98.703, -217.874}, {108.336, -215.293}, {117.902, -212.473}, {127.395, -209.414}, {136.808, -206.121}, {146.136, -202.593}, {155.374, -198.834}, {164.515, -194.846}, {173.553, -190.631}, {182.484, -186.192}, {191.302, -181.532}, {200, -176.654}, {208.574, -171.56}, {217.019, -166.254}, {225.328, -160.739}, {233.497, -155.019}, {241.522, -149.097}, {249.396, -142.976}, {257.115, -136.661}, {264.674, -130.156}, {272.069, -123.464}, {279.295, -116.59}, {286.347, -109.538}, {293.221, -102.313}, {299.912, -94.9179}, {306.418, -87.3586}, {312.733, -79.6394}, {318.853, -71.7653}, {324.775, -63.741}, {330.496, -55.5715}, {336.01, -47.262}, {341.316, -38.8176}, {346.41, -30.2435}, {351.289, -21.5451}, {355.949, -12.7278}, {360.388, -3.79702}, {364.602, 5.24164}, {368.59, 14.3826}, {372.349, 23.6201}, {375.877, 32.9484}, {379.171, 42.3618}, {382.229, 51.8544}, {385.05, 61.4203}, {387.631, 71.0535}, {389.971, 80.7481}, {392.069, 90.498}, {393.923, 100.297}, {395.532, 110.14}, {396.896, 120.019}, {398.012, 129.929}, {398.882, 139.864}, {399.503, 149.818}, {399.876, 159.784}, {400, 169.756}, {-400, 169.756}, {-399.876, 159.784}, {-399.503, 149.818}, {-398.882, 139.864}, {-398.012, 129.929}, {-396.896, 120.019}, {-395.532, 110.14}, {-393.923, 100.297}, {-392.069, 90.498}, {-389.971, 80.7481}, {-387.631, 71.0535}, {-385.05, 61.4203}, {-382.229, 51.8544}, {-379.171, 42.3618}, {-375.877, 32.9484}, {-372.349, 23.6201}, {-368.59, 14.3826}, {-364.602, 5.24164}, {-360.388, -3.79702}, {-355.949, -12.7278}, {-351.289, -21.5451}, {-346.41, -30.2435}, {-341.316, -38.8176}, {-336.01, -47.262}, {-330.496, -55.5715}, {-324.775, -63.741}, {-318.853, -71.7653}, {-312.733, -79.6394}, {-306.418, -87.3586}, {-299.912, -94.9179}, {-293.221, -102.313}, {-286.347, -109.538}, {-279.295, -116.59}, {-272.069, -123.464}, {-264.674, -130.156}, {-257.115, -136.661}, {-249.396, -142.976}, {-241.522, -149.097}, {-233.497, -155.019}, {-225.328, -160.739}, {-217.019, -166.254}, {-208.574, -171.56}, {-200, -176.654}, {-191.302, -181.532}, {-182.484, -186.192}, {-173.553, -190.631}, {-164.515, -194.846}, {-155.374, -198.834}, {-146.136, -202.593}, {-136.808, -206.121}, {-127.395, -209.414}, {-117.902, -212.473}, {-108.336, -215.293}, {-98.703, -217.874}, {-89.0084, -220.215}, {-79.2585, -222.313}, {-69.4593, -224.167}, {-59.6169, -225.776}, {-49.7375, -227.139}, {-39.8271, -228.256}, {-29.892, -229.125}, {-19.9384, -229.746}, {-9.97228, -230.119}})},
                "ArchPolygon127",
            },
            // Non-convex inputs: should produce multiple convex parts.
            {  // L-shape.
                {build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {0, 2}})},
                "LShape",
            }, {  // Cross/plus shape.
                {build_shape({
                    {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {1, 1}})},
                "CrossShape",
            }, {  // U-shape.
                {build_shape({
                    {0, 0}, {3, 0}, {3, 3}, {2, 3},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
                "UShape",
            }, {  // W-shape.
                {build_shape({
                    {0, 0}, {5, 0}, {5, 3}, {4, 3},
                    {4, 1}, {3, 1}, {3, 2}, {2, 2},
                    {2, 1}, {1, 1}, {1, 3}, {0, 3}})},
                "WShape",
            }, {  // T-shape.
                {build_shape({
                    {0, 2}, {1, 2}, {1, 0}, {2, 0},
                    {2, 2}, {3, 2}, {3, 3}, {0, 3}})},
                "TShape",
            }, {  // Staircase shape.
                {build_shape({
                    {0, 0}, {4, 0}, {4, 2}, {3, 2},
                    {3, 1}, {1, 1}, {1, 2}, {0, 2}})},
                "StaircaseShape",
            },
            // Shapes with holes.
            {  // Square ring: square with a square hole.
                {
                    build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
                    {build_shape({{1, 1}, {3, 1}, {3, 3}, {1, 3}})}
                },
                "SquareRing",
            }, {  // Octagon with a diamond hole.
                {
                    build_shape({{1, 0}, {3, 0}, {4, 1}, {4, 3}, {3, 4}, {1, 4}, {0, 3}, {0, 1}}),
                    {build_shape({{2, 1}, {3, 2}, {2, 3}, {1, 2}})}
                },
                "OctagonWithDiamondHole",
            },
        }),
        [](const testing::TestParamInfo<ConvexPartitionTest::ParamType>& info) {
            return info.param.name;
        });
