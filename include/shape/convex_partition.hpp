#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Partition a polygon with holes into convex polygons.
 *
 * The union of the parts is 'shape' and their interiors are pairwise
 * disjoint. Each part is anticlockwise and has no redundant or aligned
 * vertices. The number of parts isn't guaranteed to be minimal.
 *
 * Throw std::invalid_argument if 'shape' isn't valid (see
 * ShapeWithHoles::check) or isn't a polygon.
 */
std::vector<Shape> compute_convex_partition(
        const ShapeWithHoles& shape);

}
