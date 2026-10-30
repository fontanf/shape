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
 * Output of no_fit_polygon for two general shapes.
 *
 * The positions of the orbiting shape making the two shapes overlap are the
 * points strictly inside 'nfp', except the points of 'feasible_elements' and
 * 'feasible_points'.
 */
struct NoFitPolygonOutput
{
    /**
     * Regions of the NFP.
     *
     * It follows the output conventions of the boolean operations (see
     * boolean_operations.hpp).
     */
    MultiShapeWithHoles nfp;

    /**
     * Elements strictly inside 'nfp' whose points are positions without
     * overlap.
     *
     * For example, when the orbiting shape fits exactly the width of a
     * slot or of a hole of the fixed shape, it can still slide along a line
     * segment there.
     */
    std::vector<ShapeElement> feasible_elements;

    /**
     * Points strictly inside 'nfp' and not on 'feasible_elements' which are
     * positions without overlap.
     *
     * For example, when the orbiting shape fits exactly a hole of the fixed
     * shape.
     */
    std::vector<Point> feasible_points;
};

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
 * hence it is a MultiShapeWithHoles. The positions without overlap of null
 * area, which it can't represent, are given separately (see
 * NoFitPolygonOutput).
 */
NoFitPolygonOutput no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape);

}
