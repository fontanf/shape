#include "shape/clean.hpp"

#include "shape/element_intersections.hpp"
#include "shape/equalize.hpp"

#include <iostream>

using namespace shape;

std::pair<bool, Shape> shape::remove_redundant_vertices(
        const Shape& shape)
{
    //std::cout << "remove_redundant_vertices " << shape.to_string(2) << std::endl;
    if (!shape.check()) {
        throw std::invalid_argument(
                "shape::remove_redundant_vertices: invalid input shape.");
    }

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
    shape_new.elements.back().end = shape_new.elements.front().start;

    if (!shape_new.check()) {
        throw std::invalid_argument(
                "shape::remove_redundant_vertices: invalid output shape.");
    }
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

ShapeWithHoles shape::clean_extreme_slopes_outer(
        const Shape& shape_orig)
{
    //std::cout << "clean_extreme_slopes_outer shape " << shape_orig.to_string(0) << std::endl;
    Shape shape = shape_orig;
    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;
    ElementPos element_prev_pos = shape.elements.size() - 2;
    ElementPos element_cur_pos = shape.elements.size() - 1;
    for (ElementPos element_next_pos = 0;
            element_next_pos < (ElementPos)shape.elements.size();
            ++element_next_pos) {
        ShapeElement& element_prev = shape.elements[element_prev_pos];
        ShapeElement& element_cur = shape.elements[element_cur_pos];
        ShapeElement& element_next = shape.elements[element_next_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        //std::cout << "element_prev " << element_prev.to_string() << std::endl;
        //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
        //std::cout << "element_next " << element_next.to_string() << std::endl;
        //std::cout << "slope " << slope << std::endl;
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if (element_cur.start.x < element_cur.end.x) {
                if (element_cur.start.y < element_cur.end.y) {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.end.x, element_cur.start.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_prev " << element_prev.to_string() << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                        }
                    }
                } else {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.start.x, element_cur.end.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                            //std::cout << "element_next " << element_next.to_string() << std::endl;
                        }
                    }
                }
            } else {
                if (element_cur.start.y < element_cur.end.y) {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.start.x, element_cur.end.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                            //std::cout << "element_next " << element_next.to_string() << std::endl;
                        }
                    }
                } else {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.end.x, element_cur.start.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_prev " << element_prev.to_string() << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                        }
                    }
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if (element_cur.start.y < element_cur.end.y) {
                if (element_cur.start.x < element_cur.end.x) {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.end.x, element_cur.start.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                            //std::cout << "element_next " << element_next.to_string() << std::endl;
                        }
                    }
                } else {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.start.x, element_cur.end.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_prev " << element_prev.to_string() << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                        }
                    }
                }
            } else {
                if (element_cur.start.x < element_cur.end.x) {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.start.x, element_cur.end.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_prev " << element_prev.to_string() << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                        }
                    }
                } else {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.end.x, element_cur.start.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                            //std::cout << "update" << std::endl;
                            //std::cout << "element_cur  " << element_cur.to_string() << std::endl;
                            //std::cout << "element_next " << element_next.to_string() << std::endl;
                        }
                    }
                }
            }
        }

        element_prev_pos = element_cur_pos;
        element_cur_pos = element_next_pos;
    }
    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;
    //write_json({{shape_orig}, {shape}}, {}, "clean_extreme_slopes_outer_output.json");
    //std::cout << "clean_extreme_slopes_outer end shape " << shape.to_string(0) << std::endl;
    return {shape};
}

std::vector<Shape> shape::clean_extreme_slopes_inner(
        const Shape& shape_orig)
{
    Shape shape = shape_orig;
    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;
    ElementPos element_prev_pos = shape.elements.size() - 2;
    ElementPos element_cur_pos = shape.elements.size() - 1;
    for (ElementPos element_next_pos = 0;
            element_next_pos < (ElementPos)shape.elements.size();
            ++element_next_pos) {
        ShapeElement& element_prev = shape.elements[element_prev_pos];
        ShapeElement& element_cur = shape.elements[element_cur_pos];
        ShapeElement& element_next = shape.elements[element_next_pos];

        double slope
            = (element_cur.end.y - element_cur.start.y)
            / (element_cur.end.x - element_cur.start.x);
        //std::cout << "element " << element.to_string() << " slope " << slope << std::endl;
        //std::cout << "element_prev " << element_prev.to_string() << std::endl;
        if (element_cur.type != ShapeElementType::LineSegment) {
        } else if (element_cur.start.x != element_cur.end.x && std::abs(slope) > 1e2) {
            if (element_cur.start.x < element_cur.end.x) {
                if (element_cur.start.y < element_cur.end.y) {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.start.x, element_cur.end.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                        }
                    }
                } else {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.end.x, element_cur.start.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                        }
                    }
                }
            } else {
                if (element_cur.start.y < element_cur.end.y) {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.end.x, element_cur.start.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                        }
                    }
                } else {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.start.x, element_cur.end.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                        }
                    }
                }
            }
        } else if (element_cur.start.y != element_cur.end.y && std::abs(slope) < 1e-2) {
            if (element_cur.start.y < element_cur.end.y) {
                if (element_cur.start.x < element_cur.end.x) {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.start.x, element_cur.end.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                        }
                    }
                } else {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.end.x, element_cur.start.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                        }
                    }
                }
            } else {
                if (element_cur.start.x < element_cur.end.x) {
                    // Update element_next.
                    if (element_next.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_cur.start, {element_cur.end.x, element_cur.start.y},
                                element_next.start, element_next.end);
                        if (p.first) {
                            element_cur.end = p.second;
                            element_next.start = p.second;
                        }
                    }
                } else {
                    // Update element_prev.
                    if (element_prev.type == ShapeElementType::LineSegment) {
                        auto p = compute_line_intersection(
                                element_prev.start, element_prev.end,
                                {element_cur.start.x, element_cur.end.y}, element_cur.end);
                        if (p.first) {
                            element_prev.end = p.second;
                            element_cur.start = p.second;
                        }
                    }
                }
            }
        }

        element_prev_pos = element_cur_pos;
        element_cur_pos = element_next_pos;
    }
    shape = equalize_shape(shape);
    shape = remove_redundant_vertices(shape).second;
    shape = remove_aligned_vertices(shape).second;
    return {shape};
}
