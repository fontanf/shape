#pragma once

#include "shape/shape.hpp"

namespace shape
{

/*
 * The boolean operations below share the following conventions.
 *
 * Inputs must be valid shapes with holes (see ShapeWithHoles::check), but the
 * center of a circular arc doesn't need to be exactly at the same distance
 * from its start and its end: the arc used is the one whose center is the
 * projection of its center on the perpendicular bisector of its start and
 * its end (see ShapeElement::recompute_center).
 *
 * In the output, the outlines and the holes are anticlockwise and have no
 * redundant or aligned vertices (see remove_redundant_vertices and
 * remove_aligned_vertices). The output is empty if the result is empty.
 *
 * Throw std::invalid_argument if a shape or a hole has no element. Invalid
 * inputs may also make these functions throw std::invalid_argument (e.g. if a
 * hole isn't inside its outline) or std::logic_error, or return a wrong
 * result. With valid inputs, a std::logic_error indicates a bug.
 */

/**
 * Compute the union of a given set of shapes.
 *
 * Each connected part of the union is returned as its own ShapeWithHoles.
 * Parts touching only at a point are separate parts.
 */
MultiShapeWithHoles compute_union(
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Write the inputs of compute_union to a JSON file in the format of the test
 * fixtures in data/tests/boolean_operations/union/.
 */
void compute_union_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of shapes.
 */
MultiShapeWithHoles compute_intersection(
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Write the inputs of compute_intersection to a JSON file in the format of the
 * test fixtures in data/tests/boolean_operations/intersection/.
 */
void compute_intersection_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes);

/**
 * Compute the intersection of a given set of multi-shapes.
 *
 * Each multi-shape is the union of its `ShapeWithHoles`; a point belongs to
 * the result if it belongs to at least one `ShapeWithHoles` of every
 * multi-shape.
 *
 * The result is empty if one of the multi-shapes is empty.
 */
MultiShapeWithHoles compute_intersection(
        const std::vector<MultiShapeWithHoles>& multi_shapes);

/**
 * Same as compute_intersection, but return the individual faces of the
 * intersection instead of merging them into a single MultiShapeWithHoles.
 *
 * Faces of touching/adjacent regions are not unioned together, so each
 * face's provenance (e.g. which grid cell it came from) can still be
 * recovered afterwards, typically via find_point_strictly_inside().
 *
 * Each face is returned as a ShapeWithHoles without holes.
 */
std::vector<ShapeWithHoles> compute_intersection_faces(
        const std::vector<MultiShapeWithHoles>& multi_shapes);

/**
 * Convenience overload of compute_intersection for two single shapes.
 */
MultiShapeWithHoles compute_intersection(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

/**
 * Compute the difference between two multi-shapes.
 *
 * A point belongs to the result if it belongs to at least one
 * `ShapeWithHoles` of 'shapes_1' and to no `ShapeWithHoles` of 'shapes_2'.
 */
MultiShapeWithHoles compute_difference(
        const MultiShapeWithHoles& shapes_1,
        const MultiShapeWithHoles& shapes_2);

/**
 * Convenience overload of compute_difference for two single shapes.
 */
MultiShapeWithHoles compute_difference(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

/**
 * Compute the symmetric difference between two multi-shapes.
 *
 * A point belongs to the result if it belongs to the union of 'shapes_1' or
 * to the union of 'shapes_2', but not to both.
 */
MultiShapeWithHoles compute_symmetric_difference(
        const MultiShapeWithHoles& shapes_1,
        const MultiShapeWithHoles& shapes_2);

/**
 * Convenience overload of compute_symmetric_difference for two single
 * shapes.
 */
MultiShapeWithHoles compute_symmetric_difference(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

/**
 * Extract the outer boundary of a shape whose boundary may intersect or
 * overlap itself.
 *
 * The result is anticlockwise and has no redundant or aligned vertices.
 */
Shape extract_outline(
        const Shape& shape);

/**
 * Extract the faces of a shape, that is, the regions delimited by its
 * boundary when the boundary intersects or overlaps itself.
 *
 * Each face is anticlockwise, has a strictly positive area, and has no
 * redundant or aligned vertices.
 */
std::vector<Shape> extract_faces(
        const Shape& shape);

/**
 * Compute line segments connecting the holes of a shape to the rest of its
 * boundary, so that the boundary and these line segments form a single
 * connected set.
 *
 * Holes touching each other, directly or through other holes, form a group.
 * For each group which doesn't touch the outline, the bridge is a horizontal
 * line segment going from the leftmost point of the group to the left, until
 * the first point of the outline or of another hole. It starts at that point
 * and ends at the leftmost point of the group.
 *
 * Throw std::invalid_argument if a hole isn't inside the outline.
 */
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
