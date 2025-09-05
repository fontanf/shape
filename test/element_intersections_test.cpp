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
    bool strict = false;
    std::vector<Point> expected_result;

    template <class basic_json>
    static ComputeIntersectionsTestParams from_json(
            basic_json& json_item)
    {
        ComputeIntersectionsTestParams test_params;
        test_params.element_1 = ShapeElement::from_json(json_item["element_1"]);
        test_params.element_2 = ShapeElement::from_json(json_item["element_2"]);
        test_params.strict = json_item["strict"];
        for (auto& json_point: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(Point::from_json(json_point.value()));
        return test_params;
    }

    static ComputeIntersectionsTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::ComputeIntersectionsTestParams::read_json: "
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
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_intersections" << std::endl;
    for (const Point& point: test_params.expected_result)
        std::cout << "- " << point.to_string() << std::endl;

    std::vector<Point> intersections = compute_intersections(
            test_params.element_1,
            test_params.element_2,
            test_params.strict);
    std::cout << "intersections" << std::endl;
    for (const Point& point: intersections) {
        std::cout << "- " << std::setprecision(15) << point.x
            << " " << std::setprecision(15) << point.y
            << std::endl;
    }

    ASSERT_EQ(intersections.size(), test_params.expected_result.size());
    for (const Point& expected_intersection: test_params.expected_result) {
        EXPECT_NE(std::find_if(
                    intersections.begin(),
                    intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsTestParams>{
            {  // Non-intersecting line segments
                build_shape({{0, 0}, {0, 1}}, true).elements.front(),
                build_shape({{1, 0}, {1, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Simple line segment intersection.
                build_shape({{1, 0}, {1, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                false,
                {{1, 1}},
            }, {  // One line segment touching another.
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // One line segment touching another (strict).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 1}, {2, 1}}, true).elements.front(),
                true,
                {},
            }, {  // Two identical line segments.
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 0}, {0, 2}},
            }, {  // Two identical line segments (reversed).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 2}, {0, 0}}, true).elements.front(),
                false,
                {{0, 0}, {0, 2}},
            }, {  // Two identical line segments (reversed) (strict).
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                true,
                {},
            }, {  // Two overlapping line segments.
                build_shape({{0, 0}, {0, 3}}, true).elements.front(),
                build_shape({{0, 1}, {0, 4}}, true).elements.front(),
                false,
                {{0, 1}, {0, 3}},
            }, {  // Two overlapping line segments (strict).
                build_shape({{0, 0}, {0, 3}}, true).elements.front(),
                build_shape({{0, 1}, {0, 4}}, true).elements.front(),
                true,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                build_shape({{2, 0}, {2, 2}}, true).elements.front(),
                false,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                build_shape({{1, 1}, {2, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Non-intersecting line segment and circular arc.
                build_shape({{2, 0}, {0, 0, 1}, {0, 2}}, true).elements.front(),
                build_shape({{2, 1}, {3, 1}}, true).elements.front(),
                false,
                {},
            }, {  // Intersecting line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Intersecting line segment and circular arc.
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                build_shape({{0, 0}, {0, 2}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Intersecting line segment and circular arc at two points.
                build_shape({{-1, -2}, {-1, 2}}, true).elements.front(),
                build_shape({{1, 1}, {0, 0, 1}, {1, -1}}, true).elements.front(),
                false,
                {{-1, -1}, {-1, 1}},
            }, {
                build_shape({{39.2075327238964, 921.938482687602}, {39.2004011663701, 921.949097066976}}, true).elements.front(),
                build_shape({{39.2075268512914, 921.938491537799}, {39.2066965548415, 921.937934215805, -1}, {39.2075327238964, 921.938482687602}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{398683.828041 / (1 << 14), 9213001.274628 / (1 << 14)}, {398845.790901 / (1 << 14), 9213041.800811 / (1 << 14)}}, true).elements.front(),
                build_shape({{398645.799546 / (1 << 14), 9213137.752427 / (1 << 14)}, {398972.799200 / (1 << 14), 9213516.000000 / (1 << 14), 1}, {398972.799200 / (1 << 14), 9213016.000000 / (1 << 14)}}, true).elements.front(),
                false,
                {{398828.138764737 / (1 << 14), 9213037.38391307 / (1 << 14)}},
            }, {  // Touching line segment and circular arc.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 1}, {1, 1}}, true).elements.front(),
                false,
                {{0, 1}},
            }, {  // Touching line segment and circular arc (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 1}, {1, 1}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc at two points.
                build_shape({{2, 0}, {-2, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Touching line segment and circular arc at two points (strict).
                build_shape({{2, 0}, {-2, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc.
                build_shape({{5, -1}, {7, -1, 1}, {7, -3}}, true).elements.front(),
                build_shape({{6, 5}, {7, -3}}, true).elements.front(),
                true,
                {},
            }, {  // Touching line segment and circular arc.
                build_shape({{5, -1}, {7, -1, 1}, {7, -3}}, true).elements.front(),
                build_shape({{6, 5}, {7, -3}}, true).elements.front(),
                false,
                {{7, -3}},
            }, {  // Non-intersecting circular arcs.
                build_shape({{2, 0}, {0, 0, 1}, {0, 2}}, true).elements.front(),
                build_shape({{3, 0}, {1, 0, 1}, {1, 2}}, true).elements.front(),
                false,
                {},
            }, {  // Intersecting circular arcs.
                build_shape({{3, 0}, {1, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {-1, 0, 1}, {-3, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs.
                build_shape({{3, 0}, {1, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-3, 0}, {-1, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs.
                build_shape({{-1, 0}, {1, 0, -1}, {3, 0}}, true).elements.front(),
                build_shape({{-3, 0}, {-1, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{0, 1.73205080756888}},
            }, {  // Intersecting circular arcs at two points.
                build_shape({{-2, 1}, {-1, 0, -1}, {-2, -1}}, true).elements.front(),
                build_shape({{+2, 1}, {+1, 0, +1}, {+2, -1}}, true).elements.front(),
                false,
                {{0, -1}, {0, 1}},
            }, {  // Touching circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, 1}, {1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Touching circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, 1}, {1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Identical circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Identical circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Identical circular arcs (reversed).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {  // Identical circular arcs (reversed) (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{-1, 0}, {0, 0, -1}, {1, 0}}, true).elements.front(),
                true,
                {},
            }, {  // Overlapping circular arcs.
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 1}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                false,
                {{0, 1}, {-1, 0}},
            }, {  // Overlapping circular arcs (strict).
                build_shape({{1, 0}, {0, 0, 1}, {-1, 0}}, true).elements.front(),
                build_shape({{0, 1}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{1.96721311, -0.78740157}, {60.98360656, -0.78740157}}, true).elements.front(),
                build_shape({{1.941450017182601, -0.7869799841717368}, {1.96721311, -0, 1}, {1.96721311, -0.78740157}}, true).elements.front(),
                false,
                {{1.96721311, -0.78740157}},
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                build_shape({{-2, 0}, {1, 0}}, true).elements.front(),
                false,
                {{1, 0}, {-1, 0}},
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, -1}}, true).elements.front(),
                build_shape({{1, 0}, {1, 1}}, true).elements.front(),
                false,
                {{1, 0}},
            }, {
                build_shape({{23.5210023, 10.26937568}, {23.62204724, 11.81102362}}, true).elements.front(),
                build_shape({{23.62204724, 14.18786926646811}, {23.62204724, 11.81102362}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{23.62204724, 11.81102362}, {23.62204724, 23.62204724}}, true).elements.front(),
                build_shape({{27.43352932, 18.85373163}, {23.62204724, 14.18786926646811}}, true).elements.front(),
                true,
                {},
            }, {
                build_shape({{6.146061335761145, 4.1594407268466}, {6.354632555761145, 4.0610510468466}}, true).elements.front(),
                build_shape({{6.180307632126364, 3.737782318431193}, {6.180307632126364, 4.143285659332687}}, true).elements.front(),
                true,
                {},
            },
            }));


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
                    "shape::IntersectShapeShapeElementTestParams::read_json: "
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
    write_json({{test_params.shape}}, {test_params.element}, "intersect_input.json");

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
                    "shape::IntersectShapeShapeTestParams::read_json: "
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
