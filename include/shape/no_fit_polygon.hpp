#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the No-Fit Polygon (NFP) of two convex shapes.
 *
 * The NFP of a fixed shape A and an orbiting shape B is the locus of the
 * reference point of B (its origin) as B slides around the boundary of A
 * without overlap. Equivalently, it is the Minkowski sum A ⊕ (−B).
 *
 * A point p lies inside the NFP iff placing B's origin at p causes B and A
 * to overlap. A point on the boundary means they just touch.
 *
 * Both shapes must be convex (CCW winding). Elements may be a mix of
 * LineSegment and CircularArc, with any number of arcs, as long as each
 * individual arc spans < 180° -- e.g. the result of inflating a convex
 * polygon, which rounds every corner into its own arc, always satisfies
 * this, since a single polygon vertex's exterior turning angle can never
 * reach a half turn.
 *
 * The returned shape is CCW and free of collinear (aligned) vertices.
 *
 * Complexity: O(m + n) where m, n are the element counts of A and B.
 *
 * Throw std::runtime_error if a shape isn't convex or has a circular arc
 * spanning 180° or more.
 */
Shape no_fit_polygon(
        const Shape& fixed_shape,
        const Shape& orbiting_shape);

/**
 * Compute the No-Fit Polygon (NFP) of two general (possibly non-convex)
 * shapes with holes.
 *
 * Same definition as for convex shapes: a point lies strictly inside the NFP
 * iff placing the origin of the orbiting shape at it makes the two shapes
 * overlap. For example, positions of the orbiting shape inside a hole of the
 * fixed shape, without overlap, are holes of the NFP.
 *
 * The result may consist of several disconnected components or contain holes,
 * hence the return type is a MultiShapeWithHoles. It follows the output
 * conventions of the boolean operations (see boolean_operations.hpp).
 */
MultiShapeWithHoles no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape);

}
