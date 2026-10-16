#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Convert a shape element of type CircularArc into multiple shape elements
 * of type LineSegment.
 *
 * The line segments form a chain from the start to the end of the arc. The
 * arc is considered as part of a boundary with the material on its left:
 * if 'outer' is true, the chain is on the right of the arc, so that
 * replacing the arc by it only adds area; otherwise, it is on its left, so
 * that it only removes area.
 *
 * The number of line segments is the length of the arc divided by
 * 'segment_length', rounded up, and increased if needed for the chain to be
 * on the requested side of the arc (for example, at least 4 for a full
 * circle).
 *
 * Throw std::runtime_error if 'circular_arc' isn't a CircularArc.
 */
std::vector<ShapeElement> approximate_circular_arc_by_line_segments(
        const ShapeElement& circular_arc,
        LengthDbl segment_length,
        bool outer);

/**
 * Approximate a closed shape by a polygon, replacing each circular arc by
 * line segments (see approximate_circular_arc_by_line_segments).
 *
 * If 'outer' is true, the result contains 'shape'. Otherwise, it is
 * contained in 'shape'; if removing area splits the shape into several
 * parts, the largest one is returned, and the result is empty if nothing is
 * left.
 *
 * Throw std::invalid_argument if 'shape' is a path.
 */
ShapeWithHoles approximate_shape_by_line_segments(
        const Shape& shape,
        LengthDbl segment_length,
        bool outer);

/**
 * Write the inputs of approximate_shape_by_line_segments to a JSON file in
 * the format of the test fixtures in
 * data/tests/approximation/approximate_shape_by_line_segments/.
 */
void approximate_shape_by_line_segments_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl segment_length,
        bool outer);

/**
 * Approximate a path by a path made of line segments, replacing each
 * circular arc by a chain of line segments between the same endpoints (see
 * approximate_circular_arc_by_line_segments, with 'outer' true).
 *
 * Throw std::invalid_argument if 'path' isn't a path.
 */
Shape approximate_path_by_line_segments(
        const Shape& path,
        LengthDbl segment_length);

/**
 * Approximate a shape with holes by a polygon with holes containing it: the
 * circular arcs of the outline are replaced by line segments outside of
 * them, and the ones of the holes by line segments inside the holes (see
 * approximate_circular_arc_by_line_segments).
 */
ShapeWithHoles approximate_by_line_segments(
        const ShapeWithHoles& shape,
        LengthDbl segment_length);

/**
 * Write the inputs of approximate_by_line_segments to a JSON file in the
 * format of the test fixtures in
 * data/tests/approximation/approximate_by_line_segments/.
 */
void approximate_by_line_segments_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl segment_length);

}
