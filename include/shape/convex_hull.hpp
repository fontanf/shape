#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the convex hull of a polygon, that is, the smallest convex polygon
 * containing it.
 *
 * The vertices of the result are vertices of 'shape'. The result is
 * anticlockwise and has no vertex on the line between its two neighbors.
 *
 * A shape with fewer than 3 elements is returned unchanged.
 *
 * Throw std::invalid_argument if 'shape' contains a circular arc or is a
 * path.
 */
Shape convex_hull(
        const Shape& shape);

}
