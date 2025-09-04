#include "shape/shape.hpp"

#include "shape/clean.hpp"
#include "shape/element_intersections.hpp"
#include "shape/intersection_tree.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace shape;

LengthDbl shape::largest_power_of_two_lesser_or_equal(LengthDbl value)
{
    double res = 1;
    if (res > value) {
        while (res > value)
            res /= 2;
    } else {
        while (res * 2 < value)
            res *= 2;
    }
    return res;
}

LengthDbl shape::smallest_power_of_two_greater_or_equal(LengthDbl value)
{
    double res = 1;
    if (res < value) {
        while (res < value)
            res *= 2;
    } else {
        while (res / 2 > value)
            res /= 2;
    }
    return res;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Point /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string shape::to_string(double value)
{
    std::streamsize precision = std::cout.precision();
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<LengthDbl>::digits10 + 1)
        << value << std::setprecision(precision);
    return ss.str();
}

std::string Point::to_string() const
{
    std::streamsize precision = std::cout.precision();
    std::stringstream ss;
    ss << std::setprecision(std::numeric_limits<LengthDbl>::digits10 + 1)
        << "(" << x << ", " << y << ")"
        << std::setprecision(precision);
    return ss.str();
    //return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

Point shape::operator+(
        const Point& point_1,
        const Point& point_2)
{
    return {point_1.x + point_2.x, point_1.y + point_2.y};
}

Point shape::operator*(
        LengthDbl scalar,
        const Point& point)
{
    return {scalar * point.x, scalar * point.y};
}

Point shape::operator-(
        const Point& point_1,
        const Point& point_2)
{
    return {point_1.x - point_2.x, point_1.y - point_2.y};
}

LengthDbl shape::norm(
        const Point& vector)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

LengthDbl shape::squared_norm(
        const Point& vector)
{
    return vector.x * vector.x + vector.y * vector.y;
}

LengthDbl shape::distance(
        const Point& point_1,
        const Point& point_2)
{
    return norm(point_2 - point_1);
}

LengthDbl shape::squared_distance(
        const Point& point_1,
        const Point& point_2)
{
    return squared_norm(point_2 - point_1);
}

LengthDbl shape::dot_product(
        const Point& vector_1,
        const Point& vector_2)
{
    return vector_1.x * vector_2.x + vector_1.y * vector_2.y;
}

LengthDbl shape::cross_product(
        const Point& vector_1,
        const Point& vector_2)
{
    return vector_1.x * vector_2.y - vector_2.x * vector_1.y;
}

Point& Point::shift(
        LengthDbl x,
        LengthDbl y)
{
    this->x += x;
    this->y += y;
    return *this;
}

Point Point::rotate(
        Angle angle) const
{
    if (equal(angle, 0.0) || equal(angle, 360)) {
        return *this;
    } else if (equal(angle, 180) || equal(angle, -180)) {
        Point point_out;
        point_out.x = -x;
        point_out.y = -y;
        return point_out;
    } else if (equal(angle, 90) || equal(angle, -270)) {
        Point point_out;
        point_out.x = -y;
        point_out.y = x;
        return point_out;
    } else if (equal(angle, 270) || equal(angle, -90)) {
        Point point_out;
        point_out.x = y;
        point_out.y = -x;
        return point_out;
    } else {
        Point point_out;
        angle = M_PI * angle / 180;
        point_out.x = std::cos(angle) * x - std::sin(angle) * y;
        point_out.y = std::sin(angle) * x + std::cos(angle) * y;
        return point_out;
    }
}

Point Point::rotate_radians(
        Angle angle) const
{
    Point point_out;
    point_out.x = std::cos(angle) * x - std::sin(angle) * y;
    point_out.y = std::sin(angle) * x + std::cos(angle) * y;
    return point_out;
}

Point Point::rotate_radians(
        const Point& center,
        Angle angle) const
{
    Point point_out;
    point_out.x = center.x
        + std::cos(angle) * (this->x - center.x)
        - std::sin(angle) * (this->y - center.y);
    point_out.y = center.y
        + std::sin(angle) * (this->x - center.x)
        + std::cos(angle) * (this->y - center.y);
    return point_out;
}

Point Point::axial_symmetry_identity_line() const
{
    Point point_out;
    point_out.x = y;
    point_out.y = x;
    return point_out;
}

Point Point::axial_symmetry_y_axis() const
{
    Point point_out;
    point_out.x = -x;
    point_out.y = y;
    return point_out;
}

Point Point::axial_symmetry_x_axis() const
{
    Point point_out;
    point_out.x = x;
    point_out.y = -y;
    return point_out;
}

Angle shape::angle_radian(
        const Point& vector)
{
    Angle a = std::atan2(vector.y, vector.x);
    if (strictly_lesser(a, 0))
        a += 2 * M_PI;
    return a;
}

Angle shape::angle_radian(
        const Point& vector_1,
        const Point& vector_2)
{
    Angle a = atan2(vector_2.y, vector_2.x) - atan2(vector_1.y, vector_1.x);
    if (a < 0)
        a += 2 * M_PI;
    return a;
}

Point ShapeElement::middle() const
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        Point point;
        point.x = (this->start.x + this->end.x) / 2;
        point.y = (this->start.y + this->end.y) / 2;
        return point;
    } case ShapeElementType::CircularArc: {
        if (this->orientation == ShapeElementOrientation::Anticlockwise) {
            Angle angle = angle_radian(
                    this->start - this->center,
                    this->end - this->center);
            return this->start.rotate_radians(this->center, angle / 2.);
        } else {
            Angle angle = angle_radian(
                this->end - this->center,
                this->start - this->center);
            return this->end.rotate_radians(this->center, angle / 2.);
        }
    }
    }
    return {0, 0};
}

std::pair<Point, Point> ShapeElement::min_max() const
{
    LengthDbl x_min = (std::min)(this->start.x, this->end.x);
    LengthDbl x_max = (std::max)(this->start.x, this->end.x);
    LengthDbl y_min = (std::min)(this->start.y, this->end.y);
    LengthDbl y_max = (std::max)(this->start.y, this->end.y);

    if (this->type == ShapeElementType::CircularArc) {
        LengthDbl radius = distance(this->center, this->start);
        Angle starting_angle = shape::angle_radian(this->start - this->center);
        Angle ending_angle = shape::angle_radian(this->end - this->center);
        if (this->orientation != ShapeElementOrientation::Anticlockwise)
            std::swap(starting_angle, ending_angle);
        //std::cout << "starting_angle " << starting_angle << " ending_angle " << ending_angle << std::endl;
        if (starting_angle == ending_angle) {
            x_min = this->center.x - radius;
            x_max = this->center.x + radius;
            y_min = this->center.y - radius;
            y_max = this->center.y + radius;
        } else if (starting_angle < ending_angle) {
            if (starting_angle <= M_PI
                    && ending_angle >= M_PI) {
                x_min = std::min(x_min, this->center.x - radius);
            }
            if (starting_angle == 0)
                x_max = std::max(x_max, this->center.x + radius);
            if (starting_angle <= 3 * M_PI / 2
                    && ending_angle >= 3 * M_PI / 2 ) {
                y_min = std::min(y_min, this->center.y - radius);
            }
            if (starting_angle <= M_PI / 2
                    && ending_angle >= M_PI / 2) {
                y_max = std::max(y_max, this->center.y + radius);
            }
        } else {  // starting_angle > ending_angle
            if (starting_angle <= M_PI
                    || ending_angle >= M_PI) {
                x_min = std::min(x_min, this->center.x - radius);
            }
            x_max = std::max(x_max, this->center.x + radius);
            if (starting_angle <= 3 * M_PI / 2
                    || ending_angle >= 3 * M_PI / 2) {
                y_min = std::min(y_min, this->center.y - radius);
            }
            if (starting_angle <= M_PI / 2
                    || ending_angle >= M_PI / 2) {
                y_max = std::max(y_max, this->center.y + radius);
            }
        }
    }

    return {{x_min, y_min}, {x_max, y_max}};
}

std::pair<Point, Point> ShapeElement::furthest_points(Angle angle) const
{
    if (angle == 0.0) {
        //std::cout << this->to_string() << std::endl;
        Point point_min;
        Point point_max;
        if (this->start.y < this->end.y) {
            point_min = this->start;
            point_max = this->end;
        } else {
            point_min = this->end;
            point_max = this->start;
        }

        if (this->type == ShapeElementType::CircularArc) {
            LengthDbl radius = distance(this->center, this->start);
            if (this->orientation == ShapeElementOrientation::Full) {
                point_min.y = this->center.y - radius;
                point_min.x = this->center.x;
                point_max.y = this->center.y + radius;
                point_max.x = this->center.x;
            } else {
                Angle starting_angle = shape::angle_radian(this->start - this->center);
                Angle ending_angle = shape::angle_radian(this->end - this->center);
                if (this->orientation != ShapeElementOrientation::Anticlockwise)
                    std::swap(starting_angle, ending_angle);
                //std::cout << "starting_angle " << starting_angle << " ending_angle " << ending_angle << std::endl;
                if (starting_angle <= ending_angle) {
                    if (starting_angle <= 3 * M_PI / 2
                            && ending_angle >= 3 * M_PI / 2 ) {
                        point_min.y = this->center.y - radius;
                        point_min.x = this->center.x;
                    }
                    if (starting_angle <= M_PI / 2
                            && ending_angle >= M_PI / 2) {
                        point_max.y = this->center.y + radius;
                        point_max.x = this->center.x;
                    }
                } else {  // starting_angle > ending_angle
                    if (starting_angle <= 3 * M_PI / 2
                            || ending_angle >= 3 * M_PI / 2) {
                        point_min.y = this->center.y - radius;
                        point_min.x = this->center.x;
                    }
                    if (starting_angle <= M_PI / 2
                            || ending_angle >= M_PI / 2) {
                        point_max.y = this->center.y + radius;
                        point_max.x = this->center.x;
                    }
                }
            }
        }

        return {point_min, point_max};
    }

    ShapeElement element_tmp = this->rotate(-angle);
    auto mm = element_tmp.furthest_points(0.0);
    return {mm.first.rotate(angle), mm.second.rotate(angle)};
}

std::pair<ShapeElement, ShapeElement> ShapeElement::split(const Point& point) const
{
    switch (this->type) {
    case ShapeElementType::LineSegment: {
        ShapeElement element_1 = *this;
        ShapeElement element_2 = *this;
        element_1.end = point;
        element_2.start = point;
        return {element_1, element_2};
    } case ShapeElementType::CircularArc: {
        if (orientation != ShapeElementOrientation::Full) {
            ShapeElement element_1 = *this;
            ShapeElement element_2 = *this;
            element_1.end = point;
            element_2.start = point;
            return {element_1, element_2};
        } else if (!equal(this->start, this->end)) {
            ShapeElement element_1 = *this;
            element_1.orientation = ShapeElementOrientation::Anticlockwise;
            element_1.start = point;
            element_1.end = point;
            ShapeElement element_2 = *this;
            element_2.orientation = ShapeElementOrientation::Anticlockwise;
            element_2.start = point;
            element_2.end = point;
            return {element_1, element_2};
        } else {
            ShapeElement element_1 = *this;
            ShapeElement element_2 = *this;
            element_1.end = point;
            element_2.start = point;
            return {element_1, element_2};
        }
    }
    }
    return {};
}

int shape::counter_clockwise(
        const Point& point_1,
        const Point& point_2,
        const Point& point_3)
{
    AreaDbl area = (point_2.x - point_1.x) * (point_3.y - point_1.y)
        - (point_2.y - point_1.y) * (point_3.x - point_1.x);
    if (strictly_greater(area, 0)) {
        return -1;
    } else if (strictly_lesser(area, 0)) {
        return 1;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ShapeElement /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

LengthDbl ShapeElement::length() const
{
    switch (type) {
    case ShapeElementType::LineSegment:
        return distance(this->start, this->end);
    case ShapeElementType::CircularArc:
        LengthDbl r = distance(this->center, this->start);
        if (this->orientation == ShapeElementOrientation::Full) {
            return 2 * M_PI * r;
        } if (this->orientation == ShapeElementOrientation::Anticlockwise) {
            return angle_radian(this->start - this->center, this->end - this->center) * r;
        } else {
            return angle_radian(this->end - this->center, this->start - this->center) * r;
        }
    }
    return -1;
}

bool ShapeElement::contains(const Point& point) const
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        return equal(
                distance(this->start, point) + distance(point, this->end),
                distance(this->start, this->end));
    } case ShapeElementType::CircularArc: {
        // Check if point lies on circle
        if (!equal(
                    distance(point, this->center),
                    distance(this->start, this->center))) {
            return false;
        }

        if (this->orientation == ShapeElementOrientation::Full)
            return true;

        // Calculate angles
        Angle point_angle = angle_radian(point - this->center);
        Angle start_angle = angle_radian(this->start - this->center);
        Angle end_angle = angle_radian(this->end - this->center);

        if (this->orientation == ShapeElementOrientation::Anticlockwise) {
            Angle a0 = angle_radian(
                    this->start - this->center,
                    this->end - this->center);
            Angle a = angle_radian(
                    this->start - this->center,
                    point - this->center);
            return !strictly_greater(a, a0);
        } else {
            Angle a0 = angle_radian(
                    this->end - this->center,
                    this->start - this->center);
            Angle a = angle_radian(
                    this->end - this->center,
                    point - this->center);
            return !strictly_greater(a, a0);
        }
    }
    }
    return false;
}

std::string ShapeElement::to_string() const
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        return "LineSegment start " + start.to_string()
            + " end " + end.to_string();
    } case ShapeElementType::CircularArc: {
        return "CircularArc start " + start.to_string()
            + " end " + end.to_string()
            + " center " + center.to_string()
            + " orientation " + orientation2str(orientation);
    }
    }
    return "";
}

nlohmann::json ShapeElement::to_json() const
{
    nlohmann::json json;
    json["type"] = element2str(type);
    json["start"]["x"] = start.x;
    json["start"]["y"] = start.y;
    json["end"]["x"] = end.x;
    json["end"]["y"] = end.y;
    if (type == ShapeElementType::CircularArc) {
        json["center"]["x"] = center.x;
        json["center"]["y"] = center.y;
        json["orientation"] = orientation2str(orientation);
    }
    return json;
}

ShapeElement& ShapeElement::shift(
        LengthDbl x,
        LengthDbl y)
{
    this->start.shift(x, y);
    this->end.shift(x, y);
    this->center.shift(x, y);
    return *this;
}

ShapeElement ShapeElement::rotate(
        Angle angle) const
{
    ShapeElement element_out = *this;
    element_out.start = start.rotate(angle);
    element_out.end = end.rotate(angle);
    element_out.center = center.rotate(angle);
    return element_out;
}

ShapeElement ShapeElement::axial_symmetry_identity_line() const
{
    ShapeElement element_out = *this;
    element_out.start = this->end.axial_symmetry_identity_line();
    element_out.end = this->start.axial_symmetry_identity_line();
    element_out.center = this->center.axial_symmetry_identity_line();
    element_out.orientation = opposite(this->orientation);
    return element_out;
}

ShapeElement ShapeElement::axial_symmetry_x_axis() const
{
    ShapeElement element_out = *this;
    element_out.start = end.axial_symmetry_x_axis();
    element_out.end = start.axial_symmetry_x_axis();
    element_out.center = center.axial_symmetry_x_axis();
    element_out.orientation = opposite(this->orientation);
    return element_out;
}

ShapeElement ShapeElement::axial_symmetry_y_axis() const
{
    ShapeElement element_out = *this;
    element_out.start = end.axial_symmetry_y_axis();
    element_out.end = start.axial_symmetry_y_axis();
    element_out.center = center.axial_symmetry_y_axis();
    element_out.orientation = opposite(this->orientation);
    return element_out;
}

ShapeElement ShapeElement::reverse() const
{
    ShapeElement element_out;
    element_out.type = this->type;
    element_out.start.x = this->end.x;
    element_out.start.y = this->end.y;
    element_out.end.x = this->start.x;
    element_out.end.y = this->start.y;
    element_out.center.x = this->center.x;
    element_out.center.y = this->center.y;
    element_out.orientation = opposite(this->orientation);
    return element_out;
}

ShapeElementType shape::str2element(const std::string& str)
{
    if (str == "LineSegment"
            || str == "line_segment"
            || str == "L"
            || str == "l") {
        return ShapeElementType::LineSegment;
    } else if (str == "CircularArc"
            || str == "circular_arc"
            || str == "C"
            || str == "c") {
        return ShapeElementType::CircularArc;
    } else {
        throw std::invalid_argument("");
        return ShapeElementType::LineSegment;
    }
}

std::string shape::element2str(ShapeElementType type)
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        return "LineSegment";
    } case ShapeElementType::CircularArc: {
        return "CircularArc";
    }
    }
    return "";
}

char shape::element2char(ShapeElementType type)
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        return 'L';
    } case ShapeElementType::CircularArc: {
        return 'C';
    }
    }
    return ' ';
}

ShapeElementOrientation shape::str2orientation(const std::string& str)
{
    if (str == "Anticlockwise"
            || str == "anticlockwise"
            || str == "A"
            || str == "a") {
        return ShapeElementOrientation::Anticlockwise;
    } else if (str == "Clockwise"
            || str == "clockwise"
            || str == "C"
            || str == "c") {
        return ShapeElementOrientation::Clockwise;
    } else if (str == "Full"
            || str == "full"
            || str == "F"
            || str == "f") {
        return ShapeElementOrientation::Full;
    } else {
        throw std::invalid_argument("");
        return ShapeElementOrientation::Full;
    }
}

std::string shape::orientation2str(ShapeElementOrientation type)
{
    switch (type) {
    case ShapeElementOrientation::Anticlockwise: {
        return "Anticlockwise";
    } case ShapeElementOrientation::Clockwise: {
        return "Clockwise";
    } case ShapeElementOrientation::Full: {
        return "Full";
    }
    }
    return "";
}

char shape::orientation2char(ShapeElementOrientation type)
{
    switch (type) {
    case ShapeElementOrientation::Anticlockwise: {
        return 'A';
    } case ShapeElementOrientation::Clockwise: {
        return 'C';
    } case ShapeElementOrientation::Full: {
        return 'F';
    }
    }
    return ' ';
}

ShapeElementOrientation shape::opposite(ShapeElementOrientation orientation)
{
    switch (orientation) {
    case ShapeElementOrientation::Anticlockwise:
        return ShapeElementOrientation::Clockwise;
    case ShapeElementOrientation::Clockwise:
        return ShapeElementOrientation::Anticlockwise;
    case ShapeElementOrientation::Full:
        return ShapeElementOrientation::Full;
    }
    return ShapeElementOrientation::Full;
}

bool shape::operator<(
        const ShapeElement& element_1,
        const ShapeElement& element_2)
{
    if (element_1.type != element_2.type)
        return element_1.type < element_2.type;
    if (element_1.start.x != element_2.start.x)
        return element_1.start.x < element_2.start.x;
    if (element_1.start.y != element_2.start.y)
        return element_1.start.y < element_2.start.y;
    if (element_1.end.x != element_2.end.x)
        return element_1.end.x < element_2.end.x;
    if (element_1.end.y != element_2.end.y)
        return element_1.end.y < element_2.end.y;
    if (element_1.center.x != element_2.center.x)
        return element_1.center.x < element_2.center.x;
    if (element_1.center.y != element_2.center.y)
        return element_1.center.y < element_2.center.y;
    if (element_1.orientation != element_2.orientation)
        return element_1.orientation < element_2.orientation;
    return false;
}

ShapeElement shape::operator*(
        LengthDbl scalar,
        const ShapeElement& element)
{
    ShapeElement element_out;
    element_out.type = element.type;
    element_out.start = scalar * element.start;
    element_out.end = scalar * element.end;
    element_out.center = scalar * element.center;
    element_out.orientation = element.orientation;
    return element_out;
}

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
                "shape::circular_arc_to_line_segments: "
                "input element must be of type CircularArc; "
                "circular_arc.type: " + element2str(circular_arc.type) + ".");
    }
    if (!outer && number_of_line_segments < 1) {
        number_of_line_segments = 1;
        //throw std::runtime_error(
        //        "shape::circular_arc_to_line_segments: "
        //        "at least 1 line segment is needed to inner approximate a circular arc; "
        //        "outer: " + std::to_string(outer) + "; "
        //        "number_of_line_segments: " + std::to_string(number_of_line_segments) + ".");
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
            //throw std::runtime_error(
            //        "shape::circular_arc_to_line_segments: "
            //        "at least 2 line segments are needed to approximate the circular arc; "
            //        "circular_arc: " + circular_arc.to_string() + "; "
            //        "outer: " + std::to_string(outer) + "; "
            //        "angle: " + std::to_string(angle) + "; "
            //        "number_of_line_segments: " + std::to_string(number_of_line_segments) + ".");
        } else if (angle >= M_PI && number_of_line_segments < 3) {
            number_of_line_segments = 3;
            //throw std::runtime_error(
            //        "shape::circular_arc_to_line_segments: "
            //        "at least 3 line segments are needed to approximate the circular arc; "
            //        "circular_arc: " + circular_arc.to_string() + "; "
            //        "outer: " + std::to_string(outer) + "; "
            //        "angle: " + std::to_string(angle) + "; "
            //        "number_of_line_segments: " + std::to_string(number_of_line_segments) + ".");
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

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Shape /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string shape::shape2str(ShapeType type)
{
    switch (type) {
    case ShapeType::Circle: {
        return "C";
    } case ShapeType::Square: {
        return "S";
    } case ShapeType::Rectangle: {
        return "R";
    } case ShapeType::Polygon: {
        return "P";
    } case ShapeType::PolygonWithHoles: {
        return "PH";
    } case ShapeType::MultiPolygon: {
        return "MP";
    } case ShapeType::MultiPolygonWithHoles: {
        return "MPH";
    } case ShapeType::GeneralShape: {
        return "G";
    }
    }
    return "";
}

bool Shape::is_circle() const
{
    return (elements.size() == 1
            && elements.front().type == ShapeElementType::CircularArc);
}

bool Shape::is_square() const
{
    if (elements.size() != 4)
        return false;
    auto it_prev = std::prev(elements.end());
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->type != ShapeElementType::LineSegment)
            return false;
        // Check angle.
        Angle theta = angle_radian(it_prev->start - it_prev->end, it->end - it->start);
        if (!equal(theta, M_PI / 2))
            return false;
        // Check length.
        if (!equal(it->length(), elements[0].length()))
            return false;
        it_prev = it;
    }
    return true;
}

bool Shape::is_rectangle() const
{
    if (elements.size() != 4)
        return false;
    auto it_prev = std::prev(elements.end());
    for (auto it = elements.begin(); it != elements.end(); ++it) {
        if (it->type != ShapeElementType::LineSegment)
            return false;
        // Check angle.
        Angle theta = angle_radian(it_prev->start - it_prev->end, it->end - it->start);
        if (!equal(theta, M_PI / 2))
            return false;
        it_prev = it;
    }
    return true;
}

bool Shape::is_polygon() const
{
    for (auto it = elements.begin(); it != elements.end(); ++it)
        if (it->type != ShapeElementType::LineSegment)
            return false;
    return true;
}

AreaDbl Shape::compute_area() const
{
    AreaDbl area = 0.0;
    for (const ShapeElement& element: elements) {
        if (element.type == ShapeElementType::CircularArc
                && element.orientation == ShapeElementOrientation::Full) {
            LengthDbl radius = distance(element.center, element.start);
            return radius * radius * M_PI;
        }
        area += cross_product(element.start, element.end);
        // Handle circular arcs.
        if (element.type == ShapeElementType::CircularArc) {
            LengthDbl radius = distance(element.center, element.start);
            Angle theta = angle_radian(element.center - element.start, element.center - element.end);
            if (element.orientation == ShapeElementOrientation::Anticlockwise) {
                area += radius * radius * ((!(element.start == element.end))? theta: 2.0 * M_PI);
            } else {
                area -= radius * radius * ((!(element.start == element.end))? 2.0 * M_PI - theta: 2.0 * M_PI);
            }
        }
    }
    return area / 2;
}

std::pair<Point, Point> Shape::compute_min_max(
        Angle angle,
        bool mirror) const
{
    LengthDbl x_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl x_max = -std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_max = -std::numeric_limits<LengthDbl>::infinity();
    for (const ShapeElement& element_orig: elements) {
        ShapeElement element = (!mirror)?
            element_orig.rotate(angle):
            element_orig.axial_symmetry_y_axis().rotate(angle);
        auto mm = element.min_max();
        x_min = std::min(x_min, mm.first.x);
        x_max = std::max(x_max, mm.second.x);
        y_min = std::min(y_min, mm.first.y);
        y_max = std::max(y_max, mm.second.y);
    }
    return {{x_min, y_min}, {x_max, y_max}};
}

std::pair<Shape::FurthestPoint, Shape::FurthestPoint> Shape::compute_furthest_points(
        Angle angle) const
{
    if (angle == 0.0) {
        Point point_min;
        point_min.x = std::numeric_limits<LengthDbl>::infinity();
        point_min.y = std::numeric_limits<LengthDbl>::infinity();
        ElementPos min_element_pos = -1;
        Point point_max;
        point_max.x = -std::numeric_limits<LengthDbl>::infinity();
        point_max.y = -std::numeric_limits<LengthDbl>::infinity();
        ElementPos max_element_pos = -1;
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)this->elements.size();
                ++element_pos) {
            const ShapeElement& element = this->elements[element_pos];
            auto mm = element.furthest_points(0.0);
            if (point_min.y > mm.first.y) {
                point_min = mm.first;
                min_element_pos = element_pos;
            }
            if (point_max.y < mm.second.y) {
                point_max = mm.second;
                max_element_pos = element_pos;
            }
        }
        return {{point_min, min_element_pos}, {point_max, max_element_pos}};
    }

    Shape shape_tmp = this->rotate(-angle);
    auto mm = shape_tmp.compute_furthest_points(0.0);
    return {
        {mm.first.point.rotate(angle), mm.first.element_pos},
        {mm.second.point.rotate(angle), mm.second.element_pos}};
}

bool Shape::contains(
        const Point& point,
        bool strict) const
{
    //std::cout << "contains strict " << strict << " point " << point.to_string() << std::endl;
    //std::cout << this->to_string(0) << std::endl;
    if (this->elements.empty())
        return false;

    // First check if the point lies on any boundary.
    for (const ShapeElement& element : this->elements)
        if (element.contains(point))
            return (strict)? false: true;

    // Then use the ray-casting algorithm to check if the point is inside
    ElementPos intersection_count = 0;
    for (const ShapeElement& element: this->elements) {
        //std::cout << "element " << element.to_string() << std::endl;
        if (element.type == ShapeElementType::LineSegment) {
            // Horizontal edges are excluded.
            if (equal(element.start.y, element.end.y))
                continue;

            // Check y.
            if (strictly_greater(point.y, element.start.y)
                    && strictly_greater(point.y, element.end.y)) {
                continue;
            }
            if (strictly_lesser(point.y, element.start.y)
                    && strictly_lesser(point.y, element.end.y)) {
                continue;
            }

            bool upward = (element.end.y > element.start.y);

            if (upward) {
                // An upward edge includes its starting endpoint, and excludes its
                // final endpoint;
                if (equal(point.y, element.start.y)) {
                    if (element.start.x > point.x) {
                        intersection_count++;
                        continue;
                    }
                } else if (equal(point.y, element.end.y)) {
                    continue;
                }
            } else {
                // A downward edge excludes its starting endpoint, and includes its final endpoint;
                if (equal(point.y, element.start.y)) {
                    continue;
                } else if (equal(point.y, element.end.y)) {
                    if (element.end.x > point.x) {
                        intersection_count++;
                        continue;
                    }
                }
            }

            LengthDbl x_inters = element.start.x
                + (point.y - element.start.y)
                * (element.end.x - element.start.x)
                / (element.end.y - element.start.y);
            //std::cout << "x_inters " << x_inters << std::endl;
            if (x_inters > point.x) {
                intersection_count++;
            }
        } else if (element.type == ShapeElementType::CircularArc) {
            LengthDbl radius = distance(element.center, element.start);
            ShapeElement ray;
            ray.type = ShapeElementType::LineSegment;
            ray.start.x = point.x;
            ray.start.y = point.y;
            ray.end.x = (std::max)(point.x, element.center.x) + 2 * radius;
            ray.end.y = point.y;

            std::vector<Point> intersections = compute_intersections(ray, element);
            for (const Point& intersection: intersections) {
                if (intersection.x < point.x)
                    continue;
                //std::cout << "intersection " << intersection.to_string() << std::endl;
                if (intersection == element.start) {
                    Angle start_angle = angle_radian(element.start - element.center);
                    //std::cout << "start_angle " << start_angle
                    //    << " M_PI / 2 " << M_PI / 2
                    //    << " 3 * M_PI / 2 " << 3 * M_PI / 2
                    //    << std::endl;
                    bool start_upward = (element.orientation == ShapeElementOrientation::Anticlockwise)?
                        (strictly_lesser(start_angle, M_PI / 2) || !strictly_lesser(start_angle, 3 * M_PI / 2)):
                        (strictly_greater(start_angle, M_PI / 2) && !strictly_greater(start_angle, 3 * M_PI / 2));
                    //std::cout << "start_upward " << start_upward << std::endl;
                    if (start_upward)
                        intersection_count++;
                }
                if (intersection == element.end) {
                    Angle end_angle = angle_radian(element.end - element.center);
                    bool end_upward = (element.orientation == ShapeElementOrientation::Anticlockwise)?
                        (strictly_lesser(end_angle, M_PI / 2) || !strictly_lesser(end_angle, 3 * M_PI / 2)):
                        (!strictly_lesser(end_angle, M_PI / 2) && strictly_lesser(end_angle, 3 * M_PI / 2));
                    //std::cout << "end_upward " << end_upward << std::endl;
                    if (!end_upward)
                        intersection_count++;
                }
                if (!(intersection == element.start)
                        && !(intersection == element.end)) {
                    //std::cout << "intersection_count++" << std::endl;
                    intersection_count++;
                }
            }
        }
    }

    // If the number of intersections is odd, the point is inside the shape
    //std::cout << "intersection_count " << intersection_count << std::endl;
    return (intersection_count % 2 == 1);
}

Shape& Shape::shift(
        LengthDbl x,
        LengthDbl y)
{
    for (ShapeElement& element: elements)
        element.shift(x, y);
    return *this;
}

Shape Shape::rotate(Angle angle) const
{
    Shape shape;
    shape.is_path = this->is_path;
    for (const ShapeElement& element: elements) {
        ShapeElement element_new = element.rotate(angle);
        shape.elements.push_back(element_new);
    }
    return shape;
}

Shape Shape::axial_symmetry_identity_line() const
{
    Shape shape;
    shape.is_path = this->is_path;
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        ShapeElement element_new = it->axial_symmetry_identity_line();
        shape.elements.push_back(element_new);
    }
    return shape;
}

Shape Shape::axial_symmetry_x_axis() const
{
    Shape shape;
    shape.is_path = this->is_path;
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        ShapeElement element_new = it->axial_symmetry_x_axis();
        shape.elements.push_back(element_new);
    }
    return shape;
}

Shape Shape::axial_symmetry_y_axis() const
{
    Shape shape;
    shape.is_path = this->is_path;
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        ShapeElement element_new = it->axial_symmetry_y_axis();
        shape.elements.push_back(element_new);
    }
    return shape;
}

Shape Shape::reverse() const
{
    Shape shape;
    shape.is_path = this->is_path;
    for (auto it = elements.rbegin(); it != elements.rend(); ++it)
        shape.elements.push_back(it->reverse());
    return shape;
}

std::pair<LengthDbl, LengthDbl> Shape::compute_width_and_height(
        Angle angle,
        bool mirror) const
{
    auto points = compute_min_max(angle, mirror);
    LengthDbl width = points.second.x - points.first.x;
    LengthDbl height = points.second.y - points.first.y;
    return {width, height};
}

bool Shape::check() const
{
    // Check that the shape is not empty.
    if (elements.empty())
        return false;

    // If the shape is not a path, check that it is closed.
    if (!this->is_path) {
        if (!equal(this->elements.back().end, this->elements.front().start)) {
            std::cout << this->to_string(1) << std::endl;
            std::cout << "shape is not closed." << std::endl;
            return false;
        }
    }

    // Check that the shape doesn't contain null angles.
    ElementPos element_prev_pos = this->elements.size() - 1;
    for (ElementPos element_cur_pos = 0;
            element_cur_pos < (ElementPos)this->elements.size();
            ++element_cur_pos) {
        const ShapeElement& element_prev = this->elements[element_prev_pos];
        const ShapeElement& element = this->elements[element_cur_pos];

        if (std::isnan(element.start.x)
                || std::isnan(element.start.y)
                || std::isnan(element.end.x)
                || std::isnan(element.end.y)
                || std::isnan(element.center.x)
                || std::isnan(element.center.y)) {
            std::cout << this->to_string(1) << std::endl;
            std::cout << "element_pos       " << element_cur_pos << std::endl;
            std::cout << "element       " << element.to_string() << std::endl;
            return false;
        }

        if (element_prev.type == ShapeElementType::LineSegment
                && element.type == ShapeElementType::LineSegment
                && !(element_prev.start == element_prev.end)
                && !(element.start == element.end)) {
            Angle angle = angle_radian(
                    element_prev.start - element_prev.end,
                    element.end - element.start);
            if (angle == 0) {
                std::cout << this->to_string(1) << std::endl;
                std::cout << "element_prev_pos  " << element_prev_pos << std::endl;
                std::cout << "element_pos       " << element_cur_pos << std::endl;
                std::cout << "element_prev  " << element_prev.to_string() << std::endl;
                std::cout << "element       " << element.to_string() << std::endl;
                std::cout << "angle  " << angle << std::endl;
                return false;
            }

            if (element_prev.start == element.end) {
                std::cout << this->to_string(1) << std::endl;
                std::cout << "element_pos       " << element_cur_pos << std::endl;
                std::cout << "element_prev_pos  " << element_prev_pos << std::endl;
                std::cout << "element       " << element.to_string() << std::endl;
                std::cout << "element_prev  " << element_prev.to_string() << std::endl;
                return false;
            }
        }

        element_prev_pos = element_cur_pos;
    }
    // TODO
    return true;
}

std::string Shape::to_string(
        Counter indentation) const
{
    std::string s = "";
    std::string indent = std::string(indentation, ' ');
    if (is_path) {
        s += "path (# elements " + std::to_string(elements.size()) + ")\n";
        for (Counter pos = 0; pos < (Counter)elements.size(); ++pos)
            s += indent + elements[pos].to_string() + ((pos < (Counter)elements.size() - 1)? "\n": "");
    } else if (is_circle()) {
        const Point& center = elements.front().center;
        LengthDbl radius = distance(center, elements.front().start);
        s += "circle (center: " + center.to_string() + "; radius: " + std::to_string(radius) + ")";
    } else if (is_square()) {
        s += "square (side: " + std::to_string(elements.front().length()) + ")";
    } else if (is_rectangle()) {
        s += "rectangle"
            " (side 1: " + std::to_string(elements[0].length())
            + "; side 2: " + std::to_string(elements[1].length()) + ")";
    } else if (is_polygon()) {
        s += "polygon (# edges " + std::to_string(elements.size()) + ")\n";
        for (Counter pos = 0; pos < (Counter)elements.size(); ++pos)
            s += indent + elements[pos].to_string() + ((pos < (Counter)elements.size() - 1)? "\n": "");
    } else {
        s += "shape (# elements " + std::to_string(elements.size()) + ")\n";
        for (Counter pos = 0; pos < (Counter)elements.size(); ++pos)
            s += indent + elements[pos].to_string() + ((pos < (Counter)elements.size() - 1)? "\n": "");
    }
    return s;
}

nlohmann::json Shape::to_json() const
{
    nlohmann::json json;
    json["is_path"] = this->is_path;
    json["type"] = "general";
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)this->elements.size();
            ++element_pos) {
        json["elements"][element_pos] = this->elements[element_pos].to_json();
    }
    return json;
}

Shape Shape::read_json(
        const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.good()) {
        throw std::runtime_error(
                "shape::Shape::read_json: "
                "unable to open file \"" + file_path + "\".");
    }

    nlohmann::json j;
    file >> j;
    return from_json(j);
}

void Shape::write_json(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json = this->to_json();

    file << std::setw(4) << json << std::endl;
}

std::string Shape::to_svg() const
{
    std::string s = "M";
    if (is_circle()) {
        const ShapeElement& element = elements.front();
        Point center = {element.center.x, -(element.center.y)};
        Point start = {element.start.x, -(element.start.y)};
        LengthDbl radius = distance(center, start);
        s += std::to_string(center.x - radius) + "," + std::to_string(center.y);
        s += "a" + std::to_string(radius) + ","
            + std::to_string(radius) + ",0,1,0,"
            + std::to_string(radius * 2) + ",0,";
        s += "a" + std::to_string(radius) + ","
            + std::to_string(radius) + ",0,1,0,"
            + std::to_string(-radius * 2) + ",0Z";
    } else {
        for (const ShapeElement& element: elements) {
            Point center = {element.center.x, -(element.center.y)};
            Point start = {element.start.x, -(element.start.y)};
            Point end = {element.end.x, -(element.end.y)};
            s += std::to_string(start.x) + "," + std::to_string(start.y);
            if (element.type == ShapeElementType::LineSegment) {
                s += "L";
            } else {
                LengthDbl radius = distance(center, start);
                Angle theta = angle_radian(start - center, end - center);
                int large_arc_flag = (theta > M_PI)? 0: 1;
                int sweep_flag = (element.orientation == ShapeElementOrientation::Anticlockwise)? 0: 1;
                s += "A" + std::to_string(radius) + ","
                    + std::to_string(radius) + ",0,"
                    + std::to_string(large_arc_flag) + ","
                    + std::to_string(sweep_flag) + ",";
            }
        }
        s += std::to_string(elements.front().start.x)
            + "," + std::to_string(-(elements.front().start.y))
            + "Z";
    }

    return s;
}

void Shape::write_svg(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }
    auto mm = compute_min_max(0.0);

    LengthDbl width = (mm.second.x - mm.first.x);
    LengthDbl height = (mm.second.y - mm.first.y);

    std::string s = "<svg viewBox=\""
        + std::to_string(mm.first.x)
        + " " + std::to_string(-mm.first.y - height)
        + " " + std::to_string(width)
        + " " + std::to_string(height)
        + "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    file << s;

    file << "<path d=\"" << to_svg() << "\""
        << " stroke=\"black\""
        << " stroke-width=\"0.1\""
        << " fill=\"blue\""
        << " fill-opacity=\"0.2\""
        << "/>" << std::endl;

    file << "</svg>" << std::endl;
}

Shape shape::build_triangle(
        const Point& p1,
        const Point& p2,
        const Point& p3)
{
    Shape shape;
    {
        ShapeElement element;
        element.type = ShapeElementType::LineSegment;
        element.start = p1;
        element.end = p2;
        shape.elements.push_back(element);
    }
    {
        ShapeElement element;
        element.type = ShapeElementType::LineSegment;
        element.start = p2;
        element.end = p3;
        shape.elements.push_back(element);
    }
    {
        ShapeElement element;
        element.type = ShapeElementType::LineSegment;
        element.start = p3;
        element.end = p1;
        shape.elements.push_back(element);
    }
    return shape;
}

Shape shape::operator*(
        LengthDbl scalar,
        const Shape& shape)
{
    Shape shape_new;
    shape_new.is_path = shape.is_path;
    for (const ShapeElement& element: shape.elements)
        shape_new.elements.push_back(scalar * element);
    return shape_new;
}

Shape shape::approximate_by_line_segments(
        const Shape& shape_orig,
        LengthDbl segment_length,
        bool outer)
{
    //std::cout << "approximate_by_line_segments" << std::endl;
    Shape shape = remove_redundant_vertices(shape_orig).second;

    Shape shape_new;
    shape_new.is_path = shape.is_path;
    for (const ShapeElement& element: shape.elements) {
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            shape_new.elements.push_back(element);
            break;
        } case ShapeElementType::CircularArc: {
            std::vector<ShapeElement> approximated_element
                = approximate_circular_arc_by_line_segments(
                        element,
                        segment_length,
                        outer);
            for (const ShapeElement& e: approximated_element)
                shape_new.elements.push_back(e);
            break;
        }
        }
    }

    shape_new = remove_redundant_vertices(shape_new).second;
    //std::cout << "approximate_by_line_segments end" << std::endl;
    return shape_new;
}

Shape shape::build_shape(
        const std::vector<BuildShapeElement>& points,
        bool is_path)
{
    Shape shape;
    Point point_prev = {points.front().x, points.front().y};
    ShapeElementType type = ShapeElementType::LineSegment;
    ShapeElementOrientation orientation = ShapeElementOrientation::Anticlockwise;
    Point center = {0, 0};
    for (ElementPos pos = 1; pos <= (ElementPos)points.size(); ++pos) {
        const BuildShapeElement& point = points[(pos != points.size())? pos: 0];
        if (is_path && pos == points.size())
            break;
        if (point.type == 0) {
            ShapeElement element;
            element.type = type;
            element.start = point_prev;
            element.end = {point.x, point.y};
            element.center = center;
            element.orientation = orientation;
            shape.elements.push_back(element);

            point_prev = element.end;
            orientation = ShapeElementOrientation::Anticlockwise;
            center = {0, 0};
            type = ShapeElementType::LineSegment;
        } else {
            orientation = (point.type == 1)?
                ShapeElementOrientation::Anticlockwise:
                ShapeElementOrientation::Clockwise;
            center = {point.x, point.y};
            type = ShapeElementType::CircularArc;
        }
    }
    shape.is_path = is_path;
    return shape;
}

Shape shape::build_path(
        const std::vector<BuildShapeElement>& points)
{
    return build_shape(points, true);
}

ShapeElement shape::build_line_segment(
        const BuildShapeElement& start,
        const BuildShapeElement& end)
{
    return build_shape({start, end}, true).elements.front();
}

ShapeElement shape::build_circular_arc(
        const BuildShapeElement& start,
        const BuildShapeElement& center,
        const BuildShapeElement& end)
{
    return build_shape({start, center, end}, true).elements.front();
}

bool ShapeWithHoles::contains(
        const Point& point,
        bool strict) const
{
    if (!this->shape.contains(point, strict))
        return false;
    for (const Shape& hole: this->holes)
        if (hole.contains(point, !strict))
            return false;
    return true;
}

Shape ShapeWithHoles::bridge_holes() const
{
    Shape shape = this->shape;

    auto mm = this->compute_min_max();

    // Check if a hole is already touching the outer boundary.
    std::vector<uint8_t> is_hole_processed(this->holes.size(), 0);
    IntersectionTree intersection_tree({}, this->shape.elements, {});
    ElementPos shape_contact_element_pos = -1;
    ElementPos hole_contact_element_pos = -1;
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)this->holes.size();
            ++hole_pos) {
        const Shape& hole = this->holes[hole_pos];
        for (ElementPos hole_element_pos = 0;
                hole_element_pos < (ElementPos)hole.elements.size();
                ++hole_element_pos) {
            const ShapeElement& hole_element = hole.elements[hole_element_pos];
            auto intersection_output = intersection_tree.intersect(hole_element, false);
            if (!intersection_output.element_ids.empty()) {
                hole_contact_element_pos = hole_element_pos;
                shape_contact_element_pos = intersection_output.element_ids.front();
                break;
            }
        }

        if (hole_contact_element_pos != -1) {
            const ShapeElement& hole_contact_element = hole.elements[hole_contact_element_pos];
            const ShapeElement& shape_contact_element = shape.elements[shape_contact_element_pos];

            Shape new_shape;
            for (ElementPos shape_element_pos = 0;
                    shape_element_pos < (ElementPos)shape.elements.size();
                    ++shape_element_pos) {
                const ShapeElement& shape_element = shape.elements[shape_element_pos];
                if (shape_element_pos != shape_contact_element_pos) {
                    new_shape.elements.push_back(shape_element);
                } else {
                    auto intersections = compute_intersections(
                            shape_contact_element,
                            hole_contact_element);
                    const Point& intersection = intersections.front();
                    auto p_shape = shape_element.split(intersection);
                    auto p_hole = hole_contact_element.reverse().split(intersection);
                    // Add first part of the shape element.
                    if (!equal(p_shape.first.start, p_shape.first.end))
                        new_shape.elements.push_back(p_shape.first);
                    // Add first part of the hole element.
                    if (!equal(p_hole.second.start, p_hole.second.end))
                        new_shape.elements.push_back(p_hole.second);
                    // Add hole.
                    for (ElementPos hole_element_pos_tmp = (ElementPos)hole.elements.size() - 1;
                            hole_element_pos_tmp >= 1;
                            --hole_element_pos_tmp) {
                        ElementPos hole_element_pos = (hole_contact_element_pos + hole_element_pos_tmp) % hole.elements.size();
                        const ShapeElement& hole_element = hole.elements[hole_element_pos];
                        new_shape.elements.push_back(hole_element.reverse());
                    }
                    // Add second part of the hole element.
                    if (!equal(p_hole.first.start, p_hole.first.end))
                        new_shape.elements.push_back(p_hole.first);
                    // Add second part of the element.
                    if (!equal(p_shape.second.start, p_shape.second.end))
                        new_shape.elements.push_back(p_shape.second);
                }
            }
            is_hole_processed[hole_pos] = 1;
            shape = new_shape;
        }
    }

    // For each hole, compute_the left-most point.
    std::vector<Shape::FurthestPoint> holes_left_most_points(this->holes.size());
    std::vector<ShapePos> sorted_holes;
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)this->holes.size();
            ++hole_pos) {
        const Shape& hole = this->holes[hole_pos];
        if (is_hole_processed[hole_pos])
            continue;
        holes_left_most_points[hole_pos] = hole.compute_furthest_points(90).second;
        sorted_holes.push_back(hole_pos);
    }

    // Sort holes by left-most point.
    std::iota(sorted_holes.begin(), sorted_holes.end(), 0);
    std::sort(
            sorted_holes.begin(),
            sorted_holes.end(),
            [&holes_left_most_points](
                ShapePos hole_pos_1,
                ShapePos hole_pos_2)
            {
                return holes_left_most_points[hole_pos_1].point.x
                    < holes_left_most_points[hole_pos_2].point.x;
            });

    for (ShapePos hole_pos: sorted_holes) {
        const Shape& hole = this->holes[hole_pos];
        //std::cout << "hole_pos " << hole_pos << std::endl;

        // Find point to connect the brige.
        LengthDbl y = holes_left_most_points[hole_pos].point.y;
        ShapeElement element_0;
        element_0.type = ShapeElementType::LineSegment;
        element_0.start.x = mm.first.x;
        element_0.start.y = y;
        element_0.end.x = holes_left_most_points[hole_pos].point.x;
        element_0.end.y = y;
        ElementPos shape_element_pos_best = -1;
        Point intersection_best = {0, 0};
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            auto shape_element_mm = shape_element.min_max();
            if (strictly_greater(shape_element_mm.first.y, y))
                continue;
            if (strictly_lesser(shape_element_mm.second.y, y))
                continue;
            auto intersections = compute_intersections(
                    element_0,
                    shape_element,
                    false);
            for (const Point& intersection: intersections) {
                if (shape_element_pos_best == -1
                        || strictly_lesser(intersection_best.x, intersection.x)) {
                    intersection_best = intersection;
                    shape_element_pos_best = shape_element_pos;
                }
            }
        }

        // Build new shape.
        Shape new_shape;
        for (ElementPos shape_element_pos = 0;
                shape_element_pos < (ElementPos)shape.elements.size();
                ++shape_element_pos) {
            const ShapeElement& shape_element = shape.elements[shape_element_pos];
            if (shape_element_pos != shape_element_pos_best) {
                new_shape.elements.push_back(shape_element);
            } else {
                ElementPos hole_contact_element_pos = holes_left_most_points[hole_pos].element_pos;
                const Point& hole_contact_point = holes_left_most_points[hole_pos].point;
                //std::cout << "hole_contact_point " << hole_contact_point.to_string() << std::endl;
                //std::cout << "intersection_best " << intersection_best.to_string() << std::endl;
                auto p_hole = hole.elements[hole_contact_element_pos].reverse().split(hole_contact_point);
                auto p_shape = shape_element.split(intersection_best);
                // Add first part of the element.
                if (!equal(p_shape.first.start, p_shape.first.end))
                    new_shape.elements.push_back(p_shape.first);
                // Add bridge.
                ShapeElement line_segment_1;
                line_segment_1.type = ShapeElementType::LineSegment;
                line_segment_1.start = intersection_best;
                line_segment_1.end = hole_contact_point;
                new_shape.elements.push_back(line_segment_1);
                // Add first part of the hole element.
                if (!equal(p_hole.second.start, p_hole.second.end))
                    new_shape.elements.push_back(p_hole.second);
                // Add hole.
                for (ElementPos hole_element_pos_tmp = (ElementPos)hole.elements.size() - 1;
                        hole_element_pos_tmp >= 1;
                        --hole_element_pos_tmp) {
                    ElementPos hole_element_pos = (hole_contact_element_pos + hole_element_pos_tmp) % hole.elements.size();
                    const ShapeElement& hole_element = hole.elements[hole_element_pos];
                    new_shape.elements.push_back(hole_element.reverse());
                }
                // Add second part of the hole element.
                if (!equal(p_hole.first.start, p_hole.first.end))
                    new_shape.elements.push_back(p_hole.first);
                // Add bridge.
                ShapeElement line_segment_2;
                line_segment_2.type = ShapeElementType::LineSegment;
                line_segment_2.start = hole_contact_point;
                line_segment_2.end = intersection_best;
                new_shape.elements.push_back(line_segment_2);
                // Add second part of the element.
                if (!equal(p_shape.second.start, p_shape.second.end))
                    new_shape.elements.push_back(p_shape.second);
            }
        }
        shape = new_shape;
    }

    return shape;
}

std::string ShapeWithHoles::to_string(
        Counter indentation) const
{
    std::string s = "";
    std::string indent = std::string(indentation, ' ');
    s += "shape " + shape.to_string(indentation) + "\n";
    if (holes.size() == 1) {
        s += indent + "- holes: " + holes.front().to_string(indentation + 2) + "\n";
    } else if (holes.size() >= 2) {
        s += indent + "- holes\n";
        for (const Shape& hole: holes)
            s += indent + "  - " + hole.to_string(indentation + 4) + "\n";
    }
    return s;
}

nlohmann::json ShapeWithHoles::to_json() const
{
    nlohmann::json json = this->shape.to_json();
    for (Counter hole_pos = 0;
            hole_pos < (Counter)this->holes.size();
            ++hole_pos) {
        const Shape& hole = this->holes[hole_pos];
        json["holes"][hole_pos] = hole.to_json();
    }
    return json;
}

ShapeWithHoles ShapeWithHoles::read_json(
        const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.good()) {
        throw std::runtime_error(
                "shape::ShapeWithHoles::read_json: "
                "unable to open file \"" + file_path + "\".");
    }

    nlohmann::json j;
    file >> j;
    return from_json(j);
}

void ShapeWithHoles::write_json(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json = this->to_json();

    file << std::setw(4) << json << std::endl;
}

std::string ShapeWithHoles::to_svg(
        const std::string& fill_color) const
{
    std::string s = "<path d=\"" + shape.to_svg();
    for (const Shape& hole: holes)
        s += hole.reverse().to_svg();
    s += "\""
        " stroke=\"black\""
        " stroke-width=\"0.1\"";
    if (!fill_color.empty()) {
        s += " fill=\"" + fill_color + "\""
            " fill-opacity=\"0.2\"";
    }
    s += "/>\n";
    return s;
}

void ShapeWithHoles::write_svg(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }

    auto mm = shape.compute_min_max(0.0);
    LengthDbl width = (mm.second.x - mm.first.x);
    LengthDbl height = (mm.second.y - mm.first.y);

    std::string s = "<svg viewBox=\""
        + std::to_string(mm.first.x)
        + " " + std::to_string(-mm.first.y - height)
        + " " + std::to_string(width)
        + " " + std::to_string(height)
        + "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    file << s;

    file << "<g>" << std::endl;
    file << to_svg();
    //file << "<text x=\"" << std::to_string(x)
    //    << "\" y=\"" << std::to_string(-y)
    //    << "\" dominant-baseline=\"middle\" text-anchor=\"middle\">"
    //    << std::to_string(item_shape_pos)
    //    << "</text>" << std::endl;
    file << "</g>" << std::endl;

    file << "</svg>" << std::endl;
}

ShapeWithHoles shape::operator*(
        LengthDbl scalar,
        const ShapeWithHoles& shape)
{
    ShapeWithHoles shape_new;
    shape_new.shape = scalar * shape.shape;
    for (const Shape& hole: shape.holes)
        shape_new.holes.push_back(scalar * hole);
    return shape_new;
}

void shape::write_json(
        const std::vector<ShapeWithHoles>& shapes,
        const std::vector<ShapeElement>& elements,
        const std::string& file_path)
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        json["shapes"][shape_pos] = shapes[shape_pos].to_json();
    }
    for (ElementPos element_pos = 0;
            element_pos < (ShapePos)elements.size();
            ++element_pos) {
        json["elements"][element_pos] = elements[element_pos].to_json();
    }

    file << std::setw(4) << json << std::endl;
}

std::pair<Point, Point> shape::compute_min_max(
    const std::vector<ShapeWithHoles>& shapes)
{
    LengthDbl x_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl x_max = -std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_max = -std::numeric_limits<LengthDbl>::infinity();
    for (const ShapeWithHoles& shape: shapes) {
        auto points = shape.compute_min_max();
        x_min = std::min(x_min, points.first.x);
        x_max = std::max(x_max, points.second.x);
        y_min = std::min(y_min, points.first.y);
        y_max = std::max(y_max, points.second.y);
    }
    return {{x_min, y_min}, {x_max, y_max}};
}

void shape::write_svg(
        const std::vector<ShapeWithHoles>& shapes,
        const std::string& file_path)
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                "Unable to open file \"" + file_path + "\".");
    }

    auto mm = compute_min_max(shapes);
    LengthDbl width = (mm.second.x - mm.first.x);
    LengthDbl height = (mm.second.y - mm.first.y);

    std::string s = "<svg viewBox=\""
        + std::to_string(mm.first.x)
        + " " + std::to_string(-mm.first.y - height)
        + " " + std::to_string(width)
        + " " + std::to_string(height)
        + "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    file << s;

    for (ShapePos shape_pos = 0;
             shape_pos < (ShapePos) shapes.size();
             ++shape_pos) {
        const auto& shape = shapes[shape_pos];
        file << "<g>" << std::endl;
        file << shape.to_svg("blue");
        file << "</g>" << std::endl;
    }

    file << "</svg>" << std::endl;
}

bool shape::operator==(
        const ShapeElement& element_1,
        const ShapeElement& element_2)
{
    if (element_1.type != element_2.type)
        return false;
    if (!(element_1.start == element_2.start))
        return false;
    if (!(element_1.end == element_2.end))
        return false;
    if (element_1.type == ShapeElementType::CircularArc) {
        if (!(element_1.center == element_2.center))
            return false;
        if (element_1.orientation != element_2.orientation)
            return false;
    }
    return true;
}

bool shape::equal(
        const ShapeElement& element_1,
        const ShapeElement& element_2)
{
    if (element_1.type != element_2.type)
        return false;
    if (element_1.type == ShapeElementType::CircularArc) {
        if (element_1.orientation != element_2.orientation)
            return false;
        if (!equal(element_1.center, element_2.center))
            return false;
        if (element_1.orientation == ShapeElementOrientation::Full) {
            LengthDbl radius_1 = distance(element_1.center, element_1.start);
            LengthDbl radius_2 = distance(element_2.center, element_2.start);
            return equal(radius_1, radius_2);
        }
    }
    if (!equal(element_1.start, element_2.start))
        return false;
    if (!equal(element_1.end, element_2.end))
        return false;
    return true;
}

bool shape::operator==(
        const Shape& shape_1,
        const Shape& shape_2)
{
    // First, check if both shapes have the same number of elements.
    if (shape_1.elements.size() != shape_2.elements.size())
        return false;

    ElementPos offset = -1;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape_2.elements.size();
            ++element_pos) {
        if (shape_2.elements[element_pos] == shape_1.elements[0]) {
            offset = element_pos;
            break;
        }
    }
    if (offset == -1)
        return false;

    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape_2.elements.size();
            ++element_pos) {
        ElementPos element_pos_2 = (element_pos + offset) % shape_2.elements.size();
        if (!(shape_1.elements[element_pos] == shape_2.elements[element_pos_2])) {
            return false;
        }
    }

    return true;
}

bool shape::equal(
        const Shape& shape_1,
        const Shape& shape_2)
{
    // First, check if both shapes have the same number of elements.
    if (shape_1.elements.size() != shape_2.elements.size())
        return false;

    ElementPos offset = -1;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape_2.elements.size();
            ++element_pos) {
        if (equal(shape_2.elements[element_pos], shape_1.elements[0])) {
            offset = element_pos;
            break;
        }
    }
    if (offset == -1)
        return false;

    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape_2.elements.size();
            ++element_pos) {
        ElementPos element_pos_2 = (element_pos + offset) % shape_2.elements.size();
        if (!equal(shape_1.elements[element_pos], shape_2.elements[element_pos_2])) {
            return false;
        }
    }

    return true;
}

bool shape::operator==(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2)
{
    if (!(shape_1.shape == shape_2.shape))
        return false;

    if (shape_1.holes.size() != shape_2.holes.size())
        return false;
    for (const Shape& hole_1: shape_1.holes) {
         if (std::find(
                    shape_2.holes.begin(),
                    shape_2.holes.end(),
                    hole_1) == shape_2.holes.end()) {
             return false;
         }
    }

    return true;
}

bool shape::equal(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2)
{
    if (!equal(shape_1.shape, shape_2.shape))
        return false;

    if (shape_1.holes.size() != shape_2.holes.size())
        return false;
    for (const Shape& hole_1: shape_1.holes) {
         if (std::find_if(
                    shape_2.holes.begin(),
                    shape_2.holes.end(),
                    [&hole_1](const Shape& hole) { return equal(hole, hole_1); })
                 == shape_2.holes.end()) {
             return false;
         }
    }

    return true;
}
