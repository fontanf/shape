#include "shape/approximation.hpp"

//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include "test_params.hpp"

using namespace shape;


struct ApproximateCircularArcByLineSegmentsTestParams
{
    ShapeElement circular_arc;
    LengthDbl number_of_line_segments;
    bool outer;
    std::vector<ShapeElement> expected_line_segments;
};

class ApproximateCircularArcByLineSegmentsTest: public testing::TestWithParam<ApproximateCircularArcByLineSegmentsTestParams> { };

TEST_P(ApproximateCircularArcByLineSegmentsTest, ApproximateCircularArcByLineSegments)
{
    ApproximateCircularArcByLineSegmentsTestParams test_params = GetParam();
    std::cout << "circular_arc" << std::endl;
    std::cout << test_params.circular_arc.to_string() << std::endl;
    std::cout << "expected_line_segments" << std::endl;
    for (const ShapeElement& line_segment: test_params.expected_line_segments)
        std::cout << line_segment.to_string() << std::endl;
    LengthDbl segment_length = test_params.circular_arc.length() / test_params.number_of_line_segments;
    std::vector<ShapeElement> line_segments = approximate_circular_arc_by_line_segments(
            test_params.circular_arc,
            segment_length,
            test_params.outer);
    std::cout << "line_segments" << std::endl;
    for (const ShapeElement& line_segment: line_segments)
        std::cout << line_segment.to_string() << std::endl;

    ASSERT_EQ(line_segments.size(), test_params.number_of_line_segments);
    for (ElementPos pos = 0; pos < test_params.number_of_line_segments; ++pos) {
        //std::cout << std::setprecision (15) << line_segments[pos].start.x << std::endl;
        EXPECT_TRUE(equal(line_segments[pos], test_params.expected_line_segments[pos]));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ApproximateCircularArcByLineSegmentsTest,
        testing::ValuesIn(std::vector<ApproximateCircularArcByLineSegmentsTestParams>{
            {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                1,
                false,
                build_shape({{1, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                1,
                true,
                build_shape({{1, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                2,
                true,
                build_shape({{1, 0}, {1, 1}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                2,
                false,
                build_shape({{1, 0}, {0, 0}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}}, true).elements.front(),
                3,
                true,
                build_shape({{1, 0}, {1, 0.414213562373095}, {0.414213562373095, 1}, {0, 1}}, true).elements
            }, {
                build_shape({{1, 0}, {1, 1, -1}, {0, 1}}, true).elements.front(),
                3,
                false,
                build_shape({{1, 0}, {0.585786437626905, 0}, {0, 0.585786437626905}, {0, 1}}, true).elements
            }}));


struct ApproximateShapeByLineSegmentsTestParams
{
    Shape shape;
    LengthDbl segment_length;
    bool outer;
    ShapeWithHoles expected_output;


    static ApproximateShapeByLineSegmentsTestParams from_json(
            nlohmann::basic_json<>& json_item)
    {
        ApproximateShapeByLineSegmentsTestParams test_params;
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.segment_length = json_item["segment_length"];
        test_params.outer = json_item["outer"];
        if (json_item.contains("expected_output"))
            test_params.expected_output = ShapeWithHoles::from_json(json_item["expected_output"]);
        return test_params;
    }

    static ApproximateShapeByLineSegmentsTestParams read_json(
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

class ApproximateShapeByLineSegmentsTest: public testing::TestWithParam<ApproximateShapeByLineSegmentsTestParams> { };

TEST_P(ApproximateShapeByLineSegmentsTest, ApproximateShapeByLineSegments)
{
    ApproximateShapeByLineSegmentsTestParams test_params = GetParam();
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "segment_length " << shape::to_string(test_params.segment_length) << std::endl;
    std::cout << "outer " << test_params.outer << std::endl;
    std::cout << "expected_output " << test_params.expected_output.to_string(0) << std::endl;
    //Writer writer;
    //writer.add_shape(test_params.shape);
    //if (!test_params.expected_output.shape.elements.empty())
    //    writer.add_shape_with_holes(test_params.expected_output);
    //writer.write_json("approximate_shape_by_line_segments_input.json");;

    auto output = approximate_shape_by_line_segments(
        test_params.shape,
        test_params.segment_length,
        test_params.outer);
    std::cout << "output " << output.to_string(0) << std::endl;
    //Writer().add_shape_with_holes(output).write_json("approximate_shape_by_line_segments_output.json");

    EXPECT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ApproximateShapeByLineSegmentsTest,
        testing::ValuesIn(std::vector<ApproximateShapeByLineSegmentsTestParams>{
            {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}, {0, 0}}),
                1.0,
                true,
                {
                    build_shape({{1, 0}, {1, 1}, {0, 1}, {0, 0}}),
                },
            },
            {
                build_shape({{1, 0}, {0, 0, 1}, {0, 1}, {0, 0}}),
                2,
                false,
                {
                    build_shape({{1, 0}, {0, 1}, {0, 0}}),
                },
            },
            ApproximateShapeByLineSegmentsTestParams::read_json(
                    (fs::path("data") / "tests" / "approximation" / "approximate_shape_by_line_segments" / "0.json").string()),
            ApproximateShapeByLineSegmentsTestParams::read_json(
                    (fs::path("data") / "tests" / "approximation" / "approximate_shape_by_line_segments" / "1.json").string()),
            }));
