#include "shape/clean.hpp"

#include "shape/elements_intersections.hpp"
#include "shape/equalize.hpp"
#include "shape/boolean_operations.hpp"
//#include "shape/writer.hpp"

//#include <iostream>

using namespace shape;

std::pair<bool, Shape> shape::remove_redundant_vertices(
        const Shape& shape)
{
    //std::cout << "remove_redundant_vertices " << shape.to_string(2) << std::endl;

    if (shape.elements.size() <= 3)
        return {false, shape};

    ElementPos number_of_elements_removed = 0;
    Shape shape_new;
    shape_new.is_path = shape.is_path;

    ElementPos element_prev_pos = shape.elements.size() - 1;
    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)shape.elements.size();
            ++element_cur_pos) {

        //std::cout << "element_cur_pos " << element_cur_pos << std::endl;
        ElementPos element_next_pos = element_cur_pos + 1;
        const ShapeElement& element_prev = shape.elements[element_prev_pos];
        const ShapeElement& element = shape.elements[element_cur_pos];
        const ShapeElement& element_next = (element_next_pos < shape.elements.size())?
            shape.elements[element_next_pos]:
            shape_new.elements.front();
        bool useless = false;
        if (equal(element.start.x, element.end.x)
                && equal(element.start.y, element.end.y)) {
            useless = true;
        }
        if (!useless || shape.elements.size() - number_of_elements_removed <= 3) {
            if (!shape_new.elements.empty())
                shape_new.elements.back().end = element.start;
            shape_new.elements.push_back(element);
            element_prev_pos = element_cur_pos;
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
    bool b;

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

    ElementPos number_of_elements_removed = 0;
    Shape shape_new;
    shape_new.is_path = shape.is_path;

    ElementPos element_prev_pos = shape.elements.size() - 1;
    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)shape.elements.size();
            ++element_cur_pos) {

        //std::cout << "element_cur_pos " << element_cur_pos << std::endl;
        ElementPos element_next_pos = element_cur_pos + 1;
        const ShapeElement& element_prev = shape.elements[element_prev_pos];
        const ShapeElement& element_cur = shape.elements[element_cur_pos];
        bool useless = false;
        if (element_cur.type == ShapeElementType::LineSegment
                && element_prev.type == ShapeElementType::LineSegment) {
            //std::cout << "element_prev  " << element_prev_pos << " " << element_prev.to_string() << std::endl;
            //std::cout << "element_cur   " << element_cur_pos << " " << element_cur.to_string() << std::endl;
            if (element_prev.start.x == element_cur.start.x
                    && element_cur.start.x == element_cur.end.x) {
                //std::cout << "useless " << element_cur.to_string() << std::endl;
                useless = true;
            } else if (element_prev.start.y == element_cur.start.y
                    && element_cur.start.y == element_cur.end.y) {
                //std::cout << "useless " << element_cur.to_string() << std::endl;
                useless = true;
            } else {
                Point normal;
                normal.x = element_cur.end.y - element_prev.start.y;
                normal.y = element_prev.start.x - element_cur.end.x;
                Point p = element_cur.start + normal;
                auto intersection = compute_line_intersection(
                        element_prev.start,
                        element_cur.end,
                        element_cur.start,
                        p);
                if (intersection.first
                        && equal(element_cur.start, intersection.second)) {
                    useless = true;
                }
            }
        }
        if (!useless || shape.elements.size() - number_of_elements_removed <= 3) {
            if (!shape_new.elements.empty())
                shape_new.elements.back().end = element_cur.start;
            shape_new.elements.push_back(element_cur);
            element_prev_pos = element_cur_pos;
        } else {
            number_of_elements_removed++;
        }
    }
    if (!shape.is_path)
        shape_new.elements.back().end = shape_new.elements.front().start;

    return {(number_of_elements_removed > 0), shape_new};
}

std::pair<bool, ShapeWithHoles> shape::remove_aligned_vertices(
        const ShapeWithHoles& shape)
{
    ShapeWithHoles res;
    bool b;

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

    Shape output;
    output.is_path = shape.is_path;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        const ShapeElement& element_next = (element_pos + 1 != shape.elements.size())?
            shape.elements[element_pos + 1]:
            shape.elements[0];
        if (equal(element, element_next.reverse())) {
            element_pos++;
            continue;
        }
        output.elements.push_back(element);
    }

    return output;
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
        Shape shape_orig)
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
                //std::cout << "remove" << std::endl;
                //std::cout << "element_cur:  " << element_cur.to_string() << std::endl;
                //std::cout << "element_next: " << element_next.to_string() << std::endl;
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
                //std::cout << "remove" << std::endl;
                //std::cout << "element_cur:  " << element_cur.to_string() << std::endl;
                //std::cout << "element_prev: " << element_prev.to_string() << std::endl;
            }
        }
        shape_new.elements[0].end = end;
        std::reverse(shape_new.elements.begin(), shape_new.elements.end());
        shape = shape_new;
    }

    return shape;
}

Shape clean_extreme_slopes_inner_1(
        const Shape& shape_orig)
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
            }
        }
        shape_new.elements[0].end = end;
        std::reverse(shape_new.elements.begin(), shape_new.elements.end());
        shape = shape_new;
    }

    return shape;
}

Shape clean_extreme_slopes_outer_2(
        const Shape& shape_orig)
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
        const Shape& shape_orig)
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

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_outer_1(shape);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_outer_2(shape);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

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
    Shape shape = shape_orig;

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_inner_1(shape);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    shape = clean_extreme_slopes_inner_2(shape);

    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;

    if (!shape.check()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "invalid shape after extreme slopes cleaning.");
    }

    return {shape};
}

std::vector<ShapeWithHoles> shape::fix_self_intersections(
        const ShapeWithHoles& shape)
{
    //std::cout << "fix_self_intersections" << std::endl;
    //Writer().add_shape_with_holes(shape).write_json("fix_self_intersections_input.json");
    std::vector<ShapeWithHoles> shapes = bridge_touching_holes(shape);
    if (shapes.size() == 1)
        return {shape};
    std::vector<ShapeWithHoles> output(shapes.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        output[shape_pos] = compute_union({shapes[shape_pos]}).front();
    }
    return output;
}
