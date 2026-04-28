#include "shape/clean.hpp"

//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct RemoveRedundantVerticesTestParams
{
    std::string name;
    Shape shape;
    Shape expected_shape;
};

void PrintTo(const RemoveRedundantVerticesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_shape " << params.expected_shape.to_string(0) << "\n";
}

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
                "DuplicateVertexShape",
                build_shape({{0, 0}, {0, 0}, {100, 0}, {100, 100}}),
                build_shape({{0, 0}, {100, 0}, {100, 100}}),
            }, {
                "DuplicateVertexPath",
                build_path({{0, 0}, {0, 0}, {100, 0}}),
                build_path({{0, 0}, {100, 0}}),
            }}),
        [](const testing::TestParamInfo<RemoveRedundantVerticesTest::ParamType>& info) {
            return info.param.name;
        });


struct CleanExtremeSlopesOuterTestParams
{
    std::string name;
    Shape shape;
    ShapeWithHoles expected_output;


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
        CleanExtremeSlopesOuterTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.expected_output = ShapeWithHoles::from_json(json["expected_output"]);
        return test_params;
    }
};

void PrintTo(const CleanExtremeSlopesOuterTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected output " << params.expected_output.to_string(0) << "\n";
}

class CleanExtremeSlopesOuterTest: public testing::TestWithParam<CleanExtremeSlopesOuterTestParams> { };

TEST_P(CleanExtremeSlopesOuterTest, CleanExtremeSlopesOuter)
{
    CleanExtremeSlopesOuterTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
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
                "Square",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                },
            },
            CleanExtremeSlopesOuterTestParams::read_json(
                    (fs::path("data") / "tests" / "clean" / "clean_extreme_slopes_outer" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<CleanExtremeSlopesOuterTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct FixSelfIntersectionsTestParams
{
    std::string name;
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> expected_output;
};

void PrintTo(const FixSelfIntersectionsTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected shapes:\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << shape.to_string(2) << "\n";
}

class FixSelfIntersectionsTest: public testing::TestWithParam<FixSelfIntersectionsTestParams> { };

TEST_P(FixSelfIntersectionsTest, FixSelfIntersections)
{
    FixSelfIntersectionsTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    std::vector<ShapeWithHoles> output = fix_self_intersections(test_params.shape);
    std::cout << "output:" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << shape.to_string(2) << std::endl;

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
        FixSelfIntersectionsTest,
        testing::ValuesIn(std::vector<FixSelfIntersectionsTestParams>{
            {
                "Square",
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                },
            }, {
                "TouchingSquares",
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 1}, {0, 1}})},
                {
                    {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
                    {build_shape({{1, 1}, {2, 1}, {2, 2}, {1, 2}})},
                },
            },
        }),
        [](const testing::TestParamInfo<FixSelfIntersectionsTest::ParamType>& info) {
            return info.param.name;
        });
