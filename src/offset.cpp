#include "shape/offset.hpp"

#include "shape/self_intersections_removal.hpp"

//#include <iostream>

using namespace shape;

std::pair<Shape, std::vector<Shape>> shape::inflate(
        const Shape& shape_orig,
        LengthDbl offset,
        bool remove_self_intersections)
{
    //std::cout << "inflate " << shape.to_string(2) << std::endl;

    Shape shape = remove_redundant_vertices(shape_orig).second;

    if (offset < 0.0) {
        throw std::invalid_argument(
                "shape::inflate: offset must be >= 0.0; "
                "offset: " + std::to_string(offset) + ".");
    }

    if (offset == 0.0)
        return {shape, {}};

    Shape shape_new;
    ElementPos element_pos_prev = shape.elements.size() - 1;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        const ShapeElement& element_prev = shape.elements[element_pos_prev];

        Point start;

        ShapeElement element_new;

        switch (element.type) {
        case shape::ShapeElementType::LineSegment: {
            Point normal;
            normal.x = element.end.y - element.start.y;
            normal.y = element.start.x - element.end.x;
            LengthDbl normal_length = norm(normal);
            normal.x = normal.x / normal_length;
            normal.y = normal.y / normal_length;

            element_new.type = ShapeElementType::LineSegment;
            element_new.start.x = element.start.x + offset * normal.x;
            element_new.start.y = element.start.y + offset * normal.y;
            element_new.end.x = element.end.x + offset * normal.x;
            element_new.end.y = element.end.y + offset * normal.y;
            break;

        } case shape::ShapeElementType::CircularArc: {
            Point normal_start = element.start - element.center;
            LengthDbl normal_start_length = norm(normal_start);
            normal_start.x = normal_start.x / normal_start_length;
            normal_start.y = normal_start.y / normal_start_length;
            Point normal_end = element.end - element.center;
            LengthDbl normal_end_length = norm(normal_end);
            normal_end.x = normal_end.x / normal_end_length;
            normal_end.y = normal_end.y / normal_end_length;

            element_new.type = ShapeElementType::CircularArc;
            element_new.start.x = element.start.x + offset * normal_start.x;
            element_new.start.y = element.start.y + offset * normal_start.y;
            element_new.end.x = element.end.x + offset * normal_end.x;
            element_new.end.y = element.end.y + offset * normal_end.y;
            element_new.center = element.center;
            element_new.anticlockwise = element.anticlockwise;
            break;
        }
        }

        // Add circular arc that links element_prev to element.
        if (element_pos > 0) {
            ShapeElement element_inter_new;
            element_inter_new.type = ShapeElementType::CircularArc;
            element_inter_new.start = shape_new.elements.back().end;
            element_inter_new.end = element_new.start;
            element_inter_new.center = element.start;
            element_inter_new.anticlockwise = true;
            shape_new.elements.push_back(element_inter_new);

            // Check radius.
            LengthDbl radius = distance(element_inter_new.start, element_inter_new.center);
            if (!equal(radius, offset)) {
                throw std::logic_error(
                        "shape::inflate: radius != offset; "
                        "radius: " + std::to_string(radius) + "; "
                        "offset: " + std::to_string(offset) + "; "
                        "center: " + element_inter_new.center.to_string() + "; "
                        "start: " + element_inter_new.start.to_string() + ".");
            }
        }

        // Add inflated element.
        shape_new.elements.push_back(element_new);
    }

    // Add circular arc that links last to first element.
    ShapeElement element_inter_new;
    element_inter_new.type = ShapeElementType::CircularArc;
    element_inter_new.start = shape_new.elements.back().end;
    element_inter_new.end = shape_new.elements.front().start;
    element_inter_new.center = shape.elements.back().end;
    element_inter_new.anticlockwise = true;
    shape_new.elements.push_back(element_inter_new);

    shape_new = remove_redundant_vertices(shape_new).second;
    if (!remove_self_intersections)
        return {shape_new, {}};
    return shape::remove_self_intersections(shape_new);
}

std::vector<Shape> shape::deflate(
        const Shape& shape_orig,
        LengthDbl offset,
        bool extract_all_holes)
{
    if (offset < 0.0) {
        throw std::invalid_argument(
                "shape::inflate: offset must be >= 0.0; "
                "offset: " + std::to_string(offset) + ".");
    }

    Shape shape = remove_redundant_vertices(shape_orig).second;

    if (offset == 0)
        return {shape};

    Shape shape_new;
    ElementPos element_pos_prev = shape.elements.size() - 1;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        const ShapeElement& element_prev = shape.elements[element_pos_prev];

        Point start;

        ShapeElement element_new;

        switch (element.type) {
        case shape::ShapeElementType::LineSegment: {
            Point normal;
            normal.x = element.end.y - element.start.y;
            normal.y = element.start.x - element.end.x;
            LengthDbl normal_length = norm(normal);
            normal.x = normal.x / normal_length;
            normal.y = normal.y / normal_length;

            element_new.type = ShapeElementType::LineSegment;
            element_new.start.x = element.start.x - offset * normal.x;
            element_new.start.y = element.start.y - offset * normal.y;
            element_new.end.x = element.end.x - offset * normal.x;
            element_new.end.y = element.end.y - offset * normal.y;
            break;

        } case shape::ShapeElementType::CircularArc: {
            Point normal_start = element.start - element.center;
            LengthDbl normal_start_length = norm(normal_start);
            normal_start.x = normal_start.x / normal_start_length;
            normal_start.y = normal_start.y / normal_start_length;
            Point normal_end = element.end - element.center;
            LengthDbl normal_end_length = norm(normal_end);
            normal_end.x = normal_end.x / normal_end_length;
            normal_end.y = normal_end.y / normal_end_length;

            element_new.type = ShapeElementType::CircularArc;
            element_new.start.x = element.start.x - offset * normal_start.x;
            element_new.start.y = element.start.y - offset * normal_start.y;
            element_new.end.x = element.end.x - offset * normal_end.x;
            element_new.end.y = element.end.y - offset * normal_end.y;
            element_new.center = element.center;
            element_new.anticlockwise = element.anticlockwise;
            break;
        }
        }

        // Add circular arc that links element_prev to element.
        if (element_pos > 0) {
            ShapeElement element_inter_new;
            element_inter_new.type = ShapeElementType::CircularArc;
            element_inter_new.start = shape_new.elements.back().end;
            element_inter_new.end = element_new.start;
            element_inter_new.center = element.start;
            element_inter_new.anticlockwise = false;
            shape_new.elements.push_back(element_inter_new);

            // Check radius.
            LengthDbl radius = distance(element_inter_new.start, element_inter_new.center);
            if (!equal(radius, offset)) {
                throw std::logic_error(
                        "shape::inflate: radius != offset; "
                        "radius: " + std::to_string(radius) + "; "
                        "offset: " + std::to_string(offset) + ".");
            }
        }

        // Add inflated element.
        shape_new.elements.push_back(element_new);
    }

    // Add circular arc that links last to first element.
    ShapeElement element_inter_new;
    element_inter_new.type = ShapeElementType::CircularArc;
    element_inter_new.start = shape_new.elements.back().end;
    element_inter_new.end = shape_new.elements.front().start;
    element_inter_new.center = shape.elements.back().end;
    element_inter_new.anticlockwise = false;
    shape_new.elements.push_back(element_inter_new);

    shape_new = remove_redundant_vertices(shape_new).second;
    if (!extract_all_holes)
        return {shape_new};
    return extract_all_holes_from_self_intersecting_hole(shape_new);
}
