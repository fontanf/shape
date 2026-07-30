#include "shape/basic_shapes.hpp"

#include "shape/elements_intersections.hpp"

#include <gtest/gtest.h>

using namespace shape;

TEST(BasicShapesTest, PurePolygonProducesNoCircularSegment)
{
    Shape s = build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}});
    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    ASSERT_EQ(decomp.basic_shapes.size(), 1);
    EXPECT_EQ(decomp.basic_shapes[0].type, BasicShapeType::ConvexPolygon);
}

TEST(BasicShapesTest, TriangleWithInflatedHypotenuse)
{
    // Right triangle (0,0)-(1,0)-(0,1) where the hypotenuse is replaced by a
    // convex arc bulging outward, centered on the right-angle vertex (0,0)
    // (both (1,0) and (0,1) are at distance 1 from it, a valid arc).
    Shape s = build_shape({{0, 0}, {1, 0}, {0, 0, 1}, {0, 1}});
    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    ASSERT_EQ(decomp.basic_shapes.size(), 2);

    Counter segment_pos = -1;
    Counter polygon_pos = -1;
    for (Counter basic_shape_pos = 0;
            basic_shape_pos < (Counter)decomp.basic_shapes.size();
            ++basic_shape_pos) {
        if (decomp.basic_shapes[basic_shape_pos].type == BasicShapeType::CircularSegment)
            segment_pos = basic_shape_pos;
        else
            polygon_pos = basic_shape_pos;
    }
    ASSERT_NE(segment_pos, -1);
    ASSERT_NE(polygon_pos, -1);

    const BasicShape& segment = decomp.basic_shapes[segment_pos];
    EXPECT_NEAR(segment.circle_center.x, 0.0, 1e-9);
    EXPECT_NEAR(segment.circle_center.y, 0.0, 1e-9);
    EXPECT_NEAR(segment.circle_radius, 1.0, 1e-9);
    ASSERT_EQ(segment.shape.elements.size(), 2);
    EXPECT_EQ(segment.shape.elements[0].type, ShapeElementType::CircularArc);
    EXPECT_EQ(segment.shape.elements[1].type, ShapeElementType::LineSegment);
    // The arc element itself must carry the same circle as circle_center/radius.
    EXPECT_NEAR(segment.shape.elements[0].center.x, segment.circle_center.x, 1e-9);
    EXPECT_NEAR(segment.shape.elements[0].center.y, segment.circle_center.y, 1e-9);
    EXPECT_TRUE(segment.shape.is_convex());

    // The remaining polygon must still be the plain right triangle.
    EXPECT_TRUE(decomp.basic_shapes[polygon_pos].shape.is_convex());
    for (const ShapeElement& element: decomp.basic_shapes[polygon_pos].shape.elements)
        EXPECT_EQ(element.type, ShapeElementType::LineSegment);
}

TEST(BasicShapesTest, ArcOfAtLeast180DegreesIsSubdivided)
{
    // A shape whose whole boundary is a single 270-degree arc (plus its
    // closing chord): must be subdivided since 270 >= 180, so every
    // resulting CircularSegment must have an arc strictly less than 180.
    Shape s = build_shape({{1, 0}, {0, 0, 1}, {-1, 0}, {0, 0, 1}, {0, -1}});
    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    for (const BasicShape& basic_shape: decomp.basic_shapes) {
        if (basic_shape.type != BasicShapeType::CircularSegment)
            continue;
        const ShapeElement& arc = basic_shape.shape.elements[0];
        LengthDbl arc_angle = angle_radian(
                arc.start - arc.center,
                arc.end - arc.center);
        EXPECT_LT(arc_angle, M_PI);
    }
}

TEST(BasicShapesTest, HoleWithMaterialBulgingIntoIt)
{
    // A square with a square hole in the middle, except one edge of the hole
    // is replaced by a clockwise arc (material bulging into the void, the
    // hole equivalent of a convex bump), centered above the hole's top edge
    // so the arc dips down into the hole.
    ShapeWithHoles s;
    s.shape = build_shape({{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    s.holes = {build_shape({{3, 3}, {7, 3}, {7, 7}, {5, 8, -1}, {3, 7}})};

    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    Counter segment_pos = -1;
    for (Counter basic_shape_pos = 0;
            basic_shape_pos < (Counter)decomp.basic_shapes.size();
            ++basic_shape_pos) {
        if (decomp.basic_shapes[basic_shape_pos].type == BasicShapeType::CircularSegment) {
            ASSERT_EQ(segment_pos, -1) << "expected exactly one circular segment";
            segment_pos = basic_shape_pos;
        }
    }
    ASSERT_NE(segment_pos, -1) << "the clockwise arc in the hole must be cut into a circular segment";

    const BasicShape& segment = decomp.basic_shapes[segment_pos];
    EXPECT_NEAR(segment.circle_center.x, 5.0, 1e-9);
    EXPECT_NEAR(segment.circle_center.y, 8.0, 1e-9);
    EXPECT_NEAR(segment.circle_radius, std::sqrt(5.0), 1e-9);
    ASSERT_EQ(segment.shape.elements.size(), 2);
    EXPECT_EQ(segment.shape.elements[0].type, ShapeElementType::CircularArc);
    // The arc was Clockwise in the hole's own loop (convex-from-material
    // there), but the standalone segment.shape must itself be a valid
    // anticlockwise shape, so it is stored reversed here.
    EXPECT_EQ(segment.shape.elements[0].orientation, ShapeElementOrientation::Anticlockwise);

    // Every basic shape (including the segment itself) must be convex.
    for (const BasicShape& basic_shape: decomp.basic_shapes)
        EXPECT_TRUE(basic_shape.shape.is_convex());
}

TEST(BasicShapesTest, HoleArcSubdividedToAvoidCrossingAnotherHole)
{
    // Same bulging hole as above, but a second hole is placed directly in
    // the path of the first arc's natural chord ((7,7)-(3,7)): if
    // chord_intersects_boundary only checked the arc's own loop, this chord
    // would be accepted even though it cuts straight through hole_2.
    ShapeWithHoles s;
    s.shape = build_shape({{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    s.holes = {
        build_shape({{3, 3}, {7, 3}, {7, 7}, {5, 8, -1}, {3, 7}}),
        build_shape({{4, 6.5}, {6, 6.5}, {6, 7.5}, {4, 7.5}}),
    };

    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    bool found_segment = false;
    for (const BasicShape& basic_shape: decomp.basic_shapes) {
        if (basic_shape.type != BasicShapeType::CircularSegment)
            continue;
        found_segment = true;
        ASSERT_EQ(basic_shape.shape.elements.size(), 2);
        const ShapeElement& chord = basic_shape.shape.elements[1];
        EXPECT_EQ(chord.type, ShapeElementType::LineSegment);
        // The chord must not cross hole_2's boundary.
        for (const ShapeElement& hole_2_element: s.holes[1].elements) {
            ShapeElementIntersectionsOutput result = compute_intersections(chord, hole_2_element);
            EXPECT_TRUE(result.proper_intersections.empty());
            EXPECT_TRUE(result.overlapping_parts.empty());
        }
    }
    EXPECT_TRUE(found_segment);
}

TEST(BasicShapesTest, HoleArcSubdividedToAvoidTouchingAnotherHolesVertex)
{
    // Same bulging hole as above, but hole_2 is now a small triangle with one
    // vertex sitting exactly on the first arc's natural chord (the
    // horizontal segment y=7, x in [3,7]) at (5,7): a T-junction touch, not a
    // crossing, and not at either of the chord's own endpoints ((7,7) and
    // (3,7)). Per compute_line_line_intersections, a point that coincides
    // with one line's own endpoint lands in improper_intersections, not
    // proper_intersections, so this specifically exercises that category.
    ShapeWithHoles s;
    s.shape = build_shape({{0, 0}, {10, 0}, {10, 10}, {0, 10}});
    Shape hole_2 = build_shape({{5, 7}, {5.5, 7.5}, {4.5, 7.5}});
    s.holes = {
        build_shape({{3, 3}, {7, 3}, {7, 7}, {5, 8, -1}, {3, 7}}),
        hole_2,
    };

    BasicShapesDecomposition decomp = decompose_into_basic_shapes(s);

    bool found_segment = false;
    for (const BasicShape& basic_shape: decomp.basic_shapes) {
        if (basic_shape.type != BasicShapeType::CircularSegment)
            continue;
        found_segment = true;
        ASSERT_EQ(basic_shape.shape.elements.size(), 2);
        const ShapeElement& chord = basic_shape.shape.elements[1];
        // The chord must not even touch hole_2's boundary.
        for (const ShapeElement& hole_2_element: hole_2.elements) {
            ShapeElementIntersectionsOutput result = compute_intersections(chord, hole_2_element);
            EXPECT_TRUE(result.proper_intersections.empty());
            EXPECT_TRUE(result.improper_intersections.empty());
            EXPECT_TRUE(result.overlapping_parts.empty());
        }
    }
    EXPECT_TRUE(found_segment);
}
