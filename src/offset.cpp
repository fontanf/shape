#include "shape/offset.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
//#include "shape/writer.hpp"

//#include <iostream>
#include <fstream>

using namespace shape;

namespace
{

Shape inflate_element(
        const ShapeElement& element,
        double inflate,
        double deflate)
{
    Shape shape;
    switch (element.type) {
    case shape::ShapeElementType::LineSegment: {
        Point normal;
        normal.x = element.end.y - element.start.y;
        normal.y = element.start.x - element.end.x;
        LengthDbl normal_length = norm(normal);
        normal.x = normal.x / normal_length;
        normal.y = normal.y / normal_length;

        Point p1;
        p1.x = element.start.x + inflate * normal.x;
        p1.y = element.start.y + inflate * normal.y;
        Point p2;
        p2.x = element.end.x + inflate * normal.x;
        p2.y = element.end.y + inflate * normal.y;
        Point p3;
        p3.x = element.end.x - deflate * normal.x;
        p3.y = element.end.y - deflate * normal.y;
        Point p4;
        p4.x = element.start.x - deflate * normal.x;
        p4.y = element.start.y - deflate * normal.y;

        ShapeElement element_1;
        element_1.type = ShapeElementType::LineSegment;
        element_1.start = p1;
        element_1.end = p2;
        shape.elements.push_back(element_1);
        ShapeElement element_2;
        element_2.type = ShapeElementType::LineSegment;
        element_2.start = p2;
        element_2.end = p3;
        shape.elements.push_back(element_2);
        ShapeElement element_3;
        element_3.type = ShapeElementType::LineSegment;
        element_3.start = p3;
        element_3.end = p4;
        shape.elements.push_back(element_3);
        ShapeElement element_4;
        element_4.type = ShapeElementType::LineSegment;
        element_4.start = p4;
        element_4.end = p1;
        shape.elements.push_back(element_4);
        break;

    } case shape::ShapeElementType::CircularArc: {
        Point normal_start = (element.orientation == shape::ShapeElementOrientation::Anticlockwise)?
            element.start - element.center:
            element.center - element.start;
        LengthDbl normal_start_length = norm(normal_start);
        normal_start.x = normal_start.x / normal_start_length;
        normal_start.y = normal_start.y / normal_start_length;
        Point normal_end = (element.orientation == shape::ShapeElementOrientation::Anticlockwise)?
            element.end - element.center:
            element.center - element.end;
        LengthDbl normal_end_length = norm(normal_end);
        normal_end.x = normal_end.x / normal_end_length;
        normal_end.y = normal_end.y / normal_end_length;

        Point p1;
        p1.x = element.start.x + inflate * normal_start.x;
        p1.y = element.start.y + inflate * normal_start.y;
        Point p2;
        p2.x = element.end.x + inflate * normal_end.x;
        p2.y = element.end.y + inflate * normal_end.y;
        Point p3;
        p3.x = element.end.x - deflate * normal_end.x;
        p3.y = element.end.y - deflate * normal_end.y;
        Point p4;
        p4.x = element.start.x - deflate * normal_start.x;
        p4.y = element.start.y - deflate * normal_start.y;

        ShapeElement element_1;
        element_1.type = ShapeElementType::CircularArc;
        element_1.start = p1;
        element_1.end = p2;
        element_1.center = element.center;
        element_1.orientation = element.orientation;
        shape.elements.push_back(element_1);
        ShapeElement element_2;
        element_2.type = ShapeElementType::LineSegment;
        element_2.start = p2;
        element_2.end = p3;
        shape.elements.push_back(element_2);
        ShapeElement element_3;
        element_3.type = ShapeElementType::CircularArc;
        element_3.start = p3;
        element_3.end = p4;
        element_3.center = element.center;
        element_3.orientation = opposite(element.orientation);
        shape.elements.push_back(element_3);
        ShapeElement element_4;
        element_4.type = ShapeElementType::LineSegment;
        element_4.start = p4;
        element_4.end = p1;
        shape.elements.push_back(element_4);
        break;
    }
    }
    return shape;
}

}

ShapeWithHoles shape::inflate(
        const ShapeWithHoles& shape_orig,
        LengthDbl offset)
{
    //std::cout << std::endl;
    //std::cout << "inflate offset " << offset << " shape " << shape_orig.to_string(0) << std::endl;
    //inflate_export_inputs("inflate_input.json", shape_orig, offset);
    if (offset < 0.0) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": offset must be >= 0.0; "
                "offset: " + std::to_string(offset) + ".");
    }

    if (offset == 0.0)
        return shape_orig;

    ShapeWithHoles shape = remove_redundant_vertices(shape_orig).second;
    shape = remove_aligned_vertices(shape).second;
    //std::cout << "shape " << shape.to_string(0) << std::endl;

    // Write input to json for tests.
    //{
    //    std::string file_path = "inflate_input.json";
    //    std::ofstream file{file_path};
    //    nlohmann::json json;
    //    json["shape"] = shape.to_json();
    //    json["offset"] = offset;
    //    file << std::setw(4) << json << std::endl;
    //}

    // Remove small holes.
    ShapeWithHoles shape_tmp;
    shape_tmp.shape = shape.shape;
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];
        auto wh = hole.compute_width_and_height();
        if (!strictly_greater(wh.first, 2 * offset)
                || !strictly_greater(wh.second, 2 * offset)) {
            continue;
        }
        shape_tmp.holes.push_back(hole);
    }
    shape = shape_tmp;

    std::vector<ShapeWithHoles> union_input = {{shape}};

    // Inflate outline.
    if (shape.shape.is_circle()) {
        Shape circle = shape.shape;
        ShapeElement& element = circle.elements[0];
        LengthDbl radius_orig = distance(element.center, element.start);
        LengthDbl radius = radius_orig + offset;
        element.start = {element.center.x + radius, element.center.y};
        element.end = element.start;
        union_input.push_back({circle});
    } else {
        ElementPos element_prev_pos = shape.shape.elements.size() - 1;
        const ShapeElement& element_prev = shape.shape.elements[element_prev_pos];
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)shape.shape.elements.size();
                ++element_pos) {
            const ShapeElement& element_prev = shape.shape.elements[element_prev_pos];
            const ShapeElement& element = shape.shape.elements[element_pos];
            //std::cout << "element      " << element.to_string() << std::endl;
            //std::cout << "element_prev " << element_prev.to_string() << std::endl;

            Shape rectangle = inflate_element(element, offset, 0);

            Angle angle = (
                    element.jet(element.start, false)
                    - element_prev.jet(element_prev.end, true)).tangent_angle;

            if (angle >= M_PI) {
                Point point_prev = element_prev.end + offset * element_prev.normal(element_prev.end);
                Point point_curr = element.start + offset * element.normal(element.start);
                ShapeWithHoles sector;
                ShapeElement element_1;
                element_1.type = ShapeElementType::CircularArc;
                element_1.start = point_prev;
                element_1.end = point_curr;
                element_1.center = element.start;
                element_1.orientation = ShapeElementOrientation::Anticlockwise;
                sector.shape.elements.push_back(element_1);
                ShapeElement element_2;
                element_2.type = ShapeElementType::LineSegment;
                element_2.start = point_curr;
                element_2.end = element.start;
                sector.shape.elements.push_back(element_2);
                ShapeElement element_3;
                element_3.type = ShapeElementType::LineSegment;
                element_3.start = element.start;
                element_3.end = point_prev;
                sector.shape.elements.push_back(element_3);
                //std::cout << "sector " << sector.to_string(0) << std::endl;
                union_input.push_back(sector);
            }

            //std::cout << "rectangle " << rectangle.to_string(0) << std::endl;
            union_input.push_back({rectangle});

            element_prev_pos = element_pos;
        }
    }

    // Deflate holes.
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];

        if (hole.is_circle()) {
            Shape circle = hole;
            ShapeElement& element = circle.elements[0];
            LengthDbl radius_orig = distance(element.center, element.start);
            if (strictly_greater(radius_orig, offset)) {
                LengthDbl radius = radius_orig - offset;
                element.start = {element.center.x + radius, element.center.y};
                element.end = element.start;
                union_input.push_back({circle});
            }
        } else {
            ElementPos element_prev_pos = hole.elements.size() - 1;
            const ShapeElement& element_prev = hole.elements[element_prev_pos];
            for (ElementPos element_pos = 0;
                    element_pos < (ElementPos)hole.elements.size();
                    ++element_pos) {
                const ShapeElement& element_prev = hole.elements[element_prev_pos];
                const ShapeElement& element = hole.elements[element_pos];

                Shape rectangle = inflate_element(element, 0, offset);

                Angle angle = (
                        element.jet(element.start, false)
                        - element_prev.jet(element_prev.end, true)).tangent_angle;

                if (angle <= M_PI) {
                    Point point_prev = element_prev.end - offset * element_prev.normal(element_prev.end);
                    Point point_curr = element.start - offset * element.normal(element.start);
                    ShapeWithHoles sector;
                    ShapeElement element_1;
                    element_1.type = ShapeElementType::CircularArc;
                    element_1.start = point_curr;
                    element_1.end = point_prev;
                    element_1.center = element.start;
                    element_1.orientation = ShapeElementOrientation::Anticlockwise;
                    sector.shape.elements.push_back(element_1);
                    ShapeElement element_2;
                    element_2.type = ShapeElementType::LineSegment;
                    element_2.start = point_prev;
                    element_2.end = element.start;
                    sector.shape.elements.push_back(element_2);
                    ShapeElement element_3;
                    element_3.type = ShapeElementType::LineSegment;
                    element_3.start = element.start;
                    element_3.end = point_curr;
                    sector.shape.elements.push_back(element_3);
                    union_input.push_back(sector);
                }

                union_input.push_back({rectangle});

                element_prev_pos = element_pos;
            }
        }
    }

    //write_json(union_input, {}, "union_input.json");
    //compute_union_export_inputs("compute_union_input.json", union_input);
    return compute_union(union_input).front();
}

ShapeWithHoles shape::inflate(
        const Shape& shape_orig,
        LengthDbl offset)
{
    //std::cout << "inflate " << shape_orig.elements.size() << std::endl;
    //std::cout << "inflate " << shape.to_string(2) << std::endl;

    Shape shape = remove_redundant_vertices(shape_orig).second;
    shape = remove_aligned_vertices(shape).second;

    if (offset < 0.0) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": offset must be >= 0.0; "
                "offset: " + std::to_string(offset) + ".");
    }

    if (offset == 0.0)
        return {shape, {}};

    if (shape_orig.is_circle()) {
        ShapeWithHoles output;
        Shape shape = shape_orig;
        ShapeElement& element = shape.elements[0];
        LengthDbl radius_orig = distance(element.center, element.start);
        LengthDbl radius = radius_orig + offset;
        element.start = {element.center.x + radius, element.center.y};
        element.end = element.start;
        output.shape = shape;
        if (shape_orig.is_path && strictly_greater(radius_orig, offset)) {
            Shape hole = shape_orig;
            ShapeElement& element = hole.elements[0];
            LengthDbl hole_radius = radius_orig - offset;
            element.start = {element.center.x + hole_radius, element.center.y};
            element.end = element.start;
            output.holes.push_back(hole);
        }
        return output;
    }

    std::vector<ShapeWithHoles> union_input;
    if (!shape.is_path)
        union_input.push_back({shape});

    // Inflate outline.
    ElementPos element_prev_pos = shape.elements.size() - 1;
    const ShapeElement& element_prev = shape.elements[element_prev_pos];
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];
        const ShapeElement& element_prev = shape.elements[element_prev_pos];

        Shape rectangle = (!shape.is_path)?
            inflate_element(element, offset, 0):
            inflate_element(element, offset, offset);

        //std::cout << "element_pos " << element_pos << std::endl;
        //std::cout << "rectangle " << rectangle.to_string(2) << std::endl;

        Angle angle = (
                element.jet(element.start, false)
                - element_prev.jet(element_prev.end, true)).tangent_angle;
        //std::cout << "angle " << angle << std::endl;

        if (!shape.is_path) {
            if (angle >= M_PI) {
                Point point_prev = element_prev.end + offset * element_prev.normal(element_prev.end);
                Point point_curr = element.start + offset * element.normal(element.start);
                ShapeWithHoles sector;
                ShapeElement element_1;
                element_1.type = ShapeElementType::CircularArc;
                element_1.start = point_prev;
                element_1.end = point_curr;
                element_1.center = element.start;
                element_1.orientation = ShapeElementOrientation::Anticlockwise;
                sector.shape.elements.push_back(element_1);
                ShapeElement element_2;
                element_2.type = ShapeElementType::LineSegment;
                element_2.start = point_curr;
                element_2.end = element.start;
                sector.shape.elements.push_back(element_2);
                ShapeElement element_3;
                element_3.type = ShapeElementType::LineSegment;
                element_3.start = element.start;
                element_3.end = point_prev;
                sector.shape.elements.push_back(element_3);
                union_input.push_back(sector);
            }
        } else {
            //std::cout << element.to_string() << std::endl;
            //std::cout << "angle " << to_string(angle) << std::endl;
            if (element_pos == 0) {
                Point point_curr_1 = element.start + offset * element.normal(element.start);
                Point point_curr_2 = element.start - offset * element.normal(element.start);
                ShapeWithHoles sector;
                ShapeElement element_1;
                element_1.type = ShapeElementType::LineSegment;
                element_1.start = point_curr_1;
                element_1.end = point_curr_2;
                sector.shape.elements.push_back(element_1);
                ShapeElement element_2;
                element_2.type = ShapeElementType::CircularArc;
                element_2.start = point_curr_2;
                element_2.end = point_curr_1;
                element_2.center = element.start;
                element_2.orientation = ShapeElementOrientation::Anticlockwise;
                sector.shape.elements.push_back(element_2);
                union_input.push_back(sector);
            } else if (equal(angle, M_PI)) {
                //std::cout << "equal" << std::endl;
            } else if (angle >= M_PI) {
                //std::cout << "greater" << std::endl;
                Point point_prev = element_prev.end + offset * element_prev.normal(element_prev.end);
                Point point_curr = element.start + offset * element.normal(element.start);
                ShapeWithHoles sector;
                ShapeElement element_1;
                element_1.type = ShapeElementType::CircularArc;
                element_1.start = point_prev;
                element_1.end = point_curr;
                element_1.center = element.start;
                element_1.orientation = ShapeElementOrientation::Anticlockwise;
                sector.shape.elements.push_back(element_1);
                ShapeElement element_2;
                element_2.type = ShapeElementType::LineSegment;
                element_2.start = point_curr;
                element_2.end = element.start;
                sector.shape.elements.push_back(element_2);
                ShapeElement element_3;
                element_3.type = ShapeElementType::LineSegment;
                element_3.start = element.start;
                element_3.end = point_prev;
                sector.shape.elements.push_back(element_3);
                union_input.push_back(sector);
            } else {
                //std::cout << "smaller" << std::endl;
                Point point_prev = element_prev.end - offset * element_prev.normal(element_prev.end);
                Point point_curr = element.start - offset * element.normal(element.start);
                ShapeWithHoles sector;
                ShapeElement element_1;
                element_1.type = ShapeElementType::CircularArc;
                element_1.start = point_curr;
                element_1.end = point_prev;
                element_1.center = element.start;
                element_1.orientation = ShapeElementOrientation::Anticlockwise;
                sector.shape.elements.push_back(element_1);
                ShapeElement element_2;
                element_2.type = ShapeElementType::LineSegment;
                element_2.start = point_prev;
                element_2.end = element.start;
                sector.shape.elements.push_back(element_2);
                ShapeElement element_3;
                element_3.type = ShapeElementType::LineSegment;
                element_3.start = element.start;
                element_3.end = point_curr;
                sector.shape.elements.push_back(element_3);
                union_input.push_back(sector);
            }
        }

        union_input.push_back({rectangle});

        element_prev_pos = element_pos;
    }

    if (shape.is_path) {
        Point point_prev_1 = element_prev.end + offset * element_prev.normal(element_prev.end);
        Point point_prev_2 = element_prev.end - offset * element_prev.normal(element_prev.end);
        ShapeWithHoles sector;
        ShapeElement element_1;
        element_1.type = ShapeElementType::LineSegment;
        element_1.start = point_prev_2;
        element_1.end = point_prev_1;
        sector.shape.elements.push_back(element_1);
        ShapeElement element_2;
        element_2.type = ShapeElementType::CircularArc;
        element_2.start = point_prev_1;
        element_2.end = point_prev_2;
        element_2.center = element_prev.end;
        element_2.orientation = ShapeElementOrientation::Anticlockwise;
        sector.shape.elements.push_back(element_2);
        union_input.push_back(sector);
    }

    //compute_union_export_inputs("union_input.json", union_input);
    //Writer().add_shapes_with_holes(union_input).write_json("shapes.json");
    return compute_union(union_input).front();
}

std::vector<Shape> shape::deflate(
        const Shape& shape_orig,
        LengthDbl offset)
{
    if (offset < 0.0) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": offset must be >= 0.0; "
                "offset: " + std::to_string(offset) + ".");
    }

    if (shape_orig.is_path) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": input Shape must not be a path.");
    }

    if (shape_orig.is_circle()) {
        Shape shape = shape_orig;
        ShapeElement& element = shape.elements[0];
        LengthDbl radius_orig = distance(element.center, element.start);
        LengthDbl radius = radius_orig - offset;
        element.start = {element.center.x + radius, element.center.y};
        element.end = element.start;
        return {shape};
    }

    auto wh = shape_orig.compute_width_and_height();
    if (!strictly_greater(2 * wh.first, offset)
            && !strictly_greater(2 * wh.second, offset)) {
        return {};
    }

    Shape shape = remove_redundant_vertices(shape_orig).second;
    shape = remove_aligned_vertices(shape).second;

    if (offset == 0)
        return {{shape_orig}};

    std::vector<ShapeWithHoles> difference_input;

    // Inflate outline.
    ElementPos element_prev_pos = shape.elements.size() - 1;
    const ShapeElement& element_prev = shape.elements[element_prev_pos];
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element_prev = shape.elements[element_prev_pos];
        const ShapeElement& element = shape.elements[element_pos];

        Shape rectangle = inflate_element(element, 0, offset);

        Angle angle = (
                element.jet(element.start, false)
                - element_prev.jet(element_prev.end, true)).tangent_angle;

        if (angle <= M_PI) {
            Point point_prev = element_prev.end - offset * element_prev.normal(element_prev.end);
            Point point_curr = element.start - offset * element.normal(element.start);
            ShapeWithHoles sector;
            ShapeElement element_1;
            element_1.type = ShapeElementType::CircularArc;
            element_1.start = point_curr;
            element_1.end = point_prev;
            element_1.center = element.start;
            element_1.orientation = ShapeElementOrientation::Anticlockwise;
            sector.shape.elements.push_back(element_1);
            ShapeElement element_2;
            element_2.type = ShapeElementType::LineSegment;
            element_2.start = point_prev;
            element_2.end = element.start;
            sector.shape.elements.push_back(element_2);
            ShapeElement element_3;
            element_3.type = ShapeElementType::LineSegment;
            element_3.start = element.start;
            element_3.end = point_curr;
            sector.shape.elements.push_back(element_3);
            difference_input.push_back(sector);
        }

        difference_input.push_back({rectangle});

        element_prev_pos = element_pos;
    }

    auto difference_output = compute_difference({shape}, difference_input);
    std::vector<Shape> output;
    for (const ShapeWithHoles& shape: difference_output)
        output.push_back(shape.shape);
    return output;
}

void shape::inflate_export_inputs(
        const std::string& file_path,
        const ShapeWithHoles& shape,
        LengthDbl offset)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape"] = shape.to_json();
    json["offset"] = offset;
    file << std::setw(4) << json << std::endl;
}

void shape::inflate_export_inputs(
        const std::string& file_path,
        const Shape& shape,
        LengthDbl offset)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    json["shape"] = shape.to_json();
    json["offset"] = offset;
    file << std::setw(4) << json << std::endl;
}
