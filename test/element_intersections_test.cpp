#include "shape/element_intersections.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

#include "test_params.hpp"

using namespace shape;
namespace fs = boost::filesystem;


struct ComputeIntersectionsTestParams
{
    ShapeElement element_1;
    ShapeElement element_2;
    ShapeElementIntersectionsOutput expected_result;

    template <class basic_json>
    static ComputeIntersectionsTestParams from_json(
            basic_json& json_item)
    {
        ComputeIntersectionsTestParams test_params;
        test_params.element_1 = ShapeElement::from_json(json_item["element_1"]);
        test_params.element_2 = ShapeElement::from_json(json_item["element_2"]);
        for (auto& json_element: json_item["expected_result"]["overlapping_parts"].items())
            test_params.expected_result.overlapping_parts.emplace_back(ShapeElement::from_json(json_element.value()));
        for (auto& json_point: json_item["expected_result"]["improper_intersections"].items())
            test_params.expected_result.improper_intersections.emplace_back(Point::from_json(json_point.value()));
        for (auto& json_point: json_item["expected_result"]["proper_intersections"].items())
            test_params.expected_result.proper_intersections.emplace_back(Point::from_json(json_point.value()));
        return test_params;
    }

    static ComputeIntersectionsTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class ComputeIntersectionsTest: public testing::TestWithParam<ComputeIntersectionsTestParams> { };

TEST_P(ComputeIntersectionsTest, ComputeIntersections)
{
    ComputeIntersectionsTestParams test_params = GetParam();
    std::cout << "element_1 " << test_params.element_1.to_string() << std::endl;
    std::cout << "element_2 " << test_params.element_2.to_string() << std::endl;
    std::cout << "expected_result.overlapping_parts" << std::endl;
    for (const ShapeElement& overlapping_part: test_params.expected_result.overlapping_parts)
        std::cout << "- " << overlapping_part.to_string() << std::endl;
    std::cout << "expected_result.impropoer_intersections" << std::endl;
    for (const Point& point: test_params.expected_result.improper_intersections)
        std::cout << "- " << point.to_string() << std::endl;
    std::cout << "expected_result.propoer_intersections" << std::endl;
    for (const Point& point: test_params.expected_result.proper_intersections)
        std::cout << "- " << point.to_string() << std::endl;
    //write_json({}, {test_params.element_1, test_params.element_2}, "element_intersections_input.json");

    ShapeElementIntersectionsOutput intersections = compute_intersections(
            test_params.element_1,
            test_params.element_2);
    std::cout << "result.overlapping_parts" << std::endl;
    for (const ShapeElement& overlapping_part: intersections.overlapping_parts)
        std::cout << "- " << overlapping_part.to_string() << std::endl;
    std::cout << "result.improper_intersections" << std::endl;
    for (const Point& point: intersections.improper_intersections)
        std::cout << "- " << point.to_string() << std::endl;
    std::cout << "result.proper_intersections" << std::endl;
    for (const Point& point: intersections.proper_intersections)
        std::cout << "- " << point.to_string() << std::endl;

    ASSERT_EQ(intersections.overlapping_parts.size(), test_params.expected_result.overlapping_parts.size());
    for (const auto& expected_intersection: test_params.expected_result.overlapping_parts) {
        EXPECT_NE(std::find_if(
                    intersections.overlapping_parts.begin(),
                    intersections.overlapping_parts.end(),
                    [&expected_intersection](const ShapeElement& overlapping_part) { return equal(overlapping_part, expected_intersection); }),
                intersections.overlapping_parts.end());
    }
    ASSERT_EQ(intersections.improper_intersections.size(), test_params.expected_result.improper_intersections.size());
    for (const Point& expected_intersection: test_params.expected_result.improper_intersections) {
        EXPECT_NE(std::find_if(
                    intersections.improper_intersections.begin(),
                    intersections.improper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.improper_intersections.end());
    }
    ASSERT_EQ(intersections.proper_intersections.size(), test_params.expected_result.proper_intersections.size());
    for (const Point& expected_intersection: test_params.expected_result.proper_intersections) {
        EXPECT_NE(std::find_if(
                    intersections.proper_intersections.begin(),
                    intersections.proper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.proper_intersections.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsTestParams>{
            {  // Non-intersecting line segments
                build_line_segment({0, 0}, {0, 1}),
                build_line_segment({1, 0}, {1, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segments
                build_line_segment({11, 1}, {10, 2}),
                build_line_segment({9, 3}, {11, 3}),
                {{}, {}, {}},
            }, {  // Simple line segment intersection.
                build_line_segment({1, 0}, {1, 2}),
                build_line_segment({0, 1}, {2, 1}),
                {{}, {}, {{1, 1}}},
            }, {  // One line segment touching another.
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 1}, {2, 1}),
                {{}, {{0, 1}}, {}},
            }, {  // Two identical line segments.
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 0}, {0, 2}),
                {{build_line_segment({0, 0}, {0, 2})}, {}, {}},
            }, {  // Two identical line segments (reversed).
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 2}, {0, 0}),
                {{build_line_segment({0, 0}, {0, 2})}, {}, {}},
            }, {  // Two overlapping line segments.
                build_line_segment({0, 0}, {0, 3}),
                build_line_segment({0, 1}, {0, 4}),
                {{build_line_segment({0, 1}, {0, 3})}, {}, {}},
            }, {  // Two overlapping line segments.
                build_line_segment({144.25196848, 31.49606296}, {134.8031497200001, 31.49606295999999}),
                build_line_segment({135.4330709199999, 31.49606296}, {8.818897719999987, 31.49606296}),
                {{build_line_segment({134.8031497200001, 31.49606296}, {135.4330709199999, 31.49606296})}, {}, {}},
            }, {  // Two touching colinear line segments.
                build_line_segment({0, 0}, {0, 1}),
                build_line_segment({0, 1}, {0, 2}),
                {{}, {{0, 1}}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({2, 0}, {2, 2}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({1, 1}, {2, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({2, 0}, {0, 2}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({2, 1}, {3, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_line_segment({15, 0}, {35, 0}),
                build_circular_arc({2.5, -9.682458365518542}, {2.5, 9.682458365518542}, {5, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {}},
            }, {  // Intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 0}, {0, 2}),
                {{}, {}, {{0, 1}}},
            }, {  // Intersecting line segment and circular arc.
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({0, 0}, {0, 2}),
                {{}, {}, {{0, 1}}},
            }, {  // Intersecting line segment and circular arc at two points.
                build_line_segment({-1, -2}, {-1, 2}),
                build_circular_arc({1, 1}, {1, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{-1, -1}, {-1, 1}}},
            }, {
                build_line_segment({39.2075327238964, 921.938482687602}, {39.2004011663701, 921.949097066976}),
                build_circular_arc({39.2075268512914, 921.938491537799}, {39.2075327238964, 921.938482687602}, {39.2066965548415, 921.937934215805}, ShapeElementOrientation::Clockwise),
                {{}, {{39.2075327238964, 921.938482687602}}, {}},
            }, {
                build_line_segment({398683.828041/(1<<14), 9213001.274628/(1<<14)}, {398845.790901/(1<<14), 9213041.800811/(1<<14)}),
                build_circular_arc({398645.799546/(1<<14), 9213137.752427/(1<<14)}, {398972.799200/(1<<14), 9213016.000000/(1<<14)}, {398972.799200/(1<<14), 9213516.000000/(1<<14)}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{398828.138764737/(1<<14), 9213037.38391307/(1<<14)}}},
            }, {  // Touching line segment and circular arc.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({-1, 1}, {1, 1}),
                {{}, {{0, 1}}, {}},
            }, {  // Touching line segment and circular arc at two points.
                build_line_segment({2, 0}, {-2, 0}),
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1, 0}, {-1, 0}}, {}},
            }, {  // Touching line segment and circular arc.
                build_circular_arc({5, -1}, {7, -3}, {7, -1}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({6, 5}, {7, -3}),
                {{}, {{7, -3}}, {}},
            }, {  // Non-intersecting circular arcs.
                build_circular_arc({2, 0}, {0, 2}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({3, 0}, {1, 2}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({3, 0}, {-1, 0}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({1, 0}, {-3, 0}, { -1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({3, 0}, {-1, 0}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-3, 0}, {1, 0}, {-1, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({-1, 0}, {3, 0}, {1, 0}, ShapeElementOrientation::Clockwise),
                build_circular_arc({-3, 0}, {1, 0}, {-1, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs at two points.
                build_circular_arc({-2, 1}, {-2, -1}, {-1, 0}, ShapeElementOrientation::Clockwise),
                build_circular_arc({2, 1}, {2, -1}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{0, -1}, {0, 1}}},
            }, {  // Touching circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1, 0}, {-1, 0}}, {}},
            }, {  // Identical circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {  // Identical circular arcs (reversed).
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                {{build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {  // Overlapping circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({0, 1}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {
                build_line_segment({1.96721311, -0.78740157}, {60.98360656, -0.78740157}),
                build_circular_arc({1.941450017182601, -0.7869799841717368}, {1.96721311, -0.78740157}, {1.96721311, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1.96721311, -0.78740157}}, {}},
            }, {
                build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({-2, 0}, {1, 0}),
                {{}, {{1, 0}}, {{-1, 0}}},
            }, {
                build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({1, 0}, {1, 1}),
                {{}, {{1, 0}}, {}},
            }, {
                build_line_segment({23.5210023, 10.26937568}, {23.62204724, 11.81102362}),
                build_line_segment({23.62204724, 14.18786926646811}, {23.62204724, 11.81102362}),
                {{}, {{23.62204724, 11.81102362}}, {}},
            }, {
                build_line_segment({23.62204724, 11.81102362}, {23.62204724, 23.62204724}),
                build_line_segment({27.43352932, 18.85373163}, {23.62204724, 14.18786926646811}),
                {{}, {{23.62204724, 14.18786926646811}}, {}},
            }, {
                build_line_segment({6.146061335761145, 4.1594407268466}, {6.354632555761145, 4.0610510468466}),
                build_line_segment({6.180307632126364, 3.737782318431193}, {6.180307632126364, 4.143285659332687}),
                {{}, {{6.180307632126364, 4.143285659332687}}, {}},
            }, {
                build_line_segment({100, 150}, {0, 0}),
                build_line_segment({33.33333333333334, 50}, {50, 75}),
                {{build_line_segment({33.33333333333334, 50}, {50, 75})}, {}, {}},
            }, {  // Overlapping line segments with numerical issues.
                build_line_segment({56.45661773889309, 154.3210357234225}, {74.21458856416608, 160.7844084041661}),
                build_line_segment({67.70333256416608, 158.4145050441661}, {85.46130335612965, 164.8778778164265}),
                {{build_line_segment({67.70333256416608, 158.4145050441661}, {74.21458856416608, 160.7844084041661})}, {}, {}},
            }}));


struct IntersectShapeTestParams
{
    Shape shape;
    bool expected_result;


    template <class basic_json>
    static IntersectShapeTestParams from_json(basic_json& json_item)
    {
        IntersectShapeTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.expected_result = json_item["expected_result"];
        return test_params;
    }

    static IntersectShapeTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class IntersectShapeTest: public testing::TestWithParam<IntersectShapeTestParams> { };

TEST_P(IntersectShapeTest, IntersectShape)
{
    IntersectShapeTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "expected_result " << test_params.expected_result << std::endl;
    //write_json({{test_params.shape}}, {}, "intersect_input.json");

    bool result = strictly_intersect(
            test_params.shape);
    std::cout << "result " << result << std::endl;

    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeTestParams>{
            {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                false,
            }, {
                build_shape({{0, 0}, {2, 2}, {2, 0}, {0, 2}}),
                true,
            }, {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                true,
            }, {
                build_shape({{0, 0}, {4, 0}, {4, 4}, {2, 4}, {2, 3}, {3, 3}, {3, 2}, {1, 2}, {1, 3}, {2, 3}, {2, 4}, {0, 4}}),
                false,
            }, {
                build_shape({{0, 0}, {6, 0}, {3, 2}, {2, 1}, {4, 1}, {3, 2}}),
                true,
            }, {
                build_shape({{0, 0}, {6, 0}, {3, 2}, {4, 1}, {2, 1}, {3, 2}}),
                false,
            }}));


struct IntersectShapeShapeElementTestParams
{
    Shape shape;
    ShapeElement element;
    bool strict = false;
    bool expected_result;


    template <class basic_json>
    static IntersectShapeShapeElementTestParams from_json(basic_json& json_item)
    {
        IntersectShapeShapeElementTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.element = ShapeElement::from_json(json_item["element"]);
        test_params.strict = json_item["strict"];
        test_params.expected_result = json_item["expected_result"];
        return test_params;
    }

    static IntersectShapeShapeElementTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class IntersectShapeShapeElementTest: public testing::TestWithParam<IntersectShapeShapeElementTestParams> { };

TEST_P(IntersectShapeShapeElementTest, IntersectShapeShapeElement)
{
    IntersectShapeShapeElementTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "element " << test_params.element.to_string() << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_result " << test_params.expected_result << std::endl;
    //write_json({{test_params.shape}}, {test_params.element}, "intersect_input.json");

    bool result = intersect(
            test_params.shape,
            test_params.element,
            test_params.strict);
    std::cout << "result " << result << std::endl;

    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeShapeElementTest,
        testing::ValuesIn(std::vector<IntersectShapeShapeElementTestParams>{
            IntersectShapeShapeElementTestParams::read_json(
                    (fs::path("data") / "tests" / "element_intersections" / "shape_shape_element" / "0.json").string()),
            {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_line_segment({3, 0}, {3, 2}),
                false,
                false,
            }, {
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_line_segment({1, 1}, {1, 3}),
                false,
                true,
            }, {
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_line_segment({1, 1}, {1, 3}),
                false,
                false,
            },
            }));


struct IntersectShapeShapeTestParams
{
    Shape shape_1;
    Shape shape_2;
    bool strict = false;
    bool expected_result;


    template <class basic_json>
    static IntersectShapeShapeTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeShapeTestParams test_params;
        test_params.shape_1 = Shape::from_json(json_item["shape_1"]);
        test_params.shape_2 = Shape::from_json(json_item["shape_2"]);
        test_params.strict = json_item["strict"];
        test_params.expected_result = json_item["expected_result"];
        return test_params;
    }

    static IntersectShapeShapeTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class IntersectShapeShapeTest: public testing::TestWithParam<IntersectShapeShapeTestParams> { };

TEST_P(IntersectShapeShapeTest, IntersectShapeShape)
{
    IntersectShapeShapeTestParams test_params = GetParam();
    std::cout << "shape_1 " << test_params.shape_1.to_string(0) << std::endl;
    std::cout << "shape_2 " << test_params.shape_2.to_string(0) << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_result " << test_params.expected_result << std::endl;

    bool result = intersect(
            test_params.shape_1,
            test_params.shape_2,
            test_params.strict);
    std::cout << "result " << result << std::endl;

    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeShapeTestParams>{
            {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                true,
                true,
            }, {
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_path({{3, 0}, {3, 2}}),
                false,
                false,
            }, {
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_path({{1, 1}, {1, 3}}),
                false,
                true,
            }, {
                build_path({{1, 1}, {1, 3}}),
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                false,
                true,
            }, {
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_path({{1, 1}, {1, 3}}),
                false,
                false,
            }, {
                build_path({{1, 1}, {1, 3}}),
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                false,
                false,
            },
            IntersectShapeShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "element_intersections" / "shape_shape" / "0.json").string()),
            }));


struct IntersectShapeWithHolesShapeTestParams
{
    ShapeWithHoles shape_1;
    Shape shape_2;
    bool strict = false;
    bool expected_result;


    template <class basic_json>
    static IntersectShapeWithHolesShapeTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeWithHolesShapeTestParams test_params;
        test_params.shape_1 = ShapeWithHoles::from_json(json_item["shape_1"]);
        test_params.shape_2 = Shape::from_json(json_item["shape_2"]);
        test_params.strict = json_item["strict"];
        test_params.expected_result = json_item["expected_result"];
        return test_params;
    }

    static IntersectShapeWithHolesShapeTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class IntersectShapeWithHolesShapeTest: public testing::TestWithParam<IntersectShapeWithHolesShapeTestParams> { };

TEST_P(IntersectShapeWithHolesShapeTest, IntersectShapeWithHolesShape)
{
    IntersectShapeWithHolesShapeTestParams test_params = GetParam();
    std::cout << "shape_1 " << test_params.shape_1.to_string(0) << std::endl;
    std::cout << "shape_2 " << test_params.shape_2.to_string(0) << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_result " << test_params.expected_result << std::endl;
    //write_json({test_params.shape_1, {test_params.shape_2}}, {}, "intersect_input.json");

    bool result = intersect(
            test_params.shape_1,
            test_params.shape_2,
            test_params.strict);
    std::cout << "result " << result << std::endl;

    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeWithHolesShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeWithHolesShapeTestParams>{
            {
                {
                    build_shape({{500, 500}, {0, 500}, {0, 0}, {500, 0}}),
                    {build_shape({{100, 100}, {400, 100}, {400, 400}, {100, 400}})}
                },
                build_shape({{100, 200}, {200, 200}, {200, 400}, {100, 400}}),
                true,
                false,
            }, {
                {
                    build_circle(10).shift(5, 0),
                },
                build_shape({
                        {2.5, -9.682458365518542},
                        {5, 0, -1},
                        {2.5, 9.682458365518542},
                        {0, 0, 1}}),
                true,
                false,
            }, {
                {
                    build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {3, 2}, {3, 3}, {0, 3}}),
                },
                build_shape({{1, 1}, {2, 1}, {3, 1}, {4, 1}, {4, 2}, {3, 2}, {2, 2}, {1, 2}}),
                true,
                false,
            }, {
                {
                    build_shape({{2, 0}, {5, 0}, {5, 3}, {2, 3}, {2, 2}, {4, 2}, {4, 1}, {2, 1}}),
                },
                build_shape({{1, 1}, {2, 1}, {3, 1}, {4, 1}, {4, 2}, {3, 2}, {2, 2}, {1, 2}}),
                true,
                false,
            }, {
                {
                    build_circle(10),
                },
                build_shape({
                        {2.5, -9.682458365518542},
                        {5, 0, 1},
                        {2.5, 9.682458365518542},
                        {0, 0, -1}}),
                true,
                false,
            }, {
                {
                    build_shape({
                            {19.68503937, 17.7480315},
                            {19.68503937, 15.7480315, 1},
                            {17.68503937, 15.7480315},
                            {19.68503937, 15.7480315}}),
                },
                build_shape({
                        {5.93700787, 5.93700787},
                        {17.68503937, 5.93700787},
                        {17.68503937, 15.7480315},
                        {19.68503937, 15.7480315, -1},
                        {5.93700787, 17.68503937}}),
                true,
                false,
            }}));


struct IntersectShapeWithHolesShapeWithHolesTestParams
{
    ShapeWithHoles shape_1;
    ShapeWithHoles shape_2;
    bool strict = false;
    bool expected_result;


    template <class basic_json>
    static IntersectShapeWithHolesShapeWithHolesTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeWithHolesShapeWithHolesTestParams test_params;
        test_params.shape_1 = ShapeWithHoles::from_json(json_item["shape_1"]);
        test_params.shape_2 = ShapeWithHoles::from_json(json_item["shape_2"]);
        test_params.strict = json_item["strict"];
        test_params.expected_result = json_item["expected_result"];
        return test_params;
    }

    static IntersectShapeWithHolesShapeWithHolesTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class IntersectShapeWithHolesShapeWithHolesTest: public testing::TestWithParam<IntersectShapeWithHolesShapeWithHolesTestParams> { };

TEST_P(IntersectShapeWithHolesShapeWithHolesTest, IntersectShapeWithHolesShapeWithHoles)
{
    IntersectShapeWithHolesShapeWithHolesTestParams test_params = GetParam();
    std::cout << "shape_1 " << test_params.shape_1.to_string(0) << std::endl;
    std::cout << "shape_2 " << test_params.shape_2.to_string(0) << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_result " << test_params.expected_result << std::endl;

    bool result = intersect(
            test_params.shape_1,
            test_params.shape_2,
            test_params.strict);
    std::cout << "result " << result << std::endl;

    EXPECT_EQ(result, test_params.expected_result);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeWithHolesShapeWithHolesTest,
        testing::ValuesIn(std::vector<IntersectShapeWithHolesShapeWithHolesTestParams>{
            {
                {build_shape({{100, 200}, {200, 200}, {200, 400}, {100, 400}})},
                {
                    build_shape({{500, 500}, {0, 500}, {0, 0}, {500, 0}}),
                    {build_shape({{100, 100}, {400, 100}, {400, 400}, {100, 400}})}
                },
                true,
                false,
            }}));
