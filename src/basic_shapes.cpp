#include "shape/basic_shapes.hpp"

#include "shape/convex_partition.hpp"
#include "shape/elements_intersections.hpp"

using namespace shape;

////////////////////////////////////////////////////////////////////////////////

namespace
{

std::pair<ShapeElement, ShapeElement> split_arc_at_midpoint(const ShapeElement& arc)
{
    Point midpoint = arc.middle();
    ShapeElement first_half = build_circular_arc(arc.start, midpoint, arc.center, arc.orientation);
    ShapeElement second_half = build_circular_arc(midpoint, arc.end, arc.center, arc.orientation);
    return std::make_pair(first_half, second_half);
}

/**
 * Return true if 'chord' meets 'element' anywhere other than at chord's own
 * start or end point. Every intersection category is considered (a proper
 * crossing, an overlapping collinear part, or an improper intersection such
 * as a tangential touch or a shared endpoint of 'element'): the only
 * tolerated contact is at the chord's own endpoints, where it is expected to
 * meet its neighbouring elements.
 */
bool chord_intersects_element(
        const ShapeElement& chord,
        const ShapeElement& element)
{
    ShapeElementIntersectionsOutput result = compute_intersections(chord, element);
    if (!result.overlapping_parts.empty())
        return true;
    for (const Point& point: result.proper_intersections)
        if (!equal(point, chord.start) && !equal(point, chord.end))
            return true;
    for (const Point& point: result.improper_intersections)
        if (!equal(point, chord.start) && !equal(point, chord.end))
            return true;
    return false;
}

/**
 * Return the loop at loop_idx: index i in [0, shape.holes.size()) is
 * shape.holes[i]; the outer boundary is returned for either out-of-range
 * sentinel, -1 or shape.holes.size() (one step before the first hole or one
 * step past the last), so it is reachable the same way from either end.
 */
Shape& loop_shape(ShapeWithHoles& shape, Counter loop_idx)
{
    if (loop_idx == -1 || loop_idx == (Counter)shape.holes.size())
        return shape.shape;
    return shape.holes[loop_idx];
}

/**
 * Return true if the chord p1-p2 meets any element of 'shape' (the outer
 * boundary or any hole). No exclusions are needed for the arc being cut or
 * its two neighbours: they only ever meet the chord exactly at p1 or p2,
 * which chord_intersects_element already tolerates.
 */
bool chord_intersects_boundary(
        const ShapeWithHoles& shape,
        const Point& p1,
        const Point& p2)
{
    ShapeElement chord = build_line_segment(p1, p2);

    for (const ShapeElement& element: shape.shape.elements)
        if (chord_intersects_element(chord, element))
            return true;
    for (const Shape& hole: shape.holes)
        for (const ShapeElement& element: hole.elements)
            if (chord_intersects_element(chord, element))
                return true;

    return false;
}

/**
 * Find and cut off convex-from-the-material circular segments from a single
 * loop of 'shape' (loop_idx: -1 for the outer boundary, in [0,
 * shape.holes.size()) for a hole), appending each one to 'segments'.
 * 'convex_orientation' is the arc orientation that counts as convex from the
 * material's side for this loop (Anticlockwise for the outer boundary,
 * Clockwise for a hole). Mutates the loop's elements in place, replacing
 * each cut arc by its chord.
 */
void cut_circular_segments(
        ShapeWithHoles& shape,
        Counter loop_idx,
        ShapeElementOrientation convex_orientation,
        std::vector<BasicShape>& segments)
{
    std::vector<ShapeElement>& current_elements = loop_shape(shape, loop_idx).elements;

    bool found_arc = true;
    while (found_arc) {
        found_arc = false;
        for (Counter element_idx = 0;
                element_idx < (Counter)current_elements.size();
                ++element_idx) {
            const ShapeElement& element = current_elements[element_idx];
            if (element.type != ShapeElementType::CircularArc)
                continue;
            if (element.orientation != convex_orientation)
                continue;

            found_arc = true;

            // Check whether the arc subtends >= 180°; if so the tangent lines
            // at the endpoints are parallel and no finite apex exists.
            LengthDbl radius = element.radius();
            LengthDbl arc_angle = element.length() / radius;

            bool needs_subdivision = (arc_angle >= M_PI - 1e-6)
                    || chord_intersects_boundary(shape, element.start, element.end);

            if (needs_subdivision) {
                std::pair<ShapeElement, ShapeElement> halves =
                        split_arc_at_midpoint(element);
                current_elements.erase(current_elements.begin() + element_idx);
                current_elements.insert(current_elements.begin() + element_idx, halves.second);
                current_elements.insert(current_elements.begin() + element_idx, halves.first);
                break;
            }

            // Cut the circular segment: its own shape is the arc closed by
            // the chord back to its start, and the arc is replaced by that
            // same chord in the remaining boundary.
            //
            // A Shape must itself be provided anticlockwise: for the outer
            // boundary the arc is already Anticlockwise, so close it as-is;
            // for a hole the cut-off arc is Clockwise (convex-from-material
            // there), so the whole 2-element loop must be reversed to be a
            // valid standalone anticlockwise shape.
            BasicShape segment_basic_shape;
            segment_basic_shape.type = BasicShapeType::CircularSegment;
            segment_basic_shape.circle_center = element.center;
            segment_basic_shape.circle_radius = radius;
            if (convex_orientation == ShapeElementOrientation::Anticlockwise) {
                segment_basic_shape.shape.elements = {
                    element,
                    build_line_segment(element.end, element.start)};
            } else {
                segment_basic_shape.shape.elements = {
                    element.reverse(),
                    build_line_segment(element.start, element.end)};
            }
            segments.push_back(segment_basic_shape);

            current_elements[element_idx] = build_line_segment(element.start, element.end);
            break;
        }
    }
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

BasicShapesDecomposition shape::decompose_into_basic_shapes(
        const ShapeWithHoles& shape)
{
    BasicShapesDecomposition output;

    ShapeWithHoles remaining = shape;

    cut_circular_segments(remaining, -1, ShapeElementOrientation::Anticlockwise, output.basic_shapes);
    for (Counter loop_idx = 0; loop_idx < (Counter)remaining.holes.size(); ++loop_idx) {
        cut_circular_segments(
                remaining, loop_idx,
                ShapeElementOrientation::Clockwise,
                output.basic_shapes);
    }

    // Decompose the remaining pure polygon (with holes) into convex parts.
    for (const Shape& convex_part: compute_convex_partition(remaining)) {
        BasicShape polygon_basic_shape;
        polygon_basic_shape.type = BasicShapeType::ConvexPolygon;
        polygon_basic_shape.shape = convex_part;
        output.basic_shapes.push_back(polygon_basic_shape);
    }

    return output;
}
