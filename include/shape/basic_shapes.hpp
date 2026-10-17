#pragma once

#include "shape/shape.hpp"

#include <vector>

namespace shape
{

enum class BasicShapeType
{
    ConvexPolygon,
    CircularSegment,
    Circle,
};

/**
 * A basic shape:
 * - ConvexPolygon: shape is the polygon K
 * - CircularSegment: shape is the segment itself, closed by the arc and its
 *   chord (one CircularArc element followed by one LineSegment element back
 *   to the arc's start); circle_center and circle_radius redundantly give
 *   the arc's circle.
 * - Circle: shape is a full circle (a single CircularArc element of
 *   orientation Full); circle_center and circle_radius redundantly give it.
 */
struct BasicShape
{
    BasicShapeType type = BasicShapeType::ConvexPolygon;
    Shape shape;
    Point circle_center = {0, 0};
    LengthDbl circle_radius = 0;
};

/** Output of decompose_into_basic_shapes. */
struct BasicShapesDecomposition
{
    std::vector<BasicShape> basic_shapes;
};

/**
 * Decompose a shape (with holes) whose boundary consists of line segments and
 * convex circular arcs into basic objects: convex polygons, circular segments
 * and circles.
 *
 * The union of the basic shapes is 'shape' and their interiors are pairwise
 * disjoint. Each basic shape is anticlockwise.
 *
 * A disk, i.e. a shape without holes whose outline is a full circle, gives a
 * single Circle. Otherwise, the arc of each circular segment spans strictly
 * less than 180°, and each arc of 'shape' is the union of the arcs of one or
 * several circular segments; an outline which is a full circle is the union
 * of the arcs of several circular segments.
 *
 * Holes are stored in the same anticlockwise direction as the outer boundary
 * (see Shape's own doc comment), so the arc orientation that is "convex from
 * the material's side" is Anticlockwise for the outer boundary but Clockwise
 * for a hole: a Clockwise arc in a hole is material bulging into the void,
 * the hole equivalent of a convex bump. A hole which is a full circle is
 * concave from the material's side.
 *
 * Throw std::invalid_argument if 'shape' isn't valid (see
 * ShapeWithHoles::check), or if it has a circular arc which isn't convex from
 * the material's side, such as a hole which is a full circle.
 */
BasicShapesDecomposition decompose_into_basic_shapes(const ShapeWithHoles& shape);

/** Convenience overload for a shape with no holes. */
inline BasicShapesDecomposition decompose_into_basic_shapes(const Shape& shape)
{
    ShapeWithHoles shape_with_holes;
    shape_with_holes.shape = shape;
    return decompose_into_basic_shapes(shape_with_holes);
}

}
