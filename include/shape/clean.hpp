#pragma once

#include "shape.hpp"

namespace shape
{

/**
 * Remove the elements of a shape whose start and end are equal (within
 * tolerance), except full circles.
 *
 * The end of the element before each removed element is moved to the start
 * of the element after it, so that the shape stays connected.
 *
 * A closed shape keeps at least 3 elements: a closed shape with 3 elements or
 * fewer is returned unchanged.
 *
 * The boolean returned is true iff at least one element has been removed.
 */
std::pair<bool, Shape> remove_redundant_vertices(
        const Shape& shape);

/**
 * Apply remove_redundant_vertices to the outline and to each hole of a shape
 * with holes.
 *
 * The boolean returned is true iff at least one element has been removed.
 */
std::pair<bool, ShapeWithHoles> remove_redundant_vertices(
        const ShapeWithHoles& shape);


/**
 * Remove the backtracks of a shape, that is, the pairs of consecutive
 * elements where the second one is the reverse of the first one (within
 * tolerance).
 *
 * The result has no backtracks, including the ones which only appear once
 * other backtracks are removed and, for a closed shape, the ones across its
 * first element.
 */
Shape remove_backtracks(
        const Shape& shape);


/**
 * Replace the circular arcs of a shape which are flat within tolerance, that
 * is, whose middle is within tolerance of their chord, by line segments.
 *
 * Full circles are never replaced.
 */
Shape flatten_arcs(
        const Shape& shape);

/**
 * Apply flatten_arcs to the outline and to each hole of a shape with holes.
 */
ShapeWithHoles flatten_arcs(
        const ShapeWithHoles& shape_with_holes);


/**
 * Move the center of each circular arc of a shape so that it is at the same
 * distance from its start and its end: the new center is the projection of
 * the old one on the perpendicular bisector of the start and the end (see
 * ShapeElement::recompute_center).
 *
 * The start and end of the elements are not changed. Full circles are not
 * changed.
 */
Shape recompute_centers(
        const Shape& shape);

/**
 * Apply recompute_centers to the outline and to each hole of a shape with
 * holes.
 */
ShapeWithHoles recompute_centers(
        const ShapeWithHoles& shape_with_holes);


/**
 * Merge the consecutive line segments of a shape which are aligned within
 * tolerance.
 *
 * Each removed vertex is within tolerance of the line segment replacing it,
 * even when many vertices along a slightly curved line are replaced by a
 * single line segment.
 *
 * Vertices adjacent to a circular arc are never removed. A shape keeps at
 * least 3 elements: a shape with 3 elements or fewer is returned unchanged.
 * For a path, the first and the last vertices are kept.
 *
 * The boolean returned is true iff at least one vertex has been removed.
 */
std::pair<bool, Shape> remove_aligned_vertices(
        const Shape& shape);

/**
 * Apply remove_aligned_vertices to the outline and to each hole of a shape
 * with holes.
 *
 * The boolean returned is true iff at least one vertex has been removed.
 */
std::pair<bool, ShapeWithHoles> remove_aligned_vertices(
        const ShapeWithHoles& shape);


/**
 * Remove the holes of a shape whose area is strictly smaller (within
 * tolerance) than 'area_limit'.
 */
ShapeWithHoles remove_small_holes(
        const ShapeWithHoles& shape_with_holes,
        AreaDbl area_limit);


/**
 * Make the almost vertical (absolute slope greater than 1e2) and almost
 * horizontal (absolute slope smaller than 1e-2) line segments of a shape
 * exactly vertical or horizontal, by only adding area to the shape.
 *
 * The result contains 'shape'. It is a single shape, but it may have holes.
 *
 * Throw std::invalid_argument if 'shape' isn't valid (see Shape::check), or
 * if the cleaning produced an invalid shape.
 */
ShapeWithHoles clean_extreme_slopes_outer(
        const Shape& shape);

/**
 * Same as clean_extreme_slopes_outer, but by only removing area from the
 * shape.
 *
 * The result is contained in 'shape'. It is a list of shapes because
 * removing area may split the shape into several pieces.
 *
 * Throw std::invalid_argument if 'shape' isn't valid (see Shape::check), or
 * if the cleaning produced an invalid shape.
 */
std::vector<Shape> clean_extreme_slopes_inner(
        const Shape& shape);


/**
 * Split a shape with holes into several shapes with holes if a chain of
 * touching holes goes from its outline back to its outline, and thus
 * disconnects it.
 *
 * Holes touching the outline (or touching other holes) at a single point
 * without disconnecting the shape are kept as is. If the shape isn't
 * disconnected, it is returned unchanged.
 */
MultiShapeWithHoles fix_self_intersections(
        const ShapeWithHoles& shape);

}
