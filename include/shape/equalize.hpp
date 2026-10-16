#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Same as equalize_points, for values.
 */
std::vector<LengthDbl> equalize_values(
        const std::vector<LengthDbl>& values);

/**
 * Replace points which are equal within tolerance by a single
 * representative.
 *
 * The output has one point for each input point, at the same position, and:
 * - each output point is an input point equal (within tolerance) to the
 *   input point it replaces, so each point moves by at most the tolerance;
 * - two different output points are not equal (within tolerance);
 * - identical input points give identical output points.
 *
 * However, two input points equal within tolerance may be replaced by two
 * different representatives, e.g. the two ends of a chain of points in which
 * each point is equal to the next one.
 */
std::vector<Point> equalize_points(
        const std::vector<Point>& points);

/**
 * Apply equalize_values to the x-coordinates, and separately to the
 * y-coordinates, of the starts, ends and centers of the elements of a shape.
 */
Shape equalize_shape(
        const Shape& shape);

/**
 * Same as equalize_shape for a shape, with the coordinates of the outline
 * and of all the holes equalized together.
 */
ShapeWithHoles equalize_shape(
        const ShapeWithHoles& shape);

}
