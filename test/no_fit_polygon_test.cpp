#include "shape/no_fit_polygon.hpp"

#include "shape/shapes_intersections.hpp"

#include <gtest/gtest.h>

using namespace shape;


////////////////////////////////////////////////////////////////////////////////
// Convex overload
////////////////////////////////////////////////////////////////////////////////

struct NoFitPolygonConvexTestParams
{
    Shape fixed_shape;
    Shape orbiting_shape;
    Shape expected_nfp;
};

class NoFitPolygonConvexTest:
    public testing::TestWithParam<NoFitPolygonConvexTestParams> { };

TEST_P(NoFitPolygonConvexTest, NoFitPolygonConvex)
{
    NoFitPolygonConvexTestParams test_params = GetParam();
    std::cout << "fixed_shape " << test_params.fixed_shape.to_string(0) << std::endl;
    std::cout << "orbiting_shape " << test_params.orbiting_shape.to_string(0) << std::endl;

    Shape nfp = no_fit_polygon(test_params.fixed_shape, test_params.orbiting_shape);

    std::cout << "nfp " << nfp.to_string(0) << std::endl;

    EXPECT_TRUE(equal(nfp, test_params.expected_nfp));

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
                build_rectangle(-2, 4, -2, 4),
            }, {  // Rectangle with itself.
                build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}),
                build_shape({{0, 0}, {3, 0}, {3, 2}, {0, 2}}),
                build_rectangle(-3, 3, -2, 2),
            }, {  // Square and triangle.
                build_rectangle(0, 4, 0, 4),
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                build_shape({{-1, -2}, {3, -2}, {4, 0}, {4, 4}, {-2, 4}, {-2, 0}}),
            }, {  // Triangle and triangle.
                build_shape({{0, 0}, {4, 0}, {2, 4}}),
                build_shape({{0, 0}, {2, 0}, {1, 2}}),
                build_shape({{-1, -2}, {3, -2}, {4, 0}, {2, 4}, {0, 4}, {-2, 0}}),
            }, {  // Convex pentagon and unit square.
                build_shape({{0, 0}, {4, 0}, {5, 2}, {3, 4}, {1, 4}}),
                build_rectangle(0, 1, 0, 1),
                build_shape({{-1, -1}, {4, -1}, {5, 1}, {5, 2}, {3, 4}, {0, 4}, {-1, 0}}),
            },
        }));


////////////////////////////////////////////////////////////////////////////////
// General (non-convex) overload
////////////////////////////////////////////////////////////////////////////////

struct NoFitPolygonGeneralTestParams
{
    ShapeWithHoles fixed_shape;
    ShapeWithHoles orbiting_shape;
    ShapePos expected_num_components;
};

class NoFitPolygonGeneralTest:
    public testing::TestWithParam<NoFitPolygonGeneralTestParams> { };

TEST_P(NoFitPolygonGeneralTest, NoFitPolygonGeneral)
{
    NoFitPolygonGeneralTestParams test_params = GetParam();
    std::cout << "fixed_shape " << test_params.fixed_shape.to_string(0) << std::endl;
    std::cout << "orbiting_shape " << test_params.orbiting_shape.to_string(0) << std::endl;

    std::vector<ShapeWithHoles> nfp = no_fit_polygon(
            test_params.fixed_shape,
            test_params.orbiting_shape);

    std::cout << "nfp (" << nfp.size() << " component(s))" << std::endl;
    for (const ShapeWithHoles& component: nfp)
        std::cout << "  " << component.to_string(0) << std::endl;

    EXPECT_EQ((ShapePos)nfp.size(), test_params.expected_num_components);

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
                1,
            }, {  // L-shape fixed, unit square orbiting: one connected NFP.
                {build_shape({{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}}), {}},
                {build_rectangle(0, 1, 0, 1), {}},
                1,
            }, {  // Two L-shapes.
                {build_shape({{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}}), {}},
                {build_shape({{0, 0}, {2, 0}, {2, 1}, {1, 1}, {1, 2}, {0, 2}}), {}},
                1,
            }, {  // T-shape fixed, unit square orbiting.
                {build_shape({{0, 2}, {1, 2}, {1, 0}, {2, 0}, {2, 2}, {3, 2}, {3, 3}, {0, 3}}), {}},
                {build_rectangle(0, 1, 0, 1), {}},
                1,
            },
        }));
