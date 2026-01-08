#include "shape/shapes_intersections.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

//#include "test_params.hpp"

using namespace shape;
namespace fs = boost::filesystem;


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

    bool result = intersect(test_params.shape);
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
                    (fs::path("data") / "tests" / "elements_intersections" / "shape_shape_element" / "0.json").string()),
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
    //write_json({{test_params.shape_1}, {test_params.shape_2}}, {}, "intersect_input.json");

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
            }, {
                IntersectShapeShapeTestParams::read_json(
                        (fs::path("data") / "tests" / "elements_intersections" / "shape_shape" / "0.json").string()),
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
