//#define CLEAN_ENABLE_DEBUG

#include "shape/clean.hpp"

#include "shape/elements_intersections.hpp"
#include "shape/equalize.hpp"
#include "shape/boolean_operations.hpp"
#include "shape/shapes_intersections.hpp"
#ifdef CLEAN_ENABLE_DEBUG
#include "shape/writer.hpp"
#include <fstream>
#endif

#include <algorithm>
#include <cmath>
#include <limits>

//#include <iostream>

using namespace shape;

std::pair<bool, Shape> shape::remove_redundant_vertices(
        const Shape& shape)
{
    //std::cout << "remove_redundant_vertices " << shape.to_string(2) << std::endl;

    if (!shape.is_path && shape.elements.size() <= 3)
        return {false, shape};

    ElementPos number_of_elements_removed = 0;
    Shape shape_new;
    shape_new.is_path = shape.is_path;

    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)shape.elements.size();
            ++element_cur_pos) {

        //std::cout << "element_cur_pos " << element_cur_pos << std::endl;
        const ShapeElement& element = shape.elements[element_cur_pos];
        // A full circle starts and ends at the same point, but it doesn't
        // have a null length.
        bool useless = false;
        if (!(element.type == ShapeElementType::CircularArc
                    && element.orientation == ShapeElementOrientation::Full)
                && equal(element.start.x, element.end.x)
                && equal(element.start.y, element.end.y)) {
            useless = true;
        }
        if (!useless || (!shape.is_path && shape.elements.size() - number_of_elements_removed <= 3)) {
            if (!shape_new.elements.empty())
                shape_new.elements.back().end = element.start;
            shape_new.elements.push_back(element);
        } else {
            number_of_elements_removed++;
        }
    }
    if (!shape.is_path)
        shape_new.elements.back().end = shape_new.elements.front().start;

    //if (!shape_new.check()) {
    //    throw std::invalid_argument(
    //            FUNC_SIGNATURE + ": invalid output shape.");
    //}
    return {(number_of_elements_removed > 0), shape_new};
}

std::pair<bool, ShapeWithHoles> shape::remove_redundant_vertices(
        const ShapeWithHoles& shape)
{
    ShapeWithHoles res;
    bool b = false;

    auto p = remove_redundant_vertices(shape.shape);
    b |= p.first;
    res.shape = p.second;

    res.holes = std::vector<Shape>(shape.holes.size());
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];
        auto p = remove_redundant_vertices(hole);
        b |= p.first;
        res.holes[hole_pos] = p.second;
    }

    return {b, res};
}

std::pair<bool, Shape> shape::remove_aligned_vertices(
        const Shape& shape)
{
    //std::cout << "remove_aligned_vertices " << shape.to_string(2) << std::endl;

    if (shape.elements.size() <= 3)
        return {false, shape};

    // The start of an element is removed if the line from the start of the
    // last element kept to the end of the element goes within the tolerance
    // of it, and of every vertex removed since the last element kept, that
    // is, through the square of half-side 1e-6 around each of them, in which
    // 'equal' considers points equal to it. Checking only the vertex removed
    // would allow removing vertices one after the other along a slightly
    // curved line, moving the result away from the shape by much more than
    // the tolerance.
    // The directions from the start of the last element kept going through
    // the square of a vertex form an interval of angles, so only the bounds
    // of the intersection of the intervals of the vertices removed since the
    // last element kept are needed. The angles are measured from the
    // direction of the first of these vertices.
    Angle infinity = std::numeric_limits<Angle>::infinity();
    auto relative_angle = [](
            const Point& reference_direction,
            const Point& direction)
    {
        return std::atan2(
                cross_product(reference_direction, direction),
                dot_product(reference_direction, direction));
    };
    auto compute_interval = [infinity, &relative_angle](
            const Point& apex,
            const Point& reference_direction,
            const Point& vertex) -> std::pair<Angle, Angle>
    {
        if (equal(apex, vertex))
            return {-infinity, infinity};
        // Seen from outside of the square, its corners are within a quarter
        // turn of its center.
        Point center_direction = vertex - apex;
        Angle center_angle = relative_angle(reference_direction, center_direction);
        Angle offset_min = infinity;
        Angle offset_max = -infinity;
        for (LengthDbl dx: {-1e-6, 1e-6}) {
            for (LengthDbl dy: {-1e-6, 1e-6}) {
                Point corner = {vertex.x + dx, vertex.y + dy};
                Angle offset = relative_angle(center_direction, corner - apex);
                offset_min = (std::min)(offset_min, offset);
                offset_max = (std::max)(offset_max, offset);
            }
        }
        return {center_angle + offset_min, center_angle + offset_max};
    };

    // For a closed shape, start from a vertex which can't be removed, even
    // considering only its two elements, so that the vertices removed
    // together are never on both sides of the start.
    ElementPos number_of_elements = shape.elements.size();
    ElementPos start_pos = 0;
    if (!shape.is_path) {
        for (ElementPos element_pos = 0;
                element_pos < number_of_elements;
                ++element_pos) {
            const ShapeElement& element_prev = shape.elements[(element_pos + number_of_elements - 1) % number_of_elements];
            const ShapeElement& element = shape.elements[element_pos];
            if (element_prev.type != ShapeElementType::LineSegment
                    || element.type != ShapeElementType::LineSegment
                    || equal(element.end, element_prev.start)) {
                start_pos = element_pos;
                break;
            }
            Point reference_direction = element.start - element_prev.start;
            std::pair<Angle, Angle> interval = compute_interval(
                    element_prev.start,
                    reference_direction,
                    element.start);
            Angle angle = relative_angle(reference_direction, element.end - element_prev.start);
            if (angle < interval.first || angle > interval.second) {
                start_pos = element_pos;
                break;
            }
        }
    }

    ElementPos number_of_elements_removed = 0;
    Shape shape_new;
    shape_new.is_path = shape.is_path;
    // Position in 'shape_new' of the first element kept before 'start_pos',
    // to restore the order of the elements at the end.
    ElementPos new_first_pos = 0;

    ElementPos element_prev_pos = -1;
    Point reference_direction = {1, 0};
    Angle angle_min = -infinity;
    Angle angle_max = infinity;
    for (ElementPos counter = 0;
            counter < number_of_elements;
            ++counter) {
        ElementPos element_cur_pos = (start_pos + counter) % number_of_elements;
        const ShapeElement& element_cur = shape.elements[element_cur_pos];
        //std::cout << "element_cur   " << element_cur_pos << " " << element_cur.to_string() << std::endl;
        bool useless = false;
        Point reference_direction_new = reference_direction;
        std::pair<Angle, Angle> interval_new;
        // The first vertex is kept.
        if (counter > 0) {
            const ShapeElement& element_prev = shape.elements[element_prev_pos];
            if (element_prev.type == ShapeElementType::LineSegment
                    && element_cur.type == ShapeElementType::LineSegment
                    && !equal(element_cur.end, element_prev.start)) {
                if (element_prev_pos == (element_cur_pos + number_of_elements - 1) % number_of_elements)
                    reference_direction_new = element_cur.start - element_prev.start;
                std::pair<Angle, Angle> interval = compute_interval(
                        element_prev.start,
                        reference_direction_new,
                        element_cur.start);
                interval_new = {
                    (std::max)(angle_min, interval.first),
                    (std::min)(angle_max, interval.second)};
                Angle angle = relative_angle(
                        reference_direction_new,
                        element_cur.end - element_prev.start);
                if (angle >= interval_new.first && angle <= interval_new.second)
                    useless = true;
            }
        }
        if (!useless || number_of_elements - number_of_elements_removed <= 3) {
            if (!shape_new.elements.empty())
                shape_new.elements.back().end = element_cur.start;
            if (element_cur_pos < start_pos && new_first_pos == 0)
                new_first_pos = shape_new.elements.size();
            shape_new.elements.push_back(element_cur);
            element_prev_pos = element_cur_pos;
            angle_min = -infinity;
            angle_max = infinity;
        } else {
            //std::cout << "useless " << element_cur.to_string() << std::endl;
            shape_new.elements.back().end = element_cur.end;
            reference_direction = reference_direction_new;
            angle_min = interval_new.first;
            angle_max = interval_new.second;
            number_of_elements_removed++;
        }
    }
    if (!shape.is_path)
        shape_new.elements.back().end = shape_new.elements.front().start;
    std::rotate(
            shape_new.elements.begin(),
            shape_new.elements.begin() + new_first_pos,
            shape_new.elements.end());

    return {(number_of_elements_removed > 0), shape_new};
}

std::pair<bool, ShapeWithHoles> shape::remove_aligned_vertices(
        const ShapeWithHoles& shape)
{
    ShapeWithHoles res;
    bool b = false;

    auto p = remove_aligned_vertices(shape.shape);
    b |= p.first;
    res.shape = p.second;

    res.holes = std::vector<Shape>(shape.holes.size());
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];
        auto p = remove_aligned_vertices(hole);
        b |= p.first;
        res.holes[hole_pos] = p.second;
    }

    return {b, res};
}

ShapeWithHoles shape::remove_small_holes(
        const ShapeWithHoles& shape_with_holes,
        AreaDbl area_limit)
{
    //std::cout << "remove_small_holes " << shape_with_holes.holes.size() << std::endl;
    ShapeWithHoles new_shape_with_holes;
    new_shape_with_holes.shape = shape_with_holes.shape;
    for (const Shape& hole: shape_with_holes.holes) {
        if (strictly_lesser(hole.compute_area(), area_limit))
            continue;
        new_shape_with_holes.holes.push_back(hole);
    }
    //std::cout << "remove_small_holes end " << new_shape_with_holes.holes.size() << std::endl;
    return new_shape_with_holes;
}

Shape shape::remove_backtracks(
        const Shape& shape)
{
    //std::cout << "remove_backtracks " << shape.to_string(0) << std::endl;

    // Handle circles.
    if (shape.elements.size() == 1)
        return shape;

    // The output elements are used as a stack, so that removing a backtrack
    // reveals the backtrack around it, if any.
    Shape output;
    output.is_path = shape.is_path;
    for (const ShapeElement& element: shape.elements) {
        if (!output.elements.empty()
                && equal(output.elements.back(), element.reverse())) {
            output.elements.pop_back();
            continue;
        }
        output.elements.push_back(element);
    }

    // For a closed shape, remove the backtracks across its start.
    if (!shape.is_path) {
        ElementPos number_of_elements_removed_at_start = 0;
        while ((ElementPos)output.elements.size() - number_of_elements_removed_at_start >= 2
                && equal(
                    output.elements[number_of_elements_removed_at_start],
                    output.elements.back().reverse())) {
            number_of_elements_removed_at_start++;
            output.elements.pop_back();
        }
        output.elements.erase(
                output.elements.begin(),
                output.elements.begin() + number_of_elements_removed_at_start);
    }

    return output;
}

Shape shape::flatten_arcs(
        const Shape& shape)
{
    Shape new_shape = shape;
    for (ShapeElement& element: new_shape.elements) {
        // A full circle (start == end) is never flat: its chord midpoint
        // formula degenerates to its start/end point, which trivially lies
        // on the arc and would otherwise cause it to be misdetected as flat
        // and collapsed into a zero-length line segment.
        if (element.type == ShapeElementType::CircularArc
                && element.orientation != ShapeElementOrientation::Full
                && element.contains((element.start + element.end) / 2)) {
            element.type = ShapeElementType::LineSegment;
        }
    }
    return new_shape;
}

ShapeWithHoles shape::flatten_arcs(
        const ShapeWithHoles& shape_with_holes)
{
    ShapeWithHoles new_shape_with_holes = shape_with_holes;
    new_shape_with_holes.shape = flatten_arcs(shape_with_holes.shape);
    new_shape_with_holes.holes = std::vector<Shape>(shape_with_holes.holes.size());
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape_with_holes.holes.size();
            ++hole_pos) {
        const Shape& hole = shape_with_holes.holes[hole_pos];
        new_shape_with_holes.holes[hole_pos] = flatten_arcs(hole);
    }
    return new_shape_with_holes;
}

Shape shape::recompute_centers(
        const Shape& shape)
{
    Shape new_shape = shape;
    for (ShapeElement& element: new_shape.elements) {
        if (element.type == ShapeElementType::CircularArc
                && !(element.start == element.end)) {
            element.center = element.recompute_center();
        }
    }
    return new_shape;
}

ShapeWithHoles shape::recompute_centers(
        const ShapeWithHoles& shape_with_holes)
{
    ShapeWithHoles new_shape_with_holes = shape_with_holes;
    for (ShapeElement& element: new_shape_with_holes.shape.elements) {
        if (element.type == ShapeElementType::CircularArc
                && !(element.start == element.end)) {
            element.center = element.recompute_center();
        }
    }
    for (Shape& hole: new_shape_with_holes.holes) {
        for (ShapeElement& element: hole.elements) {
            if (element.type == ShapeElementType::CircularArc
                    && !(element.start == element.end)) {
                element.center = element.recompute_center();
            }
        }
    }
    return new_shape_with_holes;
}

namespace
{

Shape clean_extreme_slopes_outer_1(
        Shape shape_orig,
        std::vector<Shape>& triangles_added)
{
    Shape shape = shape_orig;

    {
        Shape shape_new;
        shape_new.is_path = shape.is_path;
        Point start = shape.elements.front().start;
        for (ElementPos element_cur_pos = 0;
                element_cur_pos < (ElementPos)shape.elements.size();
                ++element_cur_pos) {
            ShapeElement element_cur = shape.elements[element_cur_pos];
            element_cur.start = start;
            const ShapeElement& element_next = (element_cur_pos + 1 < shape.elements.size())?
                shape.elements[element_cur_pos + 1]:
                shape.elements[0];

            // Check if current element must be removed.
            bool remove = false;
            if (element_cur.type == ShapeElementType::LineSegment
                    && element_next.type == ShapeElementType::LineSegment) {
                // The angle with the next element is between pi/2 and pi
                Angle angle = angle_radian(
                        element_cur.start - element_cur.end,
                        element_next.end - element_next.start);
                if (angle >= M_PI / 2 && angle <= M_PI) {
                    // Next element has extreme slope.
                    double slope
                        = (element_next.end.y - element_next.start.y)
                        / (element_next.end.x - element_next.start.x);
                    if (element_next.start.x != element_next.end.x && std::abs(slope) > 1e2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_next.start.x < element_next.end.x  // >
                                    && element_next.start.y < element_next.end.y)  // ^
                                || (element_next.start.x > element_next.end.x  // <
                                    && element_next.start.y > element_next.end.y)) {  // v
                            auto p = compute_line_intersection(
                                    element_cur.start, element_cur.end,
                                    {element_next.end.x, element_next.start.y}, element_next.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    } else if (element_next.start.y != element_next.end.y && std::abs(slope) < 1e-2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_next.start.x < element_next.end.x  // >
                                    && element_next.start.y > element_next.end.y)  // v
                                || (element_next.start.x > element_next.end.x  // <
                                    && element_next.start.y < element_next.end.y)) {  // ^
                            auto p = compute_line_intersection(
                                    element_cur.start, element_cur.end,
                                    {element_next.start.x, element_next.end.y}, element_next.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    }
                }
            }

            if (!remove) {
                shape_new.elements.push_back(element_cur);
                start = element_cur.end;
            } else {
                triangles_added.push_back(build_triangle(
                            start, element_cur.end, element_next.end));
            }
        }
        shape_new.elements[0].start = start;
        shape = shape_new;
    }

    {
        Shape shape_new;
        shape_new.is_path = shape.is_path;
        Point end = shape.elements.back().end;
        for (ElementPos element_cur_pos = shape.elements.size() - 1;
                element_cur_pos >= 0;
                --element_cur_pos) {
            ShapeElement element_cur = shape.elements[element_cur_pos];
            element_cur.end = end;
            const ShapeElement& element_prev = (element_cur_pos - 1 >= 0)?
                shape.elements[element_cur_pos - 1]:
                shape.elements.back();

            // Check if current element must be removed.
            bool remove = false;
            if (element_cur.type == ShapeElementType::LineSegment
                    && element_prev.type == ShapeElementType::LineSegment) {
                // The angle with the prev element is between pi/2 and pi
                Angle angle = angle_radian(
                        element_prev.start - element_prev.end,
                        element_cur.end - element_cur.start);
                //std::cout << "angle " << angle << std::endl;
                if (angle >= M_PI / 2 && angle <= M_PI) {
                    // Next element has extreme slope.
                    double slope
                        = (element_prev.end.y - element_prev.start.y)
                        / (element_prev.end.x - element_prev.start.x);
                    if (element_prev.start.x != element_prev.end.x && std::abs(slope) > 1e2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_prev.start.x < element_prev.end.x  // >
                                    && element_prev.start.y > element_prev.end.y)  // v
                                || (element_prev.start.x > element_prev.end.x  // <
                                    && element_prev.start.y < element_prev.end.y)) {  // ^
                            auto p = compute_line_intersection(
                                    element_prev.start, {element_prev.start.x, element_prev.end.y},
                                    element_cur.start, element_cur.end);
                            //std::cout << p.first << " " << p.second.to_string() << std::endl;
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    } else if (element_prev.start.y != element_prev.end.y && std::abs(slope) < 1e-2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_prev.start.x < element_prev.end.x  // >
                                    && element_prev.start.y < element_prev.end.y)  // ^
                                || (element_prev.start.x > element_prev.end.x  // <
                                    && element_prev.start.y > element_prev.end.y)) {  // v
                            auto p = compute_line_intersection(
                                    element_prev.start, {element_prev.end.x, element_prev.start.y},
                                    element_cur.start, element_cur.end);
                            //std::cout << p.first << " " << p.second.to_string() << std::endl;
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    }
                }
            }

            if (!remove) {
                shape_new.elements.push_back(element_cur);
                end = element_cur.start;
            } else {
                triangles_added.push_back(build_triangle(
                            element_prev.start, element_cur.start, end));
            }
        }
        shape_new.elements[0].end = end;
        std::reverse(shape_new.elements.begin(), shape_new.elements.end());
        shape = shape_new;
    }

    return shape;
}

Shape clean_extreme_slopes_inner_1(
        const Shape& shape_orig,
        std::vector<Shape>& triangles_removed)
{
    Shape shape = shape_orig;

    {
        Shape shape_new;
        shape_new.is_path = shape.is_path;
        Point start = shape.elements.front().start;
        for (ElementPos element_cur_pos = 0;
                element_cur_pos < (ElementPos)shape.elements.size();
                ++element_cur_pos) {
            ShapeElement element_cur = shape.elements[element_cur_pos];
            element_cur.start = start;
            const ShapeElement& element_next = (element_cur_pos + 1 < shape.elements.size())?
                shape.elements[element_cur_pos + 1]:
                shape.elements[0];

            // Check if current element must be removed.
            bool remove = false;
            if (element_cur.type == ShapeElementType::LineSegment
                    && element_next.type == ShapeElementType::LineSegment) {
                // The angle with the next element is between pi/2 and pi
                Angle angle = angle_radian(
                        element_cur.start - element_cur.end,
                        element_next.end - element_next.start);
                if (angle >= M_PI / 2 && angle <= M_PI) {
                    // Next element has extreme slope.
                    double slope
                        = (element_next.end.y - element_next.start.y)
                        / (element_next.end.x - element_next.start.x);
                    if (element_next.start.x != element_next.end.x && std::abs(slope) > 1e2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_next.start.x > element_next.end.x  // <
                                    && element_next.start.y < element_next.end.y)  // ^
                                || (element_next.start.x < element_next.end.x  // >
                                    && element_next.start.y > element_next.end.y)) {  // v
                            auto p = compute_line_intersection(
                                    element_cur.start, element_cur.end,
                                    {element_next.end.x, element_next.start.y}, element_next.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    } else if (element_next.start.y != element_next.end.y && std::abs(slope) < 1e-2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_next.start.x > element_next.end.x  // <
                                    && element_next.start.y > element_next.end.y)  // v
                                || (element_next.start.x < element_next.end.x  // >
                                    && element_next.start.y < element_next.end.y)) {  // ^
                            auto p = compute_line_intersection(
                                    element_cur.start, element_cur.end,
                                    {element_next.start.x, element_next.end.y}, element_next.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    }
                }
            }

            if (!remove) {
                shape_new.elements.push_back(element_cur);
                start = element_cur.end;
            } else {
                triangles_removed.push_back(build_triangle(
                            start, element_cur.end, element_next.end));
            }
        }
        shape_new.elements[0].start = start;
        shape = shape_new;
    }

    {
        Shape shape_new;
        shape_new.is_path = shape.is_path;
        Point end = shape.elements.back().end;
        for (ElementPos element_cur_pos = shape.elements.size() - 1;
                element_cur_pos >= 0;
                --element_cur_pos) {
            ShapeElement element_cur = shape.elements[element_cur_pos];
            element_cur.end = end;
            const ShapeElement& element_prev = (element_cur_pos - 1 >= 0)?
                shape.elements[element_cur_pos - 1]:
                shape.elements.back();

            // Check if current element must be removed.
            bool remove = false;
            if (element_cur.type == ShapeElementType::LineSegment
                    && element_prev.type == ShapeElementType::LineSegment) {
                // The angle with the prev element is between pi/2 and pi
                Angle angle = angle_radian(
                        element_prev.start - element_prev.end,
                        element_cur.end - element_cur.start);
                if (angle >= M_PI / 2 && angle <= M_PI) {
                    // Next element has extreme slope.
                    double slope
                        = (element_prev.end.y - element_prev.start.y)
                        / (element_prev.end.x - element_prev.start.x);
                    if (element_prev.start.x != element_prev.end.x && std::abs(slope) > 1e2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_prev.start.x > element_prev.end.x  // <
                                    && element_prev.start.y > element_prev.end.y)  // v
                                || (element_prev.start.x < element_prev.end.x  // >
                                    && element_prev.start.y < element_prev.end.y)) {  // ^
                            auto p = compute_line_intersection(
                                    element_prev.start, {element_prev.start.x, element_prev.end.y},
                                    element_cur.start, element_cur.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    } else if (element_prev.start.y != element_prev.end.y && std::abs(slope) < 1e-2) {
                        // The intersection point doesn't belong to the current element.
                        if ((element_prev.start.x > element_prev.end.x  // <
                                    && element_prev.start.y < element_prev.end.y)  // ^
                                || (element_prev.start.x < element_prev.end.x  // >
                                    && element_prev.start.y > element_prev.end.y)) {  // v
                            auto p = compute_line_intersection(
                                    element_prev.start, {element_prev.end.x, element_prev.start.y},
                                    element_cur.start, element_cur.end);
                            if (p.first && !element_cur.contains(p.second))
                                remove = true;
                        }
                    }
                }
            }

            if (!remove) {
                shape_new.elements.push_back(element_cur);
                end = element_cur.start;
            } else {
                triangles_removed.push_back(build_triangle(
                            element_prev.start, element_cur.start, end));
            }
        }
        shape_new.elements[0].end = end;
        std::reverse(shape_new.elements.begin(), shape_new.elements.end());
        shape = shape_new;
    }

    return shape;
}

Shape clean_extreme_slopes_outer_2(
        const Shape& shape_orig,
        std::vector<Shape>& triangles_added)
{
    if (!shape_orig.check()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "invalid input shape.");
    }

    Shape shape = shape_orig;
    ElementPos element_prev_pos = shape.elements.size() - 1;
    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)shape.elements.size();
            ++element_cur_pos) {
        ShapeElement& element_prev = shape.elements[element_prev_pos];
        ShapeElement& element_cur = shape.elements[element_cur_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_prev.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if ((element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y < element_cur.end.y)  // ^
                    || (element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y > element_cur.end.y)) {  // v
                // Update element_prev.
                auto p = compute_line_intersection(
                        element_prev.start, element_prev.end,
                        {element_cur.end.x, element_cur.start.y}, element_cur.end);
                if (p.first) {
                    triangles_added.push_back(build_triangle(
                                element_prev.start, element_cur.start, element_cur.end));
                    element_prev.end = p.second;
                    element_cur.start = p.second;
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if ((element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y > element_cur.end.y)  // v
                    || (element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y < element_cur.end.y)) {  // ^
                // Update element_prev.
                auto p = compute_line_intersection(
                        element_prev.start, element_prev.end,
                        {element_cur.start.x, element_cur.end.y}, element_cur.end);
                if (p.first) {
                    triangles_added.push_back(build_triangle(
                                element_prev.start, element_cur.start, element_cur.end));
                    element_prev.end = p.second;
                    element_cur.start = p.second;
                }
            }
        }
        element_prev_pos = element_cur_pos;
    }
    ElementPos element_next_pos = 0;
    for (ElementPos element_cur_pos = shape.elements.size() - 1;
            element_cur_pos >= 0;
            --element_cur_pos) {
        ShapeElement& element_cur = shape.elements[element_cur_pos];
        ShapeElement& element_next = shape.elements[element_next_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_next.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if ((element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y > element_cur.end.y)  // v
                    || (element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y < element_cur.end.y)) {  // ^
                // Update element_next.
                auto p = compute_line_intersection(
                        element_cur.start, {element_cur.start.x, element_cur.end.y},
                        element_next.start, element_next.end);
                if (p.first) {
                    triangles_added.push_back(build_triangle(
                                element_cur.start, element_cur.end, element_next.end));
                    element_cur.end = p.second;
                    element_next.start = p.second;
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if ((element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y < element_cur.end.y)  // ^
                    || (element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y > element_cur.end.y)) {  // v
                // Update element_next.
                auto p = compute_line_intersection(
                        element_cur.start, {element_cur.end.x, element_cur.start.y},
                        element_next.start, element_next.end);
                if (p.first) {
                    triangles_added.push_back(build_triangle(
                                element_cur.start, element_cur.end, element_next.end));
                    element_cur.end = p.second;
                    element_next.start = p.second;
                }
            }
        }

        element_next_pos = element_cur_pos;
    }

    return shape;
}

Shape clean_extreme_slopes_inner_2(
        const Shape& shape_orig,
        std::vector<Shape>& triangles_removed)
{
    Shape shape = shape_orig;
    ElementPos element_prev_pos = shape.elements.size() - 1;
    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)shape.elements.size();
            ++element_cur_pos) {
        ShapeElement& element_prev = shape.elements[element_prev_pos];
        ShapeElement& element_cur = shape.elements[element_cur_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        //std::cout << "element " << element.to_string() << " slope " << slope << std::endl;
        //std::cout << "element_prev " << element_prev.to_string() << std::endl;
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_prev.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if ((element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y < element_cur.end.y)  // ^
                    || (element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y > element_cur.end.y)) {  // v
                // Update element_prev.
                auto p = compute_line_intersection(
                        element_prev.start, element_prev.end,
                        {element_cur.end.x, element_cur.start.y}, element_cur.end);
                if (p.first) {
                    triangles_removed.push_back(build_triangle(
                                element_prev.start, element_cur.start, element_cur.end));
                    element_prev.end = p.second;
                    element_cur.start = p.second;
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if ((element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y > element_cur.end.y)  // v
                    || (element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y < element_cur.end.y)) {  // ^
                // Update element_prev.
                auto p = compute_line_intersection(
                        element_prev.start, element_prev.end,
                        {element_cur.start.x, element_cur.end.y}, element_cur.end);
                if (p.first) {
                    triangles_removed.push_back(build_triangle(
                                element_prev.start, element_cur.start, element_cur.end));
                    element_prev.end = p.second;
                    element_cur.start = p.second;
                }
            }
        }

        element_prev_pos = element_cur_pos;
    }
    ElementPos element_next_pos = 0;
    for (ElementPos element_cur_pos = shape.elements.size() - 1;
            element_cur_pos >= 0;
            --element_cur_pos) {
        ShapeElement& element_cur = shape.elements[element_cur_pos];
        ShapeElement& element_next = shape.elements[element_next_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        //std::cout << "element " << element.to_string() << " slope " << slope << std::endl;
        //std::cout << "element_prev " << element_prev.to_string() << std::endl;
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_next.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if ((element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y > element_cur.end.y)  // v
                    || (element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y < element_cur.end.y)) {  // ^
                // Update element_next.
                auto p = compute_line_intersection(
                        element_cur.start, {element_cur.start.x, element_cur.end.y},
                        element_next.start, element_next.end);
                if (p.first) {
                    triangles_removed.push_back(build_triangle(
                                element_cur.start, element_cur.end, element_next.end));
                    element_cur.end = p.second;
                    element_next.start = p.second;
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if ((element_cur.start.x > element_cur.end.x  // <
                        && element_cur.start.y < element_cur.end.y)  // ^
                    || (element_cur.start.x < element_cur.end.x  // >
                        && element_cur.start.y > element_cur.end.y)) {  // v
                // Update element_next.
                auto p = compute_line_intersection(
                        element_cur.start, {element_cur.end.x, element_cur.start.y},
                        element_next.start, element_next.end);
                if (p.first) {
                    triangles_removed.push_back(build_triangle(
                                element_cur.start, element_cur.end, element_next.end));
                    element_cur.end = p.second;
                    element_next.start = p.second;
                }
            }
        }

        element_next_pos = element_cur_pos;
    }

    return shape;
}

}

ShapeWithHoles shape::clean_extreme_slopes_outer(
        const Shape& shape_orig)
{
    if (!shape_orig.check()) {
        //write_json({{shape_orig}}, {}, "clean_extreme_slopes_outer_input.json");
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": invalid input shape.");
    }

    Shape shape = shape_orig;
    std::vector<Shape> triangles_added;

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_outer_1(shape, triangles_added);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_outer_2(shape, triangles_added);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    if (!triangles_added.empty() && intersect(shape)) {
        // The naive in-place mutations performed by clean_extreme_slopes_outer_1
        // and clean_extreme_slopes_outer_2 can produce a self-intersecting
        // shape. When that happens, fall back to a robust reconstruction:
        // union the corner triangles identified by those two passes with the
        // original shape, the same approach used by simplify() to recover
        // from invalid in-place simplifications.
        std::vector<ShapeWithHoles> shapes_to_union = {{shape_orig}};
        for (const Shape& triangle: triangles_added)
            shapes_to_union.push_back({triangle});
        std::vector<ShapeWithHoles> u = compute_union(shapes_to_union).shapes_with_holes;
        return u.front();
    }

    if (!shape.check()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "invalid shape after extreme slopes cleaning.");
    }

    return {shape};
}

std::vector<Shape> shape::clean_extreme_slopes_inner(
        const Shape& shape_orig)
{
    if (!shape_orig.check()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": invalid input shape.");
    }

    Shape shape = shape_orig;
    std::vector<Shape> triangles_removed;

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_inner_1(shape, triangles_removed);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_inner_2(shape, triangles_removed);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    if (!triangles_removed.empty() && intersect(shape)) {
        // The naive in-place mutations performed by clean_extreme_slopes_inner_1
        // and clean_extreme_slopes_inner_2 can produce a self-intersecting
        // shape. When that happens, fall back to a robust reconstruction: cut
        // the corner triangles identified by those two passes directly out of
        // the original shape using a boolean difference, the same approach
        // used by simplify() to recover from invalid in-place simplifications.
        std::vector<ShapeWithHoles> triangles_removed_swh;
        for (const Shape& triangle: triangles_removed)
            triangles_removed_swh.push_back({triangle});
        std::vector<ShapeWithHoles> difference = compute_difference(
                MultiShapeWithHoles{{{shape_orig}}},
                MultiShapeWithHoles{triangles_removed_swh}).shapes_with_holes;

        std::vector<Shape> output;
        for (const ShapeWithHoles& shape_with_holes: difference)
            output.push_back(shape_with_holes.shape);
        return output;
    }

    if (!shape.check()) {
#ifdef CLEAN_ENABLE_DEBUG
        Writer().add_shape(shape_orig).add_shape(shape).write_json("tmp.json");
        std::ofstream file{"clean_extreme_slopes_inner_inputs.json"};
        nlohmann::json json;
        json["shape"] = shape_orig.to_json();
        file << std::setw(4) << json << std::endl;
#endif
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "invalid shape after extreme slopes cleaning.");
    }

    return {shape};
}

MultiShapeWithHoles shape::fix_self_intersections(
        const ShapeWithHoles& shape)
{
    //std::cout << "fix_self_intersections" << std::endl;
    //Writer().add_shape_with_holes(shape).write_json("fix_self_intersections_input.json");
    std::vector<ShapeWithHoles> shapes = bridge_touching_holes(shape).shapes_with_holes;
    if (shapes.size() == 1)
        return {{shape}};
    MultiShapeWithHoles output;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        MultiShapeWithHoles u = compute_union({shapes[shape_pos]});
        if (!u.shapes_with_holes.empty())
            output.shapes_with_holes.push_back(u.shapes_with_holes.front());
    }
    return output;
}
