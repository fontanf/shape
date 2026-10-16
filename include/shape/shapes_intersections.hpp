#pragma once

#include "shape/shape.hpp"
#include "shape/elements_intersections.hpp"

namespace shape
{

/*
 * The functions below share the following conventions.
 *
 * A closed shape stands for the region delimited by its boundary, boundary
 * included, and a shape with holes for the region of its outline minus the
 * interior of its holes. A path or an element stands for its points only.
 *
 * Without 'strict', two objects intersect if they have at least one point in
 * common (within tolerance), so objects touching each other intersect.
 *
 * With 'strict', two closed shapes intersect only if their interiors
 * overlap, and a path or an element intersects a closed shape only if it goes
 * strictly inside it: objects only touching each other don't intersect.
 * Strict intersections are not defined between two paths or between a path
 * and an element.
 */

/**
 * Return true iff two elements have at least one point in common.
 */
bool intersect(
        const ShapeElement& element_1,
        const ShapeElement& element_2);


/**
 * Return true iff the boundary of a shape intersects itself, that is, if two
 * of its elements have a point in common other than the vertex shared by two
 * consecutive elements.
 *
 * For a path, the last element and the first element are not consecutive.
 */
bool intersect(
        const Shape& shape);

/**
 * A pair of intersecting elements, identified by their positions, and their
 * intersection.
 */
struct ElementElementIntersection
{
    ElementPos element_id_1 = -1;
    ElementPos element_id_2 = -1;
    ShapeElementIntersectionsOutput intersections;
};

/**
 * Compute the pairs of elements of a shape which make its boundary intersect
 * itself (see intersect(const Shape&)).
 */
std::vector<ElementElementIntersection> compute_intersecting_elements(
        const Shape& shape);


/**
 * Return true iff a shape and an element intersect.
 *
 * Throw std::invalid_argument if 'strict' is true and 'shape' is a path.
 */
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

/**
 * Write the inputs of intersect to a JSON file in the format of the test
 * fixtures in data/tests/shapes_intersections/intersect_shape_shape_element/.
 */
void intersect_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        const ShapeElement& element,
        bool strict = false);

/**
 * Compute the points where an element meets the boundary of a shape, sorted
 * from the start of the element.
 *
 * Each point comes with the position of the element of 'shape' it is on. An
 * overlapping part gives its two endpoints. A point on several elements of
 * 'shape', such as a vertex, is returned once for each of them.
 *
 * If 'only_min_max' is true, only the first and the last points are returned
 * (twice the same point if there is a single one).
 */
std::vector<ShapePoint> compute_intersections(
        const ShapeElement& element,
        const Shape& shape,
        bool only_min_max = false);

/**
 * Compute the points where an element goes into or out of the interior of a
 * shape, sorted from the start of the element.
 *
 * Each point comes with the position of the element of 'shape' it is on. The
 * endpoints of the element aren't returned. A point where the element only
 * touches the boundary of the shape isn't returned.
 *
 * If 'only_first' is true, only the first point is returned.
 *
 * Throw std::invalid_argument if 'shape' is a path.
 */
std::vector<ShapePoint> compute_strict_intersections(
        const ShapeElement& element,
        const Shape& shape,
        bool only_first = false);


/**
 * A point where a path meets the boundary of a shape, with the positions of
 * the elements of the path and of the shape it is on.
 */
struct PathShapeIntersectionPoint
{
    ElementPos path_element_pos;
    ElementPos shape_element_pos;
    Point point;
};

/**
 * Return true iff two shapes intersect.
 *
 * Throw std::invalid_argument if 'strict' is true and both shapes are paths.
 */
bool intersect(
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict = false);

/**
 * Write the inputs of intersect to a JSON file in the format of the test
 * fixtures in data/tests/shapes_intersections/intersect_shape_shape/.
 */
void intersect_export_inputs(
        const std::string& file_path,
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict = false);

/**
 * Same as compute_intersections for an element, but for a path: the points
 * are sorted from the start of the path.
 */
std::vector<PathShapeIntersectionPoint> compute_intersections(
        const Shape& path,
        const Shape& shape,
        bool only_min_max = false);

/**
 * Write the inputs of compute_intersections to a JSON file in the format of
 * the test fixtures in
 * data/tests/shapes_intersections/compute_intersections_path_shape/.
 */
void compute_intersections_export_inputs(
        const std::string& file_path,
        const Shape& path,
        const Shape& shape,
        bool only_min_max);

/**
 * Same as compute_strict_intersections for an element, but for a path: the
 * points are sorted from the start of the path, and the start and the end of
 * the path aren't returned.
 *
 * Throw std::invalid_argument if 'shape' is a path.
 */
std::vector<PathShapeIntersectionPoint> compute_strict_intersections(
        const Shape& path,
        const Shape& shape,
        bool only_first = false);

/**
 * Write the inputs of compute_strict_intersections to a JSON file in the
 * format of the test fixtures in
 * data/tests/shapes_intersections/compute_strict_intersections_path_shape/.
 */
void compute_strict_intersections_export_inputs(
        const std::string& file_path,
        const Shape& path,
        const Shape& shape,
        bool only_first);


/**
 * Return true iff the outline or one of the holes of a shape with holes
 * intersects itself (see intersect(const Shape&)).
 *
 * The intersections between the outline and the holes, or between two
 * holes, are not checked.
 */
bool intersect(
        const ShapeWithHoles& shape_with_holes);


/**
 * Return true iff a shape with holes and an element intersect.
 */
bool intersect(
        const ShapeWithHoles& shape_with_holes,
        const ShapeElement& element,
        bool strict = false);

inline bool intersect(
        const ShapeElement& element,
        const ShapeWithHoles& shape_with_holes,
        bool strict = false)
{
    return intersect(shape_with_holes, element, strict);
}

/**
 * Write the inputs of intersect to a JSON file in the format of the test
 * fixtures in
 * data/tests/shapes_intersections/intersect_shape_with_holes_shape_element/.
 */
void intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes,
        const ShapeElement& element,
        bool strict = false);


/**
 * Return true iff a shape with holes and a shape intersect.
 */
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

/**
 * Write the inputs of intersect to a JSON file in the format of the test
 * fixtures in
 * data/tests/shapes_intersections/intersect_shape_with_holes_shape/.
 */
void intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes_1,
        const Shape& shape_2,
        bool strict = false);


/**
 * Return true iff two shapes with holes intersect.
 */
bool intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict = false);

/**
 * Write the inputs of intersect to a JSON file in the format of the test
 * fixtures in
 * data/tests/shapes_intersections/intersect_shape_with_holes_shape_with_holes/.
 */
void intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict = false);

}
