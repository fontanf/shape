#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Convert a shape element of type CircularArc into multiple shape elements
 * of type LineSegment.
 */
std::vector<ShapeElement> approximate_circular_arc_by_line_segments(
        const ShapeElement& circular_arc,
        LengthDbl segment_length,
        bool outer);

ShapeWithHoles approximate_by_line_segments(
        const ShapeWithHoles& shape,
        LengthDbl segment_length);

}
