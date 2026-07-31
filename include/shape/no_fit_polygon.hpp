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
 * Both shapes must be convex (CCW winding); an exception is thrown
 * otherwise. Elements may be a mix of LineSegment and CircularArc, with any
 * number of arcs, as long as each individual arc spans < 180° (an exception
 * is thrown otherwise) -- e.g. the result of inflating a convex polygon,
 * which rounds every corner into its own arc, always satisfies this, since
 * a single polygon vertex's exterior turning angle can never reach a half
 * turn.
 *
 * The returned shape is CCW and free of collinear (aligned) vertices.
 *
 * Algorithm: rotating-calipers Minkowski sum, generalized from convex
 * polygons to convex shapes with (multiple, < 180°) circular arcs by
 * treating an arc as a continuous range of tangent directions instead of a
 * single one, splitting/summing as needed against the other shape.
 * Complexity: O(m + n) where m, n are the element counts of A and B.
 */
Shape no_fit_polygon(
        const Shape& fixed_shape,
        const Shape& orbiting_shape);

/**
 * Compute the No-Fit Polygon (NFP) of two general (possibly non-convex)
 * shapes.
 *
 * The algorithm decomposes each shape into convex polygons and circular
 * segments via decompose_into_basic_shapes, computes the convex NFP for
 * every pair of parts, then returns the union of all those convex NFPs.
 *
 * The result may consist of several disconnected components or contain holes,
 * hence the return type is a MultiShapeWithHoles.
 */
MultiShapeWithHoles no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape);

}
