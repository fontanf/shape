//#define OFFSET_TEST_DEBUG

#include "shape/offset.hpp"

#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

using namespace shape;


struct InflateShapeTestParams
{
    std::string name;
    Shape shape;
    LengthDbl offset;
    ShapeWithHoles expected_output;


    static InflateShapeTestParams read_json(
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
        InflateShapeTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.offset = json["offset"];
        if (json.contains("expected_output"))
            test_params.expected_output = ShapeWithHoles::from_json(json["expected_output"]);
        return test_params;
    }
};

void PrintTo(const InflateShapeTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "offset " << params.offset << "\n";
    *os << "expected_output " << params.expected_output.to_string(0) << "\n";
}

class InflateShapeTest: public testing::TestWithParam<InflateShapeTestParams> { };

TEST_P(InflateShapeTest, InflateShape)
{
    InflateShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef OFFSET_TEST_DEBUG
    Writer writer;
    writer.add_shape(test_params.shape);
    if (!test_params.expected_output.shape.elements.empty())
        writer.add_shape_with_holes(test_params.expected_output);
    writer.write_json("inflate_shape_input.json");;
#endif

    auto output = inflate(
        test_params.shape,
        test_params.offset);
    std::cout << "output " << output.to_string(0) << std::endl;
#ifdef OFFSET_TEST_DEBUG
    writer.add_shape_with_holes(output).write_json("inflate_shape_output.json");
#endif

    EXPECT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        InflateShapeTest,
        testing::ValuesIn(std::vector<InflateShapeTestParams>{
            {
                "VerticalSegment",
                build_path({{0, 0}, {0, 10}}),
                1.0,
                {
                    build_shape({{-1, 0}, {0, 0, 1}, {1, 0}, {1, 10}, {0, 10, 1}, {-1, 10}}),
                },
            },
            {
                "DiagonalSegment",
                build_path({{6, 5}, {7, 13}}),
                1e-3,
                {
                    build_shape({
                            {6.000992277876714, 4.999875965265411},
                            {7.000992277876714, 12.99987596526541},
                            {7, 13, 1},
                            {6.999007722123286, 13.00012403473459},
                            {5.999007722123286, 5.000124034734589},
                            {6, 5, 1}}),
                },
            },
            {
                "TwoSegmentPath",
                build_path({{8, 0}, {10, 20}, {12, -10}}),
                1e-3,
                {
                    build_shape({
                            {8.00099503719021, -9.950371902099892e-05},
                            {9.99980066316971, 19.98795675607598},
                            {11.99900221484214, -10.00006651901052},
                            {12, -10, 1},
                            {12.00099778515786, -9.999933480989476},
                            {10.00099778515786, 20.00006651901052},
                            {10.00000000004284, 20.00000000000286, 1},
                            {9.99900496280979, 20.00009950371902},
                            {7.99900496280979, 9.950371902099892e-05},
                            {8, 0, 1}}),
                },
            },
            {
                "ArcPath1",
                build_path({{1, 0}, {0, 0, 1}, {0, 1}}),
                1,
                {
                    build_shape({
                            {0, 0},
                            {1, 0, 1},
                            {2, 0},
                            {0, 0, 1},
                            {0, 2},
                            {0, 1, 1}})
                },
            },
            {
                "ArcPath2",
                build_path({{1, 0}, {0, 0, 1}, {0, 1}}),
                2,
                {
                    build_shape({
                            {-0.82287565553229536, -0.82287565553229536},
                            {1, 0, 1},
                            {3, 0},
                            {0, 0, 1},
                            {0, 3},
                            {0, 1, 1}})
                },
            },
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "0.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "1.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "2.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "3.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "4.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "5.json").string()),
            InflateShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate_shape" / "6.json").string()),
        }),
        [](const testing::TestParamInfo<InflateShapeTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct InflateShapeWithHolesTestParams
{
    std::string name;
    ShapeWithHoles shape;
    LengthDbl offset = 0;
    ShapeWithHoles expected_output;


    static InflateShapeWithHolesTestParams read_json(
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
        InflateShapeWithHolesTestParams test_params;
        test_params.name = file_path;
        test_params.shape = ShapeWithHoles::from_json(json["shape"]);
        test_params.offset = json["offset"];
        if (json.contains("expected_output"))
            test_params.expected_output = ShapeWithHoles::from_json(json["expected_output"]);
        return test_params;
    }
};

void PrintTo(const InflateShapeWithHolesTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "offset " << params.offset << "\n";
    *os << "expected_output " << params.expected_output.to_string(0) << "\n";
}

class InflateShapeWithHolesTest: public testing::TestWithParam<InflateShapeWithHolesTestParams> { };

TEST_P(InflateShapeWithHolesTest, InflateShapeWithHoles)
{
    InflateShapeWithHolesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
#ifdef OFFSET_TEST_DEBUG
    Writer().add_shape_with_holes(test_params.shape).add_shape_with_holes(test_params.expected_output).write_json("inflate_input.json");
#endif

    if (!test_params.shape.shape.check()) {
        throw std::invalid_argument(FUNC_SIGNATURE);
    }
    auto output = inflate(
        test_params.shape,
        test_params.offset);
    std::cout << "output " << output.to_string(0) << std::endl;
#ifdef OFFSET_TEST_DEBUG
    Writer().add_shape_with_holes(test_params.shape).add_shape_with_holes(output).write_json("inflate_output.json");
#endif

    EXPECT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        InflateShapeWithHolesTest,
        testing::ValuesIn(std::vector<InflateShapeWithHolesTestParams>{
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "000.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "001.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "002.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "003.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "004.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "005.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "006.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "007.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "008.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "009.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "010.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "011.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "012.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "013.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "014.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "015.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "016.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "017.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "018.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "019.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "020.json").string()),
            InflateShapeWithHolesTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "inflate" / "021.json").string()),
        }),
        [](const testing::TestParamInfo<InflateShapeWithHolesTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct DeflateTestParams
{
    std::string name;
    Shape shape;
    LengthDbl offset = 0;
    std::vector<Shape> expected_output;


    static DeflateTestParams read_json(
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
        DeflateTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.offset = json["offset"];
        if (json.contains("expected_output"))
            for (auto& json_shape: json["expected_output"].items())
                test_params.expected_output.emplace_back(Shape::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const DeflateTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "hole " << params.shape.to_string(0) << "\n";
    *os << "offset " << params.offset << "\n";
    *os << "expected_output\n";
    for (const Shape& hole: params.expected_output)
        *os << "- " << hole.to_string(2) << "\n";
}

class DeflateTest: public testing::TestWithParam<DeflateTestParams> { };

TEST_P(DeflateTest, Deflate)
{
    DeflateTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    auto output = deflate(
        test_params.shape,
        test_params.offset);
    std::cout << "output" << std::endl;
    for (const Shape& hole: output)
        std::cout << "- " << hole.to_string(2) << std::endl;

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (const Shape& expected_hole: test_params.expected_output) {
        EXPECT_NE(std::find(
                      output.begin(),
                      output.end(),
                      expected_hole),
                  output.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        DeflateTest,
        testing::ValuesIn(std::vector<DeflateTestParams>{
            DeflateTestParams::read_json(
                    (fs::path("data") / "tests" / "offset" / "deflate" / "000.json").string()),
        }),
        [](const testing::TestParamInfo<DeflateTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });
