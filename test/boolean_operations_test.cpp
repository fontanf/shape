#include "shape/boolean_operations.hpp"

#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include "test_params.hpp"

using namespace shape;


struct ComputeBooleanUnionTestParams: TestParams<ComputeBooleanUnionTestParams>
{
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;


    static ComputeBooleanUnionTestParams from_json(
            nlohmann::basic_json<>& json_item)
    {
        ComputeBooleanUnionTestParams test_params = TestParams::from_json(json_item);
        for (auto& json_shape: json_item["shapes"].items())
            test_params.shapes.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        for (auto& json_shape: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

class ComputeBooleanUnionTest: public testing::TestWithParam<ComputeBooleanUnionTestParams> { };

TEST_P(ComputeBooleanUnionTest, ComputeBooleanUnion)
{
    ComputeBooleanUnionTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << " (" << test_params.description << ")" << "..." << std::endl;
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    auto result = compute_union(
            test_params.shapes);
    std::cout << "result" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    if (test_params.write_json || test_params.write_svg) {
        std::string base_filename = "union_" + fs::path(test_params.name).filename().replace_extension("").string();

        if (test_params.write_json) {
            Writer().add_shapes_with_holes(test_params.shapes).write_json(base_filename + "_shapes.json");
            Writer().add_shapes_with_holes(test_params.expected_result).write_json(base_filename + "_expected_result.json");
            Writer().add_shapes_with_holes(result).write_json(base_filename + "_result.json");
        }
        if (test_params.write_svg) {
            Writer().add_shapes_with_holes(test_params.shapes).write_svg(base_filename + "_shapes.svg");
            Writer().add_shapes_with_holes(test_params.expected_result).write_svg(base_filename + "_expected_result.svg");
            Writer().add_shapes_with_holes(result).write_svg(base_filename + "_result.svg");
        }

    }

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
        testing::ValuesIn(ComputeBooleanUnionTestParams::read_dir((fs::path("data") / "tests" / "boolean_operations" / "union").string())));


struct ComputeBooleanIntersectionTestParams: TestParams<ComputeBooleanIntersectionTestParams>
{
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;


    static ComputeBooleanIntersectionTestParams from_json(
        nlohmann::basic_json<>& json_item)
    {
        ComputeBooleanIntersectionTestParams test_params = TestParams::from_json(json_item);
        for (auto& json_shape: json_item["shapes"].items())
            test_params.shapes.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        for (auto& json_shape: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

class ComputeBooleanIntersectionTest: public testing::TestWithParam<ComputeBooleanIntersectionTestParams> { };

TEST_P(ComputeBooleanIntersectionTest, ComputeBooleanIntersection)
{
    ComputeBooleanIntersectionTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << " (" << test_params.description << ")" << "..." << std::endl;
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    auto result = compute_intersection(
            test_params.shapes);
    std::cout << "result" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    if (test_params.write_json || test_params.write_svg) {
        std::string base_filename = "intersection_" + fs::path(test_params.name).filename().replace_extension("").string();

        if (test_params.write_json) {
            Writer().add_shapes_with_holes(test_params.shapes).write_json(base_filename + "_shapes.json");
            Writer().add_shapes_with_holes(test_params.expected_result).write_json(base_filename + "_expected_result.json");
            Writer().add_shapes_with_holes(result).write_json(base_filename + "_result.json");
        }
        if (test_params.write_svg) {
            Writer().add_shapes_with_holes(test_params.shapes).write_svg(base_filename + "_shapes.svg");
            Writer().add_shapes_with_holes(test_params.expected_result).write_svg(base_filename + "_expected_result.svg");
            Writer().add_shapes_with_holes(result).write_svg(base_filename + "_result.svg");
        }

    }

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
        testing::ValuesIn(ComputeBooleanIntersectionTestParams::read_dir((fs::path("data") / "tests" / "boolean_operations" / "intersection").string())));


struct ComputeBooleanDifferenceTestParams : TestParams<ComputeBooleanDifferenceTestParams>
{
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_result;


    static ComputeBooleanDifferenceTestParams from_json(
        nlohmann::basic_json<>& json_item)
    {
        ComputeBooleanDifferenceTestParams test_params = TestParams::from_json(json_item);
        test_params.shape = ShapeWithHoles::from_json(json_item["shape"]);
        for (auto& json_shape: json_item["shapes"].items())
            test_params.shapes.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        for (auto& json_shape: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

class ComputeBooleanDifferenceTest: public testing::TestWithParam<ComputeBooleanDifferenceTestParams> { };

TEST_P(ComputeBooleanDifferenceTest, ComputeBooleanDifference)
{
    ComputeBooleanDifferenceTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << " (" << test_params.description << ")" << "..." << std::endl;
    std::cout << "shape" << std::endl;
    std::cout << "- " << test_params.shape.to_string(2) << std::endl;
    std::cout << "shapes" << std::endl;
    for (const ShapeWithHoles& shape: test_params.shapes)
        std::cout << "- " << shape.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    auto result = compute_difference(
            test_params.shape,
            test_params.shapes);
    std::cout << "result" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    test_params.write_json = true;
    if (test_params.write_json || test_params.write_svg) {
        std::string base_filename = "difference_" + fs::path(test_params.name).filename().replace_extension("").string();

        if (test_params.write_json) {
            Writer().add_shape_with_holes(test_params.shape).add_shapes_with_holes(test_params.shapes).write_json(base_filename + "_shapes.json");
            Writer().add_shapes_with_holes(test_params.expected_result).write_json(base_filename + "_expected_result.json");
            Writer().add_shapes_with_holes(result).write_json(base_filename + "_result.json");
        }
        if (test_params.write_svg) {
            Writer().add_shape_with_holes(test_params.shape).add_shapes_with_holes(test_params.shapes).write_svg(base_filename + "_shapes.svg");
            Writer().add_shapes_with_holes(test_params.expected_result).write_svg(base_filename + "_expected_result.svg");
            Writer().add_shapes_with_holes(result).write_svg(base_filename + "_result.svg");
        }
    }

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
        testing::ValuesIn(ComputeBooleanDifferenceTestParams::read_dir((fs::path("data") / "tests" / "boolean_operations" / "difference").string())));


struct ComputeBooleanSymmetricDifferenceTestParams : TestParams<ComputeBooleanSymmetricDifferenceTestParams>
{
    ShapeWithHoles shape_1;
    ShapeWithHoles shape_2;
    std::vector<ShapeWithHoles> expected_result;


    static ComputeBooleanSymmetricDifferenceTestParams from_json(
        nlohmann::basic_json<>& json_item)
    {
        ComputeBooleanSymmetricDifferenceTestParams test_params = TestParams::from_json(json_item);
        test_params.shape_1 = ShapeWithHoles::from_json(json_item["shape_1"]);
        test_params.shape_2 = ShapeWithHoles::from_json(json_item["shape_2"]);
        for (auto& json_shape: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

class ComputeBooleanSymmetricDifferenceTest: public testing::TestWithParam<ComputeBooleanSymmetricDifferenceTestParams> { };

TEST_P(ComputeBooleanSymmetricDifferenceTest, ComputeBooleanSymetricDifference)
{
    ComputeBooleanSymmetricDifferenceTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << " (" << test_params.description << ")" << "..." << std::endl;
    std::cout << "shape_1" << std::endl;
    std::cout << "- " << test_params.shape_1.to_string(2) << std::endl;
    std::cout << "shape_2" << std::endl;
    std::cout << "- " << test_params.shape_2.to_string(2) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    auto result = compute_symmetric_difference(
            test_params.shape_1,
            test_params.shape_2);
    std::cout << "result" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << "- " << shape.to_string(2) << std::endl;

    if (test_params.write_json || test_params.write_svg) {
        std::string base_filename = "symmetric_difference_" + fs::path(test_params.name).filename().replace_extension("").string();

        if (test_params.write_json) {
            Writer().add_shape_with_holes(test_params.shape_1).add_shape_with_holes(test_params.shape_2).write_json(base_filename + "_shapes.json");
            Writer().add_shapes_with_holes(test_params.expected_result).write_json(base_filename + "_expected_result.json");
            Writer().add_shapes_with_holes(result).write_json(base_filename + "_result.json");
        }
        if (test_params.write_svg) {
            Writer().add_shape_with_holes(test_params.shape_1).add_shape_with_holes(test_params.shape_2).write_svg(base_filename + "_shapes.svg");
            Writer().add_shapes_with_holes(test_params.expected_result).write_svg(base_filename + "_expected_result.svg");
            Writer().add_shapes_with_holes(result).write_svg(base_filename + "_result.svg");
        }

    }

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
        ComputeBooleanSymmetricDifferenceTest,
        testing::ValuesIn(ComputeBooleanSymmetricDifferenceTestParams::read_dir((fs::path("data") / "tests" / "boolean_operations" / "symmetric_difference").string())));

