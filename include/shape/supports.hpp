#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Parts of the boundary of a shape on which other objects can rest, and
 * which can rest on other objects, with gravity going towards decreasing y.
 *
 * Each part is a chain of consecutive line segments of the boundary, stored
 * as a Shape whose 'is_path' is false. Each part is x-monotone and has no
 * vertical line segment, and the boundary is never concave, seen from the
 * material, at a vertex inside a part.
 */
struct ShapeSupports
{
    /**
     * Parts of the boundary with the material below them, going from left to
     * right.
     */
    std::vector<Shape> supporting_parts;

    /**
     * Parts of the boundary with the material above them, going from right
     * to left.
     */
    std::vector<Shape> supported_parts;
};

/**
 * Compute the supports of the outline of a shape, or of a hole if 'is_hole'
 * is true (the material is then outside of 'shape').
 *
 * Throw std::invalid_argument if 'shape' contains a circular arc.
 */
ShapeSupports compute_shape_supports(
        const Shape& shape,
        bool is_hole);

/**
 * Compute the supports of a shape with holes: the supports of its outline
 * and of its holes.
 *
 * Throw std::invalid_argument if 'shape' contains a circular arc.
 */
ShapeSupports compute_shape_supports(
        const ShapeWithHoles& shape);

}
