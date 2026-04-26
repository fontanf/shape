#include "shape/intersection_tree.hpp"

#include <gtest/gtest.h>

using namespace shape;

struct IntersectionTreeTestParams
{
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeElement> elements;
    std::vector<Point> points;

    bool strict = false;

    std::vector<std::pair<ShapeWithHoles, IntersectionTree::IntersectOutput>> test_shapes;
    std::vector<std::pair<ShapeElement, IntersectionTree::IntersectOutput>> test_elements;
    std::vector<std::pair<Point, IntersectionTree::IntersectOutput>> test_points;

    std::vector<std::pair<ShapePos, ShapePos>> expected_intersecting_shapes;
    std::vector<std::pair<ElementPos, ElementPos>> expected_equal_points;

    std::string name;
};

void PrintTo(const IntersectionTreeTestParams& params, std::ostream* os)
{
    *os << "shapes (" << params.shapes.size() << ")\n";
    for (const ShapeWithHoles& shape: params.shapes)
        *os << "- " << shape.to_string(0) << "\n";
    *os << "strict " << params.strict << "\n";
}

class IntersectionTreeTest: public testing::TestWithParam<IntersectionTreeTestParams> { };

TEST_P(IntersectionTreeTest, IntersectionTree)
{
    IntersectionTreeTestParams test_params = GetParam();
    IntersectionTree intersection_tree(
            test_params.shapes,
            test_params.elements,
            test_params.points);

    for (const auto& p: test_params.test_shapes) {
        IntersectionTree::IntersectOutput intersections = intersection_tree.intersect(p.first, test_params.strict);
        // Check shapes.
        EXPECT_EQ(intersections.shape_ids.size(), p.second.shape_ids.size());
        for (ShapePos expected_shape_pos: p.second.shape_ids) {
            EXPECT_NE(std::find(
                        intersections.shape_ids.begin(),
                        intersections.shape_ids.end(),
                        expected_shape_pos),
                    intersections.shape_ids.end());
        }
        // Check elements.
        EXPECT_EQ(intersections.element_ids.size(), p.second.element_ids.size());
        for (ElementPos expected_element_pos: p.second.element_ids) {
            EXPECT_NE(std::find(
                        intersections.element_ids.begin(),
                        intersections.element_ids.end(),
                        expected_element_pos),
                    intersections.element_ids.end());
        }
        // Check points.
        EXPECT_EQ(intersections.point_ids.size(), p.second.point_ids.size());
        for (ElementPos expected_point_pos: p.second.point_ids) {
            EXPECT_NE(std::find(
                        intersections.point_ids.begin(),
                        intersections.point_ids.end(),
                        expected_point_pos),
                    intersections.point_ids.end());
        }
    }

    std::vector<std::pair<ShapePos, ShapePos>> intersecting_shapes = intersection_tree.compute_intersecting_shapes(test_params.strict);
    EXPECT_EQ(intersecting_shapes.size(), test_params.expected_intersecting_shapes.size());
    for (auto expected_p: test_params.expected_intersecting_shapes) {
        EXPECT_NE(std::find(
                    intersecting_shapes.begin(),
                    intersecting_shapes.end(),
                    expected_p),
                intersecting_shapes.end());
    }

    std::vector<std::pair<ElementPos, ElementPos>> equal_points = intersection_tree.compute_equal_points();
    EXPECT_EQ(equal_points.size(), test_params.expected_equal_points.size());
    for (auto expected_p: test_params.expected_equal_points) {
        EXPECT_NE(std::find(
                    equal_points.begin(),
                    equal_points.end(),
                    expected_p),
                equal_points.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectionTreeTest,
        testing::ValuesIn(std::vector<IntersectionTreeTestParams>{
            {
                {{build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}}}, {}, {},
                true,
                {}, {}, {},
                {}, {},
                "Square",
            //}, {
            //    {{build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}}, {}, {}},
            //    true,
            //    {
            //        {{build_shape({{3, 0}, {5, 0}, {5, 2}, {3, 2}}), {}}, {{}, {}, {}}},
            //        {{build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}), {}}, {{0}, {}, {}}},
            //    }, {}, {},
            //    {}, {},
            //}, {
            //    {
            //        {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}},
            //        {build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}), {}},
            //    }, {}, {},
            //    true,
            //    {}, {}, {},
            //    {{0, 1}}, {},
            //}, {
            //    {}, {}, {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}},
            //    true,
            //    {}, {},
            //    {
            //        {{3, 0}, {{}, {}, {}}},
            //        {{0, 0}, {{}, {}, {0}}},
            //        {{1e-7, 1e-7}, {{}, {}, {0}}},
            //        {{1 + 1e-7, 1 + 1e-7}, {{}, {}, {4}}},
            //    },
            //    {}, {},
            //}, {
            //    {}, {}, {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {1 + 1e-7, 1 + 1e-7}},
            //    true,
            //    {}, {}, {},
            //    {}, {{4, 6}},
            },
            }),
        [](const testing::TestParamInfo<IntersectionTreeTest::ParamType>& info) {
            return info.param.name;
        });
