#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the union of a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_union(
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_intersection(
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the difference between a given shape and a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_difference(
        const ShapeWithHoles& shape,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the difference between a given shape and a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_symmetric_difference(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

/**
 * Extract the faces of a shape.
 */
std::vector<Shape> extract_faces(
        const Shape& shape);

}
