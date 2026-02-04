#include "shape/approximation.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
#include "shape/shapes_intersections.hpp"
//#include "shape/writer.hpp"

//#include <iostream>

using namespace shape;

std::vector<ShapeElement> shape::approximate_circular_arc_by_line_segments(
        const ShapeElement& circular_arc,
        LengthDbl segment_length,
        bool outer)
{
    LengthDbl arc_length = circular_arc.length();
    ElementPos number_of_line_segments = std::ceil(arc_length / segment_length);
    //std::cout << "circular_arc " << circular_arc.to_string() << std::endl;
    //std::cout << "segment_length " << segment_length
    //    << " arc_length " << arc_length
    //    << " number_of_line_segments " << number_of_line_segments
    //    << std::endl;

    if (circular_arc.type != ShapeElementType::CircularArc) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "input element must be of type CircularArc; "
                "circular_arc.type: " + element2str(circular_arc.type) + ".");
    }
    if (!outer && number_of_line_segments < 1) {
        number_of_line_segments = 1;
    }

    Angle angle = (circular_arc.orientation == ShapeElementOrientation::Full)?
        2 * M_PI:
        (circular_arc.orientation == ShapeElementOrientation::Anticlockwise)?
        angle_radian(
            circular_arc.start - circular_arc.center,
            circular_arc.end - circular_arc.center):
        angle_radian(
            circular_arc.end - circular_arc.center,
            circular_arc.start - circular_arc.center);
    //std::cout << "number_of_line_segments " << number_of_line_segments << std::endl;
    if ((outer && circular_arc.orientation == ShapeElementOrientation::Anticlockwise)
            || (!outer && circular_arc.orientation != ShapeElementOrientation::Anticlockwise)) {
        if (angle < M_PI && number_of_line_segments < 2) {
            number_of_line_segments = 2;
        } else if (angle >= M_PI && number_of_line_segments < 3) {
            number_of_line_segments = 3;
        }
    }

    std::vector<ShapeElement> line_segments;
    LengthDbl radius = distance(circular_arc.center, circular_arc.start);
    Point point_prev = circular_arc.start;
    Point point_circle_prev = circular_arc.start;
    for (ElementPos line_segment_id = 0;
            line_segment_id < number_of_line_segments - 1;
            ++line_segment_id) {
        Angle angle_cur = (angle * (line_segment_id + 1)) / (number_of_line_segments - 1);
        if (circular_arc.orientation == ShapeElementOrientation::Clockwise)
            angle_cur *= -1;
        //std::cout << "angle_cur " << angle_cur << std::endl;
        Point point_circle = circular_arc.start.rotate_radians(
                circular_arc.center,
                angle_cur);
        //std::cout << "point_circle " << point_circle.to_string() << std::endl;
        Point point_cur;
        if ((outer && circular_arc.orientation == ShapeElementOrientation::Clockwise)
                || (!outer && circular_arc.orientation != ShapeElementOrientation::Clockwise)) {
            point_cur = point_circle;
        } else {
            // https://en.wikipedia.org/wiki/Tangent_lines_to_circles#Cartesian_equation
            // https://en.wikipedia.org/wiki/Intersection_(geometry)#Two_lines
            // The tangent line of the circle at (x1, y1) has Cartesian equation
            // (x - x1)(x1 - xc) + (y - y1)(y1 - yc) = 0
            // (x1 - xc) * x + (y1 - yc) * y - x1 * (x1 - xc) - y1 * (y1 - yc) = 0
            // At (x2, y2)
            // (x2 - xc) * x + (y2 - yc) * y - x2 * (x2 - xc) - y2 * (y1 - yc) = 0
            LengthDbl a1 = (point_circle_prev.x - circular_arc.center.x);
            LengthDbl b1 = (point_circle_prev.y - circular_arc.center.y);
            LengthDbl c1 = point_circle_prev.x * (point_circle_prev.x - circular_arc.center.x)
                + point_circle_prev.y * (point_circle_prev.y - circular_arc.center.y);
            LengthDbl a2 = (point_circle.x - circular_arc.center.x);
            LengthDbl b2 = (point_circle.y - circular_arc.center.y);
            LengthDbl c2 = point_circle.x * (point_circle.x - circular_arc.center.x)
                + point_circle.y * (point_circle.y - circular_arc.center.y);
            point_cur.x = (c1 * b2 - c2 * b1) / (a1 * b2 - a2 * b1);
            point_cur.y = (a1 * c2 - a2 * c1) / (a1 * b2 - a2 * b1);
        }
        ShapeElement line_segment;
        line_segment.start = point_prev;
        line_segment.end = point_cur;
        line_segment.type = ShapeElementType::LineSegment;
        line_segments.push_back(line_segment);
        point_prev = point_cur;
        point_circle_prev = point_circle;
    }
    ShapeElement line_segment;
    line_segment.start = point_prev;
    line_segment.end = circular_arc.end;
    line_segment.type = ShapeElementType::LineSegment;
    line_segments.push_back(line_segment);
    return line_segments;
}

ShapeWithHoles shape::approximate_by_line_segments(
        const ShapeWithHoles& shape_orig,
        LengthDbl segment_length)
{
    //std::cout << "approximate_by_line_segments" << std::endl;
    ShapeWithHoles shape = remove_redundant_vertices(shape_orig).second;

    ShapeWithHoles shape_new;
    std::vector<ShapeWithHoles> union_input = {shape};

    for (const ShapeElement& element: shape.shape.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            shape_new.shape.elements.push_back(element);
            break;
        } case ShapeElementType::CircularArc: {
            std::vector<ShapeElement> approximated_element
                = approximate_circular_arc_by_line_segments(
                        element,
                        segment_length,
                        true);
            for (const ShapeElement& new_element: approximated_element) {
                shape_new.shape.elements.push_back(new_element);
                Shape triangle = build_triangle(
                        element.center,
                        new_element.start,
                        new_element.end);
                if (triangle.compute_area() < 0)
                    triangle = triangle.reverse();
                union_input.push_back({triangle});
            }
            break;
        }
        }
    }

    for (const Shape& hole: shape.holes) {
        Shape hole_new;
        for (const ShapeElement& element: hole.elements) {
            switch (element.type) {
            case ShapeElementType::LineSegment: {
                hole_new.elements.push_back(element);
                break;
            } case ShapeElementType::CircularArc: {
                std::vector<ShapeElement> approximated_element
                    = approximate_circular_arc_by_line_segments(
                            element,
                            segment_length,
                            false);
                for (const ShapeElement& new_element: approximated_element) {
                    hole_new.elements.push_back(new_element);
                    Shape triangle = build_triangle(
                            element.center,
                            new_element.start,
                            new_element.end);
                    if (triangle.compute_area() < 0)
                        triangle = triangle.reverse();
                    union_input.push_back({triangle});
                }
                break;
            }
            }
        }
        shape_new.holes.push_back(hole);
    }

    //std::cout << "approximate_by_line_segments end" << std::endl;
    if (!intersect(shape_new))
        return shape_new;
    return compute_union(union_input).front();
}
