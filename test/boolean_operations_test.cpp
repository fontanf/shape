//#define BOOLEAN_OPERATIONS_TEST_ENABLE_DEBUG

#include "shape/boolean_operations.hpp"

#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

using namespace shape;


struct FindHolesBridgesTestParams
{
    std::string name;
    ShapeWithHoles shape;
    std::vector<ShapeElement> expected_output;

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
        FindHolesBridgesTestParams test_params;
        test_params.name = file_path;
        test_params.shape = ShapeWithHoles::from_json(json["shape"]);
        for (auto& json_element: json["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeElement::from_json(json_element.value()));
        return test_params;
    }
};

void PrintTo(const FindHolesBridgesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expected_output\n";
    for (const ShapeElement& bridge: params.expected_output)
        *os << "- " << bridge.to_string() << "\n";
}

class FindHolesBridgesTest: public testing::TestWithParam<FindHolesBridgesTestParams> { };

TEST_P(FindHolesBridgesTest, FindHolesBridges)
{
    FindHolesBridgesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<ShapeElement> output = find_holes_bridges(test_params.shape);
    std::cout << "output" << std::endl;
    for (const ShapeElement& bridge: output)
        std::cout << "- " << bridge.to_string() << std::endl;

    ASSERT_EQ(output.size(), test_params.expected_output.size());
    for (const ShapeElement& expected_bridge: test_params.expected_output) {
        EXPECT_NE(std::find_if(
                    output.begin(),
                    output.end(),
                    [&expected_bridge](const ShapeElement& bridge) { return equal(bridge, expected_bridge) || equal(bridge.reverse(), expected_bridge); }),
                output.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        FindHolesBridgesTest,
        testing::ValuesIn(std::vector<FindHolesBridgesTestParams>{
            {  // Shape without hole.
                "NoHole",
                {build_rectangle(200, 100)},
                {},
            }, {  // Shape with one touching hole.
                "OneTouchingHole",
                {
                    build_rectangle(200, 100),
                    {build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}})}
                },
                {},
            }, {  // Shape with one non-touching hole.
                "OneNonTouchingHole",
                {
                    build_rectangle(200, 100),
                    {build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}})}
                },
                {build_line_segment({0, 50}, {40, 50})},
            }, {  // Shape with one touching hole and on non-touching hole.
                "OneTouchingOneNonTouching",
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}}),
                        build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}}),
                    }
                },
                {build_line_segment({20, 50}, {40, 50})},
            }, {  // Shape with two touching holes.
                "TwoTouchingHoles",
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{0, 50}, {10, 40}, {20, 50}, {10, 60}}),
                        build_shape({{20, 50}, {30, 40}, {40, 50}, {30, 60}}),
                    }
                },
                {},
            }, {  // Shape with two holes touching each other.
                "TwoHolesTouchingEachOther",
                {
                    build_rectangle(200, 100),
                    {
                        build_shape({{20, 50}, {30, 40}, {40, 50}, {30, 60}}),
                        build_shape({{40, 50}, {50, 40}, {60, 50}, {50, 60}}),
                    }
                },
                {build_line_segment({0, 50}, {20, 50})},
            }, {  // Shape with two holes.
                "TwoSeparateHoles",
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
            }
        }),
        [](const testing::TestParamInfo<FindHolesBridgesTest::ParamType>& info) {
            return info.param.name;
        });


struct ComputeBooleanUnionTestParams
{
    std::string name;
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_output;


    static ComputeBooleanUnionTestParams read_json(
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
        ComputeBooleanUnionTestParams test_params;
        test_params.name = file_path;
        for (auto& json_shape: json["shapes"].items())
            test_params.shapes.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        if (json.contains("expected_output"))
            for (auto& json_shape: json["expected_output"].items())
                test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const ComputeBooleanUnionTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "shapes\n";
    for (const ShapeWithHoles& shape: params.shapes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << "- " << shape.to_string(2) << "\n";
}

class ComputeBooleanUnionTest: public testing::TestWithParam<ComputeBooleanUnionTestParams> { };

TEST_P(ComputeBooleanUnionTest, ComputeBooleanUnion)
{
    ComputeBooleanUnionTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef BOOLEAN_OPERATIONS_TEST_ENABLE_DEBUG
    Writer().add_shapes_with_holes(test_params.shapes).write_json("compute_union_input.json");
    Writer().add_shapes_with_holes(test_params.expected_output).write_json("compute_union_expected_output.json");
#endif

    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)test_params.shapes.size();
            ++shape_pos) {
        const ShapeWithHoles& shape = test_params.shapes[shape_pos];
        if (!shape.shape.check()) {
            throw std::invalid_argument(FUNC_SIGNATURE);
        }
    }
    auto output = compute_union(
            test_params.shapes).shapes_with_holes;
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << "- " << shape.to_string(2) << std::endl;

#ifdef BOOLEAN_OPERATIONS_TEST_ENABLE_DEBUG
    Writer().add_shapes_with_holes(output).write_json("compute_union_output.json");
#endif

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
        ComputeBooleanUnionTest,
        testing::ValuesIn(std::vector<ComputeBooleanUnionTestParams>{
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "000.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "001.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "002.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "003.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "004.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "005.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "006.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "007.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "008.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "009.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "010.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "011.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "012.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "013.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "014.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "015.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "016.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "017.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "018.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "019.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "020.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "021.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "022.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "023.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "024.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "025.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "026.json").string()),
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "027.json").string()),
            // Regression test for fontanf/packingsolver issue #563: the 56
            // input shapes in 028.json form a closed "sleeve" of overlapping
            // rectangles and circular-arc sectors around a real polygon
            // boundary and its hole (produced by shape::inflate offsetting
            // each edge). Their union can never legitimately be empty, but
            // compute_union() currently returns zero faces for this input,
            // which downstream (in shape::inflate) turns into a segfault
            // when the empty result is blindly dereferenced with .front().
            // The exact correct output for this input hasn't been
            // determined yet, so expected_output is a placeholder single
            // empty shape for now -- to be filled in once the underlying
            // bug is fixed.
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "028.json").string()),
            // Regression test for fontanf/packingsolver issue #558: the 17
            // input shapes in 029.json are a subset of the 114
            // per-fixed-part NFP group unions computed while building the
            // no-fit-polygon of a sawtooth-strip item against a
            // 180-degree-rotated copy of itself. Their union legitimately
            // contains one large (~326-element, ~1981-area) real polygon
            // component -- confirmed by computing this union with
            // cross_product() left as the naive "vector_1.x * vector_2.y -
            // vector_2.x * vector_1.y" (its result here does not depend on
            // FMA availability or -ffp-contract, since it is naive on every
            // tested compiler/flag combination for this input) -- but with
            // cross_product() computed via std::fma instead (as it is
            // written today, for portability -- see strictly_lesser_angle),
            // that same real component is lost entirely:
            // bridge_touching_holes/fix_self_intersections shatters it into
            // only degenerate near-zero-area slivers, the same failure mode
            // as the original empty-result bug this issue started from.
            // This reproduces that regression in isolation, without the
            // full simplify()+no_fit_polygon() pipeline it was originally
            // found in (which takes several seconds to run). The exact
            // correct output isn't pinned down either (this shape sits on
            // top of several near-exact-tangency ties), so expected_output
            // is likewise a placeholder single empty shape for now.
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "029.json").string()),
            // Same issue #558 divergence as 029.json, minimized further: a
            // scan of all pairs among 029.json's 17 shapes found none that
            // diverge between cross_product() computed via std::fma versus
            // left naive, but a scan of all triples found many that do --
            // 030.json is shapes 0, 1 and 3 of 029.json's list, one such
            // triple. Here the divergence is milder than 029.json's (it is
            // not about losing a large real component, both regimes agree
            // on a ~4019.776-area main component to 9 significant figures)
            // but the exact split into components still differs: fma gives
            // 4 components ([636, 6, 3, 3]-element, including 2 extra
            // degenerate 3-element slivers not present in the naive
            // regime's 2 components ([634, 6]-element), and the main
            // component's element count itself differs (636 vs 634). No
            // exact expected_output is recorded yet for the same reason as
            // 029.json.
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "030.json").string()),
            // Same issue #558 divergence as 030.json, cropped down to just
            // the region where it actually occurs: diffing the two regimes'
            // arrangement-graph node lists for 030.json located the extra
            // degenerate slivers fma produces within x in [13, 20]
            // (y in [6.5, 7.9]). 031.json keeps, from each of 030.json's 3
            // shapes, only the contiguous run of elements with x in
            // [13, 17] (a window around two of those three slivers), closed
            // back into a valid (non-self-intersecting) polygon with a
            // 3-segment path per shape that steps outside the window's y
            // range and back rather than cutting straight across it. This
            // still diverges -- fma gives 3 components ([10, 6, 5]-element),
            // naive gives 2 ([12, 6]-element) -- with only 29 elements total
            // across the 3 input shapes, instead of 029.json's ~7353 or
            // 030.json's ~1299. No exact expected_output is recorded yet
            // for the same reason as 029.json and 030.json.
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "031.json").string()),
            // Regression test for fontanf/packingsolver issue #558:
            // unrelated to the missing-split-point bug 029/030/031.json
            // cover, found while sweeping the item's simplification ratio
            // through no_fit_polygon(shape_0, shape_r) -- at ratio 0.008
            // (389-vertex simplified shape, vs. 442 at the ratio 0.001 used
            // elsewhere), the same final compute_union(group_unions) call
            // in no_fit_polygon() threw instead of returning a result:
            // "face area is not positive; area: -2.573084." 032.json is 3
            // of the 87 per-fixed_part group unions from that call,
            // minimized by bisection (from 87 down to 3; the minimal 3
            // still threw the same error, with a different reported area
            // since removing shapes changes which face ends up negative --
            // observed -14.249123 for this exact triple).
            //
            // Root cause: three near-duplicate, near-parallel edges (from
            // three near-identical periodic copies of the same shape) sit
            // within compute_line_intersection's 1e-6 tolerance of each
            // other, but a fourth edge ("wall") crosses them at a shallow
            // angle (~2.66 degrees). That amplifies the sub-tolerance
            // perpendicular gap between the near-duplicate edges by
            // ~1/sin(angle) (~22x) once projected along wall's own
            // direction, so wall's genuine crossing with each near-duplicate
            // landed 1.5e-5 to 1.7e-5 away from wall's own endpoint instead
            // of snapping to it -- well outside the 1e-6 point tolerance,
            // even though the underlying edges were within it. This left
            // three separate, nearly-collinear vertices next to each other
            // instead of one, which defeated compute_arcs_next's angle sort
            // (an exact tie between two of the resulting near-duplicate
            // fragments) and traced a face with the wrong winding. Fixed by
            // reordering compute_line_intersection's checks so the
            // perpendicular-distance-to-zero tests (which are not affected
            // by the shallow-angle amplification) run before the
            // axis-aligned substitution branches; see also the two new
            // "wall" cases added to elements_intersections_test.cpp's
            // ComputeIntersectionsTest for the underlying, minimal
            // reproduction of this same mechanism.
            ComputeBooleanUnionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "union" / "032.json").string()),
        }),
        [](const testing::TestParamInfo<ComputeBooleanUnionTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });

struct ComputeBooleanIntersectionTestParams
{
    std::string name;
    std::vector<ShapeWithHoles> shapes;
    std::vector<ShapeWithHoles> expected_output;


    static ComputeBooleanIntersectionTestParams read_json(
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
        ComputeBooleanIntersectionTestParams test_params;
        test_params.name = file_path;
        for (auto& json_shape: json["shapes"].items())
            test_params.shapes.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const ComputeBooleanIntersectionTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "shapes\n";
    for (const ShapeWithHoles& shape: params.shapes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << "- " << shape.to_string(2) << "\n";
}

class ComputeBooleanIntersectionTest: public testing::TestWithParam<ComputeBooleanIntersectionTestParams> { };

TEST_P(ComputeBooleanIntersectionTest, ComputeBooleanIntersection)
{
    ComputeBooleanIntersectionTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

#ifdef BOOLEAN_OPERATIONS_TEST_ENABLE_DEBUG
    Writer().add_shapes_with_holes(test_params.shapes).write_json("compute_intersection_input.json");
    Writer().add_shapes_with_holes(test_params.expected_output).write_json("compute_intersection_expected_output.json");
#endif

    auto output = compute_intersection(
            test_params.shapes).shapes_with_holes;
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << "- " << shape.to_string(2) << std::endl;

#ifdef BOOLEAN_OPERATIONS_TEST_ENABLE_DEBUG
    Writer().add_shapes_with_holes(output).write_json("compute_intersection_output.json");
#endif

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
        ComputeBooleanIntersectionTest,
        testing::ValuesIn(std::vector<ComputeBooleanIntersectionTestParams>{
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "000.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "001.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "002.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "003.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "004.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "005.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "006.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "007.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "008.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "009.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "010.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "011.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "012.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "013.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "014.json").string()),
            ComputeBooleanIntersectionTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "intersection" / "015.json").string()),
        }),
        [](const testing::TestParamInfo<ComputeBooleanIntersectionTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ComputeBooleanIntersectionMultiShapeTestParams
{
    std::string name;
    std::vector<MultiShapeWithHoles> multi_shapes;
    std::vector<ShapeWithHoles> expected_output;
};

void PrintTo(const ComputeBooleanIntersectionMultiShapeTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "multi_shapes\n";
    for (Counter group_pos = 0;
            group_pos < (Counter)params.multi_shapes.size();
            ++group_pos) {
        *os << "group " << group_pos << "\n";
        for (const ShapeWithHoles& shape: params.multi_shapes[group_pos].shapes_with_holes)
            *os << "- " << shape.to_string(2) << "\n";
    }
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << "- " << shape.to_string(2) << "\n";
}

class ComputeBooleanIntersectionMultiShapeTest: public testing::TestWithParam<ComputeBooleanIntersectionMultiShapeTestParams> { };

TEST_P(ComputeBooleanIntersectionMultiShapeTest, ComputeBooleanIntersectionMultiShape)
{
    ComputeBooleanIntersectionMultiShapeTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    auto output = compute_intersection(
            test_params.multi_shapes).shapes_with_holes;
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanIntersectionMultiShapeTest,
        testing::ValuesIn(std::vector<ComputeBooleanIntersectionMultiShapeTestParams>{
            {  // Two groups; one group has two disjoint pieces, only parts of
               // which overlap the other group.
                "TwoGroupsWithDisjointPieces",
                {
                    MultiShapeWithHoles{{
                        {build_rectangle(0, 2, 0, 1)},
                        {build_rectangle(10, 12, 0, 1)},
                    }}, MultiShapeWithHoles{{
                        {build_rectangle(1, 11, -1, 2)},
                    }},
                },
                {
                    {build_rectangle(1, 2, 0, 1)},
                    {build_rectangle(10, 11, 0, 1)},
                },
            }, {  // Three groups, each a single shape.
                "ThreeGroupsIntersection",
                {
                    MultiShapeWithHoles{{{build_rectangle(0, 10, 0, 10)}}},
                    MultiShapeWithHoles{{{build_rectangle(5, 15, -5, 5)}}},
                    MultiShapeWithHoles{{{build_rectangle(-5, 8, 2, 8)}}},
                },
                {
                    {build_rectangle(5, 8, 2, 5)},
                },
            }, {  // A group with no shape at all represents an empty region:
               // the intersection must be empty.
                "EmptyGroupYieldsEmptyResult",
                {
                    MultiShapeWithHoles{{{build_rectangle(0, 10, 0, 10)}}},
                    MultiShapeWithHoles{},
                },
                {},
            }, {  // Groups whose unions don't overlap at all.
                "NonOverlappingGroupsYieldEmptyResult",
                {
                    MultiShapeWithHoles{{{build_rectangle(0, 5, 0, 5)}}},
                    MultiShapeWithHoles{{{build_rectangle(10, 15, 10, 15)}}},
                },
                {},
            }, {  // One group has two pieces that overlap each other,
               // forming a step/L-shaped union, intersected with a rectangle
               // straddling both pieces.
                "OverlappingPiecesWithinGroup",
                {
                    MultiShapeWithHoles{{
                        {build_rectangle(0, 2, 0, 2)},
                        {build_rectangle(1, 2, 0, 4)},
                    }}, MultiShapeWithHoles{{
                        {build_rectangle(0.5, 1.5, 1, 3)},
                    }},
                },
                {
                    {build_shape({
                        {0.5, 1}, {1.5, 1}, {1.5, 3}, {1, 3}, {1, 2}, {0.5, 2},
                    })},
                },
            },
        }),
        [](const testing::TestParamInfo<ComputeBooleanIntersectionMultiShapeTest::ParamType>& info) {
            return info.param.name;
        });


struct ComputeBooleanDifferenceTestParams
{
    std::string name;
    MultiShapeWithHoles shapes_1;
    MultiShapeWithHoles shapes;
    std::vector<ShapeWithHoles> expected_output;


    static ComputeBooleanDifferenceTestParams read_json(
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
        ComputeBooleanDifferenceTestParams test_params;
        test_params.name = file_path;
        test_params.shapes_1 = MultiShapeWithHoles::from_json(json["shapes_1"]);
        test_params.shapes = MultiShapeWithHoles::from_json(json["shapes"]);
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const ComputeBooleanDifferenceTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "shapes_1\n";
    for (const ShapeWithHoles& shape: params.shapes_1.shapes_with_holes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "shapes\n";
    for (const ShapeWithHoles& shape: params.shapes.shapes_with_holes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << "- " << shape.to_string(2) << "\n";
}

class ComputeBooleanDifferenceTest: public testing::TestWithParam<ComputeBooleanDifferenceTestParams> { };

TEST_P(ComputeBooleanDifferenceTest, ComputeBooleanDifference)
{
    ComputeBooleanDifferenceTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    auto output = compute_difference(
            test_params.shapes_1,
            test_params.shapes).shapes_with_holes;
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanDifferenceTest,
        testing::ValuesIn(std::vector<ComputeBooleanDifferenceTestParams>{
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "000.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "001.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "002.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "003.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "004.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "005.json").string()),
            ComputeBooleanDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "difference" / "006.json").string()),
        }),
        [](const testing::TestParamInfo<ComputeBooleanDifferenceTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ComputeBooleanSymmetricDifferenceTestParams
{
    std::string name;
    MultiShapeWithHoles shapes_1;
    MultiShapeWithHoles shapes_2;
    std::vector<ShapeWithHoles> expected_output;


    static ComputeBooleanSymmetricDifferenceTestParams read_json(
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
        ComputeBooleanSymmetricDifferenceTestParams test_params;
        test_params.name = file_path;
        test_params.shapes_1 = MultiShapeWithHoles::from_json(json["shapes_1"]);
        test_params.shapes_2 = MultiShapeWithHoles::from_json(json["shapes_2"]);
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const ComputeBooleanSymmetricDifferenceTestParams& params, std::ostream* os)
{
    *os << "Testing " << params.name << "...\n";
    *os << "shapes_1\n";
    for (const ShapeWithHoles& shape: params.shapes_1.shapes_with_holes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "shapes_2\n";
    for (const ShapeWithHoles& shape: params.shapes_2.shapes_with_holes)
        *os << "- " << shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << "- " << shape.to_string(2) << "\n";
}

class ComputeBooleanSymmetricDifferenceTest: public testing::TestWithParam<ComputeBooleanSymmetricDifferenceTestParams> { };

TEST_P(ComputeBooleanSymmetricDifferenceTest, ComputeBooleanSymetricDifference)
{
    ComputeBooleanSymmetricDifferenceTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    auto output = compute_symmetric_difference(
            test_params.shapes_1,
            test_params.shapes_2).shapes_with_holes;
    std::cout << "output" << std::endl;
    for (const ShapeWithHoles& shape: output)
        std::cout << "- " << shape.to_string(2) << std::endl;

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
        ComputeBooleanSymmetricDifferenceTest,
        testing::ValuesIn(std::vector<ComputeBooleanSymmetricDifferenceTestParams>{
            ComputeBooleanSymmetricDifferenceTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "symmetric_difference" / "000.json").string()),
        }),
        [](const testing::TestParamInfo<ComputeBooleanSymmetricDifferenceTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ExtractOutlineTestParams
{
    std::string name;
    Shape shape;
    Shape expected_output;


    static ExtractOutlineTestParams read_json(
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
        ExtractOutlineTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.expected_output = Shape::from_json(json["expected_output"]);
        return test_params;
    }
};

void PrintTo(const ExtractOutlineTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected_output " << params.expected_output.to_string(2) << "\n";
}

class ExtractOutlineTest: public testing::TestWithParam<ExtractOutlineTestParams> { };

TEST_P(ExtractOutlineTest, ExtractOutline)
{
    ExtractOutlineTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    Shape output = extract_outline(test_params.shape);
    std::cout << "output " << output.to_string(2) << std::endl;
    //Writer()
    //    .add_shape(test_params.shape)
    //    .add_shape(test_params.expected_output)
    //    .add_shape(output)
    //    .write_json("extract_outline_output.json");

    ASSERT_TRUE(equal(output, test_params.expected_output));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ExtractOutlineTest,
        testing::ValuesIn(std::vector<ExtractOutlineTestParams>{
            {
                "StarShape",
                build_shape({{0, 0}, {10, 0}, {10, 10}, {1, 1}, {9, 1}, {0, 10}}),
                build_shape({{0, 0}, {10, 0}, {10, 10}, {5, 5}, {0, 10}}),
            },
            ExtractOutlineTestParams::read_json(
                    (fs::path("data") / "tests" / "boolean_operations" / "extract_outline" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<ExtractOutlineTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });


struct ExtractFacesTestParams
{
    std::string name;
    Shape shape;
    std::vector<Shape> expected_output;


    static ExtractFacesTestParams read_json(
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
        ExtractFacesTestParams test_params;
        test_params.name = file_path;
        test_params.shape = Shape::from_json(json["shape"]);
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(Shape::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const ExtractFacesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const Shape& shape: params.expected_output)
        *os << shape.to_string(2) << "\n";
}

class ExtractFacesTest: public testing::TestWithParam<ExtractFacesTestParams> { };

TEST_P(ExtractFacesTest, ExtractFaces)
{
    ExtractFacesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<Shape> output = extract_faces(test_params.shape);
    std::cout << "output" << std::endl;
    for (const Shape& shape: output)
        std::cout << shape.to_string(2) << std::endl;
    //Writer()
    //    .add_shape(test_params.shape)
    //    .add_shapes(test_params.expected_output)
    //    .add_shapes(output)
    //    .write_json("bridge_touching_holes.json");

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
        ExtractFacesTest,
        testing::ValuesIn(std::vector<ExtractFacesTestParams>{
            {
                "Rhombus",
                build_shape({{0, 10}, {10, 0}, {20, 10}, {10, 20}}),
                {build_shape({{0, 10}, {10, 0}, {20, 10}, {10, 20}})},
            }, {
                "RhombusReversed",
                build_shape({{0, 10}, {10, 0}, {20, 10}, {10, 20}}).reverse(),
                {build_shape({{0, 10}, {10, 0}, {20, 10}, {10, 20}})},
            }, {
                "SelfIntersectingBowtie",
                build_shape({{0, 0}, {20, 20}, {20, 0}, {0, 20}}),
                {
                    build_shape({{0, 0}, {10, 10}, {0, 20}}),
                    build_shape({{20, 0}, {20, 20}, {10, 10}}),
                },
            }
        }),
        [](const testing::TestParamInfo<ExtractFacesTest::ParamType>& info) {
            return info.param.name;
        });


struct BridgeTouchingHolesTestParams
{
    std::string name;
    ShapeWithHoles shape;
    std::vector<ShapeWithHoles> expected_output;


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
        BridgeTouchingHolesTestParams test_params;
        test_params.name = file_path;
        test_params.shape = ShapeWithHoles::from_json(json["shape"]);
        for (auto& json_shape: json["expected_output"].items())
            test_params.expected_output.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
        return test_params;
    }
};

void PrintTo(const BridgeTouchingHolesTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(2) << "\n";
    *os << "expected_output\n";
    for (const ShapeWithHoles& shape: params.expected_output)
        *os << shape.to_string(2) << "\n";
}

class BridgeTouchingHolesTest: public testing::TestWithParam<BridgeTouchingHolesTestParams> { };

TEST_P(BridgeTouchingHolesTest, BridgeTouchingHoles)
{
    BridgeTouchingHolesTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<ShapeWithHoles> output = bridge_touching_holes(test_params.shape).shapes_with_holes;
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
                "NoHole",
                {build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})},
                {{build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}})}},
            }, {  // Shape with one hole not touching its outline.
                "OneNonTouchingHole",
                {
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                }, {{
                    build_shape({{0, 0}, {20, 0}, {20, 10}, {0, 10}}),
                    {build_shape({{15, 4}, {16, 5}, {15, 6}, {14, 5}})}
                }},
            }, {  // Shape with one hole touching its outline.
                "OneTouchingOutline",
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
                "OneTouchingVertex",
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
                "OutlineTouchingHoleTouchingHole",
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
                "ThreeChainedHoles",
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
                "VertexTouchingHole",
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
                "VertexTouchingHoleTouchingHole",
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
                "ThreeChainedHolesVertex",
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
        }),
        [](const testing::TestParamInfo<BridgeTouchingHolesTest::ParamType>& info) {
            return fs::path(info.param.name).stem().string();
        });
