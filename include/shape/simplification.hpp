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


/**
 * Output of try_round_corner.
 */
struct RoundCornerOutput
{
    /** True iff a valid rounded corner was found. */
    bool feasible = false;

    /**
     * The replacement elements in path order: trimmed first segment (if
     * non-zero length), the circular arc, trimmed second segment (if
     * non-zero length).  Contains 1 to 3 elements; zero-length segments
     * are omitted.  Meaningful only when feasible is true.
     */
    std::vector<ShapeElement> elements;
};

/**
 * Given two consecutive line segments sharing a common endpoint (the corner)
 * and a target arc radius, replace the sharp corner with a smooth
 * line - arc - line transition.
 *
 * The arc is tangent to both segments at points T1 (on element_prev) and T2
 * (on element_next), located at equal distances from the corner along each
 * segment.  The arc orientation (CCW/CW) matches the turn direction of the
 * path.
 *
 * Returns feasible = false when:
 *   - either input element is not a line segment,
 *   - the two segments are collinear or nearly antiparallel, or
 *   - the tangent length exceeds the length of either segment (radius too
 *     large for the available geometry).
 */
RoundCornerOutput try_round_corner(
        const ShapeElement& element_prev,
        const ShapeElement& element_next,
        LengthDbl radius);


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
