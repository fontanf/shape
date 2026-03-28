#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Output of try_extend_to_intersection.
 */
struct ExtendToIntersectionOutput
{
    /** True iff the two extended elements intersect at a valid point. */
    bool feasible = false;

    /**
     * Copy of element_prev with its end moved forward to the intersection
     * point.  Meaningful only when feasible is true.
     */
    ShapeElement new_element_prev;

    /**
     * Copy of element_next with its start moved backward to the intersection
     * point.  Meaningful only when feasible is true.
     */
    ShapeElement new_element_next;
};

/**
 * Given two elements that are not yet connected, try to extend the first one
 * beyond its end and the second one before its start along their underlying
 * geometries (infinite line for line segments, full circle for circular arcs)
 * until the two extended elements meet at a common point.
 *
 * This is the key primitive for removing a middle element that sits between
 * element_prev and element_next: if the operation is feasible the two
 * neighbours can be stitched directly at the intersection point.
 *
 * Returns a struct whose feasible flag is true only when a geometrically
 * valid intersection exists, i.e. the candidate point is:
 *   - at or beyond element_prev.end  in element_prev's forward direction, and
 *   - at or before element_next.start in element_next's forward direction.
 *
 * Full-circle arcs (ShapeElementOrientation::Full) are not supported; the
 * function returns feasible = false if either element is a full circle.
 */
ExtendToIntersectionOutput try_extend_to_intersection(
        const ShapeElement& element_prev,
        const ShapeElement& element_next);


struct SimplifyInputShape
{
    ShapeWithHoles shape;

    ShapePos copies;
};

std::vector<ShapeWithHoles> simplify(
        const std::vector<SimplifyInputShape>& shapes,
        AreaDbl maximum_approximation_area);

void simplify_export_inputs(
        const std::string& file_path,
        const std::vector<SimplifyInputShape>& shapes,
        AreaDbl maximum_approximation_area);

}
