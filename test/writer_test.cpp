#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <fstream>

using namespace shape;

struct WriterWriteJsonTestParams
{
    std::vector<Point> points;
    std::vector<ShapeElement> elements;
    std::vector<Shape> shapes;
    std::vector<ShapeWithHoles> shapes_with_holes;
};

void PrintTo(const WriterWriteJsonTestParams& params, std::ostream* os)
{
    *os << "points (" << params.points.size() << ")\n";
    for (const Point& point: params.points)
        *os << "- " << point.to_string() << "\n";
    *os << "elements (" << params.elements.size() << ")\n";
    for (const ShapeElement& element: params.elements)
        *os << "- " << element.to_string() << "\n";
    *os << "shapes (" << params.shapes.size() << ")\n";
    for (const Shape& shape: params.shapes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "shapes_with_holes (" << params.shapes_with_holes.size() << ")\n";
    for (const ShapeWithHoles& shape: params.shapes_with_holes)
        *os << "- " << shape.to_string(2) << "\n";
}

class WriterWriteJsonTest: public testing::TestWithParam<WriterWriteJsonTestParams> { };

TEST_P(WriterWriteJsonTest, WriterWriteJson)
{
    WriterWriteJsonTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    std::string file_path = testing::TempDir() + "writer_write_json_test.json";

    Writer writer;
    for (const Point& point: test_params.points)
        writer.add_point(point, "point");
    for (const ShapeElement& element: test_params.elements)
        writer.add_element(element, "element");
    for (const Shape& shape: test_params.shapes)
        writer.add_shape(shape, "shape");
    for (const ShapeWithHoles& shape: test_params.shapes_with_holes)
        writer.add_shape_with_holes(shape, "shape_with_holes");
    writer.write_json(file_path);

    std::ifstream file(file_path);
    nlohmann::json json;
    file >> json;

    ASSERT_EQ(json.value("points", nlohmann::json::array()).size(), test_params.points.size());
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)test_params.points.size();
            ++point_pos) {
        EXPECT_EQ(Point::from_json(json["points"][point_pos]), test_params.points[point_pos]);
        EXPECT_EQ(json["points"][point_pos]["label"], "point");
    }
    ASSERT_EQ(json.value("elements", nlohmann::json::array()).size(), test_params.elements.size());
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)test_params.elements.size();
            ++element_pos) {
        EXPECT_EQ(ShapeElement::from_json(json["elements"][element_pos]), test_params.elements[element_pos]);
        EXPECT_EQ(json["elements"][element_pos]["label"], "element");
    }
    ASSERT_EQ(json.value("shapes", nlohmann::json::array()).size(), test_params.shapes.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)test_params.shapes.size();
            ++shape_pos) {
        EXPECT_EQ(Shape::from_json(json["shapes"][shape_pos]), test_params.shapes[shape_pos]);
        EXPECT_EQ(json["shapes"][shape_pos]["label"], "shape");
    }
    ASSERT_EQ(json.value("shapes_with_holes", nlohmann::json::array()).size(), test_params.shapes_with_holes.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)test_params.shapes_with_holes.size();
            ++shape_pos) {
        EXPECT_EQ(ShapeWithHoles::from_json(json["shapes_with_holes"][shape_pos]), test_params.shapes_with_holes[shape_pos]);
        EXPECT_EQ(json["shapes_with_holes"][shape_pos]["label"], "shape_with_holes");
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        WriterWriteJsonTest,
        testing::ValuesIn(std::vector<WriterWriteJsonTestParams>{
            {  // 0. Points only.
                {{1, 2}, {3, 4}},
                {},
                {},
                {},
            }, {  // 1. Everything.
                {{1, 2}},
                {build_line_segment({0, 0}, {1, 1})},
                {build_shape({{0, 0}, {1, 0}, {1, 1}})},
                {{build_rectangle(0, 4, 0, 4), {build_rectangle(1, 2, 1, 2)}}},
            },
        }),
        [](const testing::TestParamInfo<WriterWriteJsonTest::ParamType>& info) {
            return std::to_string(info.index);
        });
