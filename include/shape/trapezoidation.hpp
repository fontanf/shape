#pragma once

#include "shape/trapezoid.hpp"

namespace shape
{

/**
 * Partition a polygon with holes into generalized trapezoids whose parallel
 * sides are horizontal.
 *
 * The union of the trapezoids is 'shape' and their interiors are pairwise
 * disjoint. Each trapezoid has a strictly positive height, but one of its
 * parallel sides may have a null length, i.e. it may be a triangle.
 * Holes may touch the outline or other holes at a single point.
 *
 * Throw std::invalid_argument if 'shape' isn't valid (see
 * ShapeWithHoles::check) or isn't a polygon.
 */
std::vector<GeneralizedTrapezoid> trapezoidation(
        const ShapeWithHoles& shape);

}
