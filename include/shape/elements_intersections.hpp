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

inline bool operator==(
        const ShapeElementIntersectionsOutput& intersections_1,
        const ShapeElementIntersectionsOutput& intersections_2)
{
    if (intersections_1.overlapping_parts.size() != intersections_2.overlapping_parts.size())
        return false;
    for (const auto& expected_intersection: intersections_2.overlapping_parts) {
        if (std::find_if(
                    intersections_1.overlapping_parts.begin(),
                    intersections_1.overlapping_parts.end(),
                    [&expected_intersection](const ShapeElement& overlapping_part) { return equal(overlapping_part, expected_intersection) || equal(overlapping_part.reverse(), expected_intersection); })
                == intersections_1.overlapping_parts.end())
            return false;
    }
    if (intersections_1.improper_intersections.size() != intersections_2.improper_intersections.size())
        return false;
    for (const Point& expected_intersection: intersections_2.improper_intersections) {
        if (std::find_if(
                    intersections_1.improper_intersections.begin(),
                    intersections_1.improper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); })
                == intersections_1.improper_intersections.end())
            return false;
    }
    if (intersections_1.proper_intersections.size() != intersections_2.proper_intersections.size())
        return false;
    for (const Point& expected_intersection: intersections_2.proper_intersections) {
        if (std::find_if(
                    intersections_1.proper_intersections.begin(),
                    intersections_1.proper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); })
                == intersections_1.proper_intersections.end())
            return false;
    }
    return true;
}

ShapeElementIntersectionsOutput compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

}
