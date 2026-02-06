#include "shape/shape.hpp"

#include "shape/elements_intersections.hpp"
#include "shape/shapes_intersections.hpp"

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

nlohmann::json Point::to_json() const
{
    nlohmann::json json;
    json["x"] = this->x;
    json["y"] = this->y;
    return json;
}

std::string Point::to_svg() const
{
    std::string s = "<path d=\"";
    s += "M" + std::to_string(this->x) + "," + std::to_string(this->y);
    s += "\""
        " stroke=\"black\""
        " stroke-width=\"0.1\"";
    //if (!fill_color.empty()) {
    //    s += " fill=\"" + fill_color + "\""
    //        " fill-opacity=\"0.2\"";
    //}
    s += "/>\n";
    return s;
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

Point shape::operator/(
        const Point& point,
        LengthDbl scalar)
{
    return {point.x / scalar, point.y / scalar};
}

Point shape::normalize(
        const Point& vector)
{
    return vector / norm(vector);
}

LengthDbl shape::squared_norm(
        const Point& vector)
{
    return vector.x * vector.x + vector.y * vector.y;
}

LengthDbl shape::squared_distance(
        const Point& point_1,
        const Point& point_2)
{
    return squared_norm(point_2 - point_1);
}

LengthDbl shape::distance_point_to_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2)
{
    return std::abs(
            (line_point_2.y - line_point_1.y) * point.x
            - (line_point_2.x - line_point_1.x) * point.y
            + line_point_2.x * line_point_1.y
            - line_point_2.y * line_point_1.x)
        / distance(line_point_1, line_point_2);
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

Point Point::rotate(
        const Point& center,
        Angle angle) const
{
    return center + (*this - center).rotate(angle);
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

Point ShapeElement::middle(
            const Point& point_1,
            const Point& point_2) const
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        Point point;
        point.x = (point_1.x + point_2.x) / 2;
        point.y = (point_1.y + point_2.y) / 2;
        return point;
    } case ShapeElementType::CircularArc: {
        if (this->orientation == ShapeElementOrientation::Anticlockwise) {
            Angle angle = angle_radian(
                    point_1 - this->center,
                    point_2 - this->center);
            return point_1.rotate_radians(this->center, angle / 2.);
        } else {
            Angle angle = angle_radian(
                this->end - this->center,
                this->start - this->center);
            return point_2.rotate_radians(this->center, angle / 2.);
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
            if (point_max.y < this->start.y)
                point_max = this->start;
            if (point_max.y < this->end.y)
                point_max = this->end;
            if (point_min.y > this->start.y)
                point_min = this->start;
            if (point_min.y > this->end.y)
                point_min = this->end;
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

void ShapeElement::recompute_center()
{
    if (this->type != ShapeElementType::CircularArc) {
        throw std::invalid_argument(FUNC_SIGNATURE);
    }
    Point middle = (this->start + this->end) / 2;
    Point diff = (this->end - this->start);
    Point normal = {-diff.y, diff.x};
    LengthDbl t = dot_product((this->center - middle), normal) / dot_product(normal, normal);
    this->center = middle + t * normal;
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

std::string Jet::to_string() const
{
    return "tangent_angle " + shape::to_string(this->tangent_angle)
        + " curvature " + shape::to_string(this->curvature);
}

Jet shape::operator-(
        const Jet& jet_1,
        const Jet& jet_2)
{
    Jet jet;
    jet.tangent_angle = jet_1.tangent_angle - jet_2.tangent_angle;
    jet.curvature = jet_1.curvature - jet_2.curvature;
    if (strictly_lesser(jet.tangent_angle, 0.0))
        jet.tangent_angle += 2 * M_PI;
    return jet;
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

LengthDbl ShapeElement::length() const
{
    switch (this->type) {
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

LengthDbl ShapeElement::length(const Point& point) const
{
    switch (this->type) {
    case ShapeElementType::LineSegment:
        return distance(this->start, point);
    case ShapeElementType::CircularArc:
        LengthDbl r = distance(this->center, this->start);
        if (this->orientation == ShapeElementOrientation::Anticlockwise
                || this->orientation == ShapeElementOrientation::Full) {
            return angle_radian(this->start - this->center, point - this->center) * r;
        } else {
            return angle_radian(point - this->center, this->start - this->center) * r;
        }
    }
    return -1;
}

Point ShapeElement::point(LengthDbl length) const
{
    switch (this->type) {
    case ShapeElementType::LineSegment: {
        return this->start + length / this->length() * (this->end - this->start);
    } case ShapeElementType::CircularArc: {
        Angle theta = angle_radian(this->start - this->center, this->end - this->center);
        return this->start.rotate_radians(
                this->center,
                length / this->length() * theta);
    } default: {
        throw std::invalid_argument(FUNC_SIGNATURE);
        return {0, 0};
    }
    }
    return {0, 0};
}

Point ShapeElement::find_point_between(
        const Point& point_1,
        const Point& point_2) const
{
    LengthDbl l1 = this->length(point_1);
    LengthDbl l2 = this->length(point_2);
    return this->point((l1 + l2) / 2);
}

Jet ShapeElement::jet(
        const Point& point,
        bool reverse) const
{
    switch (type) {
    case ShapeElementType::LineSegment: {
        Jet jet;
        if (!reverse) {
            jet.tangent_angle = angle_radian(this->end - this->start);
        } else {
            jet.tangent_angle = angle_radian(this->start - this->end);
        }
        jet.curvature = 0;
        return jet;
    } case ShapeElementType::CircularArc: {
        LengthDbl radius = distance(this->center, point);
        Point p = point - this->center;
        Jet jet;
        if ((this->orientation != ShapeElementOrientation::Clockwise
                    && !reverse)
                || (this->orientation == ShapeElementOrientation::Clockwise
                    && reverse)) {
            jet.tangent_angle = angle_radian({-p.y, p.x});
            jet.curvature = 1.0 / radius;
        } else {
            jet.tangent_angle = angle_radian({p.y, -p.x});
            jet.curvature = -1.0 / radius;
        }
        return jet;
    }
    }
    return {};
}

bool ShapeElement::same_direction(
        const ShapeElement& overlapping_part) const
{
    if (this->orientation == ShapeElementOrientation::Full) {
        return (overlapping_part.orientation == ShapeElementOrientation::Full
            || overlapping_part.orientation == ShapeElementOrientation::Anticlockwise);
    }
    return this->length(overlapping_part.start) < this->length(overlapping_part.end);
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
    json["start"] = start.to_json();
    json["end"] = end.to_json();
    if (type == ShapeElementType::CircularArc) {
        json["center"] = center.to_json();
        json["orientation"] = orientation2str(orientation);
    }
    return json;
}

std::string ShapeElement::to_svg() const
{
    std::string s = "<path d=\"";
    s += "M";
    if (this->type == ShapeElementType::CircularArc
            && this->orientation == ShapeElementOrientation::Full) {
        Point center = {this->center.x, -(this->center.y)};
        Point start = {this->start.x, -(this->start.y)};
        LengthDbl radius = distance(center, start);
        s += std::to_string(center.x - radius) + "," + std::to_string(center.y);
        s += "a" + std::to_string(radius) + ","
            + std::to_string(radius) + ",0,1,0,"
            + std::to_string(radius * 2) + ",0,";
        s += "a" + std::to_string(radius) + ","
            + std::to_string(radius) + ",0,1,0,"
            + std::to_string(-radius * 2) + ",0Z";
    } else {
        Point center = {this->center.x, -(this->center.y)};
        Point start = {this->start.x, -(this->start.y)};
        Point end = {this->end.x, -(this->end.y)};
        s += std::to_string(start.x) + "," + std::to_string(start.y);
        if (this->type == ShapeElementType::LineSegment) {
            s += "L";
        } else {
            LengthDbl radius = distance(center, start);
            Angle theta = angle_radian(start - center, end - center);
            int large_arc_flag = (theta > M_PI)? 0: 1;
            int sweep_flag = (this->orientation == ShapeElementOrientation::Anticlockwise)? 0: 1;
            s += "A" + std::to_string(radius) + ","
                + std::to_string(radius) + ",0,"
                + std::to_string(large_arc_flag) + ","
                + std::to_string(sweep_flag) + ",";
        }
        s += std::to_string(this->end.x)
            + "," + std::to_string(-(this->end.y));
    }
    s += "\""
        " stroke=\"black\""
        " stroke-width=\"0.1\""
        " fill=\"none\"";
    s += "/>\n";
    return s;
}

void ShapeElement::write_svg(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
    }
    auto mm = this->min_max();

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
    element_out.start = this->end;
    element_out.end = this->start;
    if (this->type == ShapeElementType::CircularArc) {
        element_out.center = this->center;
        element_out.orientation = opposite(this->orientation);
    }
    return element_out;
}

ShapeElement shape::build_line_segment(
        const Point& start,
        const Point& end)
{
    ShapeElement element;
    element.type = ShapeElementType::LineSegment;
    element.start = start;
    element.end = end;
    return element;
}

ShapeElement shape::build_circular_arc(
        const Point& start,
        const Point& end,
        const Point& center,
        const ShapeElementOrientation& orientation)
{
    ShapeElement element;
    element.type = ShapeElementType::CircularArc;
    element.start = start;
    element.end = end;
    element.center = center;
    element.orientation = orientation;
    return element;
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
        throw std::invalid_argument(FUNC_SIGNATURE);
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
        throw std::invalid_argument(FUNC_SIGNATURE);
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
            && elements.front().type == ShapeElementType::CircularArc
            && elements.front().orientation == ShapeElementOrientation::Full);
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
        if (!equal(theta, 3 * M_PI / 2))
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
        if (!equal(theta, 3 * M_PI / 2))
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
            if (element.orientation == ShapeElementOrientation::Anticlockwise) {
                Angle theta = angle_radian(element.center - element.start, element.center - element.end);
                area += radius * radius * (theta - std::sin(theta));
            } else {
                Angle theta = angle_radian(element.center - element.end, element.center - element.start);
                area -= radius * radius * (theta - std::sin(theta));
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

            ShapeElementIntersectionsOutput intersections = compute_intersections(ray, element);
            for (const Point& intersection: intersections.proper_intersections) {
                if (intersection.x < point.x)
                    continue;
                intersection_count++;
            }
            for (const Point& intersection: intersections.improper_intersections) {
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

Point Shape::find_point_strictly_inside() const
{
    auto mm = this->compute_min_max();
    for (Counter k = 2; k < 8; ++k) {
        for (Counter k2 = 1; k2 < k; ++k2) {
            LengthDbl y = mm.first.y + (mm.second.y - mm.first.y) * k2 / k;
            Point point_min_1 = {
                std::numeric_limits<LengthDbl>::infinity(),
                std::numeric_limits<LengthDbl>::infinity()};
            Point point_min_2 = {
                std::numeric_limits<LengthDbl>::infinity(),
                std::numeric_limits<LengthDbl>::infinity()};
            ShapeElement ray;
            ray.type = ShapeElementType::LineSegment;
            ray.start.x = mm.first.x - 1;
            ray.start.y = y;
            ray.end.x = mm.second.x + 1;
            ray.end.y = y;
            for (ElementPos element_pos = 0;
                    element_pos < this->elements.size();
                    ++element_pos) {
                const ShapeElement& element = this->elements[element_pos];
                ShapeElementIntersectionsOutput intersections = compute_intersections(ray, element);
                for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
                    const Point& intersection = (overlapping_part.start.x < overlapping_part.end.x)?
                        overlapping_part.start:
                        overlapping_part.end;
                    if (!strictly_lesser(point_min_1.x, intersection.x)) {
                        point_min_2 = intersection;
                        point_min_1 = intersection;
                    } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                        point_min_2 = intersection;
                    }
                }
                for (const Point& intersection: intersections.improper_intersections) {
                    if (!strictly_lesser(point_min_1.x, intersection.x)) {
                        point_min_2 = point_min_1;
                        point_min_1 = intersection;
                    } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                        point_min_2 = intersection;
                    }
                }
                for (const Point& intersection: intersections.proper_intersections) {
                    if (!strictly_lesser(point_min_1.x, intersection.x)) {
                        point_min_2 = point_min_1;
                        point_min_1 = intersection;
                    } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                        point_min_2 = intersection;
                    }
                }
            }
            if (equal(point_min_1.x, point_min_2.x))
                continue;
            return {(point_min_1.x + point_min_2.x) / 2, y};
        }
    }
    return this->elements.front().start;
}

bool Shape::is_strictly_closer_to_path_start(
        const ShapePoint& point_1,
        const ShapePoint& point_2) const
{
    if (point_1.element_pos != point_2.element_pos)
        return point_1.element_pos < point_2.element_pos;
    const ShapeElement& element = this->elements[point_1.element_pos];
    return strictly_lesser(element.length(point_1.point), element.length(point_2.point));
}

ShapePoint Shape::find_point_between(
        const ShapePoint& point_1,
        const ShapePoint& point_2) const
{
    const ShapeElement& element_1 = this->elements[point_1.element_pos];
    if (point_1.element_pos == point_2.element_pos) {
        return {point_1.element_pos, element_1.find_point_between(point_1.point, point_2.point)};
    } else if (!equal(point_1.point, element_1.end)) {
        return {point_1.element_pos, element_1.end};
    } else {
        const ShapeElement& element_next = this->elements[point_1.element_pos + 1];
        LengthDbl l_next = (point_2.element_pos == point_1.element_pos + 1)?
            element_next.length(point_2.point):
            element_next.length();
        return {point_1.element_pos + 1, element_next.point(l_next / 2)};
    }
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

std::vector<Shape> Shape::split(const std::vector<ShapePoint>& points) const
{
    std::vector<Shape> output;

    std::vector<ShapePoint> sorted_points = points;
    std::sort(
            sorted_points.begin(),
            sorted_points.end(),
            [this](
                const ShapePoint& point_1,
                const ShapePoint& point_2)
            {
                return is_strictly_closer_to_path_start(point_1, point_2);
            });

    const ShapePoint& point_first = sorted_points.front();
    const ShapePoint& point_last = sorted_points.back();
    if (this->is_path) {
        // For a path, create two paths from start to first point and from last
        // point to end.
        {
            Shape path_1;
            path_1.is_path = true;
            for (ElementPos element_pos = 0;
                    element_pos < point_first.element_pos;
                    ++element_pos) {
                const ShapeElement& element = this->elements[element_pos];
                path_1.elements.push_back(element);
            }
            path_1.elements.push_back(this->elements[point_first.element_pos].split(point_first.point).first);
            output.push_back(path_1);
        }

        {
            Shape path_2;
            path_2.is_path = true;
            path_2.elements.push_back(this->elements[point_last.element_pos].split(point_last.point).second);
            for (ElementPos element_pos = point_last.element_pos + 1;
                    element_pos < (ElementPos)this->elements.size();
                    ++element_pos) {
                const ShapeElement& element = this->elements[element_pos];
                path_2.elements.push_back(element);
            }
            output.push_back(path_2);
        }

    } else {
        // For a shape, create a path from last point to first point.
        Shape path;
        path.is_path = true;
        path.elements.push_back(this->elements[point_last.element_pos].split(point_last.point).second);
        for (ElementPos element_pos = point_last.element_pos + 1;
                element_pos < (ElementPos)this->elements.size();
                ++element_pos) {
            const ShapeElement& element = this->elements[element_pos];
            path.elements.push_back(element);
        }
        for (ElementPos element_pos = 0;
                element_pos < point_first.element_pos;
                ++element_pos) {
            const ShapeElement& element = this->elements[element_pos];
            path.elements.push_back(element);
        }
        path.elements.push_back(this->elements[point_first.element_pos].split(point_first.point).first);
        output.push_back(path);
    }

    // Create intermediate paths.
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)sorted_points.size() - 1;
            ++point_pos) {
        const ShapePoint& point_start = sorted_points[point_pos];
        const ShapePoint& point_end = sorted_points[point_pos + 1];

        Shape path;
        path.is_path = true;
        path.elements.push_back(this->elements[point_start.element_pos].split(point_start.point).second);
        for (ElementPos element_pos = point_start.element_pos + 1;
                element_pos < point_end.element_pos;
                ++element_pos) {
            const ShapeElement& element = this->elements[element_pos];
            path.elements.push_back(element);
        }
        path.elements.push_back(this->elements[point_end.element_pos].split(point_end.point).first);
        output.push_back(path);
    }

    return output;
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

    if (intersect(*this)) {
        std::cout << this->to_string(1) << std::endl;
        std::cout << "shape self intersect." << std::endl;
        return false;
    }

    AreaDbl area = this->compute_area();
    if (strictly_lesser(area, 0)) {
        std::cout << this->to_string(1) << std::endl;
        std::cout << "negative area: " << area << "." << std::endl;
        return false;
    }

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
                FUNC_SIGNATURE + ": "
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
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json = this->to_json();

    file << std::setw(4) << json << std::endl;
}

std::string Shape::to_svg_path() const
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
            + "," + std::to_string(-(elements.front().start.y));
        if (!this->is_path)
            s += "Z";
    }

    return s;
}

std::string Shape::to_svg(
            const std::string& fill_color) const
{
    std::string s = "<path d=\"" + this->to_svg_path();
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

void Shape::write_svg(
        const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
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
    shape.elements = std::vector<ShapeElement>(3);
    shape.elements[0] = build_line_segment(p1, p2);
    shape.elements[1] = build_line_segment(p2, p3);
    shape.elements[2] = build_line_segment(p3, p1);
    return shape;
}

Shape shape::build_square(
        LengthDbl size_length)
{
    Shape shape;
    shape.elements = std::vector<ShapeElement>(4);
    shape.elements[0] = build_line_segment({0, 0}, {size_length, 0});
    shape.elements[1] = build_line_segment({size_length, 0}, {size_length, size_length});
    shape.elements[2] = build_line_segment({size_length, size_length}, {0, size_length});
    shape.elements[3] = build_line_segment({0, size_length}, {0, 0});
    return shape;
}

Shape shape::build_rectangle(
        const Point& p1,
        const Point& p2)
{
    Shape shape;
    shape.elements = std::vector<ShapeElement>(4);
    shape.elements[0] = build_line_segment(p1, {p2.x, p1.y});
    shape.elements[1] = build_line_segment({p2.x, p1.y}, p2);
    shape.elements[2] = build_line_segment(p2, {p1.x, p2.y});
    shape.elements[3] = build_line_segment({p1.x, p2.y}, p1);
    return shape;
}

Shape shape::build_circle(LengthDbl radius)
{
    Shape shape;
    ShapeElement element;
    element.type = ShapeElementType::CircularArc;
    element.start.x = radius;
    element.start.y = 0;
    element.end.x = radius;
    element.end.y = 0;
    element.center.x = 0;
    element.center.y = 0;
    element.orientation = ShapeElementOrientation::Full;
    shape.elements.push_back(element);
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

Shape shape::build_shape(
        const std::vector<ShapeElement>& elements)
{
    Shape shape;
    shape.elements = elements;
    return shape;
}

Shape shape::build_path(
        const std::vector<BuildShapeElement>& points)
{
    return build_shape(points, true);
}

Shape shape::build_path(
        const std::vector<ShapeElement>& elements)
{
    Shape shape;
    shape.elements = elements;
    shape.is_path = true;
    return shape;
}

AreaDbl ShapeWithHoles::compute_area() const
{
    AreaDbl area = this->shape.compute_area();
    for (const Shape& hole: this->holes)
        area -= hole.compute_area();
    return area;
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

Point ShapeWithHoles::find_point_strictly_inside() const
{
    auto mm = this->compute_min_max();
    for (Counter k = 2; k < 8; ++k) {
        for (Counter k2 = 1; k2 < k; ++k2) {
            LengthDbl y = mm.first.y + (mm.second.y - mm.first.y) * k2 / k;
            Point point_min_1 = {
                std::numeric_limits<LengthDbl>::infinity(),
                std::numeric_limits<LengthDbl>::infinity()};
            Point point_min_2 = {
                std::numeric_limits<LengthDbl>::infinity(),
                std::numeric_limits<LengthDbl>::infinity()};
            ShapeElement ray;
            ray.type = ShapeElementType::LineSegment;
            ray.start.x = mm.first.x - 1;
            ray.start.y = y;
            ray.end.x = mm.second.x + 1;
            ray.end.y = y;
            for (ShapePos shape_pos = -1;
                    shape_pos < (ShapePos)this->holes.size();
                    ++shape_pos) {
                const Shape& shape = (shape_pos == -1)?
                    this->shape:
                    this->holes[shape_pos];
                for (ElementPos element_pos = 0;
                        element_pos < shape.elements.size();
                        ++element_pos) {
                    const ShapeElement& element = shape.elements[element_pos];
                    ShapeElementIntersectionsOutput intersections = compute_intersections(ray, element);
                    for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
                        const Point& intersection = (overlapping_part.start.x < overlapping_part.end.x)?
                            overlapping_part.start:
                            overlapping_part.end;
                        if (!strictly_lesser(point_min_1.x, intersection.x)) {
                            point_min_2 = intersection;
                            point_min_1 = intersection;
                        } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                            point_min_2 = intersection;
                        }
                    }
                    for (const Point& intersection: intersections.improper_intersections) {
                        if (!strictly_lesser(point_min_1.x, intersection.x)) {
                            point_min_2 = point_min_1;
                            point_min_1 = intersection;
                        } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                            point_min_2 = intersection;
                        }
                    }
                    for (const Point& intersection: intersections.proper_intersections) {
                        if (!strictly_lesser(point_min_1.x, intersection.x)) {
                            point_min_2 = point_min_1;
                            point_min_1 = intersection;
                        } else if (!strictly_lesser(point_min_2.x, intersection.x)) {
                            point_min_2 = intersection;
                        }
                    }
                }
            }
            if (equal(point_min_1.x, point_min_2.x))
                continue;
            return {(point_min_1.x + point_min_2.x) / 2, y};
        }
    }
    return this->shape.elements.front().start;
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
                FUNC_SIGNATURE + ": "
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
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json = this->to_json();

    file << std::setw(4) << json << std::endl;
}

std::string ShapeWithHoles::to_svg(
        const std::string& fill_color) const
{
    std::string s = "<path d=\"" + shape.to_svg_path();
    for (const Shape& hole: holes)
        s += hole.reverse().to_svg_path();
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
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
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

bool shape::operator==(
        const Jet& jet_1,
        const Jet& jet_2)
{
    if (jet_1.tangent_angle != jet_2.tangent_angle)
        return false;
    if (jet_1.curvature != jet_2.curvature)
        return false;
    return true;
}

bool shape::operator<(
        const Jet& jet_1,
        const Jet& jet_2)
{
    if (equal(jet_1.tangent_angle, 0) && strictly_lesser(jet_1.curvature, 0)) {
        if (!equal(jet_2.tangent_angle, 0)
                || (equal(jet_2.tangent_angle, 0) && !strictly_lesser(jet_2.curvature, 0))) {
            return false;
        } else {
            return strictly_greater(jet_1.curvature, jet_2.curvature);
        }
    } else if (equal(jet_2.tangent_angle, 0) && strictly_lesser(jet_2.curvature, 0)) {
        return true;
    }
    if (!equal(jet_1.tangent_angle, jet_2.tangent_angle))
        return jet_1.tangent_angle < jet_2.tangent_angle;
    return strictly_lesser(jet_1.curvature, jet_2.curvature);
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
