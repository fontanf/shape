#include "shape/shapes_intersections.hpp"

#include "shape/elements_intersections.hpp"
#include "shape/boolean_operations.hpp"
#include "shape/intersection_tree.hpp"
//#include "shape/writer.hpp"

//#include <iostream>
#include <fstream>

using namespace shape;

bool shape::intersect(
        const ShapeElement& element_1,
        const ShapeElement& element_2)
{
    ShapeElementIntersectionsOutput intersections = compute_intersections(
            element_1,
            element_2);
    if (!intersections.proper_intersections.empty())
        return true;
    if (!intersections.improper_intersections.empty())
        return true;
    if (!intersections.overlapping_parts.empty())
        return true;
    return false;
}

bool shape::intersect(
        const Shape& shape)
{
    IntersectionTree intersection_tree({}, {shape.elements}, {});
    std::vector<ElementElementIntersection> intersecting_elements
        = intersection_tree.compute_intersecting_elements(false);
    if (shape.is_path) {
        for (const ElementElementIntersection& intersection: intersecting_elements) {
            if (!intersection.intersections.proper_intersections.empty())
                return true;
            if (!intersection.intersections.overlapping_parts.empty())
                return true;
            for (const Point& point: intersection.intersections.improper_intersections) {
                bool ok = false;
                if ((intersection.element_id_1 + 1 == intersection.element_id_2)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_1];
                    if (equal(point, element.end))
                        ok = true;
                }
                if ((intersection.element_id_2 + 1 == intersection.element_id_1)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_2];
                    if (equal(point, element.end))
                        ok = true;
                }
                if (!ok)
                    return true;
            }
        }
    } else {
        ElementPos n = shape.elements.size();
        for (const ElementElementIntersection& intersection: intersecting_elements) {
            if (!intersection.intersections.proper_intersections.empty())
                return true;
            if (!intersection.intersections.overlapping_parts.empty())
                return true;
            for (const Point& point: intersection.intersections.improper_intersections) {
                bool ok = false;
                if ((intersection.element_id_1 + 1 == intersection.element_id_2)
                        || (intersection.element_id_1 == n - 1 && intersection.element_id_2 == 0)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_1];
                    if (equal(point, element.end))
                        ok = true;
                }
                if ((intersection.element_id_2 + 1 == intersection.element_id_1)
                        || (intersection.element_id_2 == n - 1 && intersection.element_id_1 == 0)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_2];
                    if (equal(point, element.end))
                        ok = true;
                }
                if (!ok)
                    return true;
            }
        }
    }
    return false;
}

std::vector<ElementElementIntersection> shape::compute_intersecting_elements(
        const Shape& shape)
{
    std::vector<ElementElementIntersection> output;
    IntersectionTree intersection_tree({}, {shape.elements}, {});
    std::vector<ElementElementIntersection> intersecting_elements
        = intersection_tree.compute_intersecting_elements(false);
    if (shape.is_path) {
        for (const ElementElementIntersection& intersection: intersecting_elements) {
            if (!intersection.intersections.proper_intersections.empty()) {
                output.push_back(intersection);
                continue;
            }
            if (!intersection.intersections.overlapping_parts.empty()) {
                output.push_back(intersection);
                continue;
            }
            for (const Point& point: intersection.intersections.improper_intersections) {
                bool ok = false;
                if ((intersection.element_id_1 + 1 == intersection.element_id_2)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_1];
                    if (equal(point, element.end))
                        ok = true;
                }
                if ((intersection.element_id_2 + 1 == intersection.element_id_1)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_2];
                    if (equal(point, element.end))
                        ok = true;
                }
                if (!ok) {
                    output.push_back(intersection);
                    break;
                }
            }
        }
    } else {
        ElementPos n = shape.elements.size();
        for (const ElementElementIntersection& intersection: intersecting_elements) {
            if (!intersection.intersections.proper_intersections.empty()) {
                output.push_back(intersection);
                continue;
            }
            if (!intersection.intersections.overlapping_parts.empty()) {
                output.push_back(intersection);
                continue;
            }
            for (const Point& point: intersection.intersections.improper_intersections) {
                bool ok = false;
                if ((intersection.element_id_1 + 1 == intersection.element_id_2)
                        || (intersection.element_id_1 == n - 1 && intersection.element_id_2 == 0)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_1];
                    if (equal(point, element.end))
                        ok = true;
                }
                if ((intersection.element_id_2 + 1 == intersection.element_id_1)
                        || (intersection.element_id_2 == n - 1 && intersection.element_id_1 == 0)) {
                    const ShapeElement& element = shape.elements[intersection.element_id_2];
                    if (equal(point, element.end))
                        ok = true;
                }
                if (!ok) {
                    output.push_back(intersection);
                    break;
                }
            }
        }
    }
    return output;
}

bool shape::intersect(
        const Shape& shape,
        const ShapeElement& element,
        bool strict)
{
    if (shape.is_path && strict) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "intersections involving a path and an element must not be strict.");
    }

    if (!strict) {
        for (const ShapeElement& shape_element: shape.elements)
            if (shape::intersect(element, shape_element))
                return true;
        if (!shape.is_path && shape.contains(element.start))
            return true;
        return false;
    }

    std::vector<Point> intersection_points;
    std::vector<ShapeElement> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ElementPos shape_element_pos = 0;
            shape_element_pos < (ElementPos)shape.elements.size();
            ++shape_element_pos) {
        const ShapeElement& shape_element = shape.elements[shape_element_pos];
        ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                element,
                shape_element);
        for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
            intersections.push_back({0, (ElementPos)overlapping_parts.size()});
            overlapping_parts.push_back(overlapping_part);
        }
        for (const Point& intersection: intersections_cur.improper_intersections) {
            intersections.push_back({1, (ElementPos)intersection_points.size()});
            intersection_points.push_back(intersection);
        }
        for (const Point& intersection: intersections_cur.proper_intersections) {
            intersections.push_back({1, (ElementPos)intersection_points.size()});
            intersection_points.push_back(intersection);
        }
    }
    intersections.push_back({1, (ElementPos)intersection_points.size()});
    intersection_points.push_back(element.end);
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&element, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                const Point& point_1 = (p1.first == 0)?
                    overlapping_parts[p1.second].start:
                    intersection_points[p1.second];
                const Point& point_2 = (p2.first == 0)?
                    overlapping_parts[p2.second].start:
                    intersection_points[p2.second];
                return element.length(point_1) < element.length(point_2);
            });
    Point current_point = element.start;
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        //std::cout << "pos " << pos << " current_point " << current_point.to_string() << std::endl;
        if (intersections[pos].first == 0) {
            // Overlapping part.
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second];
            if (strictly_lesser(element.length(current_point), element.length(overlapping_part.start))) {
                Point point_between = element.find_point_between(current_point, overlapping_part.start);
                if (shape.contains(point_between, true))
                    return true;
            }
            if (strictly_lesser(element.length(current_point), element.length(overlapping_part.end)))
                current_point = overlapping_part.end;
        } else {
            // Intersection point.
            const Point& point = intersection_points[intersections[pos].second];
            //std::cout << "point " << point.to_string() << std::endl;
            if (strictly_lesser(element.length(current_point), element.length(point))) {
                Point point_between = element.find_point_between(current_point, point);
                //std::cout << "point_between " << point_between.to_string() << std::endl;
                if (shape.contains(point_between, true))
                    return true;
                current_point = point;
            }
        }
    }

    return false;
}

std::vector<ShapePoint> shape::compute_intersections(
        const ShapeElement& element,
        const Shape& shape,
        bool only_min_max)
{
    std::vector<ShapePoint> output;
    for (ElementPos shape_element_pos = 0;
            shape_element_pos < (ElementPos)shape.elements.size();
            ++shape_element_pos) {
        const ShapeElement& shape_element = shape.elements[shape_element_pos];
        ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                element,
                shape_element);
        for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
            output.push_back({shape_element_pos, overlapping_part.start});
            output.push_back({shape_element_pos, overlapping_part.end});
        }
        for (const Point& intersection: intersections_cur.improper_intersections)
            output.push_back({shape_element_pos, intersection});
        for (const Point& intersection: intersections_cur.proper_intersections)
            output.push_back({shape_element_pos, intersection});
    }
    if (output.empty())
        return {};
    if (only_min_max) {
        auto p = std::minmax_element(
            output.begin(),
            output.end(),
            [&element](
                const ShapePoint& point_1,
                const ShapePoint& point_2)
            {
                return strictly_lesser(element.length(point_1.point), element.length(point_2.point));
            });
        return {*p.first, *p.second};
    }
    // Sort intersections.
    std::sort(
        output.begin(),
        output.end(),
        [&element](
            const ShapePoint& point_1,
            const ShapePoint& point_2)
        {
            return strictly_lesser(element.length(point_1.point), element.length(point_2.point));
        });
    return output;
}

std::vector<ShapePoint> shape::compute_strict_intersections(
        const ShapeElement& element,
        const Shape& shape,
        bool only_first)
{
    if (shape.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "intersections involving a path and an element must not be strict.");
    }

    std::vector<ShapePoint> intersection_points;
    std::vector<std::pair<ElementPos, ShapeElement>> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ElementPos shape_element_pos = 0;
            shape_element_pos < (ElementPos)shape.elements.size();
            ++shape_element_pos) {
        const ShapeElement& shape_element = shape.elements[shape_element_pos];
        ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                element,
                shape_element);
        for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
            intersections.push_back({0, (ElementPos)overlapping_parts.size()});
            overlapping_parts.push_back({shape_element_pos, overlapping_part});
        }
        for (const Point& intersection: intersections_cur.improper_intersections) {
            intersections.push_back({1, (ElementPos)intersection_points.size()});
            intersection_points.push_back({shape_element_pos, intersection});
        }
        for (const Point& intersection: intersections_cur.proper_intersections) {
            intersections.push_back({1, (ElementPos)intersection_points.size()});
            intersection_points.push_back({shape_element_pos, intersection});
        }
    }
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&element, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                ShapePoint point_1 = (p1.first == 0)?
                        ShapePoint{
                            overlapping_parts[p1.second].first,
                            overlapping_parts[p1.second].second.start}:
                        intersection_points[p1.second];
                ShapePoint point_2 = (p2.first == 0)?
                        ShapePoint{
                            overlapping_parts[p2.second].first,
                            overlapping_parts[p2.second].second.start}:
                        intersection_points[p2.second];
                return strictly_lesser(element.length(point_1.point), element.length(point_2.point));
            });
    std::vector<ShapePoint> output;
    intersections.push_back({1, (ElementPos)intersection_points.size()});
    intersection_points.push_back({-1, element.end});
    ShapePoint current_point = {-1, element.start};
    int status = -1;
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        if (intersections[pos].first == 0) {
            // Overlapping part.
            ElementPos shape_element_pos = overlapping_parts[intersections[pos].second].first;
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second].second;
            if (strictly_lesser(element.length(current_point.point), element.length(overlapping_part.start))) {
                Point point_between = element.find_point_between(current_point.point, overlapping_part.start);
                if (shape.contains(point_between, true)) {
                    if (status == 0) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 1;
                } else {
                    if (status == 1) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 0;
                }
            }
            if (strictly_lesser(element.length(current_point.point), element.length(overlapping_part.end)))
                current_point = {shape_element_pos, overlapping_part.end};
        } else {
            // Intersection point.
            const ShapePoint& point = intersection_points[intersections[pos].second];
            if (strictly_lesser(element.length(current_point.point), element.length(point.point))) {
                Point point_between = element.find_point_between(current_point.point, point.point);
                if (shape.contains(point_between, true)) {
                    if (status == 0) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 1;
                } else {
                    if (status == 1) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 0;
                }
            }
        }
    }
    return output;
}

bool shape::intersect(
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict)
{
    if (shape_1.is_path && shape_2.is_path && strict) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "intersections involving two paths must not be strict.");
    }

    if (!strict) {
        for (const ShapeElement& element_1: shape_1.elements)
            for (const ShapeElement& element_2: shape_2.elements)
                if (shape::intersect(element_1, element_2))
                    return true;
        if (!shape_1.is_path && shape_1.contains(shape_2.elements.front().start))
            return true;
        if (!shape_2.is_path && shape_2.contains(shape_1.elements.front().start))
            return true;
        return false;
    }

    if (shape_2.is_path)
        return intersect(shape_2, shape_1, strict);

    std::vector<ShapePoint> intersection_points;
    std::vector<std::pair<ElementPos, ShapeElement>> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ElementPos element_1_pos = 0;
            element_1_pos < (ElementPos)shape_1.elements.size();
            ++element_1_pos) {
        const ShapeElement& element_1 = shape_1.elements[element_1_pos];
        for (ElementPos element_2_pos = 0;
                element_2_pos < (ElementPos)shape_2.elements.size();
                ++element_2_pos) {
            const ShapeElement& element_2 = shape_2.elements[element_2_pos];
            ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                    element_1,
                    element_2);
            for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
                intersections.push_back({0, (ElementPos)overlapping_parts.size()});
                overlapping_parts.push_back({element_1_pos, overlapping_part});
            }
            for (const Point& intersection: intersections_cur.improper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back({element_1_pos, intersection});
            }
            for (const Point& intersection: intersections_cur.proper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back({element_1_pos, intersection});
            }
        }
    }
    if (shape_1.is_path) {
        intersections.push_back({1, (ElementPos)intersection_points.size()});
        intersection_points.push_back({(ElementPos)shape_1.elements.size() - 1, shape_1.elements.back().end});
    }
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&shape_1, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                ShapePoint point_1 = (p1.first == 0)?
                        ShapePoint{
                            overlapping_parts[p1.second].first,
                            overlapping_parts[p1.second].second.start}:
                        intersection_points[p1.second];
                ShapePoint point_2 = (p2.first == 0)?
                        ShapePoint{
                            overlapping_parts[p2.second].first,
                            overlapping_parts[p2.second].second.start}:
                        intersection_points[p2.second];
                return shape_1.is_strictly_closer_to_path_start(point_1, point_2);
            });
    ShapePoint current_point = {0, shape_1.elements.front().start};
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        if (intersections[pos].first == 0) {
            // Overlapping part.
            ElementPos element_pos = overlapping_parts[intersections[pos].second].first;
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second].second;
            ShapePoint point_start = {element_pos, overlapping_part.start};
            if (shape_1.is_strictly_closer_to_path_start(current_point, point_start)) {
                ShapePoint point_between = shape_1.find_point_between(current_point, point_start);
                if (shape_2.contains(point_between.point, true))
                    return true;
            }
            ShapePoint point_end = {element_pos, overlapping_part.end};
            if (shape_1.is_strictly_closer_to_path_start(current_point, point_end))
                current_point = point_end;
        } else {
            // Intersection point.
            const ShapePoint& point = intersection_points[intersections[pos].second];
            if (shape_1.is_strictly_closer_to_path_start(current_point, point)) {
                ShapePoint point_between = shape_1.find_point_between(current_point, point);
                if (shape_2.contains(point_between.point, true))
                    return true;
                current_point = point;
            }
        }
    }

    if (!shape_1.is_path) {
        if (shape_2.contains(shape_1.find_point_strictly_inside()))
            return true;
        if (shape_1.contains(shape_2.find_point_strictly_inside()))
            return true;
    }

    return false;
}

struct PathShapeOverlappingPart
{
    ElementPos path_element_pos;
    ElementPos shape_element_pos;
    ShapeElement overlapping_part;
};

std::vector<PathShapeIntersectionPoint> shape::compute_intersections(
        const Shape& path,
        const Shape& shape,
        bool only_min_max)
{
    std::vector<PathShapeIntersectionPoint> output;
    for (ElementPos path_element_pos = 0;
            path_element_pos < (ElementPos)path.elements.size();
            ++path_element_pos) {
        const ShapeElement& path_element = path.elements[path_element_pos];
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                    path_element,
                    shape_element);
            //if (!intersections_cur.overlapping_parts.empty()
            //        || !intersections_cur.improper_intersections.empty()
            //        || !intersections_cur.proper_intersections.empty()) {
            //    std::cout << "element_1 " << path_element.to_string() << std::endl;
            //    std::cout << "element_2 " << shape_element.to_string() << std::endl;
            //    std::cout << "intersections " << intersections_cur.to_string(0) << std::endl;
            //}
            for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
                output.push_back({path_element_pos, shape_element_pos, overlapping_part.start});
                output.push_back({path_element_pos, shape_element_pos, overlapping_part.end});
            }
            for (const Point& intersection: intersections_cur.improper_intersections)
                output.push_back({path_element_pos, shape_element_pos, intersection});
            for (const Point& intersection: intersections_cur.proper_intersections)
                output.push_back({path_element_pos, shape_element_pos, intersection});
        }
    }
    if (output.empty())
        return {};
    //std::cout << "output.size() " << output.size() << std::endl;
    //for (const auto& point: output) {
    //    std::cout << "path_element_pos " << point.path_element_pos
    //        << " shape_element_pos " << point.shape_element_pos
    //        << " point " << point.point.to_string()
    //        << " l " << path.elements[point.path_element_pos].length(point.point)
    //        << std::endl;
    //}
    if (only_min_max) {
        auto p = std::minmax_element(
            output.begin(),
            output.end(),
            [&path](
                const PathShapeIntersectionPoint& p1,
                const PathShapeIntersectionPoint& p2)
            {
                ShapePoint point_1 = {p1.path_element_pos, p1.point};
                ShapePoint point_2 = {p2.path_element_pos, p2.point};
                return path.is_strictly_closer_to_path_start(point_1, point_2);
            });
        return {*p.first, *p.second};
    }
    // Sort intersections.
    std::sort(
        output.begin(),
        output.end(),
        [&path](
            const PathShapeIntersectionPoint& p1,
            const PathShapeIntersectionPoint& p2)
        {
            ShapePoint point_1 = {p1.path_element_pos, p1.point};
            ShapePoint point_2 = {p2.path_element_pos, p2.point};
            return path.is_strictly_closer_to_path_start(point_1, point_2);
        });
    return output;
}

std::vector<PathShapeIntersectionPoint> shape::compute_strict_intersections(
        const Shape& path,
        const Shape& shape,
        bool only_first)
{
    if (shape.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "intersections involving a path and an element must not be strict.");
    }

    std::vector<PathShapeIntersectionPoint> intersection_points;
    std::vector<PathShapeOverlappingPart> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ElementPos path_element_pos = 0;
            path_element_pos < (ElementPos)path.elements.size();
            ++path_element_pos) {
        const ShapeElement& path_element = path.elements[path_element_pos];
        //std::cout << "path_element " << path_element.to_string() << std::endl;
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            //std::cout << "shape_element " << shape_element.to_string() << std::endl;
            ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                    path_element,
                    shape_element);
            //std::cout << intersections_cur.to_string(0) << std::endl;
            for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
                intersections.push_back({0, (ElementPos)overlapping_parts.size()});
                overlapping_parts.push_back({path_element_pos, shape_element_pos, overlapping_part});
            }
            for (const Point& intersection: intersections_cur.improper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back({path_element_pos, shape_element_pos, intersection});
            }
            for (const Point& intersection: intersections_cur.proper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back({path_element_pos, shape_element_pos, intersection});
            }
        }
    }
    intersections.push_back({1, (ElementPos)intersection_points.size()});
    intersection_points.push_back({(ElementPos)path.elements.size() - 1, -1, path.elements.back().end});
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&path, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                ShapePoint point_1 = (p1.first == 0)?
                        ShapePoint{
                            overlapping_parts[p1.second].path_element_pos,
                            overlapping_parts[p1.second].overlapping_part.start}:
                        ShapePoint{
                            intersection_points[p1.second].path_element_pos,
                            intersection_points[p1.second].point};
                ShapePoint point_2 = (p2.first == 0)?
                        ShapePoint{
                            overlapping_parts[p2.second].path_element_pos,
                            overlapping_parts[p2.second].overlapping_part.start}:
                        ShapePoint{
                            intersection_points[p2.second].path_element_pos,
                            intersection_points[p2.second].point};
                return path.is_strictly_closer_to_path_start(point_1, point_2);
            });
    std::vector<PathShapeIntersectionPoint> output;
    PathShapeIntersectionPoint current_point = {0, -1, path.elements.front().start};
    int status = -1;
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        //std::cout << "pos " << pos
        //    << " current_point " << current_point.path_element_pos
        //    << " " << current_point.shape_element_pos
        //    << " " << current_point.point.to_string()
        //    << " status " << status
        //    << std::endl;
        ShapePoint current_path_point = {current_point.path_element_pos, current_point.point};
        if (intersections[pos].first == 0) {
            // Overlapping part.
            ElementPos shape_element_pos = overlapping_parts[intersections[pos].second].shape_element_pos;
            ElementPos path_element_pos = overlapping_parts[intersections[pos].second].path_element_pos;
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second].overlapping_part;
            //std::cout << "overlapping_part " << overlapping_part.to_string() << std::endl;
            ShapePoint point_start = {path_element_pos, overlapping_part.start};
            if (path.is_strictly_closer_to_path_start(current_path_point, point_start)) {
                ShapePoint point_between = path.find_point_between(current_path_point, point_start);
                if (shape.contains(point_between.point, true)) {
                    if (status == 0) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 1;
                } else {
                    if (status == 1) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 0;
                }
            }
            ShapePoint point_end = {path_element_pos, overlapping_part.end};
            if (path.is_strictly_closer_to_path_start(current_path_point, point_end))
                current_point = {path_element_pos, shape_element_pos, overlapping_part.end};
        } else {
            // Intersection point.
            const PathShapeIntersectionPoint& point = intersection_points[intersections[pos].second];
            //std::cout << "point " << point.path_element_pos
            //    << " " << point.point.to_string() << std::endl;
            ShapePoint path_point = {point.path_element_pos, point.point};
            if (path.is_strictly_closer_to_path_start(current_path_point, path_point)) {
                ShapePoint point_between = path.find_point_between(current_path_point, path_point);
                //std::cout << "point_between " << point_between.point.to_string() << std::endl;
                if (shape.contains(point_between.point, true)) {
                    if (status == 0) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 1;
                } else {
                    if (status == 1) {
                        output.push_back(current_point);
                        if (only_first)
                            return output;
                    }
                    status = 0;
                }
                current_point = point;
            }
        }
    }
    return output;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes)
{
    if (intersect(shape_with_holes.shape))
        return true;
    for (const Shape& hole: shape_with_holes.holes)
        if (intersect(hole))
            return true;
    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes,
        const ShapeElement& element,
        bool strict)
{
    if (!strict) {
        for (const ShapeElement& shape_element: shape_with_holes.shape.elements)
            if (shape::intersect(shape_element, element))
                return true;
        for (const Shape& hole: shape_with_holes.holes)
            for (const ShapeElement& shape_element: hole.elements)
                if (shape::intersect(shape_element, element))
                    return true;
        if (shape_with_holes.contains(element.start))
            return true;
        return false;
    }

    std::vector<Point> intersection_points;
    std::vector<ShapeElement> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ShapePos shape_pos = -1;
            shape_pos < (ShapePos)shape_with_holes.holes.size();
            ++shape_pos) {
        const Shape& shape = (shape_pos == -1)? shape_with_holes.shape: shape_with_holes.holes[shape_pos];
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                    element,
                    shape_element);
            for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
                intersections.push_back({0, (ElementPos)overlapping_parts.size()});
                overlapping_parts.push_back(overlapping_part);
            }
            for (const Point& intersection: intersections_cur.improper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back(intersection);
            }
            for (const Point& intersection: intersections_cur.proper_intersections) {
                intersections.push_back({1, (ElementPos)intersection_points.size()});
                intersection_points.push_back(intersection);
            }
        }
    }
    intersections.push_back({1, (ElementPos)intersection_points.size()});
    intersection_points.push_back(element.end);
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&element, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                const Point& point_1 = (p1.first == 0)?
                    overlapping_parts[p1.second].start:
                    intersection_points[p1.second];
                const Point& point_2 = (p2.first == 0)?
                    overlapping_parts[p2.second].start:
                    intersection_points[p2.second];
                return element.length(point_1) < element.length(point_2);
            });
    Point current_point = element.start;
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        //std::cout << "pos " << pos << " current_point " << current_point.to_string() << std::endl;
        if (intersections[pos].first == 0) {
            // Overlapping part.
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second];
            if (strictly_lesser(element.length(current_point), element.length(overlapping_part.start))) {
                Point point_between = element.find_point_between(current_point, overlapping_part.start);
                if (shape_with_holes.contains(point_between, true))
                    return true;
            }
            if (strictly_lesser(element.length(current_point), element.length(overlapping_part.end)))
                current_point = overlapping_part.end;
        } else {
            // Intersection point.
            const Point& point = intersection_points[intersections[pos].second];
            //std::cout << "point " << point.to_string() << std::endl;
            if (strictly_lesser(element.length(current_point), element.length(point))) {
                Point point_between = element.find_point_between(current_point, point);
                //std::cout << "point_between " << point_between.to_string() << std::endl;
                if (shape_with_holes.contains(point_between, true))
                    return true;
                current_point = point;
            }
        }
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes,
        const Shape& shape,
        bool strict)
{
    if (!strict) {
        for (const ShapeElement& element_1: shape_with_holes.shape.elements)
            for (const ShapeElement& element_2: shape.elements)
                if (shape::intersect(element_1, element_2))
                    return true;
        for (const Shape& hole: shape_with_holes.holes)
            for (const ShapeElement& element_1: hole.elements)
                for (const ShapeElement& element_2: shape.elements)
                    if (shape::intersect(element_1, element_2))
                        return true;
        if (shape_with_holes.contains(shape.elements.front().start))
            return true;
        if (!shape.is_path && shape.contains(shape_with_holes.shape.elements.front().start))
            return true;
        return false;
    }

    std::vector<ShapePoint> intersection_points;
    std::vector<std::pair<ElementPos, ShapeElement>> overlapping_parts;
    std::vector<std::pair<int, ElementPos>> intersections;
    for (ElementPos element_1_pos = 0;
            element_1_pos < (ElementPos)shape.elements.size();
            ++element_1_pos) {
        const ShapeElement& element_1 = shape.elements[element_1_pos];
        for (ShapePos shape_pos = -1;
                shape_pos < (ShapePos)shape_with_holes.holes.size();
                ++shape_pos) {
            const Shape& shape_2 = (shape_pos == -1)? shape_with_holes.shape: shape_with_holes.holes[shape_pos];
            for (ElementPos element_2_pos = 0;
                    element_2_pos < (ElementPos)shape_2.elements.size();
                    ++element_2_pos) {
                const ShapeElement& element_2 = shape_2.elements[element_2_pos];
                ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                        element_1,
                        element_2);
                //if (!intersections_cur.overlapping_parts.empty()
                //        || !intersections_cur.improper_intersections.empty()
                //        || !intersections_cur.proper_intersections.empty()) {
                //    std::cout << "element_1 " << element_1.to_string() << std::endl;
                //    std::cout << "element_2 " << element_2.to_string() << std::endl;
                //    std::cout << "intersections " << intersections_cur.to_string(0) << std::endl;
                //}
                for (const ShapeElement& overlapping_part: intersections_cur.overlapping_parts) {
                    intersections.push_back({0, (ElementPos)overlapping_parts.size()});
                    overlapping_parts.push_back({element_1_pos, overlapping_part});
                }
                for (const Point& intersection: intersections_cur.improper_intersections) {
                    intersections.push_back({1, (ElementPos)intersection_points.size()});
                    intersection_points.push_back({element_1_pos, intersection});
                }
                for (const Point& intersection: intersections_cur.proper_intersections) {
                    intersections.push_back({1, (ElementPos)intersection_points.size()});
                    intersection_points.push_back({element_1_pos, intersection});
                }
            }
        }
    }
    if (shape.is_path) {
        intersections.push_back({1, (ElementPos)intersection_points.size()});
        intersection_points.push_back({(ElementPos)shape.elements.size() - 1, shape.elements.back().end});
    }
    // Sort intersections.
    std::sort(
            intersections.begin(),
            intersections.end(),
            [&shape, &overlapping_parts, &intersection_points](
                const std::pair<int, ElementPos>& p1,
                const std::pair<int, ElementPos>& p2)
            {
                const ShapePoint& point_1 = (p1.first == 0)?
                        ShapePoint{
                            overlapping_parts[p1.second].first,
                            overlapping_parts[p1.second].second.start}:
                        intersection_points[p1.second];
                const ShapePoint& point_2 = (p2.first == 0)?
                        ShapePoint{
                            overlapping_parts[p2.second].first,
                            overlapping_parts[p2.second].second.start}:
                        intersection_points[p2.second];
                return shape.is_strictly_closer_to_path_start(point_1, point_2);
            });
    ShapePoint current_point = {0, shape.elements.front().start};
    for (ElementPos pos = 0; pos < (ElementPos)intersections.size(); ++pos) {
        //std::cout << "pos " << pos << " current_point " << current_point.point.to_string() << std::endl;
        if (intersections[pos].first == 0) {
            // Overlapping part.
            ElementPos element_pos = overlapping_parts[intersections[pos].second].first;
            const ShapeElement& overlapping_part = overlapping_parts[intersections[pos].second].second;
            ShapePoint point_start = {element_pos, overlapping_part.start};
            if (shape.is_strictly_closer_to_path_start(current_point, point_start)) {
                ShapePoint point_between = shape.find_point_between(current_point, point_start);
                if (shape_with_holes.contains(point_between.point, true)) {
                    //shape_with_holes.shape.contains_export_inputs(
                    //        "shape_contains_inputs.json",
                    //        point_between.point,
                    //        true);
                    //std::cout << "contains " << point_between.point.to_string() << std::endl;
                    return true;
                }
            }
            ShapePoint point_end = {element_pos, overlapping_part.end};
            if (shape.is_strictly_closer_to_path_start(current_point, point_end))
                current_point = point_end;
        } else {
            // Intersection point.
            const ShapePoint& point = intersection_points[intersections[pos].second];
            //std::cout << "point " << point.point.to_string() << std::endl;
            if (shape.is_strictly_closer_to_path_start(current_point, point)) {
                ShapePoint point_between = shape.find_point_between(current_point, point);
                //std::cout << "point_between " << point_between.point.to_string() << std::endl;
                if (shape_with_holes.contains(point_between.point, true)) {
                    //shape_with_holes.shape.contains_export_inputs(
                    //        "shape_contains_inputs.json",
                    //        point_between.point,
                    //        true);
                    //std::cout << "contains " << point_between.point.to_string() << std::endl;
                    return true;
                }
                current_point = point;
            }
        }
    }

    if (!shape.is_path) {
        if (shape_with_holes.contains(shape.find_point_strictly_inside()))
            return true;
        if (shape.contains(shape_with_holes.find_point_strictly_inside()))
            return true;
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict)
{
    if (!strict) {
        for (const ShapeElement& element_1: shape_with_holes_1.shape.elements) {
            for (const ShapeElement& element_2: shape_with_holes_2.shape.elements)
                if (shape::intersect(element_1, element_2))
                    return true;
            for (const Shape& hole_2: shape_with_holes_2.holes)
                for (const ShapeElement& element_2: hole_2.elements)
                    if (shape::intersect(element_1, element_2))
                        return true;
        }
        for (const Shape& hole_1: shape_with_holes_1.holes) {
            for (const ShapeElement& element_1: hole_1.elements) {
                for (const ShapeElement& element_2: shape_with_holes_2.shape.elements)
                    if (shape::intersect(element_1, element_2))
                        return true;
                for (const Shape& hole_2: shape_with_holes_2.holes)
                    for (const ShapeElement& element_2: hole_2.elements)
                        if (shape::intersect(element_1, element_2))
                            return true;
            }
        }
        if (shape_with_holes_1.contains(shape_with_holes_2.shape.elements.front().start))
            return true;
        if (shape_with_holes_2.contains(shape_with_holes_1.shape.elements.front().start))
            return true;
        return false;
    }

    return !compute_intersection({shape_with_holes_1, shape_with_holes_2}).empty();
}

void shape::intersect_export_inputs(
        const std::string& file_path,
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape_1"] = shape_1.to_json();
    json["shape_2"] = shape_2.to_json();
    json["strict"] = strict;
    file << std::setw(4) << json << std::endl;
}

void shape::compute_intersections_export_inputs(
        const std::string& file_path,
        const Shape& path,
        const Shape& shape,
        bool only_min_max)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["path"] = path.to_json();
    json["shape"] = shape.to_json();
    json["only_min_max"] = only_min_max;
    file << std::setw(4) << json << std::endl;
}

void shape::compute_strict_intersections_export_inputs(
        const std::string& file_path,
        const Shape& path,
        const Shape& shape,
        bool only_first)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["path"] = path.to_json();
    json["shape"] = shape.to_json();
    json["only_first"] = only_first;
    file << std::setw(4) << json << std::endl;
}

void shape::intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes,
        const ShapeElement& element,
        bool strict)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape_with_holes"] = shape_with_holes.to_json();
    json["element"] = element.to_json();
    json["strict"] = strict;
    file << std::setw(4) << json << std::endl;
}

void shape::intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes_1,
        const Shape& shape_2,
        bool strict)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape_with_holes"] = shape_with_holes_1.to_json();
    json["shape"] = shape_2.to_json();
    json["strict"] = strict;
    file << std::setw(4) << json << std::endl;
}

void shape::intersect_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape_with_holes_1"] = shape_with_holes_1.to_json();
    json["shape_with_holes_2"] = shape_with_holes_2.to_json();
    json["strict"] = strict;
    file << std::setw(4) << json << std::endl;
}
