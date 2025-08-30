#pragma once

#include "shape/shape.hpp"

namespace shape
{

std::vector<Point> compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2,
        bool strict = false);

bool intersect(
        const Shape& shape);

bool intersect(
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict = false);

bool intersect(
        const Shape& shape,
        const ShapeElement& element,
        bool strict = false);

inline bool intersect(
        const ShapeElement& element,
        const Shape& shape,
        bool strict = false)
{
    return intersect(shape, element, strict);
}

bool intersect(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2,
        bool strict = false);

bool intersect(
        const ShapeWithHoles& shape_1,
        const Shape& shape_2,
        bool strict = false);

inline bool intersect(
        const Shape& shape_1,
        const ShapeWithHoles& shape_2,
        bool strict = false)
{
    return intersect(shape_2, shape_1, strict);
}

bool intersect(
        const ShapeWithHoles& shape,
        const ShapeElement& element,
        bool strict = false);

inline bool intersect(
        const ShapeElement& element,
        const ShapeWithHoles& shape,
        bool strict = false)
{
    return intersect(shape, element, strict);
}

}
