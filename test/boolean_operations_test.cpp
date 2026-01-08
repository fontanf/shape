#include "shape/boolean_operations.hpp"

#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include "test_params.hpp"

using namespace shape;


struct FindHolesBridgesTestParams
{
    ShapeWithHoles shape;
    std::vector<ShapeElement> expected_result;

    template <class basic_json>
    static FindHolesBridgesTestParams from_json(
            basic_json& json_item)
    {
        FindHolesBridgesTestParams test_params;
        test_params.shape = ShapeWithHoles::from_json(json_item["shape"]);
        for (auto& json_element: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(ShapeElement::from_json(json_element.value()));
        return test_params;
    }

    static FindHolesBridgesTestParams read_json(
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

class FindHolesBridgesTest: public testing::TestWithParam<FindHolesBridgesTestParams> { };

TEST_P(FindHolesBridgesTest, FindHolesBridges)
{
    FindHolesBridgesTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const ShapeElement& bridge: test_params.expected_result)
        std::cout << "- " << bridge.to_string() << std::endl;

    std::vector<ShapeElement> result = find_holes_bridges(test_params.shape);
    std::cout << "result" << std::endl;
    for (const ShapeElement& bridge: result)
        std::cout << "- " << bridge.to_string() << std::endl;

    ASSERT_EQ(result.size(), test_params.expected_result.size());
    for (const ShapeElement& expected_bridge: test_params.expected_result) {
        EXPECT_NE(std::find_if(
                    result.begin(),
                    result.end(),
                    [&expected_bridge](const ShapeElement& bridge) { return equal(bridge, expected_bridge) || equal(bridge.reverse(), expected_bridge); }),
                result.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        FindHolesBridgesTest,
        testing::ValuesIn(std::vector<FindHolesBridgesTestParams>{
            {  // Shape without hole.
                {build_rectangle(200, 100)},
                {},
            }, {  // Shape with one touching hole.
                {
                    build_rectangle(200, 100),
                    {build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}})}
                },
                {},
            }, {  // Shape with one non-touching hole.
                {
                    build_rectangle(200, 100),
                    {build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}})}
                },
                {build_line_segment({0, 50}, {40, 50})},
            }, {  // Shape with one touching hole and on non-touching hole.
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}}),
                        build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}}),
                    }
                },
                {build_line_segment({20, 50}, {40, 50})},
            }, {  // Shape with two touching holes.
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}}),
                        build_shape({{20, 50}, {30, 40}, {40, 50}, {30, 60}}),
                    }
                },
                {},
            }, {  // Shape with two holes touching each other.
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{20, 50}, {30, 40}, {40, 50}, {30, 60}}),
                        build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}}),
                    }
                },
                {build_line_segment({0, 50}, {20, 50})},
            }, {  // Shape with two holes.
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{20, 50}, {30, 40}, {40, 50}, {30, 60}}),
                        build_shape({{60, 50}, {70, 40}, {80, 50}, {70, 60}}),
                    }
                },
                {
                    build_line_segment({0, 50}, {20, 50}),
                    build_line_segment({40, 50}, {60, 50}),
                },
            }}));


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


struct BridgeTouchingHolesTestParams
{
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> expected_output;


    static BridgeTouchingHolesTestParams from_json(
            nlohmann::basic_json<>& json_item)
    {
        BridgeTouchingHolesTestParams test_params;
        test_params.shape = ShapeWithHoles::from_json(json_item["shape"]);
        for (auto& json_shape: json_item["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }

    static BridgeTouchingHolesTestParams read_json(
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

class BridgeTouchingHolesTest: public testing::TestWithParam<BridgeTouchingHolesTestParams> { };

TEST_P(BridgeTouchingHolesTest, BridgeTouchingHoles)
{
    BridgeTouchingHolesTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected_output" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_output)
        std::cout << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> output = bridge_touching_holes(test_params.shape);
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << shape.to_string(2) << std::endl;
    //Writer()
    //    .add_shape_with_holes(test_params.shape)
    //    .add_shapes_with_holes(test_params.expected_output)
    //    .add_shapes_with_holes(output)
    //    .write_json("bridge_touching_holes.json");

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (const ShapeWithHoles& expected_shape: test_params.expected_output) {
        EXPECT_NE(std::find_if(
                      output.begin(),
                      output.end(),
                      [&expected_shape](const ShapeWithHoles& shape) { return equal(shape, expected_shape); }),
                  output.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        BridgeTouchingHolesTest,
        testing::ValuesIn(std::vector<BridgeTouchingHolesTestParams>{
            {  // Shape without hole.
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
                {{build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})}},
            }, {  // Shape with one hole not touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                }, {{
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                }},
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {20, 5}, {19, 6}, {18, 5}}),
                    }
                },
                {{build_shape({
                        {0, 0}, {20, 0}, {20, 5},
                        {19, 4}, {18, 5}, {19, 6},
                        {20, 5}, {20, 10}, {0, 10}})}},
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {20, 0}, {19, 5}, {20, 10}, {0, 10}}),
                    {
                        build_shape({{19, 4}, {19, 6}, {17, 6}, {17, 4}}),
                    }
                },
                {{build_shape({
                        {0, 0}, {20, 0}, {19, 5},
                        {19, 4}, {17, 4}, {17, 6}, {19, 6},
                        {19, 5}, {20, 10}, {0, 10}})}},
            }, {  // Shape with one hole touching its outline and another hole touching the first hole.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{10, 1}, {11, 2}, {9, 2}}),
                        build_shape({{10, 0}, {11, 1}, {9, 1}}),
                    }
                },
                {{build_shape({
                        {0, 0},
                        {10, 0}, {9, 1},
                        {10, 1}, {9, 2}, {11, 2}, {10, 1},
                        {11, 1}, {10, 0},
                        {20, 0}, {20, 50}, {0, 50}})}},
            }, {  // Shape with 3 holes.
                {
                    build_shape({{0, 0}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{10, 1}, {11, 2}, {9, 2}}),
                        build_shape({{10, 0}, {11, 1}, {9, 1}}),
                        build_shape({{10, 2}, {11, 3}, {9, 3}}),
                    }
                },
                {{build_shape({
                        {0, 0},
                        {10, 0}, {9, 1},
                        {10, 1}, {9, 2},
                        {10, 2}, {9, 3}, {11, 3}, {10, 2},
                        {11, 2}, {10, 1},
                        {11, 1}, {10, 0},
                        {20, 0}, {20, 50}, {0, 50}})}},
            }, {  // Shape with one hole touching its outline.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                    }
                },
                {{build_shape({
                        {0, 0},
                        {10, 1}, {9, 1}, {10, 2}, {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})}},
            }, {  // Shape with one hole touching its outline and another hole touching the first hole.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 2}, {11, 2}, {10, 3}}),
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                    }
                },
                {{build_shape({
                        {0, 0},
                        {10, 1}, {9, 1},
                        {10, 2}, {9, 2}, {10, 3}, {11, 2}, {10, 2},
                        {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})}},
            }, {  // Shape with 3 holes.
                {
                    build_shape({{0, 0}, {10, 1}, {20, 0}, {20, 50}, {0, 50}}),
                    {
                        build_shape({{9, 2}, {11, 2}, {10, 3}}),
                        build_shape({{9, 1}, {11, 1}, {10, 2}}),
                        build_shape({{9, 3}, {11, 3}, {10, 4}}),
                    }
                },
                {{build_shape({
                        {0, 0},
                        {10, 1}, {9, 1},
                        {10, 2}, {9, 2},
                        {10, 3}, {9, 3}, {10, 4}, {11, 3}, {10, 3},
                        {11, 2}, {10, 2},
                        {11, 1}, {10, 1},
                        {20, 0}, {20, 50}, {0, 50}})}},
            },
            BridgeTouchingHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "shape_with_holes" / "bridge_touching_holes" / "0.json").string()),
            }));
