#pragma once

#include "shape/shape.hpp"

namespace shape
{

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

struct ShapeElementIntersectionsOutput
{
    std::vector<ShapeElement> overlapping_parts;

    std::vector<Point> improper_intersections;

    std::vector<Point> proper_intersections;


    std::string to_string(Counter indentation) const;
};

ShapeElementIntersectionsOutput compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

}
