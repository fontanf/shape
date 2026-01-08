#pragma once

#include "shape/elements_intersections.hpp"

namespace shape
{

bool intersect(
        const ShapeElement& element_1,
        const ShapeElement& element_2,
        bool strict = false);

bool intersect(
        const Shape& shape);

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

struct ShapeShapeElementIntersection
{
    ElementPos element_pos = -1;

    ShapeElementIntersectionsOutput intersections;
};

/**
 * Compute the intersection ponits between an element and the elements of a
 * shape.
 *
 * Note that the element and the shape might intersect but have no intersection
 * points. This happens if the element is contained inside the shape.
 */
std::vector<ShapeShapeElementIntersection> compute_intersections(
        const Shape& shape,
        const ShapeElement& element);

struct ComputeClosestToShapeElementStartIntersectionOutput
{
    bool intersect;

    ElementPos element_pos = -1;

    Point intersection = {0, 0};

    LengthDbl distance = std::numeric_limits<LengthDbl>::infinity();
};

/**
 * Compute the intersection points between an element and the elements of a
 * shape which are the closest from the element's start.
 */
ComputeClosestToShapeElementStartIntersectionOutput compute_closest_to_start_intersection(
        const ShapeElement& element,
        const Shape& shape,
        bool proper = false);

bool intersect(
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict = false);

struct ComputeClosestToPathStartIntersectionOutput
{
    bool intersect;

    ElementPos element_1_pos = -1;

    ElementPos element_2_pos = -1;

    Point intersection = {0, 0};
};

ComputeClosestToPathStartIntersectionOutput compute_closest_to_start_intersection(
        const Shape& path,
        const Shape& shape,
        bool strict = false);

bool intersect(
        const ShapeWithHoles& shape,
        const ShapeElement& element,
        bool strict = false);

inline bool intersect(
        const ShapeElement& element,
        const ShapeWithHoles& shape_with_holes,
        bool strict = false)
{
    return intersect(shape_with_holes, element, strict);
}

bool intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const Shape& shape_2,
        bool strict = false);

inline bool intersect(
        const Shape& shape_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict = false)
{
    return intersect(shape_with_holes_2, shape_1, strict);
}

bool intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict = false);

}
