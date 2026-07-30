#pragma once

#include "shape/shape.hpp"

#include <vector>

namespace shape
{

enum class BasicShapeType
{
    ConvexPolygon,
    CircularSegment,
};

/**
 * A basic shape:
 * - ConvexPolygon: shape is the polygon K
 * - CircularSegment: shape is the segment itself, closed by the arc and its
 *   chord (one CircularArc element followed by one LineSegment element back
 *   to the arc's start); circle_center and circle_radius redundantly give
 *   the arc's circle.
 */
struct BasicShape
{
    BasicShapeType type = BasicShapeType::ConvexPolygon;
    Shape shape;
    Point circle_center = {0, 0};
    LengthDbl circle_radius = 0;
};

struct BasicShapesDecomposition
{
    std::vector<BasicShape> basic_shapes;
};

/**
 * Decompose a shape (with holes) whose boundary consists of line segments and
 * convex circular arcs into basic objects: convex polygons and circular
 * segments.
 *
 * Each convex-from-the-material arc is cut off as a circular segment (arc +
 * chord), leaving a pure polygon (with holes) which is then split into convex
 * parts. If a chord would cross another boundary element of the same loop,
 * the arc is subdivided at its midpoint and the halves are retried.
 *
 * Holes are stored in the same anticlockwise direction as the outer boundary
 * (see Shape's own doc comment), so the arc orientation that is "convex from
 * the material's side" is Anticlockwise for the outer boundary but Clockwise
 * for a hole: a Clockwise arc in a hole is material bulging into the void,
 * the hole equivalent of a convex bump.
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
