/*

#include "shape/simplification.hpp"

#include "shape/shapes_intersections.hpp"
//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;

struct SimplificationTestParams
{
    std::vector<SimplifyInputShape> shapes;
    AreaDbl maximum_approximation_area;
    std::vector<ShapeWithHoles> expected_output;


    template <class basic_json>
    static SimplificationTestParams from_json(
            basic_json& json_item)
    {
        SimplificationTestParams test_params;
        for (const auto& json_shape: json_item["shapes"]) {
            SimplifyInputShape input_shape;
            input_shape.shape = ShapeWithHoles::from_json(json_shape["shape"]);
            input_shape.copies = json_shape["copies"];
            test_params.shapes.push_back(input_shape);
        }
        test_params.maximum_approximation_area = json_item["maximum_approximation_area"];
        if (json_item.contains("expected_output")) {
            for (const auto& json_shape: json_item["expected_output"]) {
                ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
                test_params.expected_output.push_back(shape);
            }
        }
        return test_params;
    }

    static SimplificationTestParams read_json(
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

class SimplificationTest: public testing::TestWithParam<SimplificationTestParams> { };

TEST_P(SimplificationTest, Simplification)
{
    SimplificationTestParams test_params = GetParam();

    //Writer writer;
    //for (const SimplifyInputShape& shape: test_params.shapes)
    //    writer.add_shape(shape.shape);
    //writer.write_json("simplify_input.json");

    std::vector<ShapeWithHoles> output = simplify(
            test_params.shapes,
            test_params.maximum_approximation_area);

    if (!test_params.expected_output.empty())
        for (ShapePos pos = 0; pos < (ShapePos)test_params.shapes.size(); ++pos)
            EXPECT_TRUE(equal(output[pos], test_params.expected_output[pos]));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        SimplificationTest,
        testing::ValuesIn(std::vector<SimplificationTestParams>{
            //SimplificationTestParams::read_json(
            //        (fs::path("data") / "tests" / "simplification" / "0.json").string()),
            //SimplificationTestParams::read_json(
            //        (fs::path("data") / "tests" / "simplification" / "1.json").string()),
            //SimplificationTestParams::read_json(
            //        (fs::path("data") / "tests" / "simplification" / "2.json").string()),
            }));

*/
