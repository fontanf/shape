#include "shape/shapes_intersections.hpp"

//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

//#include "test_params.hpp"

using namespace shape;
namespace fs = boost::filesystem;


struct IntersectShapeTestParams
{
    Shape shape;
    bool expected_output;


    template <class basic_json>
    static IntersectShapeTestParams from_json(basic_json& json_item)
    {
        IntersectShapeTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.expected_output = json_item["expected_output"];
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
    std::cout << "expected_output " << test_params.expected_output << std::endl;
    //write_json({{test_params.shape}}, {}, "intersect_input.json");

    bool output = intersect(test_params.shape);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
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
                true,
            }, {
                build_shape({{0, 0}, {6, 0}, {3, 2}, {2, 1}, {4, 1}, {3, 2}}),
                true,
            }, {
                build_shape({{0, 0}, {6, 0}, {3, 2}, {4, 1}, {2, 1}, {3, 2}}),
                true,
            }, {
                build_shape({
                        {31.49606296, 144.25196848},
                        {0, 144.25196848},
                        {0, 0},
                        {31.49606296, 0},
                        {31.49606296, 9.448818519999994},
                        {22.04724408, 4.7244092},
                        {25.1968504, 11.96850392},
                        {52.87627208, 11.96850392},
                        {52.87627208, 56.37795144},
                        {47.20698072, 56.37795144},
                        {47.20698072, 132.28346456},
                        {25.1968504, 132.28346456},
                        {22.04724408, 139.52755928},
                        {31.49606295999999, 134.8031497200001}}),
                false,
            }, {
                build_shape({
                        {0, 0},
                        {1, 0},
                        {1, 1},
                        {2, 1},
                        {2, 2},
                        {1, 2},
                        {1, 1},
                        {0, 1}}),
                true,
                }}));


struct IntersectShapeShapeElementTestParams
{
    Shape shape;
    ShapeElement element;
    bool strict = false;
    bool expected_output;


    template <class basic_json>
    static IntersectShapeShapeElementTestParams from_json(basic_json& json_item)
    {
        IntersectShapeShapeElementTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.element = ShapeElement::from_json(json_item["element"]);
        test_params.strict = json_item["strict"];
        test_params.expected_output = json_item["expected_output"];
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
    std::cout << "expected_output " << test_params.expected_output << std::endl;
    //Writer().add_shape(test_params.shape).add_element(test_params.element).write_json("intersect_input.json");

    bool output = intersect(
            test_params.shape,
            test_params.element,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeShapeElementTest,
        testing::ValuesIn(std::vector<IntersectShapeShapeElementTestParams>{
            IntersectShapeShapeElementTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_shape_element" / "0.json").string()),
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


struct ComputeIntersectionsPathShapeTestParams
{
    Shape path;
    Shape shape;
    bool only_min_max;
    std::vector<PathShapeIntersectionPoint> expected_output;


    template <class basic_json>
    static ComputeIntersectionsPathShapeTestParams from_json(
            basic_json& json_item)
    {
        ComputeIntersectionsPathShapeTestParams test_params;
        test_params.path = Shape::from_json(json_item["path"]);
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.only_min_max = json_item["only_min_max"];
        for (auto& json_intersection: json_item["expected_output"]) {
            PathShapeIntersectionPoint intersection;
            intersection.path_element_pos = json_intersection["path_element_pos"];
            intersection.shape_element_pos = json_intersection["shape_element_pos"];
            intersection.point = Point::from_json(json_intersection["point"]);
            test_params.expected_output.emplace_back(intersection);
        }
        return test_params;
    }

    static ComputeIntersectionsPathShapeTestParams read_json(
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

class ComputeIntersectionsPathShapeTest: public testing::TestWithParam<ComputeIntersectionsPathShapeTestParams> { };

TEST_P(ComputeIntersectionsPathShapeTest, ComputeIntersectionsPathShape)
{
    ComputeIntersectionsPathShapeTestParams test_params = GetParam();
    std::cout << "path " << test_params.path.to_string(0) << std::endl;
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "only_min_max " << test_params.only_min_max << std::endl;
    std::cout << "expected_output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: test_params.expected_output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

    std::vector<PathShapeIntersectionPoint> output = compute_intersections(
            test_params.path,
            test_params.shape,
            test_params.only_min_max);
    std::cout << "output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

    //Writer()
    //    .add_shape(test_params.path)
    //    .add_shape(test_params.shape)
    //    .write_json("compute_intersections_path_shape.json");

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (ElementPos pos = 0; pos < (ElementPos)output.size(); ++pos) {
        EXPECT_EQ(output[pos].path_element_pos, test_params.expected_output[pos].path_element_pos);
        EXPECT_EQ(output[pos].shape_element_pos, test_params.expected_output[pos].shape_element_pos);
        EXPECT_TRUE(equal(output[pos].point, test_params.expected_output[pos].point));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsPathShapeTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsPathShapeTestParams>{
            {
                build_path({
                        build_circular_arc({64, 0}, {192, 128}, {192, 0}, ShapeElementOrientation::Clockwise),
                        build_line_segment({192, 128}, {704, 128}),
                        build_circular_arc({704, 128}, {832, 2.842170943040401e-14}, {704, 2.842170943040401e-14}, ShapeElementOrientation::Clockwise)}),
                build_shape({{384, 320}, {448, -192}, {448, 832}}),
                true,
                {
                    {1, 0, {408, 128}},
                    {1, 1, {448, 128}},
                },
            },
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "0.json").string()),
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "1.json").string()),
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "2.json").string()),
            }));


struct ComputeStrictIntersectionsPathShapeTestParams
{
    Shape path;
    Shape shape;
    bool only_first;
    std::vector<PathShapeIntersectionPoint> expected_output;


    template <class basic_json>
    static ComputeStrictIntersectionsPathShapeTestParams from_json(
            basic_json& json_item)
    {
        ComputeStrictIntersectionsPathShapeTestParams test_params;
        test_params.path = Shape::from_json(json_item["path"]);
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.only_first = json_item["only_first"];
        for (auto& json_intersection: json_item["expected_output"]) {
            PathShapeIntersectionPoint intersection;
            intersection.path_element_pos = json_intersection["path_element_pos"];
            intersection.shape_element_pos = json_intersection["shape_element_pos"];
            intersection.point = Point::from_json(json_intersection["point"]);
            test_params.expected_output.emplace_back(intersection);
        }
        return test_params;
    }

    static ComputeStrictIntersectionsPathShapeTestParams read_json(
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

class ComputeStrictIntersectionsPathShapeTest: public testing::TestWithParam<ComputeStrictIntersectionsPathShapeTestParams> { };

TEST_P(ComputeStrictIntersectionsPathShapeTest, ComputeStrictIntersectionsPathShape)
{
    ComputeStrictIntersectionsPathShapeTestParams test_params = GetParam();
    std::cout << "path " << test_params.path.to_string(0) << std::endl;
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "only_first " << test_params.only_first << std::endl;
    std::cout << "expected_output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: test_params.expected_output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

    std::vector<PathShapeIntersectionPoint> output = compute_strict_intersections(
            test_params.path,
            test_params.shape,
            test_params.only_first);
    std::cout << "output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

    //Writer()
    //    .add_shape(test_params.path)
    //    .add_shape(test_params.shape)
    //    .write_json("compute_intersections_path_shape.json");

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (ElementPos pos = 0; pos < (ElementPos)output.size(); ++pos) {
        EXPECT_EQ(output[pos].path_element_pos, test_params.expected_output[pos].path_element_pos);
        EXPECT_EQ(output[pos].shape_element_pos, test_params.expected_output[pos].shape_element_pos);
        EXPECT_TRUE(equal(output[pos].point, test_params.expected_output[pos].point));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeStrictIntersectionsPathShapeTest,
        testing::ValuesIn(std::vector<ComputeStrictIntersectionsPathShapeTestParams>{
            {
                build_path({
                        build_circular_arc({64, 0}, {192, 128}, {192, 0}, ShapeElementOrientation::Clockwise),
                        build_line_segment({192, 128}, {704, 128}),
                        build_circular_arc({704, 128}, {832, 2.842170943040401e-14}, {704, 2.842170943040401e-14}, ShapeElementOrientation::Clockwise)}),
                build_shape({{384, 320}, {448, -192}, {448, 832}}),
                true,
                {{1, 0, {408, 128}}},
            },
            }));


struct IntersectShapeShapeTestParams
{
    Shape shape_1;
    Shape shape_2;
    bool strict = false;
    bool expected_output;


    template <class basic_json>
    static IntersectShapeShapeTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeShapeTestParams test_params;
        test_params.shape_1 = Shape::from_json(json_item["shape_1"]);
        test_params.shape_2 = Shape::from_json(json_item["shape_2"]);
        test_params.strict = json_item["strict"];
        test_params.expected_output = json_item["expected_output"];
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
    std::cout << "expected_output " << test_params.expected_output << std::endl;
    //Writer()
    //    .add_shape(test_params.shape_1, "Shape 1")
    //    .add_shape(test_params.shape_2, "Shape 2")
    //    .write_json("intersect_shape_shape_input.json");

    bool output = intersect(
            test_params.shape_1,
            test_params.shape_2,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
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
            }, {
                IntersectShapeShapeTestParams::read_json(
                        (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_shape" / "0.json").string()),
            }, {
                build_shape({{4, 0}, {0, 0}, {0, 2}, {1, 2}, {2, 3}, {3, 2}, {4, 2}}),
                build_shape({{0, 2}, {0, 4}, {4, 4}, {4, 2}, {3, 2}, {2, 1}, {1, 2}}),
                true,
                true,
            //}, {
            //    build_shape({{4, 0}, {0, 0}, {0, 2}, {2, 0}, {4, 2}}),
            //    build_shape({{2, 0}, {3, 2}, {1, 2}}),
            //    true,
            //    false,
            //}, {
            //    build_shape({{4, 0}, {0, 0}, {0, 2}, {2, 0}, {4, 2}}),
            //    build_shape({{2, 0}, {4, 2}, {0, 2}}),
            //    true,
            //    false,
            },
            }));


struct IntersectShapeWithHolesShapeTestParams
{
    ShapeWithHoles shape_with_holes;
    Shape shape;
    bool strict = false;
    bool expected_output;


    template <class basic_json>
    static IntersectShapeWithHolesShapeTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeWithHolesShapeTestParams test_params;
        test_params.shape_with_holes = ShapeWithHoles::from_json(json_item["shape_with_holes"]);
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.strict = json_item["strict"];
        if (json_item.contains("expected_output"))
            test_params.expected_output = json_item["expected_output"];
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
    std::cout << "shape_with_holes " << test_params.shape_with_holes.to_string(0) << std::endl;
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_output " << test_params.expected_output << std::endl;
    //Writer()
    //    .add_shape_with_holes(test_params.shape_with_holes, "Shape with holes")
    //    .add_shape(test_params.shape, "Shape")
    //    .write_json("intersect_shape_with_holes_shape_input.json");

    bool output = intersect(
            test_params.shape_with_holes,
            test_params.shape,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
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
            },
            IntersectShapeWithHolesShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_with_holes_shape" / "0.json").string()),
            }));


struct IntersectShapeWithHolesShapeWithHolesTestParams
{
    ShapeWithHoles shape_with_holes_1;
    ShapeWithHoles shape_with_holes_2;
    bool strict = false;
    bool expected_output;


    template <class basic_json>
    static IntersectShapeWithHolesShapeWithHolesTestParams from_json(
            basic_json& json_item)
    {
        IntersectShapeWithHolesShapeWithHolesTestParams test_params;
        test_params.shape_with_holes_1 = ShapeWithHoles::from_json(json_item["shape_with_holes_1"]);
        test_params.shape_with_holes_2 = ShapeWithHoles::from_json(json_item["shape_with_holes_2"]);
        test_params.strict = json_item["strict"];
        test_params.expected_output = json_item["expected_output"];
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
    std::cout << "shape_with_holes_1 " << test_params.shape_with_holes_1.to_string(0) << std::endl;
    std::cout << "shape_with_holes_2 " << test_params.shape_with_holes_2.to_string(0) << std::endl;
    std::cout << "strict " << test_params.strict << std::endl;
    std::cout << "expected_output " << test_params.expected_output << std::endl;

    bool output = intersect(
            test_params.shape_with_holes_1,
            test_params.shape_with_holes_2,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
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
