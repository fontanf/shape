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

ShapeWithHoles approximate_shape_by_line_segments(
        const Shape& shape,
        LengthDbl segment_length,
        bool outer);

void approximate_shape_by_line_segments_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl segment_length,
        bool outer);

Shape approximate_path_by_line_segments(
        const Shape& path,
        LengthDbl segment_length);

ShapeWithHoles approximate_by_line_segments(
        const ShapeWithHoles& shape,
        LengthDbl segment_length);

}
