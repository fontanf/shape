#include "shape/boolean_operations.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct ComputeBooleanUnionTestParams
{
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;

    template <class basic_json>
    static ComputeBooleanUnionTestParams from_json(
        basic_json& json_item)
    {
        ComputeBooleanUnionTestParams test_params;
        for (auto it = json_item["shapes"].begin();
                it != json_item["shapes"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.shapes.push_back(shape);
        }
        for (auto it = json_item["expected_result"].begin();
                it != json_item["expected_result"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.expected_result.push_back(shape);
        }
        return test_params;
    }

    static ComputeBooleanUnionTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::ComputeBooleanUnionTestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class ComputeBooleanUnionTest: public testing::TestWithParam<ComputeBooleanUnionTestParams> { };

TEST_P(ComputeBooleanUnionTest, ComputeBooleanUnion)
{
    ComputeBooleanUnionTestParams test_params = GetParam();
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> result = compute_union(
            test_params.shapes);
    std::cout << "union_shapes" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanUnionTest,
        testing::ValuesIn(std::vector<ComputeBooleanUnionTestParams>{
            ComputeBooleanUnionTestParams::read_json(
                (fs::path("data") / "tests" / "boolean_operations" / "union" / "0.json").string()),
            {
                {
                    {build_shape({{0, 0}, {2, 0}, {2, 1}, {0, 1}}), {}},
                    {build_shape({{1, 0}, {3, 0}, {3, 1}, {1, 1}}), {}},
                },
                {{build_shape({{0, 0}, {3, 0}, {3, 1}, {0, 1}}), {}}},
            }, {
                {
                    {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}},
                    {build_shape({{1, 1}, {3, 1}, {3, 3}, {1, 3}}), {}},
                },
                {{build_shape({{0, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 3}, {1, 3}, {1, 2}, {0, 2}}), {}}},
            }, {
                {
                    {build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {3, 2}, {3, 3}, {0, 3}}), {}},
                    {build_shape({{2, 0}, {5, 0}, {5, 3}, {2, 3}, {2, 2}, {4, 2}, {4, 1}, {2, 1}}), {}},
                },
                {{
                    build_shape({{0, 0}, {5, 0}, {5, 3}, {0, 3}}),
                    {build_shape({{1, 1}, {4, 1}, {4, 2}, {1, 2}})}
                }},
            }, {
                {
                    {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}},
                    {build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}), {}},
                },
                {{build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}), {}}},
            }, {
                {
                    {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}), {}},
                    {build_shape({{1, 0}, {3, 0}, {3, 2}, {1, 2}}), {}},
                    {build_shape({{5, 0}, {7, 0}, {7, 2}, {5, 2}}), {}},
                },
                {
                    {build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}), {}},
                    {build_shape({{5, 0}, {7, 0}, {7, 2}, {5, 2}}), {}},
                },
            }}));


struct ComputeBooleanIntersectionTestParams
{
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;

    template <class basic_json>
    static ComputeBooleanIntersectionTestParams from_json(
        basic_json& json_item)
    {
        ComputeBooleanIntersectionTestParams test_params;
        for (auto it = json_item["shapes"].begin();
                it != json_item["shapes"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.shapes.push_back(shape);
        }
        for (auto it = json_item["expected_result"].begin();
                it != json_item["expected_result"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.expected_result.push_back(shape);
        }
        return test_params;
    }

    static ComputeBooleanIntersectionTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::ComputeBooleanIntersectionTestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class ComputeBooleanIntersectionTest: public testing::TestWithParam<ComputeBooleanIntersectionTestParams> { };

TEST_P(ComputeBooleanIntersectionTest, ComputeBooleanIntersection)
{
    ComputeBooleanIntersectionTestParams test_params = GetParam();
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> result = compute_intersection(
            test_params.shapes);
    std::cout << "intersection_shapes" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanIntersectionTest,
        testing::ValuesIn(std::vector<ComputeBooleanIntersectionTestParams>{
            ComputeBooleanIntersectionTestParams::read_json(
                (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "0.json").string()),
            })
);


struct ComputeBooleanDifferenceTestParams
{
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;

    template <class basic_json>
    static ComputeBooleanDifferenceTestParams from_json(
        basic_json& json_item)
    {
        ComputeBooleanDifferenceTestParams test_params;
        test_params.shape = ShapeWithHoles::from_json(json_item["shape"]);
        for (auto it = json_item["shapes"].begin();
                it != json_item["shapes"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.shapes.push_back(shape);
        }
        for (auto it = json_item["expected_result"].begin();
                it != json_item["expected_result"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.expected_result.push_back(shape);
        }
        return test_params;
    }

    static ComputeBooleanDifferenceTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::ComputeBooleanDifferenceTestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class ComputeBooleanDifferenceTest: public testing::TestWithParam<ComputeBooleanDifferenceTestParams> { };

TEST_P(ComputeBooleanDifferenceTest, ComputeBooleanDifference)
{
    ComputeBooleanDifferenceTestParams test_params = GetParam();
    std::cout << "shape" << std::endl;
    std::cout << "- " << test_params.shape.to_string(2) << std::endl;
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> result = compute_difference(
            test_params.shape,
            test_params.shapes);
    std::cout << "difference_shapes" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanDifferenceTest,
        testing::ValuesIn(std::vector<ComputeBooleanDifferenceTestParams>{
            ComputeBooleanDifferenceTestParams::read_json(
                (fs::path("data") / "tests" / "boolean_operations" / "difference" / "0.json").string()),
            })
);


struct ComputeBooleanSymetricDifferenceTestParams
{
    ShapeWithHoles shape_1;
    ShapeWithHoles shape_2;
    std::vector<ShapeWithHoles> expected_result;

    template <class basic_json>
    static ComputeBooleanSymetricDifferenceTestParams from_json(
        basic_json& json_item)
    {
        ComputeBooleanSymetricDifferenceTestParams test_params;
        test_params.shape_1 = ShapeWithHoles::from_json(json_item["shape_1"]);
        test_params.shape_2 = ShapeWithHoles::from_json(json_item["shape_2"]);
        for (auto it = json_item["expected_result"].begin();
                it != json_item["expected_result"].end();
                ++it) {
            auto json_shape = *it;
            ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
            test_params.expected_result.push_back(shape);
        }
        return test_params;
    }

    static ComputeBooleanSymetricDifferenceTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "shape::ComputeBooleanSymetricDifferenceTestParams::read_json: "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        return from_json(json);
    }
};

class ComputeBooleanSymetricDifferenceTest: public testing::TestWithParam<ComputeBooleanSymetricDifferenceTestParams> { };

TEST_P(ComputeBooleanSymetricDifferenceTest, ComputeBooleanSymetricDifference)
{
    ComputeBooleanSymetricDifferenceTestParams test_params = GetParam();
    std::cout << "shape_1" << std::endl;
    std::cout << "- " << test_params.shape_1.to_string(2) << std::endl;
    std::cout << "shape_2" << std::endl;
    std::cout << "- " << test_params.shape_2.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> result = compute_symmetric_difference(
            test_params.shape_1,
            test_params.shape_2);
    std::cout << "symetric_difference_shapes" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanSymetricDifferenceTest,
        testing::ValuesIn(std::vector<ComputeBooleanSymetricDifferenceTestParams>{
            ComputeBooleanSymetricDifferenceTestParams::read_json(
                (fs::path("data") / "tests" / "boolean_operations" / "symetric_difference" / "0.json").string()),
            })
);

