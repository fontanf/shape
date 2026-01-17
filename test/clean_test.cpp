#include "shape/clean.hpp"

//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct RemoveRedundantVerticesTestParams
{
    Shape shape;
    Shape expected_shape;
};

class RemoveRedundantVerticesTest: public testing::TestWithParam<RemoveRedundantVerticesTestParams> { };

TEST_P(RemoveRedundantVerticesTest, RemoveRedundantVertices)
{
    RemoveRedundantVerticesTestParams test_params = GetParam();
    Shape cleaned_shape = remove_redundant_vertices(test_params.shape).second;
    std::cout << cleaned_shape.to_string(0) << std::endl;
    EXPECT_EQ(test_params.expected_shape, cleaned_shape);
}

INSTANTIATE_TEST_SUITE_P(
        ,
        RemoveRedundantVerticesTest,
        testing::ValuesIn(std::vector<RemoveRedundantVerticesTestParams>{
            {
                build_shape({{0, 0}, {0, 0}, {100, 0}, {100, 100}}),
                build_shape({{0, 0}, {100, 0}, {100, 100}})
            }}));


struct CleanExtremeSlopesOuterTestParams
{
    Shape shape;
    ShapeWithHoles expected_output;


    template <class basic_json>
    static CleanExtremeSlopesOuterTestParams from_json(
            basic_json& json_item)
    {
        CleanExtremeSlopesOuterTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.expected_output = ShapeWithHoles::from_json(json_item["expected_output"]);
        return test_params;
    }

    static CleanExtremeSlopesOuterTestParams read_json(
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

class CleanExtremeSlopesOuterTest: public testing::TestWithParam<CleanExtremeSlopesOuterTestParams> { };

TEST_P(CleanExtremeSlopesOuterTest, CleanExtremeSlopesOuter)
{
    CleanExtremeSlopesOuterTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected output " << test_params.expected_output.to_string(0) << std::endl;
    ShapeWithHoles output = clean_extreme_slopes_outer(test_params.shape);
    std::cout << "output " << output.to_string(0) << std::endl;
    //Writer().add_shape_with_holes(output).write_json("clean_extreme_slopes_outer_output.json");

    ASSERT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        CleanExtremeSlopesOuterTest,
        testing::ValuesIn(std::vector<CleanExtremeSlopesOuterTestParams>{
            {
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                },
            },
            CleanExtremeSlopesOuterTestParams::read_json(
                    (fs::path("data") / "tests" / "clean" / "clean_extreme_slopes_outer" / "0.json").string()),
            }));


struct FixSelfIntersectionsTestParams
{
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> expected_result;
};

class FixSelfIntersectionsTest: public testing::TestWithParam<FixSelfIntersectionsTestParams> { };

TEST_P(FixSelfIntersectionsTest, FixSelfIntersections)
{
    FixSelfIntersectionsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(2) << std::endl;
    std::cout << "expected shapes:" << std::endl;
    for (const ShapeWithHoles& shape: test_params.expected_result)
        std::cout << shape.to_string(2) << std::endl;
    std::vector<ShapeWithHoles> result = fix_self_intersections(test_params.shape);
    std::cout << "result:" << std::endl;
    for (const ShapeWithHoles& shape: result)
        std::cout << shape.to_string(2) << std::endl;

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
        FixSelfIntersectionsTest,
        testing::ValuesIn(std::vector<FixSelfIntersectionsTestParams>{
            {
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                },
            }, {
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                    {build_shape({{1, 1}, {2, 1}, {2, 2}, {1, 2}})},
                },
            },
            }));
