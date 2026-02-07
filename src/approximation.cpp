#include "shape/approximation.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
#include "shape/shapes_intersections.hpp"
//#include "shape/writer.hpp"

//#include <iostream>
#include <fstream>

using namespace shape;

std::vector<ShapeElement> shape::approximate_circular_arc_by_line_segments(
        const ShapeElement& circular_arc,
        LengthDbl segment_length,
        bool outer)
{
    //std::cout << "circular_arc " << circular_arc.to_string() << std::endl;

    if (circular_arc.type != ShapeElementType::CircularArc) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "input element must be of type CircularArc; "
                "circular_arc.type: " + element2str(circular_arc.type) + ".");
    }

    LengthDbl arc_length = circular_arc.length();
    ElementPos number_of_line_segments = std::ceil(arc_length / segment_length);
    //std::cout << "segment_length " << segment_length
    //    << " arc_length " << arc_length
    //    << " number_of_line_segments " << number_of_line_segments
    //    << std::endl;

    if (!outer && number_of_line_segments < 1)
        number_of_line_segments = 1;

    Angle angle = (circular_arc.orientation == ShapeElementOrientation::Full)?
        2 * M_PI:
        (circular_arc.orientation == ShapeElementOrientation::Anticlockwise)?
        angle_radian(
            circular_arc.start - circular_arc.center,
            circular_arc.end - circular_arc.center):
        angle_radian(
            circular_arc.end - circular_arc.center,
            circular_arc.start - circular_arc.center);
    if ((outer && circular_arc.orientation == ShapeElementOrientation::Anticlockwise)
            || (!outer && circular_arc.orientation != ShapeElementOrientation::Anticlockwise)) {
        if (angle < M_PI && number_of_line_segments < 2)
            number_of_line_segments = 2;
    }
    if (angle >= M_PI && number_of_line_segments < 3)
        number_of_line_segments = 3;
    //std::cout << "number_of_line_segments " << number_of_line_segments << std::endl;

    std::vector<ShapeElement> line_segments;
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

namespace
{

std::vector<Shape> compute_circular_arc_extras_outer(
        const ShapeElement& circular_arc,
        LengthDbl segment_length)
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
    if (angle < M_PI && number_of_line_segments < 2) {
        number_of_line_segments = 2;
    } else if (angle >= M_PI && number_of_line_segments < 3) {
        number_of_line_segments = 3;
    }

    std::vector<Shape> output;
    Point point_far_0 = circular_arc.start + (circular_arc.start - circular_arc.center);
    Point point_circle_prev = circular_arc.start;
    for (ElementPos line_segment_id = 0;
            line_segment_id < number_of_line_segments - 1;
            ++line_segment_id) {
        Angle angle_cur = (angle * (line_segment_id + 1)) / (number_of_line_segments - 1);
        if (circular_arc.orientation == ShapeElementOrientation::Clockwise)
            angle_cur *= -1;
        //std::cout << "angle_cur " << angle_cur << std::endl;
        Point point_circle;
        if (line_segment_id < number_of_line_segments - 2) {
            Point point_far = point_far_0.rotate_radians(
                    circular_arc.center,
                    angle_cur);
            auto intersections = compute_intersections(
                    circular_arc,
                    build_line_segment(circular_arc.center, point_far));
            //std::cout << circular_arc.to_string() << std::endl;
            //std::cout << build_line_segment(circular_arc.center, point_far).to_string() << std::endl;
            //std::cout << intersections.to_string(0) << std::endl;
            //Writer()
            //    .add_element(circular_arc)
            //    .add_element(build_line_segment(circular_arc.center, point_far))
            //    .write_json("compute_circular_arc_extras_outer.json");
            point_circle = intersections.proper_intersections.front();
        } else {
            point_circle = circular_arc.end;
        }
        //std::cout << "point_circle " << point_circle.to_string() << std::endl;

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
        Point point_cur;
        point_cur.x = (c1 * b2 - c2 * b1) / (a1 * b2 - a2 * b1);
        point_cur.y = (a1 * c2 - a2 * c1) / (a1 * b2 - a2 * b1);

        Shape shape;
        if (circular_arc.orientation != shape::ShapeElementOrientation::Clockwise) {
            shape.elements.push_back(build_line_segment(point_circle_prev, point_cur));
            shape.elements.push_back(build_line_segment(point_cur, point_circle));
            shape.elements.push_back(build_circular_arc(
                        point_circle,
                        point_circle_prev,
                        circular_arc.center,
                        ShapeElementOrientation::Clockwise));
        } else {
            shape.elements.push_back(build_circular_arc(
                        point_circle_prev,
                        point_circle,
                        circular_arc.center,
                        ShapeElementOrientation::Clockwise));
            shape.elements.push_back(build_line_segment(point_circle, point_cur));
            shape.elements.push_back(build_line_segment(point_cur, point_circle_prev));
        }
        output.push_back(shape);
        point_circle_prev = point_circle;
    }
    return output;
}

std::vector<Shape> compute_circular_arc_extras_inner(
        const ShapeElement& circular_arc,
        LengthDbl segment_length)
{
    //std::cout << "circular_arc " << circular_arc.to_string() << std::endl;

    if (circular_arc.type != ShapeElementType::CircularArc) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "input element must be of type CircularArc; "
                "circular_arc.type: " + element2str(circular_arc.type) + ".");
    }

    LengthDbl arc_length = circular_arc.length();
    ElementPos number_of_line_segments = std::ceil(arc_length / segment_length);
    //std::cout << "segment_length " << segment_length
    //    << " arc_length " << arc_length
    //    << " number_of_line_segments " << number_of_line_segments
    //    << std::endl;
    if (number_of_line_segments < 1)
        number_of_line_segments = 1;

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
    if (angle >= M_PI && number_of_line_segments < 3)
        number_of_line_segments = 3;

    std::vector<Shape> output;
    Point point_far_0 = circular_arc.start + (circular_arc.start - circular_arc.center);
    Point point_circle_prev = circular_arc.start;
    for (ElementPos line_segment_id = 0;
            line_segment_id < number_of_line_segments - 1;
            ++line_segment_id) {
        Angle angle_cur = (angle * (line_segment_id + 1)) / (number_of_line_segments - 1);
        if (circular_arc.orientation == ShapeElementOrientation::Clockwise)
            angle_cur *= -1;
        //std::cout << "angle_cur " << angle_cur << std::endl;
        Point point_circle;
        if (line_segment_id < number_of_line_segments - 2) {
            Point point_far = point_far_0.rotate_radians(
                    circular_arc.center,
                    angle_cur);
            point_circle = compute_intersections(
                    circular_arc,
                    build_line_segment(circular_arc.center, point_far)).proper_intersections.front();
        } else {
            point_circle = circular_arc.end;
        }
        //std::cout << "point_circle " << point_circle.to_string() << std::endl;

        Shape shape;
        if (circular_arc.orientation != shape::ShapeElementOrientation::Clockwise) {
            shape.elements.push_back(build_line_segment(point_circle_prev, point_circle));
            shape.elements.push_back(build_circular_arc(
                        point_circle,
                        point_circle_prev,
                        circular_arc.center,
                        ShapeElementOrientation::Clockwise));
        } else {
            shape.elements.push_back(build_circular_arc(
                        point_circle_prev,
                        point_circle,
                        circular_arc.center,
                        ShapeElementOrientation::Clockwise));
            shape.elements.push_back(build_line_segment(point_circle, point_circle_prev));
        }
        output.push_back(shape);
        point_circle_prev = point_circle;
    }
    return output;
}

}

ShapeWithHoles shape::approximate_shape_by_line_segments(
        const Shape& shape_orig,
        LengthDbl segment_length,
        bool outer)
{
    //std::cout << "approximate_by_line_segments" << std::endl;
    //approximate_shape_by_line_segments_export_inputs(
    //        "approximate_shape_by_line_segments_inputs.json",
    //        shape_orig,
    //        segment_length,
    //        outer);

    if (shape_orig.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "input shape must not be a path.");
    }

    Shape shape = remove_redundant_vertices(shape_orig).second;

    ShapeWithHoles shape_new;

    for (const ShapeElement& element: shape.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            shape_new.shape.elements.push_back(element);
            break;
        } case ShapeElementType::CircularArc: {
            std::vector<ShapeElement> approximated_element
                = approximate_circular_arc_by_line_segments(
                        element,
                        segment_length,
                        outer);
            for (const ShapeElement& new_element: approximated_element)
                shape_new.shape.elements.push_back(new_element);
            break;
        }
        }
    }

    //std::cout << "shape_new " << shape_new.to_string(0) << std::endl;
    if (!intersect(shape_new))
        return shape_new;

    std::vector<ShapeWithHoles> union_input = {{shape}};

    for (const ShapeElement& element: shape.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            break;
        } case ShapeElementType::CircularArc: {
            bool b =  ((outer && element.orientation != ShapeElementOrientation::Clockwise)
                    || (!outer && element.orientation == ShapeElementOrientation::Clockwise));
            std::vector<Shape> extras = (b)?
                compute_circular_arc_extras_outer(
                    element,
                    segment_length):
                compute_circular_arc_extras_inner(
                    element,
                    segment_length);
            for (const Shape& extra: extras)
                union_input.push_back({extra});
            break;
        }
        }
    }

    //Writer().add_shapes_with_holes(union_input).write_json("union_input.json");
    std::vector<ShapeWithHoles> union_output = compute_union(union_input);
    return union_output.front();
}

Shape shape::approximate_path_by_line_segments(
        const Shape& path_orig,
        LengthDbl segment_length)
{
    //std::cout << "approximate_path_by_line_segments" << std::endl;

    if (!path_orig.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "input path must be a path.");
    }

    Shape path = remove_redundant_vertices(path_orig).second;

    Shape path_new;

    for (const ShapeElement& element: path.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            path_new.elements.push_back(element);
            break;
        } case ShapeElementType::CircularArc: {
            std::vector<ShapeElement> approximated_element
                = approximate_circular_arc_by_line_segments(
                        element,
                        segment_length,
                        true);
            for (const ShapeElement& new_element: approximated_element)
                path_new.elements.push_back(new_element);
            break;
        }
        }
    }

    return path_new;
}

ShapeWithHoles shape::approximate_by_line_segments(
        const ShapeWithHoles& shape_orig,
        LengthDbl segment_length)
{
    //std::cout << "approximate_by_line_segments" << std::endl;
    //approximate_shape_by_line_segments_export_inputs(
    //        "approximate_shape_by_line_segments_inputs.json",
    //        shape_orig.shape,
    //        segment_length,
    //        true);

    ShapeWithHoles shape = remove_redundant_vertices(shape_orig).second;

    ShapeWithHoles shape_new;

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
            for (const ShapeElement& new_element: approximated_element)
                shape_new.shape.elements.push_back(new_element);
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
                for (const ShapeElement& new_element: approximated_element)
                    hole_new.elements.push_back(new_element);
                break;
            }
            }
        }
        shape_new.holes.push_back(hole);
    }

    if (!intersect(shape_new))
        return shape_new;

    std::vector<ShapeWithHoles> union_input = {shape};

    for (const ShapeElement& element: shape.shape.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            break;
        } case ShapeElementType::CircularArc: {
            std::vector<Shape> extras = compute_circular_arc_extras_outer(
                    element,
                    segment_length);
            for (const Shape& extra: extras)
                union_input.push_back({extra});
            break;
        }
        }
    }

    for (const Shape& hole: shape.holes) {
        Shape hole_new;
        for (const ShapeElement& element: hole.elements) {
            switch (element.type) {
            case ShapeElementType::LineSegment: {
                break;
            } case ShapeElementType::CircularArc: {
                std::vector<Shape> extras = compute_circular_arc_extras_inner(
                        element,
                        segment_length);
                for (const Shape& extra: extras)
                    union_input.push_back({extra});
                break;
            }
            }
        }
        shape_new.holes.push_back(hole);
    }

    //Writer().add_shapes_with_holes(union_input).write_json("union_input.json");
    std::vector<ShapeWithHoles> union_output = compute_union(union_input);
    return union_output.front();
}

void shape::approximate_shape_by_line_segments_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl segment_length,
        bool outer)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape"] = shape.to_json();
    json["segment_length"] = segment_length;
    json["outer"] = outer;
    file << std::setw(4) << json << std::endl;
}
