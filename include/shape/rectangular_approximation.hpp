#pragma once

#include "shape/shape.hpp"

namespace shape
{

struct OuterApproximationByRectanglesInputShape
{
    /** Shape to approximate. */
    ShapeWithHoles shape;

    /**
     * Number of copies of the shape: the waste of its rectangles is counted
     * once for each copy.
     */
    ShapePos copies = 1;
};

/**
 * Approximate each shape of 'shapes' by axis-aligned rectangles whose union
 * contains it.
 *
 * The waste of a shape is the area of its rectangles minus the area of the
 * shape. The waste of all the shapes, counted once for each copy, is at most
 * 'maximum_waste', and, within it, the number of rectangles is reduced as
 * much as possible.
 *
 * The circular arcs of each shape are first replaced by line segments
 * outside of them (see approximate_by_line_segments); then, the shape is
 * decomposed into trapezoids (see trapezoidation), and each trapezoid is
 * covered by its bounding box. A shape whose edges are all horizontal or
 * vertical, such as a rectangular frame, is thus covered exactly. Then:
 * - as long as the waste is larger than 'maximum_waste', the rectangle of a
 *   single trapezoid whose split into two rectangles of half its height
 *   reduces the waste the most is split, as long as its shape has fewer than
 *   'maximum_number_of_rectangles' rectangles;
 * - then, as long as the waste stays at most 'maximum_waste', the two
 *   rectangles of a same shape whose replacement by their bounding box
 *   increases the waste the least are merged.
 * Both choices are made across all the shapes, weighted by their copies.
 *
 * Where rectangles of a shape overlap, the area of the overlap counts
 * several times in their total area: the waste of the union of the
 * rectangles is at most the waste as defined above.
 *
 * If the waste can't be reduced to 'maximum_waste' within
 * 'maximum_number_of_rectangles' rectangles per shape (for example, with
 * 'maximum_waste' equal to 0 and a shape with an oblique edge), the waste
 * is larger than 'maximum_waste'.
 *
 * Return, for each shape, its rectangles.
 */
std::vector<std::vector<AxisAlignedBoundingBox>> outer_approximation_by_rectangles(
        const std::vector<OuterApproximationByRectanglesInputShape>& shapes,
        AreaDbl maximum_waste,
        ElementPos maximum_number_of_rectangles = 256);

/**
 * Write the inputs of outer_approximation_by_rectangles to a JSON file in
 * the format of the test fixtures in
 * data/tests/rectangular_approximation/outer_approximation_by_rectangles/.
 */
void outer_approximation_by_rectangles_export_inputs(
        const std::string& file_path,
        const std::vector<OuterApproximationByRectanglesInputShape>& shapes,
        AreaDbl maximum_waste,
        ElementPos maximum_number_of_rectangles = 256);

}
