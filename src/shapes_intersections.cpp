#include "shape/shapes_intersections.hpp"

#include <iostream>

using namespace shape;

bool shape::intersect(
        const ShapeElement& element_1,
        const ShapeElement& element_2,
        bool strict)
{
    ShapeElementIntersectionsOutput intersections = compute_intersections(
            element_1,
            element_2);
    if (!intersections.proper_intersections.empty())
        return true;
    if (!strict) {
        if (!intersections.improper_intersections.empty())
            return true;
        if (!intersections.overlapping_parts.empty())
            return true;
    }
    return false;
}

bool shape::intersect(
        const Shape& shape)
{
    for (ElementPos element_1_pos = 0;
            element_1_pos < (ElementPos)shape.elements.size();
            ++element_1_pos) {
        const ShapeElement& element_1 = shape.elements[element_1_pos];
        for (ElementPos element_2_pos = element_1_pos + 2;
                element_2_pos < (ElementPos)shape.elements.size();
                ++element_2_pos) {
            if (!shape.is_path
                    && element_1_pos == 0
                    && element_2_pos == shape.elements.size() - 1) {
                continue;
            }
            const ShapeElement& element_2 = shape.elements[element_2_pos];
            if (intersect(element_1, element_2, false)) {
                std::cout << element_1.to_string() << std::endl;
                std::cout << element_2.to_string() << std::endl;
                return true;
            }
        }
    }
    return false;
}

namespace
{

bool strictly_intersect(
        const Shape& shape_1,
        bool is_shape_1_hole,
        ElementPos shape_1_element_pos,
        const Shape& shape_2,
        bool is_shape_2_hole,
        ElementPos shape_2_element_pos)
{
    if (shape_1.is_path && shape_2.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "intersections involving two paths must not be strict.");
    }
    const ShapeElement& shape_1_element = shape_1.elements[shape_1_element_pos];
    const ShapeElement& shape_2_element = shape_2.elements[shape_2_element_pos];
    if (equal(shape_1_element.length(), 0))
        return false;
    if (equal(shape_2_element.length(), 0))
        return false;
    //std::cout << "shape_1_element " << shape_1_element.to_string() << std::endl;
    //std::cout << "shape_2_element " << shape_2_element.to_string() << std::endl;
    ShapeElementIntersectionsOutput intersections = compute_intersections(
            shape_1_element,
            shape_2_element);

    // If contains a strict intersection.
    if (!intersections.proper_intersections.empty())
        return true;

    // If elements overlap and have the same direction.
    if (!shape_1.is_path && !shape_2.is_path) {
        for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
            //std::cout << "overlapping_part " << overlapping_part.to_string() << std::endl;
            bool b1 = shape_1_element.same_direction(overlapping_part);
            bool b2 = shape_2_element.same_direction(overlapping_part);
            if (is_shape_1_hole)
                b1 = !b1;
            if (is_shape_2_hole)
                b2 = !b2;
            //std::cout << "b1 " << b1 << " b2 " << b2 << std::endl;
            if (b1 == b2)
                return true;
        }
    }

    bool shape_1_element_end_intersect = false;
    bool shape_2_element_end_intersect = false;
    for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
        if (equal(overlapping_part.end, shape_1_element.end))
            if (!equal(overlapping_part.end, shape_2_element.start))
                shape_1_element_end_intersect = true;
        if (equal(overlapping_part.end, shape_2_element.end))
            if (!equal(overlapping_part.end, shape_1_element.start))
                shape_2_element_end_intersect = true;
    }
    for (const Point& point: intersections.improper_intersections) {
        if (equal(point, shape_1_element.end))
            if (!equal(point, shape_2_element.start))
                shape_1_element_end_intersect = true;
        if (equal(point, shape_2_element.end))
            if (!equal(point, shape_1_element.start))
                shape_2_element_end_intersect = true;
    }
    if (shape_1.is_path && shape_1_element_pos == shape_1.elements.size())
        shape_1_element_end_intersect = false;
    if (shape_2.is_path && shape_2_element_pos == shape_2.elements.size())
        shape_2_element_end_intersect = false;
    //std::cout << "shape_1_element_end_intersect " << shape_1_element_end_intersect << std::endl;
    //std::cout << "shape_2_element_end_intersect " << shape_2_element_end_intersect << std::endl;
    if (shape_1_element_end_intersect) {
        const ShapeElement& shape_1_element_next = shape_1.elements[(shape_1_element_pos + 1) % shape_1.elements.size()];
        //std::cout << "shape_1_element_next " << shape_1_element_next.to_string() << std::endl;
        if (shape_2_element_end_intersect) {
            const ShapeElement& shape_2_element_next = shape_2.elements[(shape_2_element_pos + 1) % shape_2.elements.size()];
            //std::cout << "shape_2_element_next " << shape_2_element_next.to_string() << std::endl;
            Jet shape_1_element_jet = shape_1_element.jet(shape_1_element.end, true);
            Jet shape_2_element_jet = shape_2_element.jet(shape_2_element.end, true);
            Jet shape_1_element_next_jet = shape_1_element_next.jet(shape_1_element_next.start, false);
            Jet shape_2_element_next_jet = shape_2_element_next.jet(shape_2_element_next.start, false);
            //std::cout << "shape_1_element_jet " << shape_1_element_jet.to_string() << std::endl;
            //std::cout << "shape_1_element_next_jet " << shape_1_element_next_jet.to_string() << std::endl;
            //std::cout << "shape_2_element_jet " << shape_2_element_jet.to_string() << std::endl;
            //std::cout << "shape_2_element_next_jet " << shape_2_element_next_jet.to_string() << std::endl;
            if (!shape_1.is_path) {
                if (is_shape_1_hole)
                    std::swap(shape_1_element_jet, shape_1_element_next_jet);
                Jet jet_shape_1 = shape_1_element_next_jet - shape_1_element_jet;
                Jet jet_shape_2_1 = shape_2_element_jet - shape_1_element_jet;
                Jet jet_shape_2_2 = shape_2_element_next_jet - shape_1_element_jet;
                //std::cout << "jet_shape_1 " << jet_shape_1.to_string() << std::endl;
                //std::cout << "jet_shape_2_1 " << jet_shape_2_1.to_string() << std::endl;
                //std::cout << "jet_shape_2_2 " << jet_shape_2_2.to_string() << std::endl;
                if (strictly_greater(jet_shape_2_1, jet_shape_1))
                        return true;
                if (strictly_greater(jet_shape_2_2, jet_shape_1))
                        return true;
            }
            if (!shape_2.is_path) {
                if (is_shape_2_hole)
                    std::swap(shape_2_element_jet, shape_2_element_next_jet);
                Jet jet_shape_2 = shape_2_element_next_jet - shape_2_element_jet;
                Jet jet_shape_1_1 = shape_1_element_jet - shape_2_element_jet;
                Jet jet_shape_1_2 = shape_1_element_next_jet - shape_2_element_jet;
                //std::cout << "jet_shape_2 " << jet_shape_2.to_string() << std::endl;
                //std::cout << "jet_shape_1_1 " << jet_shape_1_1.to_string() << std::endl;
                //std::cout << "jet_shape_1_2 " << jet_shape_1_2.to_string() << std::endl;
                if (strictly_greater(jet_shape_1_1, jet_shape_2))
                        return true;
                if (strictly_greater(jet_shape_1_2, jet_shape_2))
                        return true;
            }
        } else {
            auto p = shape_2_element.split(shape_1_element.end);
            Jet shape_1_element_jet = shape_1_element.jet(shape_1_element.end, true);
            Jet shape_2_element_jet = shape_2_element.jet(shape_1_element.end, true);
            Jet shape_1_element_next_jet = shape_1_element_next.jet(shape_1_element_next.start, false);
            Jet shape_2_element_next_jet = shape_2_element.jet(shape_1_element.end, false);
            //std::cout << "shape_1_element_jet " << shape_1_element_jet.to_string() << std::endl;
            //std::cout << "shape_1_element_next_jet " << shape_1_element_next_jet.to_string() << std::endl;
            //std::cout << "shape_2_element_jet " << shape_2_element_jet.to_string() << std::endl;
            //std::cout << "shape_2_element_next_jet " << shape_2_element_next_jet.to_string() << std::endl;
            if (!shape_1.is_path) {
                if (is_shape_1_hole)
                    std::swap(shape_1_element_jet, shape_1_element_next_jet);
                Jet jet_shape_1 = shape_1_element_next_jet - shape_1_element_jet;
                Jet jet_shape_2_1 = shape_2_element_jet - shape_1_element_jet;
                Jet jet_shape_2_2 = shape_2_element_next_jet - shape_1_element_jet;
                //std::cout << "jet_shape_1 " << jet_shape_1.to_string() << std::endl;
                //std::cout << "jet_shape_2_1 " << jet_shape_2_1.to_string() << std::endl;
                //std::cout << "jet_shape_2_2 " << jet_shape_2_2.to_string() << std::endl;
                if (strictly_greater(jet_shape_2_1, jet_shape_1))
                        return true;
                if (strictly_greater(jet_shape_2_2, jet_shape_1))
                        return true;
            }
            if (!shape_2.is_path) {
                if (is_shape_2_hole)
                    std::swap(shape_2_element_jet, shape_2_element_next_jet);
                Jet jet_shape_2 = shape_2_element_next_jet - shape_2_element_jet;
                Jet jet_shape_1_1 = shape_1_element_jet - shape_2_element_jet;
                Jet jet_shape_1_2 = shape_1_element_next_jet - shape_2_element_jet;
                //std::cout << "jet_shape_2 " << jet_shape_2.to_string() << std::endl;
                //std::cout << "jet_shape_1_1 " << jet_shape_1_1.to_string() << std::endl;
                //std::cout << "jet_shape_1_2 " << jet_shape_1_2.to_string() << std::endl;
                if (strictly_greater(jet_shape_1_1, jet_shape_2))
                        return true;
                if (strictly_greater(jet_shape_1_2, jet_shape_2))
                        return true;
            }
        }
    } else if (shape_2_element_end_intersect) {
        const ShapeElement& shape_2_element_next = shape_2.elements[(shape_2_element_pos + 1) % shape_2.elements.size()];
        //std::cout << "shape_2_element_next " << shape_2_element_next.to_string() << std::endl;
        auto p = shape_1_element.split(shape_2_element.end);
        Jet shape_1_element_jet = shape_1_element.jet(shape_2_element.end, true);
        Jet shape_2_element_jet = shape_2_element.jet(shape_2_element.end, true);
        Jet shape_1_element_next_jet = shape_1_element.jet(shape_2_element.end, false);
        Jet shape_2_element_next_jet = shape_2_element_next.jet(shape_2_element_next.start, false);
        //std::cout << "shape_1_element_jet " << shape_1_element_jet.to_string() << std::endl;
        //std::cout << "shape_1_element_next_jet " << shape_1_element_next_jet.to_string() << std::endl;
        //std::cout << "shape_2_element_jet " << shape_2_element_jet.to_string() << std::endl;
        //std::cout << "shape_2_element_next_jet " << shape_2_element_next_jet.to_string() << std::endl;
        if (!shape_1.is_path) {
            if (is_shape_1_hole)
                std::swap(shape_1_element_jet, shape_1_element_next_jet);
            Jet jet_shape_1 = shape_1_element_next_jet - shape_1_element_jet;
            Jet jet_shape_2_1 = shape_2_element_jet - shape_1_element_jet;
            Jet jet_shape_2_2 = shape_2_element_next_jet - shape_1_element_jet;
            //std::cout << "jet_shape_1 " << jet_shape_1.to_string() << std::endl;
            //std::cout << "jet_shape_2_1 " << jet_shape_2_1.to_string() << std::endl;
            //std::cout << "jet_shape_2_2 " << jet_shape_2_2.to_string() << std::endl;
            if (strictly_greater(jet_shape_2_1, jet_shape_1))
                return true;
            if (strictly_greater(jet_shape_2_2, jet_shape_1))
                return true;
        }
        if (!shape_2.is_path) {
            if (is_shape_2_hole)
                std::swap(shape_2_element_jet, shape_2_element_next_jet);
            Jet jet_shape_2 = shape_2_element_next_jet - shape_2_element_jet;
            Jet jet_shape_1_1 = shape_1_element_jet - shape_2_element_jet;
            Jet jet_shape_1_2 = shape_1_element_next_jet - shape_2_element_jet;
            //std::cout << "jet_shape_2 " << jet_shape_2.to_string() << std::endl;
            //std::cout << "jet_shape_1_1 " << jet_shape_1_1.to_string() << std::endl;
            //std::cout << "jet_shape_1_2 " << jet_shape_1_2.to_string() << std::endl;
            if (strictly_greater(jet_shape_1_1, jet_shape_2))
                return true;
            if (strictly_greater(jet_shape_1_2, jet_shape_2))
                return true;
        }
    }
    return false;
}

/**
 * Find a point on a given element which is not on the outline of a given shape.
 */
std::pair<bool, Point> find_point_in_difference(
        const ShapeElement& element,
        const Shape& shape)
{
    Point current_point = element.start;
    for (;;) {
        LengthDbl current_length = element.length(current_point);
        bool is_in_overlap = false;
        Point overlap_end = current_point;
        bool is_improper_intersection = false;
        Point next_intersection = element.end;
        LengthDbl best_length = element.length();
        for (const ShapeElement& shape_element: shape.elements) {
            ShapeElementIntersectionsOutput intersections = compute_intersections(element, shape_element);
            for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
                if (equal(current_point, overlapping_part.start)) {
                    overlap_end = overlapping_part.end;
                    is_in_overlap = true;
                    break;
                }
            }
            if (is_in_overlap)
                break;
            for (const Point& intersection: intersections.improper_intersections) {
                LengthDbl length = element.length(intersection);
                if (strictly_greater(
                            length,
                            current_length)
                        && strictly_lesser(
                            length,
                            best_length)) {
                    next_intersection = intersection;
                    best_length = length;
                }
                if (equal(current_point, intersection)) {
                    is_improper_intersection = true;
                }
            }
        }
        if (is_in_overlap) {
            current_point = overlap_end;
        } else if (is_improper_intersection) {
            current_point = element.middle(current_point, next_intersection);
        } else {
            return {true, current_point};
        }
        if (equal(current_point, element.end))
            return{false, {0, 0}};
    }

    return {false, {0, 0}};
}

std::pair<bool, ShapePoint> find_point_in_difference(
        const Shape& path,
        const Shape& shape)
{
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)path.elements.size();
            ++element_pos) {
        const ShapeElement& element = path.elements[element_pos];
        auto result = find_point_in_difference(element, shape);
        if (result.first)
            return {true, {element_pos, result.second}};
    }
    return {false, {-1, {0, 0}}};
}

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

    // Check for interesctions between shapes elements.
    if (strict) {
        for (ElementPos element_1_pos = 0;
                element_1_pos < (ElementPos)shape_1.elements.size();
                ++element_1_pos) {
            for (ElementPos element_2_pos = 0;
                    element_2_pos < (ElementPos)shape_2.elements.size();
                    ++element_2_pos) {
                if (::strictly_intersect(
                            shape_1, false, element_1_pos,
                            shape_2, false, element_2_pos)) {
                    return true;
                }
            }
        }
    } else {
        for (const ShapeElement& element_1: shape_1.elements)
            for (const ShapeElement& element_2: shape_2.elements)
                if (shape::intersect(element_1, element_2))
                    return true;
    }

    // Check if shape_1 is in shape_2.
    if (!shape_2.is_path) {
        auto result = find_point_in_difference(shape_1, shape_2);
        if (!result.first) {
            // shape_1 outline is fully in the outline of shape_2.
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_2.contains(result.second.point, strict))
                return true;
        }
    }

    // Check if shape_2 is in shape_1.
    if (!shape_1.is_path) {
        auto result = find_point_in_difference(shape_2, shape_1);
        if (!result.first) {
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_1.contains(result.second.point, strict))
                return true;
        }
    }

    return false;
}

std::vector<ShapeShapeElementIntersection> shape::compute_intersections(
        const Shape& shape,
        const ShapeElement& element)
{
    std::vector<ShapeShapeElementIntersection> intersections;
    for (ElementPos shape_element_pos = 0;
            shape_element_pos < (ElementPos)shape.elements.size();
            ++shape_element_pos) {
        const ShapeElement& shape_element = shape.elements[shape_element_pos];
        ShapeElementIntersectionsOutput intersections_cur = compute_intersections(
                element,
                shape_element);
        if (!intersections_cur.overlapping_parts.empty()
                || !intersections_cur.improper_intersections.empty()
                || !intersections_cur.proper_intersections.empty()) {
            ShapeShapeElementIntersection intersection;
            intersection.element_pos = shape_element_pos;
            intersection.intersections = intersections_cur;
            intersections.push_back(intersection);
        }
    }
    return intersections;
}

ComputeClosestToShapeElementStartIntersectionOutput shape::compute_closest_to_start_intersection(
        const ShapeElement& element,
        const Shape& shape,
        bool proper)
{
    ComputeClosestToShapeElementStartIntersectionOutput output;
    output.intersect = false;
    std::vector<ShapeShapeElementIntersection> intersections = compute_intersections(shape, element);
    for (const ShapeShapeElementIntersection& intersection: intersections) {
        for (const Point& point: intersection.intersections.proper_intersections) {
            LengthDbl distance = element.length(point);
            if (output.distance > distance) {
                output.intersect = true;
                output.intersection = point;
                output.element_pos = intersection.element_pos;
                output.distance = distance;
            }
        }
        if (!proper) {
            for (const Point& point: intersection.intersections.improper_intersections) {
                LengthDbl distance = element.length(point);
                if (output.distance > distance) {
                    output.intersect = true;
                    output.intersection = point;
                    output.element_pos = intersection.element_pos;
                    output.distance = distance;
                }
            }
            for (const ShapeElement& overlapping_part: intersection.intersections.overlapping_parts) {
                for (const Point& point: {overlapping_part.start, overlapping_part.end}) {
                    LengthDbl distance = element.length(point);
                    if (output.distance > distance) {
                        output.intersect = true;
                        output.intersection = point;
                        output.element_pos = intersection.element_pos;
                        output.distance = distance;
                    }
                }
            }
        }
    }
    return output;
}

ComputeClosestToPathStartIntersectionOutput shape::compute_closest_to_start_intersection(
        const Shape& path,
        const Shape& shape,
        bool strict)
{
    for (ElementPos element_1_pos = 0;
            element_1_pos < (ElementPos)path.elements.size();
            ++element_1_pos) {
        const ShapeElement& element_1 = path.elements[element_1_pos];
        ComputeClosestToShapeElementStartIntersectionOutput output_cur
            = compute_closest_to_start_intersection(element_1, shape, strict);
        if (output_cur.intersect) {
            ComputeClosestToPathStartIntersectionOutput output;
            output.element_1_pos = element_1_pos;
            output.element_2_pos = output_cur.element_pos;
            output.intersection = output_cur.intersection;
            return output;
        }
    }
    ComputeClosestToPathStartIntersectionOutput output;
    output.intersect = false;
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
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        ShapeElement shape_element = shape.elements[element_pos];
        ShapeElementIntersectionsOutput intersections = compute_intersections(
                element,
                shape_element);
        if (!intersections.proper_intersections.empty())
            return true;
        if (!strict) {
            if (!intersections.improper_intersections.empty())
                return true;
            if (!intersections.overlapping_parts.empty())
                return true;
        }
    }

    // Check if the element is in the shape.
    if (!shape.is_path) {
        auto result = find_point_in_difference(element, shape);
        if (!result.first) {
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape.contains(result.second, strict))
                return true;
        }
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const ShapeWithHoles& shape_with_holes_2,
        bool strict)
{
    for (ShapePos shape_1_pos = -1;
            shape_1_pos < (ShapePos)shape_with_holes_1.holes.size();
            ++shape_1_pos) {
        const Shape& shape_1 = (shape_1_pos == -1)?
            shape_with_holes_1.shape:
            shape_with_holes_1.holes[shape_1_pos];
        for (ElementPos element_1_pos = 0;
                element_1_pos < (ElementPos)shape_1.elements.size();
                ++element_1_pos) {
            for (ShapePos shape_2_pos = -1;
                    shape_2_pos < (ShapePos)shape_with_holes_2.holes.size();
                    ++shape_2_pos) {
                const Shape& shape_2 = (shape_2_pos == -1)?
                    shape_with_holes_2.shape:
                    shape_with_holes_2.holes[shape_2_pos];
                for (ElementPos element_2_pos = 0;
                        element_2_pos < (ElementPos)shape_2.elements.size();
                        ++element_2_pos) {
                    if (strict) {
                        if (::strictly_intersect(
                                    shape_1, (shape_1_pos != -1), element_1_pos,
                                    shape_2, (shape_2_pos != -1), element_2_pos)) {
                            //std::cout << "intersect" << std::endl;
                            return true;
                        }
                    } else {
                        if (intersect(
                                    shape_1.elements[element_1_pos],
                                    shape_2.elements[element_2_pos])) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Check if shape_1 is in a hole of shape_2.
    for (const Shape& hole_2: shape_with_holes_2.holes) {
        auto result = find_point_in_difference(shape_with_holes_1.shape, hole_2);
        //std::cout << result.first << " " << result.second.point.to_string() << std::endl;
        if (result.first)
            if (hole_2.contains(result.second.point, strict))
                return false;
    }

    // Check if shape_2 is in a hole of shape_1.
    for (const Shape& hole_1: shape_with_holes_1.holes) {
        auto result = find_point_in_difference(shape_with_holes_2.shape, hole_1);
        //std::cout << result.first << " " << result.second.point.to_string() << std::endl;
        if (result.first)
            if (hole_1.contains(result.second.point, strict))
                return false;
    }

    // Check if shape_1 is in shape_2.
    {
        auto result = find_point_in_difference(shape_with_holes_1.shape, shape_with_holes_2.shape);
        if (!result.first) {
            // shape_1 outline is fully in the outline of shape_2.
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_with_holes_2.shape.contains(result.second.point, strict))
                return true;
        }
    }

    // Check if shape_2 is in shape_1.
    {
        auto result = find_point_in_difference(shape_with_holes_2.shape, shape_with_holes_1.shape);
        if (!result.first) {
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_with_holes_1.contains(result.second.point, strict))
                return true;
        }
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes_1,
        const Shape& shape_2,
        bool strict)
{
    for (ShapePos shape_1_pos = -1;
            shape_1_pos < (ShapePos)shape_with_holes_1.holes.size();
            ++shape_1_pos) {
        const Shape& shape_1 = (shape_1_pos == -1)?
            shape_with_holes_1.shape:
            shape_with_holes_1.holes[shape_1_pos];
        for (ElementPos element_1_pos = 0;
                element_1_pos < (ElementPos)shape_1.elements.size();
                ++element_1_pos) {
            for (ElementPos element_2_pos = 0;
                    element_2_pos < (ElementPos)shape_2.elements.size();
                    ++element_2_pos) {
                if (strict) {
                    if (::strictly_intersect(
                                shape_1, (shape_1_pos != -1), element_1_pos,
                                shape_2, false, element_2_pos)) {
                        //std::cout << "intersect" << std::endl;
                        return true;
                    }
                } else {
                    if (intersect(
                                shape_1.elements[element_1_pos],
                                shape_2.elements[element_2_pos])) {
                        return true;
                    }
                }
            }
        }
    }

    // Check if shape_2 is in a hole of shape_1.
    for (const Shape& hole_1: shape_with_holes_1.holes) {
        auto result = find_point_in_difference(shape_2, hole_1);
        if (result.first)
            if (hole_1.contains(result.second.point, strict))
                return false;
    }

    // Check if shape_1 is in shape_2.
    if (!shape_2.is_path) {
        auto result = find_point_in_difference(shape_with_holes_1.shape, shape_2);
        if (!result.first) {
            // shape_1 outline is fully in the outline of shape_2.
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_2.contains(result.second.point, strict))
                return true;
        }
    }

    // Check if shape_2 is in shape_1.
    {
        auto result = find_point_in_difference(shape_2, shape_with_holes_1.shape);
        if (!result.first) {
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_with_holes_1.contains(result.second.point, strict))
                return true;
        }
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_with_holes,
        const ShapeElement& element,
        bool strict)
{
    for (ShapePos shape_pos = -1;
            shape_pos < (ShapePos)shape_with_holes.holes.size();
            ++shape_pos) {
        const Shape& shape = (shape_pos == -1)?
            shape_with_holes.shape:
            shape_with_holes.holes[shape_pos];
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            ShapeElementIntersectionsOutput intersections = compute_intersections(
                    element,
                    shape_element);
            if (!intersections.proper_intersections.empty())
                return true;
            if (!strict) {
                if (!intersections.improper_intersections.empty())
                    return true;
                if (!intersections.overlapping_parts.empty())
                    return true;
            }
        }
    }

    // Check if the element is in a hole of the shape.
    for (const Shape& hole: shape_with_holes.holes) {
        auto result = find_point_in_difference(element, hole);
        if (result.first)
            if (hole.contains(result.second, strict))
                return false;
    }

    // Check if the element is in shape.
    {
        auto result = find_point_in_difference(element, shape_with_holes.shape);
        if (!result.first) {
            if (strict) {
                return false;
            } else {
                throw std::logic_error(FUNC_SIGNATURE);
                return true;
            }
        } else {
            if (shape_with_holes.contains(result.second, strict))
                return true;
        }
    }

    return false;
}
