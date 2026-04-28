//#define SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG

#include "shape/shapes_intersections.hpp"

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
#include "shape/writer.hpp"
#endif

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

//#include "test_params.hpp"

using namespace shape;
namespace fs = boost::filesystem;

struct IntersectShapeTestParams
{
    std::string name;
    Shape shape;
    bool expected_output;


    static IntersectShapeTestParams read_json(
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
        IntersectShapeTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const IntersectShapeTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class IntersectShapeTest: public testing::TestWithParam<IntersectShapeTestParams> { };

TEST_P(IntersectShapeTest, IntersectShape)
{
    IntersectShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);
    //write_json({{test_params.shape}}, {}, "intersect_input.json");

    bool output = intersect(test_params.shape);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeTestParams>{
            {
                "Square",
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                false,
            }, {
                "Bowtie",
                build_shape({{0, 0}, {2, 2}, {2, 0}, {0, 2}}),
                true,
            }, {
                "DoubleSquare",
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                true,
            }, {
                "UShapeWithSelfIntersection",
                build_shape({{0, 0}, {4, 0}, {4, 4}, {2, 4}, {2, 3}, {3, 3}, {3, 2}, {1, 2}, {1, 3}, {2, 3}, {2, 4}, {0, 4}}),
                true,
            }, {
                "TriangleWithInnerCross1",
                build_shape({{0, 0}, {6, 0}, {3, 2}, {2, 1}, {4, 1}, {3, 2}}),
                true,
            }, {
                "TriangleWithInnerCross2",
                build_shape({{0, 0}, {6, 0}, {3, 2}, {4, 1}, {2, 1}, {3, 2}}),
                true,
            }, {
                "ComplexNonSelfIntersecting",
                build_shape({
                        {31.49606296, 144.25196848},
                        {0, 144.25196848},
                        {0, 0},
                        {31.49606296, 0},
                        {31.49606296, 9.448818519999994},
                        {22.04724408, 4.7244092},
                        {25.1968504, 11.96850392},
                        {52.87627208, 11.96850392},
                        {52.87627208, 56.37795144},
                        {47.20698072, 56.37795144},
                        {47.20698072, 132.28346456},
                        {25.1968504, 132.28346456},
                        {22.04724408, 139.52755928},
                        {31.49606295999999, 134.8031497200001}}),
                false,
            }, {
                "TouchingSquares",
                build_shape({
                        {0, 0},
                        {1, 0},
                        {1, 1},
                        {2, 1},
                        {2, 2},
                        {1, 2},
                        {1, 1},
                        {0, 1}}),
                true,
            }
        }),
        [](const testing::TestParamInfo<IntersectShapeTest::ParamType>& info) {
            return info.param.name;
        });


struct IntersectShapeShapeElementTestParams
{
    std::string name;
    Shape shape;
    ShapeElement element;
    bool strict = false;
    bool expected_output;


    static IntersectShapeShapeElementTestParams read_json(
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
        IntersectShapeShapeElementTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.element = ShapeElement::from_json(json["element"]);
        test_params.strict = json["strict"];
        test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const IntersectShapeShapeElementTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "element " << params.element.to_string() << "\n";
    *os << "strict " << params.strict << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class IntersectShapeShapeElementTest: public testing::TestWithParam<IntersectShapeShapeElementTestParams> { };

TEST_P(IntersectShapeShapeElementTest, IntersectShapeShapeElement)
{
    IntersectShapeShapeElementTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
    Writer().add_shape(test_params.shape).add_element(test_params.element).write_json("intersect_input.json");
#endif

    bool output = intersect(
            test_params.shape,
            test_params.element,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeShapeElementTest,
        testing::ValuesIn(std::vector<IntersectShapeShapeElementTestParams>{
            IntersectShapeShapeElementTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_shape_element" / "0.json").string()),
            {
                "SquareSegmentOutside",
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_line_segment({3, 0}, {3, 2}),
                false,
                false,
            }, {
                "RectangleSegmentInside",
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_line_segment({1, 1}, {1, 3}),
                false,
                true,
            }, {
                "PathSegmentInside",
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_line_segment({1, 1}, {1, 3}),
                false,
                false,
            },
        }),
        [](const testing::TestParamInfo<IntersectShapeShapeElementTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ComputeIntersectionsPathShapeTestParams
{
    std::string name;
    Shape path;
    Shape shape;
    bool only_min_max;
    std::vector<PathShapeIntersectionPoint> expected_output;


    static ComputeIntersectionsPathShapeTestParams read_json(
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
        ComputeIntersectionsPathShapeTestParams test_params;
        test_params.name = file_path;
        test_params.path = Shape::from_json(json["path"]);
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.only_min_max = json["only_min_max"];
        for (auto& json_intersection: json["expected_output"]) {
            PathShapeIntersectionPoint intersection;
            intersection.path_element_pos = json_intersection["path_element_pos"];
            intersection.shape_element_pos = json_intersection["shape_element_pos"];
            intersection.point = Point::from_json(json_intersection["point"]);
            test_params.expected_output.emplace_back(intersection);
        }
        return test_params;
    }
};

void PrintTo(const ComputeIntersectionsPathShapeTestParams& params, std::ostream* os)
{
    *os << "path " << params.path.to_string(0) << "\n";
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "only_min_max " << params.only_min_max << "\n";
    *os << "expected_output\n";
    for (const PathShapeIntersectionPoint& intersection: params.expected_output) {
        *os << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << "\n";
    }
}

class ComputeIntersectionsPathShapeTest: public testing::TestWithParam<ComputeIntersectionsPathShapeTestParams> { };

TEST_P(ComputeIntersectionsPathShapeTest, ComputeIntersectionsPathShape)
{
    ComputeIntersectionsPathShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<PathShapeIntersectionPoint> output = compute_intersections(
            test_params.path,
            test_params.shape,
            test_params.only_min_max);
    std::cout << "output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
    Writer()
        .add_shape(test_params.path)
        .add_shape(test_params.shape)
        .write_json("compute_intersections_path_shape.json");
#endif

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (ElementPos pos = 0; pos < (ElementPos)output.size(); ++pos) {
        EXPECT_EQ(output[pos].path_element_pos, test_params.expected_output[pos].path_element_pos);
        EXPECT_EQ(output[pos].shape_element_pos, test_params.expected_output[pos].shape_element_pos);
        EXPECT_TRUE(equal(output[pos].point, test_params.expected_output[pos].point));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsPathShapeTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsPathShapeTestParams>{
            {
                "ArcLineArcCrossesTriangle",
                build_path({
                        build_circular_arc({64, 0}, {192, 128}, {192, 0}, ShapeElementOrientation::Clockwise),
                        build_line_segment({192, 128}, {704, 128}),
                        build_circular_arc({704, 128}, {832, 2.842170943040401e-14}, {704, 2.842170943040401e-14}, ShapeElementOrientation::Clockwise)}),
                build_shape({{384, 320}, {448, -192}, {448, 832}}),
                true,
                {
                    {1, 0, {408, 128}},
                    {1, 1, {448, 128}},
                },
            },
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "0.json").string()),
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "1.json").string()),
            ComputeIntersectionsPathShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "compute_intersections_path_shape" / "2.json").string()),
        }),
        [](const testing::TestParamInfo<ComputeIntersectionsPathShapeTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ComputeStrictIntersectionsPathShapeTestParams
{
    std::string name;
    Shape path;
    Shape shape;
    bool only_first;
    std::vector<PathShapeIntersectionPoint> expected_output;


    static ComputeStrictIntersectionsPathShapeTestParams read_json(
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
        ComputeStrictIntersectionsPathShapeTestParams test_params;
        test_params.name = file_path;
        test_params.path = Shape::from_json(json["path"]);
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.only_first = json["only_first"];
        for (auto& json_intersection: json["expected_output"]) {
            PathShapeIntersectionPoint intersection;
            intersection.path_element_pos = json_intersection["path_element_pos"];
            intersection.shape_element_pos = json_intersection["shape_element_pos"];
            intersection.point = Point::from_json(json_intersection["point"]);
            test_params.expected_output.emplace_back(intersection);
        }
        return test_params;
    }
};

void PrintTo(const ComputeStrictIntersectionsPathShapeTestParams& params, std::ostream* os)
{
    *os << "path " << params.path.to_string(0) << "\n";
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "only_first " << params.only_first << "\n";
    *os << "expected_output\n";
    for (const PathShapeIntersectionPoint& intersection: params.expected_output) {
        *os << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << "\n";
    }
}

class ComputeStrictIntersectionsPathShapeTest: public testing::TestWithParam<ComputeStrictIntersectionsPathShapeTestParams> { };

TEST_P(ComputeStrictIntersectionsPathShapeTest, ComputeStrictIntersectionsPathShape)
{
    ComputeStrictIntersectionsPathShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<PathShapeIntersectionPoint> output = compute_strict_intersections(
            test_params.path,
            test_params.shape,
            test_params.only_first);
    std::cout << "output" << std::endl;
    for (const PathShapeIntersectionPoint& intersection: output) {
        std::cout << "path_element_pos " << intersection.path_element_pos
            << " shape_element_pos " << intersection.shape_element_pos
            << " point " << intersection.point.to_string() << std::endl;
    }

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
    Writer()
        .add_shape(test_params.path)
        .add_shape(test_params.shape)
        .write_json("compute_intersections_path_shape.json");
#endif

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (ElementPos pos = 0; pos < (ElementPos)output.size(); ++pos) {
        EXPECT_EQ(output[pos].path_element_pos, test_params.expected_output[pos].path_element_pos);
        EXPECT_EQ(output[pos].shape_element_pos, test_params.expected_output[pos].shape_element_pos);
        EXPECT_TRUE(equal(output[pos].point, test_params.expected_output[pos].point));
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeStrictIntersectionsPathShapeTest,
        testing::ValuesIn(std::vector<ComputeStrictIntersectionsPathShapeTestParams>{
            {
                "ArcLineArcCrossesTriangle",
                build_path({
                        build_circular_arc({64, 0}, {192, 128}, {192, 0}, ShapeElementOrientation::Clockwise),
                        build_line_segment({192, 128}, {704, 128}),
                        build_circular_arc({704, 128}, {832, 2.842170943040401e-14}, {704, 2.842170943040401e-14}, ShapeElementOrientation::Clockwise)}),
                build_shape({{384, 320}, {448, -192}, {448, 832}}),
                true,
                {{1, 0, {408, 128}}},
            },
        }),
        [](const testing::TestParamInfo<ComputeStrictIntersectionsPathShapeTest::ParamType>& info) {
            return info.param.name;
        });


struct IntersectShapeShapeTestParams
{
    std::string name;
    Shape shape_1;
    Shape shape_2;
    bool strict = false;
    bool expected_output;


    static IntersectShapeShapeTestParams read_json(
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
        IntersectShapeShapeTestParams test_params;
        test_params.name = file_path;
        test_params.shape_1 = Shape::from_json(json["shape_1"]);
        test_params.shape_2 = Shape::from_json(json["shape_2"]);
        test_params.strict = json["strict"];
        test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const IntersectShapeShapeTestParams& params, std::ostream* os)
{
    *os << "shape_1 " << params.shape_1.to_string(0) << "\n";
    *os << "shape_2 " << params.shape_2.to_string(0) << "\n";
    *os << "strict " << params.strict << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class IntersectShapeShapeTest: public testing::TestWithParam<IntersectShapeShapeTestParams> { };

TEST_P(IntersectShapeShapeTest, IntersectShapeShape)
{
    IntersectShapeShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
    Writer()
        .add_shape(test_params.shape_1, "Shape 1")
        .add_shape(test_params.shape_2, "Shape 2")
        .write_json("intersect_shape_shape_input.json");
#endif

    bool output = intersect(
            test_params.shape_1,
            test_params.shape_2,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeShapeTestParams>{
            {
                "SquareOverlapStrict",
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                true,
                true,
            }, {
                "SquarePathOutside",
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
                build_path({{3, 0}, {3, 2}}),
                false,
                false,
            }, {
                "RectanglePathInside",
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_path({{1, 1}, {1, 3}}),
                false,
                true,
            }, {
                "PathInsideRectangle",
                build_path({{1, 1}, {1, 3}}),
                build_shape({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                false,
                true,
            }, {
                "TwoPaths",
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                build_path({{1, 1}, {1, 3}}),
                false,
                false,
            }, {
                "TwoPathsReversed",
                build_path({{1, 1}, {1, 3}}),
                build_path({{0, 0}, {2, 0}, {2, 4}, {0, 4}}),
                false,
                false,
            },
            IntersectShapeShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_shape" / "0.json").string()),
            {
                "ArrowShapesMeetAtTip",
                build_shape({{4, 0}, {0, 0}, {0, 2}, {1, 2}, {2, 3}, {3, 2}, {4, 2}}),
                build_shape({{0, 2}, {0, 4}, {4, 4}, {4, 2}, {3, 2}, {2, 1}, {1, 2}}),
                true,
                true,
            //}, {
            //    build_shape({{4, 0}, {0, 0}, {0, 2}, {2, 0}, {4, 2}}),
            //    build_shape({{2, 0}, {3, 2}, {1, 2}}),
            //    true,
            //    false,
            //}, {
            //    build_shape({{4, 0}, {0, 0}, {0, 2}, {2, 0}, {4, 2}}),
            //    build_shape({{2, 0}, {4, 2}, {0, 2}}),
            //    true,
            //    false,
            },
        }),
        [](const testing::TestParamInfo<IntersectShapeShapeTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct IntersectShapeWithHolesShapeTestParams
{
    std::string name;
    ShapeWithHoles shape_with_holes;
    Shape shape;
    bool strict = false;
    bool expected_output;


    static IntersectShapeWithHolesShapeTestParams read_json(
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
        IntersectShapeWithHolesShapeTestParams test_params;
        test_params.name = file_path;
        test_params.shape_with_holes = ShapeWithHoles::from_json(json["shape_with_holes"]);
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.strict = json["strict"];
        if (json.contains("expected_output"))
            test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const IntersectShapeWithHolesShapeTestParams& params, std::ostream* os)
{
    *os << "shape_with_holes " << params.shape_with_holes.to_string(0) << "\n";
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "strict " << params.strict << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class IntersectShapeWithHolesShapeTest: public testing::TestWithParam<IntersectShapeWithHolesShapeTestParams> { };

TEST_P(IntersectShapeWithHolesShapeTest, IntersectShapeWithHolesShape)
{
    IntersectShapeWithHolesShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef SHAPES_INTERSECTIONS_TEST_ENABLE_DEBUG
    Writer()
        .add_shape_with_holes(test_params.shape_with_holes, "Shape with holes")
        .add_shape(test_params.shape, "Shape")
        .write_json("intersect_shape_with_holes_shape_input.json");
#endif

    bool output = intersect(
            test_params.shape_with_holes,
            test_params.shape,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeWithHolesShapeTest,
        testing::ValuesIn(std::vector<IntersectShapeWithHolesShapeTestParams>{
            {
                "ShapeInHole",
                {
                    build_shape({{500, 500}, {0, 500}, {0, 0}, {500, 0}}),
                    {build_shape({{100, 100}, {400, 100}, {400, 400}, {100, 400}})}
                },
                build_shape({{100, 200}, {200, 200}, {200, 400}, {100, 400}}),
                true,
                false,
            }, {
                "CircleWithArcInside",
                {
                    build_circle(10).shift(5, 0),
                },
                build_shape({
                        {2.5, -9.682458365518542},
                        {5, 0, -1},
                        {2.5, 9.682458365518542},
                        {0, 0, 1}}),
                true,
                false,
            }, {
                "UShapeLineAcrossNotch",
                {
                    build_shape({{0, 0}, {3, 0}, {3, 1}, {1, 1}, {1, 2}, {3, 2}, {3, 3}, {0, 3}}),
                },
                build_shape({{1, 1}, {2, 1}, {3, 1}, {4, 1}, {4, 2}, {3, 2}, {2, 2}, {1, 2}}),
                true,
                false,
            }, {
                "CShapeLineAcrossNotch",
                {
                    build_shape({{2, 0}, {5, 0}, {5, 3}, {2, 3}, {2, 2}, {4, 2}, {4, 1}, {2, 1}}),
                },
                build_shape({{1, 1}, {2, 1}, {3, 1}, {4, 1}, {4, 2}, {3, 2}, {2, 2}, {1, 2}}),
                true,
                false,
            }, {
                "CircleWithArcOutside",
                {
                    build_circle(10),
                },
                build_shape({
                        {2.5, -9.682458365518542},
                        {5, 0, 1},
                        {2.5, 9.682458365518542},
                        {0, 0, -1}}),
                true,
                false,
            }, {
                "ArcShapeTangentToPath",
                {
                    build_shape({
                            {19.68503937, 17.7480315},
                            {19.68503937, 15.7480315, 1},
                            {17.68503937, 15.7480315},
                            {19.68503937, 15.7480315}}),
                },
                build_shape({
                        {5.93700787, 5.93700787},
                        {17.68503937, 5.93700787},
                        {17.68503937, 15.7480315},
                        {19.68503937, 15.7480315, -1},
                        {5.93700787, 17.68503937}}),
                true,
                false,
            },
            IntersectShapeWithHolesShapeTestParams::read_json(
                    (fs::path("data") / "tests" / "shapes_intersections" / "intersect_shape_with_holes_shape" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<IntersectShapeWithHolesShapeTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct IntersectShapeWithHolesShapeWithHolesTestParams
{
    std::string name;
    ShapeWithHoles shape_with_holes_1;
    ShapeWithHoles shape_with_holes_2;
    bool strict = false;
    bool expected_output;


    static IntersectShapeWithHolesShapeWithHolesTestParams read_json(
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
        IntersectShapeWithHolesShapeWithHolesTestParams test_params;
        test_params.name = file_path;
        test_params.shape_with_holes_1 = ShapeWithHoles::from_json(json["shape_with_holes_1"]);
        test_params.shape_with_holes_2 = ShapeWithHoles::from_json(json["shape_with_holes_2"]);
        test_params.strict = json["strict"];
        test_params.expected_output = json["expected_output"];
        return test_params;
    }
};

void PrintTo(const IntersectShapeWithHolesShapeWithHolesTestParams& params, std::ostream* os)
{
    *os << "shape_with_holes_1 " << params.shape_with_holes_1.to_string(0) << "\n";
    *os << "shape_with_holes_2 " << params.shape_with_holes_2.to_string(0) << "\n";
    *os << "strict " << params.strict << "\n";
    *os << "expected_output " << params.expected_output << "\n";
}

class IntersectShapeWithHolesShapeWithHolesTest: public testing::TestWithParam<IntersectShapeWithHolesShapeWithHolesTestParams> { };

TEST_P(IntersectShapeWithHolesShapeWithHolesTest, IntersectShapeWithHolesShapeWithHoles)
{
    IntersectShapeWithHolesShapeWithHolesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    bool output = intersect(
            test_params.shape_with_holes_1,
            test_params.shape_with_holes_2,
            test_params.strict);
    std::cout << "output " << output << std::endl;

    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        IntersectShapeWithHolesShapeWithHolesTest,
        testing::ValuesIn(std::vector<IntersectShapeWithHolesShapeWithHolesTestParams>{
            {
                "ShapeInHole",
                {build_shape({{100, 200}, {200, 200}, {200, 400}, {100, 400}})},
                {
                    build_shape({{500, 500}, {0, 500}, {0, 0}, {500, 0}}),
                    {build_shape({{100, 100}, {400, 100}, {400, 400}, {100, 400}})}
                },
                true,
                false,
            }
        }),
        [](const testing::TestParamInfo<IntersectShapeWithHolesShapeWithHolesTest::ParamType>& info) {
            return info.param.name;
        });
