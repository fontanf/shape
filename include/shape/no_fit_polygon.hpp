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
 * Both shapes must be convex polygons (only line-segment elements, CCW
 * winding). An exception is thrown otherwise.
 *
 * The returned shape is CCW and free of collinear (aligned) vertices.
 *
 * Algorithm: rotating-calipers Minkowski sum on convex polygons.
 * Complexity: O(m + n) where m, n are the vertex counts of A and B.
 */
Shape no_fit_polygon(
        const Shape& fixed_shape,
        const Shape& orbiting_shape);

/**
 * Compute the No-Fit Polygon (NFP) of two general (possibly non-convex)
 * shapes.
 *
 * The algorithm decomposes each shape into convex parts via
 * compute_convex_partition, computes the convex NFP for every pair of parts,
 * then returns the union of all those convex NFPs.
 *
 * The result may consist of several disconnected components or contain holes,
 * hence the return type is a vector of ShapeWithHoles.
 */
std::vector<ShapeWithHoles> no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape);

}
