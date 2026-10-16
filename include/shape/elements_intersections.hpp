#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Compute the intersection point of the infinite line through p11 and p12
 * with the infinite line through p21 and p22.
 *
 * The boolean returned is false if the lines are parallel (within
 * tolerance), including when they are the same line.
 *
 * If the intersection point is equal (within tolerance) to one of the four
 * input points, this input point is returned exactly.
 */
std::pair<bool, Point> compute_line_intersection(
        const Point& p11,
        const Point& p12,
        const Point& p21,
        const Point& p22);

/**
 * Compute the intersection points of the infinite line through line_point_1
 * and line_point_2 with the circle (circle_center, circle_radius).
 *
 * Returns 0 points when there is no intersection, 1 point when the line is
 * tangent to the circle, and 2 points otherwise.
 */
std::vector<Point> compute_line_circle_intersections(
        const Point& line_point_1,
        const Point& line_point_2,
        const Point& circle_center,
        LengthDbl circle_radius);

/**
 * Compute the intersection points of two circles.
 *
 * Returns 0 points for no intersection or concentric circles, 1 point for
 * an external/internal tangency, and 2 points otherwise.
 */
std::vector<Point> compute_circle_circle_intersections(
        const Point& center_1,
        LengthDbl radius_1,
        const Point& center_2,
        LengthDbl radius_2);

/**
 * Output of compute_intersections for two elements.
 *
 * The intersection of the two elements is the union of the overlapping
 * parts and of the intersection points. An intersection point is never on an
 * overlapping part.
 */
struct ShapeElementIntersectionsOutput
{
    /**
     * Parts of non-null length shared by both elements.
     *
     * Their direction is unspecified.
     */
    std::vector<ShapeElement> overlapping_parts;

    /**
     * Points where the elements touch without crossing: an endpoint of one
     * element on the other element, or a tangency.
     */
    std::vector<Point> improper_intersections;

    /**
     * Points where the elements cross, strictly inside both of them.
     */
    std::vector<Point> proper_intersections;


    std::string to_string(Counter indentation) const;
};

/**
 * Compute the intersection of two elements, with the tolerance.
 *
 * See ShapeElementIntersectionsOutput.
 */
ShapeElementIntersectionsOutput compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

}
