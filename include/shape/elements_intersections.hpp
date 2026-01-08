#pragma once

#include "shape/shape.hpp"

namespace shape
{

std::pair<bool, Point> compute_line_intersection(
        const Point& p11,
        const Point& p12,
        const Point& p21,
        const Point& p22);

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
