#include "shape/clean.hpp"

//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cmath>
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
            }, {
                "SingleLineSegmentPath",
                build_path({{0, 0}, {100, 0}}),
                build_path({{0, 0}, {100, 0}}),
            }, {
                "CirclePath",
                []() { Shape circle = build_circle(100); circle.is_path = true; return circle; }(),
                []() { Shape circle = build_circle(100); circle.is_path = true; return circle; }(),
            }}),
        [](const testing::TestParamInfo<RemoveRedundantVerticesTest::ParamType>& info) {
            return info.param.name;
        });


struct RemoveAlignedVerticesTestParams
{
    std::string name;
    Shape shape;
    Shape expected_shape;
};

void PrintTo(const RemoveAlignedVerticesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_shape " << params.expected_shape.to_string(0) << "\n";
}

class RemoveAlignedVerticesTest: public testing::TestWithParam<RemoveAlignedVerticesTestParams> { };

TEST_P(RemoveAlignedVerticesTest, RemoveAlignedVertices)
{
    RemoveAlignedVerticesTestParams test_params = GetParam();
    Shape cleaned_shape = remove_aligned_vertices(test_params.shape).second;
    std::cout << cleaned_shape.to_string(1) << std::endl;
    EXPECT_EQ(cleaned_shape.is_path, test_params.expected_shape.is_path);
    EXPECT_EQ(test_params.expected_shape, cleaned_shape);
}

INSTANTIATE_TEST_SUITE_P(
        ,
        RemoveAlignedVerticesTest,
        testing::ValuesIn(std::vector<RemoveAlignedVerticesTestParams>{
            {
                "AlignedVertexShape",
                build_shape({{0, 0}, {1, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
            }, {
                "AlignedVertexPath",
                build_path({{0, 0}, {1, 1}, {2, 0}, {3, 0}, {4, 0}, {5, 1}}),
                build_path({{0, 0}, {1, 1}, {2, 0}, {4, 0}, {5, 1}}),
            }, {
                // The first element of a path is not preceded by its last
                // element, even if they are aligned.
                "PathStartAlignedWithEnd",
                build_path({{0, 0}, {1, 0}, {1, 1}, {-2, 0}, {-1, -1}}),
                build_path({{0, 0}, {1, 0}, {1, 1}, {-2, 0}, {-1, -1}}),
            }, {
                // The path still ends at the end of its last element when
                // the start of this element is removed.
                "AlignedVertexAtPathEnd",
                build_path({{0, 0}, {1, 0}, {2, 5}, {3, 5}, {4, 5}}),
                build_path({{0, 0}, {1, 0}, {2, 5}, {4, 5}}),
            }}),
        [](const testing::TestParamInfo<RemoveAlignedVerticesTest::ParamType>& info) {
            return info.param.name;
        });


/** Points of a circle of center (0, radius), from its bottom anticlockwise. */
std::vector<BuildShapeElement> build_circle_bottom_points(
        LengthDbl radius,
        LengthDbl spacing,
        ElementPos number_of_points)
{
    std::vector<BuildShapeElement> points;
    for (ElementPos pos = 0; pos < number_of_points; ++pos) {
        Angle angle = -M_PI / 2 + pos * spacing / radius;
        points.push_back({radius * std::cos(angle), radius + radius * std::sin(angle)});
    }
    return points;
}

/**
 * Shape below a slightly curved line made of many vertices, starting at its
 * vertex 'first_point_pos'.
 */
Shape build_slightly_curved_shape(
        LengthDbl radius,
        LengthDbl spacing,
        ElementPos number_of_points,
        ElementPos first_point_pos = 0)
{
    std::vector<BuildShapeElement> points = build_circle_bottom_points(
            radius,
            spacing,
            number_of_points);
    points.push_back({points.back().x, 1});
    points.push_back({points.front().x, 1});
    std::rotate(points.begin(), points.begin() + first_point_pos, points.end());
    return build_shape(points);
}

struct RemoveAlignedVerticesToleranceTestParams
{
    std::string name;
    Shape shape;
};

void PrintTo(const RemoveAlignedVerticesToleranceTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
}

class RemoveAlignedVerticesToleranceTest: public testing::TestWithParam<RemoveAlignedVerticesToleranceTestParams> { };

TEST_P(RemoveAlignedVerticesToleranceTest, RemoveAlignedVerticesTolerance)
{
    RemoveAlignedVerticesToleranceTestParams test_params = GetParam();
    Shape cleaned_shape = remove_aligned_vertices(test_params.shape).second;
    std::cout << cleaned_shape.to_string(0) << std::endl;

    EXPECT_LT(cleaned_shape.elements.size(), test_params.shape.elements.size());

    // Every vertex of the shape must remain on the boundary of the cleaned
    // shape, up to the tolerance of 'equal' on each coordinate: an element
    // of the cleaned shape must go through the square of half-side 1e-6
    // around it.
    for (const ShapeElement& element: test_params.shape.elements) {
        const Point& vertex = element.start;
        bool found = false;
        for (const ShapeElement& cleaned_element: cleaned_shape.elements) {
            // Clip the element by the square.
            double t_min = 0;
            double t_max = 1;
            for (int axis = 0; axis < 2; ++axis) {
                double start = (axis == 0)? cleaned_element.start.x: cleaned_element.start.y;
                double end = (axis == 0)? cleaned_element.end.x: cleaned_element.end.y;
                double center = (axis == 0)? vertex.x: vertex.y;
                double low = center - 1e-6 - 1e-12;
                double high = center + 1e-6 + 1e-12;
                if (start == end) {
                    if (start < low || start > high)
                        t_max = -1;
                    continue;
                }
                double t_1 = (low - start) / (end - start);
                double t_2 = (high - start) / (end - start);
                t_min = (std::max)(t_min, (std::min)(t_1, t_2));
                t_max = (std::min)(t_max, (std::max)(t_1, t_2));
            }
            if (t_min <= t_max) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "vertex " << vertex.to_string();
    }
}

INSTANTIATE_TEST_SUITE_P(
        ,
        RemoveAlignedVerticesToleranceTest,
        testing::ValuesIn(std::vector<RemoveAlignedVerticesToleranceTestParams>{
            {
                // Each vertex is within the tolerance of the line from the
                // first one to the next one, but not of the line from the
                // first one to the last one.
                "SlightlyCurvedLine",
                build_slightly_curved_shape(500, 0.01, 11),
            }, {
                "LongSlightlyCurvedLine",
                build_slightly_curved_shape(1000, 0.001, 3001),
            }, {
                // The vertices removed together must not be on both sides
                // of the first vertex of the shape.
                "LongSlightlyCurvedLineStartingInTheMiddle",
                build_slightly_curved_shape(1000, 0.001, 3001, 1500),
            }, {
                // Removing (1, 0) and (2, 1.9e-6) one after the other leaves
                // (1, 0) 1.4e-6 away from the line from (0, 0) to
                // (3, 4.2e-6), within sqrt(2) * 1e-6, but 1.4e-6 away from it
                // on the y-axis.
                "NearlyHorizontalLine",
                build_shape({{0, 0}, {1, 0}, {2, 1.9e-6}, {3, 4.2e-6}, {3, 10}, {0, 10}}),
            }}),
        [](const testing::TestParamInfo<RemoveAlignedVerticesToleranceTest::ParamType>& info) {
            return info.param.name;
        });


struct FlattenArcsTestParams
{
    std::string name;
    Shape shape;
    Shape expected_shape;
};

void PrintTo(const FlattenArcsTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_shape " << params.expected_shape.to_string(0) << "\n";
}

class FlattenArcsTest: public testing::TestWithParam<FlattenArcsTestParams> { };

TEST_P(FlattenArcsTest, FlattenArcs)
{
    FlattenArcsTestParams test_params = GetParam();
    Shape output_shape = flatten_arcs(test_params.shape);
    std::cout << output_shape.to_string(0) << std::endl;
    EXPECT_EQ(test_params.expected_shape, output_shape);
}

INSTANTIATE_TEST_SUITE_P(
        ,
        FlattenArcsTest,
        testing::ValuesIn(std::vector<FlattenArcsTestParams>{
            {
                // A full circle (start == end) must not be flattened into a
                // zero-length line segment: its chord midpoint formula
                // degenerates to its start/end point, which trivially lies
                // on the arc and would otherwise be misdetected as flat.
                "Circle",
                build_circle(1.0),
                build_circle(1.0),
            }, {
                // A genuinely curved (non-flat) partial arc must not be
                // flattened either.
                "QuarterCircle",
                build_shape({build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise)}),
                build_shape({build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise)}),
            }}),
        [](const testing::TestParamInfo<FlattenArcsTest::ParamType>& info) {
            return info.param.name;
        });


struct RemoveBacktracksTestParams
{
    std::string name;
    Shape shape;
    Shape expected_shape;
};

void PrintTo(const RemoveBacktracksTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_shape " << params.expected_shape.to_string(0) << "\n";
}

class RemoveBacktracksTest: public testing::TestWithParam<RemoveBacktracksTestParams> { };

TEST_P(RemoveBacktracksTest, RemoveBacktracks)
{
    RemoveBacktracksTestParams test_params = GetParam();
    Shape output_shape = remove_backtracks(test_params.shape);
    std::cout << output_shape.to_string(1) << std::endl;
    EXPECT_EQ(output_shape.is_path, test_params.expected_shape.is_path);
    EXPECT_EQ(test_params.expected_shape, output_shape);
}

INSTANTIATE_TEST_SUITE_P(
        ,
        RemoveBacktracksTest,
        testing::ValuesIn(std::vector<RemoveBacktracksTestParams>{
            {
                "Backtrack",
                build_shape({{0, 0}, {2, 0}, {3, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
            }, {
                // The last element is the reverse of the first one.
                "BacktrackAcrossStart",
                build_shape({{-1, 0}, {0, 0}, {1, 0}, {0, 1}, {0, 0}}),
                build_shape({{0, 0}, {1, 0}, {0, 1}}),
            }, {
                // Removing a backtrack reveals another one.
                "NestedBacktracks",
                build_shape({{0, 0}, {2, 0}, {3, 0}, {4, 0}, {3, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
            }, {
                // The last element of a path is not followed by its first
                // element.
                "PathEndingWithReverseOfFirstElement",
                build_path({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {1, 0}, {0, 0}}),
                build_path({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {1, 0}, {0, 0}}),
            }}),
        [](const testing::TestParamInfo<RemoveBacktracksTest::ParamType>& info) {
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


struct CleanExtremeSlopesInnerTestParams
{
    std::string name;
    Shape shape;
    std::vector<Shape> expected_output;


    static CleanExtremeSlopesInnerTestParams read_json(
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
        CleanExtremeSlopesInnerTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(Shape::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const CleanExtremeSlopesInnerTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const Shape& shape: params.expected_output)
        *os << shape.to_string(2) << "\n";
}

class CleanExtremeSlopesInnerTest: public testing::TestWithParam<CleanExtremeSlopesInnerTestParams> { };

TEST_P(CleanExtremeSlopesInnerTest, CleanExtremeSlopesInner)
{
    CleanExtremeSlopesInnerTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<Shape> output = clean_extreme_slopes_inner(test_params.shape);
    std::cout << "output" << std::endl;
    for (const Shape& shape: output)
        std::cout << shape.to_string(2) << std::endl;

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (const Shape& expected_shape: test_params.expected_output) {
        EXPECT_NE(std::find_if(
                      output.begin(),
                      output.end(),
                      [&expected_shape](const Shape& shape) { return equal(shape, expected_shape); }),
                  output.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        CleanExtremeSlopesInnerTest,
        testing::ValuesIn(std::vector<CleanExtremeSlopesInnerTestParams>{
            {
                "Square",
                build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}}),
                {build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})},
            }, {
                "NearVerticalSpike",
                build_shape({{0, 0}, {10, -10}, {10.1, -110}, {20, 0}}),
                {build_shape({{0, 0}, {10.1, -110}, {20, 0}})},
            },
            CleanExtremeSlopesInnerTestParams::read_json(
                    (fs::path("data") / "tests" / "clean" / "clean_extreme_slopes_inner" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<CleanExtremeSlopesInnerTest::ParamType>& info) {
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
    std::vector<ShapeWithHoles> output = fix_self_intersections(test_params.shape).shapes_with_holes;
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
