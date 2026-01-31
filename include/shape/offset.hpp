#pragma once

#include "shape/shape.hpp"

namespace shape
{

ShapeWithHoles inflate(
        const ShapeWithHoles& shape,
        LengthDbl offset);

void inflate_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl offset);

ShapeWithHoles inflate(
        const Shape& shape,
        LengthDbl offset);

void inflate_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl offset);

std::vector<Shape> deflate(
        const Shape& shape,
        LengthDbl offset);

}
