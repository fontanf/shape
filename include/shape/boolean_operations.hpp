#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the union of a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_union(
        const std::vector<ShapeWithHoles>& shapes);

void compute_union_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of shapes.
 */
std::vector<ShapeWithHoles> compute_intersection(
        const std::vector<ShapeWithHoles>& shapes);

void compute_intersection_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of two multi-shapes.
 *
 * Returns what is inside at least one shape of shapes_1 and at least one shape
 * of shapes_2, i.e. (union of shapes_1) ∩ (union of shapes_2).
 */
std::vector<ShapeWithHoles> compute_group_intersection(
        const std::vector<ShapeWithHoles>& shapes_1,
        const std::vector<ShapeWithHoles>& shapes_2);

/**
 * Compute the difference between two multi-shapes.
 */
std::vector<ShapeWithHoles> compute_difference(
        const std::vector<ShapeWithHoles>& shapes_1,
        const std::vector<ShapeWithHoles>& shapes_2);

/**
 * Compute the symmetric difference between two multi-shapes.
 */
std::vector<ShapeWithHoles> compute_symmetric_difference(
        const std::vector<ShapeWithHoles>& shapes_1,
        const std::vector<ShapeWithHoles>& shapes_2);

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
std::vector<ShapeWithHoles> bridge_touching_holes(
        const ShapeWithHoles& shape);

}
