#include "shape/simplification.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;


struct SimplificationTestParams
{
    std::vector<SimplifyInputShape> shapes;
    AreaDbl maximum_approximation_area;
    std::vector<ShapeWithHoles> expected_result;


    template <class basic_json>
    static SimplificationTestParams from_json(
            basic_json& json_item)
    {
        SimplificationTestParams test_params;
        for (const auto& json_shape: json_item["shapes"]) {
            SimplifyInputShape input_shape;
            input_shape.shape = Shape::from_json(json_shape["shape"]);
            input_shape.copies = json_shape["copies"];
            input_shape.outer = json_shape["outer"];
            test_params.shapes.push_back(input_shape);
        }
        test_params.maximum_approximation_area = json_item["maximum_approximation_area"];
        if (json_item.contains("expected_result")) {
            for (const auto& json_shape: json_item["expected_result"]) {
                ShapeWithHoles shape = ShapeWithHoles::from_json(json_shape);
                test_params.expected_result.push_back(shape);
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
                    "shape::SimplificationTestParams::read_json: "
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

    std::vector<ShapeWithHoles> result = simplify(
            test_params.shapes,
            test_params.maximum_approximation_area);

    if (!test_params.expected_result.empty())
        for (ShapePos pos = 0; pos < (ShapePos)test_params.shapes.size(); ++pos)
            EXPECT_TRUE(equal(result[pos], test_params.expected_result[pos]));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        SimplificationTest,
        testing::ValuesIn(std::vector<SimplificationTestParams>{
            SimplificationTestParams::read_json(
                    (fs::path("data") / "tests" / "simplification" / "0.json").string()),
            SimplificationTestParams::read_json(
                    (fs::path("data") / "tests" / "simplification" / "1.json").string()),
            }));
