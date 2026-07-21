#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the union of a given set of shapes.
 */
MultiShapeWithHoles compute_union(
        const std::vector<ShapeWithHoles>& shapes);

void compute_union_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of shapes.
 */
MultiShapeWithHoles compute_intersection(
        const std::vector<ShapeWithHoles>& shapes);

void compute_intersection_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of multi-shapes.
 *
 * Each multi-shape is the union of its `ShapeWithHoles`; a point belongs to
 * the result if it belongs to at least one `ShapeWithHoles` of every
 * multi-shape.
 */
MultiShapeWithHoles compute_intersection(
        const std::vector<MultiShapeWithHoles>& multi_shapes);

/**
 * Compute the difference between two multi-shapes.
 */
MultiShapeWithHoles compute_difference(
        const MultiShapeWithHoles& shapes_1,
        const MultiShapeWithHoles& shapes_2);

/**
 * Compute the symmetric difference between two multi-shapes.
 */
MultiShapeWithHoles compute_symmetric_difference(
        const MultiShapeWithHoles& shapes_1,
        const MultiShapeWithHoles& shapes_2);

Shape extract_outline(
        const Shape& shape);

/**
 * Extract the faces of a shape.
 */
std::vector<Shape> extract_faces(
        const Shape& shape);

std::vector<ShapeElement> find_holes_bridges(
        const ShapeWithHoles& shape);

/**
 * Merge the holes of the shape that touch the outline into the outline.
 *
 * This creates an invalid shape but is a necessary preprocess for the
 * trapezoidation algorithm.
 */
MultiShapeWithHoles bridge_touching_holes(
        const ShapeWithHoles& shape);

}
