#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the set of points at distance at most 'offset' from a shape with
 * holes: the outline grows outwards and the holes shrink.
 *
 * The convex corners of the outline and the reflex corners of the holes
 * become circular arcs of radius 'offset'.
 *
 * A hole may close up, in which case it is removed, or split into several
 * holes.
 *
 * If 'offset' is 0, 'shape' is returned unchanged.
 *
 * Throw std::invalid_argument if 'offset' is negative.
 */
ShapeWithHoles inflate(
        const ShapeWithHoles& shape,
        LengthDbl offset);

/**
 * Write the inputs of inflate to a JSON file in the format of the
 * test fixtures in data/tests/offset/.
 */
void inflate_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl offset);

/**
 * Compute the set of points at distance at most 'offset' from a shape.
 *
 * If 'shape' is closed, this is the same as inflating a shape without holes.
 * The result may still have holes, e.g. when inflating a C-shaped outline
 * closes its opening.
 *
 * If 'shape' is a path ('is_path' is true), the result is the band of
 * half-width 'offset' around the path, with round ends. A full circle path
 * gives an annulus, or a disk if its radius is smaller than or equal to
 * 'offset'.
 *
 * If 'offset' is 0, 'shape' is returned without its redundant and aligned
 * vertices.
 *
 * Throw std::invalid_argument if 'offset' is negative.
 */
ShapeWithHoles inflate(
        const Shape& shape,
        LengthDbl offset);

/**
 * Write the inputs of inflate to a JSON file in the format of the
 * test fixtures in data/tests/offset/.
 */
void inflate_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl offset);

/**
 * Compute the set of points of a shape at distance at least 'offset' from
 * its boundary.
 *
 * The reflex corners of the shape become circular arcs of radius 'offset'.
 *
 * The result is a list of shapes because a narrow part of the shape may
 * disappear and split it into several pieces. It is empty if the whole shape
 * disappears. The pieces have no holes.
 *
 * If 'offset' is 0, 'shape' is returned unchanged.
 *
 * Throw std::invalid_argument if 'offset' is negative or if 'shape' is a
 * path.
 */
std::vector<Shape> deflate(
        const Shape& shape,
        LengthDbl offset);

}
