#include "shape/no_fit_polygon.hpp"

#include "shape/shapes_intersections.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>
#include <cmath>
#include <fstream>

namespace fs = boost::filesystem;

using namespace shape;


////////////////////////////////////////////////////////////////////////////////
// Convex overload
////////////////////////////////////////////////////////////////////////////////

struct NoFitPolygonConvexTestParams
{
    Shape fixed_shape;
    Shape orbiting_shape;
    std::vector<ShapeWithHoles> expected_nfp;
    std::string name;
};

void PrintTo(const NoFitPolygonConvexTestParams& params, std::ostream* os)
{
    *os << "fixed_shape " << params.fixed_shape.to_string(0) << "\n";
    *os << "orbiting_shape " << params.orbiting_shape.to_string(0) << "\n";
}

class NoFitPolygonConvexTest:
    public testing::TestWithParam<NoFitPolygonConvexTestParams> { };

TEST_P(NoFitPolygonConvexTest, NoFitPolygonConvex)
{
    NoFitPolygonConvexTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    Shape nfp = no_fit_polygon(test_params.fixed_shape, test_params.orbiting_shape);

    std::cout << "nfp " << nfp.to_string(0) << std::endl;

    ASSERT_EQ((ShapePos)1, (ShapePos)test_params.expected_nfp.size());
    EXPECT_TRUE(equal(ShapeWithHoles{nfp, {}}, test_params.expected_nfp[0]));

    // Oracle check: sample a grid around the NFP bounding box and verify that
    // strictly-inside positions cause overlap and strictly-outside ones do not.
    AxisAlignedBoundingBox aabb = nfp.compute_min_max();
    const double margin = 0.5;
    const double step = 0.25;
    for (double px = aabb.x_min - margin; px <= aabb.x_max + margin; px += step) {
        for (double py = aabb.y_min - margin; py <= aabb.y_max + margin; py += step) {
            Point position = {px, py};

            if (nfp.contains(position, /*strict=*/true)) {
                Shape translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_TRUE(intersect(test_params.fixed_shape, translated_orbiting))
                    << "Position (" << px << ", " << py << ") is inside the NFP "
                    << "but the translated orbiting shape does not intersect the fixed shape.";
            }
            if (!nfp.contains(position, /*strict=*/false)) {
                Shape translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_FALSE(intersect(test_params.fixed_shape, translated_orbiting))
                    << "Position (" << px << ", " << py << ") is outside the NFP "
                    << "but the translated orbiting shape intersects the fixed shape.";
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        NoFitPolygonConvexTest,
        testing::ValuesIn(std::vector<NoFitPolygonConvexTestParams>{
            {  // Square and smaller square: NFP is a larger square.
                build_rectangle(0, 4, 0, 4),
                build_rectangle(0, 2, 0, 2),
                {{build_rectangle(-2, 4, -2, 4), {}}},
                "SquareAndSmallerSquare",
            }, {  // Rectangle with itself.
                build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}),
                build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}),
                {{build_rectangle(-3, 3, -2, 2), {}}},
                "RectangleWithItself",
            }, {  // Square and triangle.
                build_rectangle(0, 4, 0, 4),
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                {{build_shape({{-1, -2}, {3, -2}, {4, 0}, {4, 4}, {-2, 4}, {-2, 0}}), {}}},
                "SquareAndTriangle",
            }, {  // Triangle and triangle.
                build_shape({{0, 0}, {4, 0}, {2, 4}}),
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                {{build_shape({{-1, -2}, {3, -2}, {4, 0}, {2, 4}, {0, 4}, {-2, 0}}), {}}},
                "TriangleAndTriangle",
            }, {  // Convex pentagon and unit square.
                build_shape({{0, 0}, {4, 0}, {5, 2}, {3, 4}, {1, 4}}),
                build_rectangle(0, 1, 0, 1),
                {{build_shape({{-1, -1}, {4, -1}, {5, 1}, {5, 2}, {3, 4}, {0, 4}, {-1, 0}}), {}}},
                "ConvexPentagonAndUnitSquare",
            },
        }),
        [](const testing::TestParamInfo<NoFitPolygonConvexTest::ParamType>& info) {
            return info.param.name;
        });


////////////////////////////////////////////////////////////////////////////////
// General (non-convex) overload
////////////////////////////////////////////////////////////////////////////////

struct NoFitPolygonGeneralTestParams
{
    ShapeWithHoles fixed_shape;
    ShapeWithHoles orbiting_shape;
    // Each entry is one acceptable full nfp result. Normally there is just
    // one, but a computation that (legitimately) depends on the exact
    // rounding of intermediate floating-point operations -- e.g. whether
    // the compiler fuses multiply-adds into a single instruction, which
    // differs by ISA (always on AArch64, opt-in via -mfma on x86-64) --
    // can settle on more than one, equally valid, exact result depending
    // on the build. Recording each such result as its own variant here
    // lets the test accept any of them, rather than only the one recorded
    // on whichever platform happened to generate the fixture.
    std::vector<std::vector<ShapeWithHoles>> expected_nfp_variants;
    std::string name;

    // Skips the bounding-box grid-sampling oracle check below: appropriate
    // for a large, real-world shape (a fixed 0.25 step over its bounding box
    // would mean hundreds of thousands of intersect() calls against a
    // several-hundred-vertex shape), where exact equality against a
    // recorded expected_nfp variant is the only practical check.
    bool skip_oracle_check = false;

    static NoFitPolygonGeneralTestParams read_json(
            const std::string& file_path,
            const std::string& name,
            bool skip_oracle_check = false)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        NoFitPolygonGeneralTestParams test_params;
        test_params.fixed_shape = ShapeWithHoles::from_json(json["shapes"][0]);
        test_params.orbiting_shape = ShapeWithHoles::from_json(json["shapes"][1]);
        if (json.contains("expected_outputs")) {
            for (auto& json_variant: json["expected_outputs"].items()) {
                std::vector<ShapeWithHoles> variant;
                for (auto& json_shape: json_variant.value().items())
                    variant.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
                test_params.expected_nfp_variants.push_back(std::move(variant));
            }
        } else {
            std::vector<ShapeWithHoles> variant;
            for (auto& json_shape: json["expected_output"].items())
                variant.emplace_back(ShapeWithHoles::from_json(json_shape.value()));
            test_params.expected_nfp_variants.push_back(std::move(variant));
        }
        test_params.name = name;
        test_params.skip_oracle_check = skip_oracle_check;
        return test_params;
    }
};

void PrintTo(const NoFitPolygonGeneralTestParams& params, std::ostream* os)
{
    *os << "fixed_shape " << params.fixed_shape.to_string(0) << "\n";
    *os << "orbiting_shape " << params.orbiting_shape.to_string(0) << "\n";
}

class NoFitPolygonGeneralTest:
    public testing::TestWithParam<NoFitPolygonGeneralTestParams> { };

TEST_P(NoFitPolygonGeneralTest, NoFitPolygonGeneral)
{
    NoFitPolygonGeneralTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    std::vector<ShapeWithHoles> nfp = no_fit_polygon(
            test_params.fixed_shape,
            test_params.orbiting_shape).shapes_with_holes;

    std::cout << "nfp (" << nfp.size() << " component(s))" << std::endl;
    for (const ShapeWithHoles& component: nfp)
        std::cout << "  " << component.to_string(0) << std::endl;

    bool matches_a_variant = false;
    for (const std::vector<ShapeWithHoles>& variant: test_params.expected_nfp_variants) {
        if (nfp.size() != variant.size())
            continue;
        bool all_equal = true;
        for (ShapePos i = 0; i < (ShapePos)nfp.size(); ++i) {
            if (!equal(nfp[i], variant[i])) {
                all_equal = false;
                break;
            }
        }
        if (all_equal) {
            matches_a_variant = true;
            break;
        }
    }
    EXPECT_TRUE(matches_a_variant)
        << "nfp did not exactly match any of the "
        << test_params.expected_nfp_variants.size()
        << " recorded expected_output variant(s).";

    if (test_params.skip_oracle_check)
        return;

    // Oracle check: sample a grid around the union of all NFP components.
    AxisAlignedBoundingBox aabb;
    for (const ShapeWithHoles& component: nfp)
        aabb = merge(aabb, component.compute_min_max());

    auto inside_nfp = [&](const Point& point) -> bool {
        for (const ShapeWithHoles& component: nfp) {
            if (component.contains(point, /*strict=*/true))
                return true;
        }
        return false;
    };

    auto outside_nfp = [&](const Point& point) -> bool {
        for (const ShapeWithHoles& component: nfp) {
            if (component.contains(point, /*strict=*/false))
                return false;
        }
        return true;
    };

    const double margin = 0.5;
    const double step = 0.25;
    for (double px = aabb.x_min - margin; px <= aabb.x_max + margin; px += step) {
        for (double py = aabb.y_min - margin; py <= aabb.y_max + margin; py += step) {
            Point position = {px, py};

            if (inside_nfp(position)) {
                ShapeWithHoles translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_TRUE(intersect(
                            test_params.fixed_shape.shape,
                            translated_orbiting.shape))
                    << "Position (" << px << ", " << py << ") is inside the NFP "
                    << "but the translated orbiting shape does not intersect the fixed shape.";
            }
            if (outside_nfp(position)) {
                ShapeWithHoles translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_FALSE(intersect(
                            test_params.fixed_shape.shape,
                            translated_orbiting.shape))
                    << "Position (" << px << ", " << py << ") is outside the NFP "
                    << "but the translated orbiting shape intersects the fixed shape.";
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        NoFitPolygonGeneralTest,
        testing::ValuesIn(std::vector<NoFitPolygonGeneralTestParams>{
            {  // Convex inputs: same result as the convex overload, one component.
                {build_rectangle(0, 4, 0, 4), {}},
                {build_rectangle(0, 2, 0, 2), {}},
                {{{build_rectangle(-2, 4, -2, 4), {}}}},
                "ConvexSquares",
            }, {  // L-shape fixed, unit square orbiting: one connected NFP.
                {build_shape({{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}}), {}},
                {build_rectangle(0, 1, 0, 1), {}},
                {{{build_shape({{4, 2}, {2, 2}, {2, 4}, {-1, 4}, {-1, -1}, {4, -1}}), {}}}},
                "LShapeAndUnitSquare",
            }, {  // Two L-shapes.
                {build_shape({{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}}), {}},
                {build_shape({{0, 0}, {2, 0}, {2, 1}, {1, 1}, {1, 2}, {0, 2}}), {}},
                {{{build_shape({{4, 2}, {2, 2}, {2, 4}, {-2, 4}, {-2, -1}, {-1, -1}, {-1, -2}, {4, -2}}), {}}}},
                "TwoLShapes",
            }, {  // T-shape fixed, unit square orbiting.
                {build_shape({{0, 2}, {1, 2}, {1, 0}, {2, 0}, {2, 2}, {3, 2}, {3, 3}, {0, 3}}), {}},
                {build_rectangle(0, 1, 0, 1), {}},
                {{{build_shape({{3, 3}, {-1, 3}, {-1, 1}, {0, 1}, {0, -1}, {2, -1}, {2, 1}, {3, 1}}), {}}}},
                "TShapeAndUnitSquare",
            },
            // Regression test for fontanf/packingsolver#558 (test cases 1
            // and 3): the two shapes in 000.json are a real sawtooth strip
            // item (fine repetitive teeth), scaled the way packingsolver
            // actually scales it (item coordinates ~250 in magnitude,
            // matching its scale_value normalization) then simplified down
            // to 442 vertices, and its own 180-degree rotation -- the
            // pairing periodic_packing.cpp relies on to detect an
            // interlocking placement.
            //
            // no_fit_polygon(shape_0, shape_r) used to segfault. Deep in the
            // computation, a face gets extracted (as a Union's outline)
            // whose boundary retraces itself almost exactly -- three
            // vertices that are collinear to within floating-point
            // precision, confirmed by compute_intersections() reporting a
            // genuine overlapping_parts match between two of the face's
            // edges, not a fuzzy near-miss. Its computed area lands right at
            // the floating-point noise floor (observed as low as ~1e-15,
            // but not reliably below any fixed epsilon -- other occurrences
            // of the same shape measured ~1.5e-12, defeating an attempted
            // fix that skipped faces below 1e-12).
            //
            // fix_self_intersections() passes this degenerate face to
            // bridge_touching_holes(), which runs a boolean Difference on
            // it. Because its boundary retraces itself, the arc-tracing
            // algorithm has no independent "reverse" arc set to recover a
            // face from (forward and reverse are the same arcs, unlike for
            // a normal, non-self-overlapping shape) -- so
            // bridge_touching_holes() legitimately returns zero components.
            // fix_self_intersections() then indexed .front() into that
            // empty result unconditionally: segfault. Fixed by only
            // appending a per-component union result when it's non-empty.
            //
            // The oracle grid-sampling check below is skipped for this case
            // (see skip_oracle_check's comment).
            NoFitPolygonGeneralTestParams::read_json(
                    (fs::path("data") / "tests" / "no_fit_polygon" / "000.json").string(),
                    "Issue558SawtoothSelfPairing",
                    /*skip_oracle_check=*/true),
        }),
        [](const testing::TestParamInfo<NoFitPolygonGeneralTest::ParamType>& info) {
            return info.param.name;
        });


////////////////////////////////////////////////////////////////////////////////
// Convex overload with circular arcs (decompose_into_basic_shapes' circular
// segments: one CircularArc element closed by its chord).
//
// No hand-derived expected shape here (impractical for arc geometry); these
// are verified purely via the oracle grid-sampling check, same technique as
// above: every point strictly inside the NFP must cause overlap when used to
// translate orbiting_shape, and every point strictly outside must not.
////////////////////////////////////////////////////////////////////////////////

namespace
{

// A circular segment: arc from angle_1_deg to angle_2_deg (< 180 degrees
// apart) around 'center' with 'radius', anticlockwise, closed by the chord
// back to the arc's start -- exactly the shape decompose_into_basic_shapes
// produces for a BasicShapeType::CircularSegment.
Shape build_circular_segment(
        Point center,
        LengthDbl radius,
        Angle angle_1_deg,
        Angle angle_2_deg)
{
    Angle a1 = angle_1_deg * M_PI / 180.0;
    Angle a2 = angle_2_deg * M_PI / 180.0;
    Point p1 = {center.x + radius * std::cos(a1), center.y + radius * std::sin(a1)};
    Point p2 = {center.x + radius * std::cos(a2), center.y + radius * std::sin(a2)};
    Shape s;
    s.elements.push_back(build_circular_arc(p1, p2, center, ShapeElementOrientation::Anticlockwise));
    s.elements.push_back(build_line_segment(p2, p1));
    return s;
}

}  // namespace

struct NoFitPolygonArcOracleTestParams
{
    Shape fixed_shape;
    Shape orbiting_shape;
    std::string name;
};

std::ostream& operator<<(std::ostream& os, const NoFitPolygonArcOracleTestParams& params)
{
    os << params.name;
    return os;
}

class NoFitPolygonArcOracleTest:
    public testing::TestWithParam<NoFitPolygonArcOracleTestParams> { };

TEST_P(NoFitPolygonArcOracleTest, NoFitPolygonArcOracle)
{
    NoFitPolygonArcOracleTestParams test_params = GetParam();
    std::cout << "fixed_shape " << test_params.fixed_shape.to_string(0) << std::endl;
    std::cout << "orbiting_shape " << test_params.orbiting_shape.to_string(0) << std::endl;

    Shape nfp = no_fit_polygon(test_params.fixed_shape, test_params.orbiting_shape);
    std::cout << "nfp " << nfp.to_string(0) << std::endl;

    AxisAlignedBoundingBox aabb = nfp.compute_min_max();
    const double margin = 0.5;
    const double step = 0.2;
    // Offset the grid off exact fractions of a degree / nice coordinates:
    // sampling exactly on a shape boundary (e.g. at an arc's own extremal
    // point) is a degenerate case for point-in-polygon ray casting that is
    // not what this test is trying to exercise.
    for (double px = aabb.x_min - margin + 0.0137; px <= aabb.x_max + margin; px += step) {
        for (double py = aabb.y_min - margin + 0.0211; py <= aabb.y_max + margin; py += step) {
            Point position = {px, py};

            if (nfp.contains(position, /*strict=*/true)) {
                Shape translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_TRUE(intersect(test_params.fixed_shape, translated_orbiting))
                    << "Position (" << px << ", " << py << ") is inside the NFP "
                    << "but the translated orbiting shape does not intersect the fixed shape.";
            }
            if (!nfp.contains(position, /*strict=*/false)) {
                Shape translated_orbiting = test_params.orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_FALSE(intersect(test_params.fixed_shape, translated_orbiting))
                    << "Position (" << px << ", " << py << ") is outside the NFP "
                    << "but the translated orbiting shape intersects the fixed shape.";
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        NoFitPolygonArcOracleTest,
        testing::ValuesIn(std::vector<NoFitPolygonArcOracleTestParams>{
            {  // Circular segment (no interaction needed with the square's edges).
                build_circular_segment({0, 0}, 2.0, 10, 80),
                build_rectangle(0, 1, 0, 1),
                "SegmentAndSquare",
            }, {  // A circular segment against itself: full arc-arc overlap
                  // (the arc-arc "sum" path, both arcs identical).
                build_circular_segment({0, 0}, 1.5, 20, 100),
                build_circular_segment({0, 0}, 1.5, 20, 100),
                "SegmentSelf",
            }, {  // Two different-radius segments with partially overlapping
                  // arc ranges: exercises peeling a lone prefix/suffix around
                  // a summed middle range.
                build_circular_segment({0, 0}, 1.0, 0, 90),
                build_circular_segment({0, 0}, 2.0, 45, 135),
                "SegmentsPartialOverlap",
            }, {  // Two segments with disjoint (non-overlapping) arc ranges.
                build_circular_segment({0, 0}, 1.0, 0, 60),
                build_circular_segment({0, 0}, 1.0, 120, 170),
                "SegmentsDisjointRanges",
            }, {  // A wide (170 degree) segment against a triangle: one of
                  // the triangle's edges falls squarely inside the arc's
                  // span, forcing a reactive mid-arc split.
                build_circular_segment({0, 0}, 1.5, 0, 170),
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                "WideSegmentAndTriangle",
            }, {  // Segment as the orbiting shape instead of fixed (exercises
                  // the negation path independently of which side has the arc).
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                build_circular_segment({0, 0}, 1.5, 0, 170),
                "TriangleAndWideSegment",
            },
        }),
        [](const testing::TestParamInfo<NoFitPolygonArcOracleTest::ParamType>& info) {
            return info.param.name;
        });


////////////////////////////////////////////////////////////////////////////////
// General overload with a rounded-corner shape, exercising
// decompose_into_basic_shapes end-to-end (not just the convex-convex core).
////////////////////////////////////////////////////////////////////////////////

TEST(NoFitPolygonGeneralArcTest, RoundedCornerSquare)
{
    // A 4x4 square with its top-right corner replaced by a quarter-circle
    // fillet of radius 1.
    Shape rounded;
    rounded.elements.push_back(build_line_segment({0, 0}, {4, 0}));
    rounded.elements.push_back(build_line_segment({4, 0}, {4, 3}));
    rounded.elements.push_back(build_circular_arc(
            {4, 3}, {3, 4}, {3, 3}, ShapeElementOrientation::Anticlockwise));
    rounded.elements.push_back(build_line_segment({3, 4}, {0, 4}));
    rounded.elements.push_back(build_line_segment({0, 4}, {0, 0}));
    ShapeWithHoles fixed_shape{rounded, {}};
    ShapeWithHoles orbiting_shape{build_rectangle(0, 1, 0, 1), {}};

    MultiShapeWithHoles nfp = no_fit_polygon(fixed_shape, orbiting_shape);
    std::cout << "nfp (" << nfp.shapes_with_holes.size() << " component(s))" << std::endl;
    for (const ShapeWithHoles& component: nfp.shapes_with_holes)
        std::cout << "  " << component.to_string(0) << std::endl;

    ASSERT_EQ((ShapePos)nfp.shapes_with_holes.size(), (ShapePos)1);

    AxisAlignedBoundingBox aabb = nfp.shapes_with_holes[0].compute_min_max();
    const double margin = 0.5;
    const double step = 0.2;
    for (double px = aabb.x_min - margin + 0.0137; px <= aabb.x_max + margin; px += step) {
        for (double py = aabb.y_min - margin + 0.0211; py <= aabb.y_max + margin; py += step) {
            Point position = {px, py};
            const ShapeWithHoles& component = nfp.shapes_with_holes[0];

            if (component.contains(position, /*strict=*/true)) {
                ShapeWithHoles translated_orbiting = orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_TRUE(intersect(fixed_shape.shape, translated_orbiting.shape))
                    << "Position (" << px << ", " << py << ") is inside the NFP "
                    << "but the translated orbiting shape does not intersect the fixed shape.";
            }
            if (!component.contains(position, /*strict=*/false)) {
                ShapeWithHoles translated_orbiting = orbiting_shape;
                translated_orbiting.shift(position.x, position.y);
                EXPECT_FALSE(intersect(fixed_shape.shape, translated_orbiting.shape))
                    << "Position (" << px << ", " << py << ") is outside the NFP "
                    << "but the translated orbiting shape intersects the fixed shape.";
            }
        }
    }
}
