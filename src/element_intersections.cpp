#include "shape/element_intersections.hpp"

//#include <iostream>

using namespace shape;

std::pair<bool, Point> shape::compute_line_intersection(
        const Point& p11,
        const Point& p12,
        const Point& p21,
        const Point& p22)
{
    if (p11 == p21 || p11 == p22) {
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
        if (denom == 0.0)
            return {false, {0, 0}};
        return {true, p11};
    } else if (p12 == p21 || p12 == p22) {
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
        if (denom == 0.0)
            return {false, {0, 0}};
        return {true, p12};
    } else if (p11.x == p12.x) {
        if (p21.x == p22.x)
            return {false, {0, 0}};
        if (p21.y == p22.y)
            return {true, {p11.x, p21.y}};
        double a = (p22.y - p21.y) / (p22.x - p21.x);
        double b = p21.y - (p22.y - p21.y) * p21.x / (p22.x - p21.x);
        Point p;
        p.x = p11.x;
        p.y = (p22.y - p21.y) * p.x / (p22.x - p21.x) + b;
        if (equal(p.y, p21.y))
            p.y = p21.y;
        if (equal(p.y, p22.y))
            p.y = p22.y;
        return {true, p};
    } else if (p11.y == p12.y) {
        if (p21.y == p22.y)
            return {false, {0, 0}};
        if (p21.x == p22.x)
            return {true, {p21.x, p11.y}};
        double a = (p22.x - p21.x) / (p22.y - p21.y);
        double b = p21.x - a * p21.y;
        Point p;
        p.y = p11.y;
        p.x = a * p.y + b;
        if (equal(p.x, p21.x))
            p.x = p21.x;
        if (equal(p.x, p22.x))
            p.x = p22.x;
        return {true, p};
    } else if (p21.x == p22.x) {
        if (p11.x == p12.x)
            return {false, {0, 0}};
        double a = (p12.y - p11.y) / (p12.x - p11.x);
        double b = p11.y - a * p11.x;
        Point p;
        p.x = p21.x;
        p.y = a * p.x + b;
        if (equal(p.y, p11.y))
            p.y = p11.y;
        if (equal(p.y, p12.y))
            p.y = p12.y;
        return {true, p};
    } else if (p21.y == p22.y) {
        if (p11.y == p12.y)
            return {false, {0, 0}};
        double a = (p12.x - p11.x) / (p12.y - p11.y);
        double b = p11.x - a * p11.y;
        Point p;
        p.y = p21.y;
        p.x = a * p.y + b;
        if (equal(p.x, p11.x))
            p.x = p11.x;
        if (equal(p.x, p12.x))
            p.x = p12.x;
        return {true, p};
    } else {
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
        if (denom == 0.0)
            return {false, {0, 0}};
        Point p;
        p.x = ((p11.x * p12.y - p11.y * p12.x) * (p21.x - p22.x) - (p11.x - p12.x) * (p21.x * p22.y - p21.y * p22.x)) / denom;
        p.y = ((p11.x * p12.y - p11.y * p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x * p22.y - p21.y * p22.x)) / denom;

        if (equal(p, p11)) {
            p = p11;
        } else if (equal(p, p12)) {
            p = p12;
        } else if (equal(p, p21)) {
            p = p21;
        } else if (equal(p, p22)) {
            p = p22;
        }

        return {true, p};
    }
}

namespace
{

// Helper function to compute line-line intersections
std::vector<Point> compute_line_line_intersections(
        const ShapeElement& line1,
        const ShapeElement& line2,
        bool strict)
{
    auto p = compute_line_intersection(line1.start, line1.end, line2.start, line2.end);

    if (!p.first) {
        // If both line segments are colinear.
        if (strict)
            return {};

        // If they are colinear, check if they are aligned.
        LengthDbl x1 = line1.start.x;
        LengthDbl y1 = line1.start.y;
        LengthDbl x2 = line1.end.x;
        LengthDbl y2 = line1.end.y;
        LengthDbl x3 = line2.start.x;
        LengthDbl y3 = line2.start.y;
        LengthDbl x4 = line2.end.x;
        LengthDbl y4 = line2.end.y;
        LengthDbl denom_2 = (x1 - x2) * (y3 - y1) - (y1 - y2) * (x3 - x1);
        if (denom_2 != 0.0)
            return {};

        // If they are aligned, check if they overlap.
        std::array<ElementPos, 4> sorted_points = {0, 1, 2, 3};
        std::sort(
                sorted_points.begin(),
                sorted_points.end(),
                [&line1, &line2](
                    ElementPos point_pos_1,
                    ElementPos point_pos_2)
                {
                    const Point& point_1 =
                        (point_pos_1 == 0)? line1.start:
                        (point_pos_1 == 1)? line1.end:
                        (point_pos_1 == 2)? line2.start:
                        line2.end;
                    const Point& point_2 =
                        (point_pos_2 == 0)? line1.start:
                        (point_pos_2 == 1)? line1.end:
                        (point_pos_2 == 2)? line2.start:
                        line2.end;
                    if (point_1.x != point_2.x)
                        return point_1.x < point_2.x;
                    return point_1.y < point_2.y;
                });
        if (sorted_points[0] + sorted_points[1] == 1
                || sorted_points[0] + sorted_points[1] == 5) {
            return {};
        }

        // Return the two interior points.
        const Point& point_1 =
            (sorted_points[1] == 0)? line1.start:
            (sorted_points[1] == 1)? line1.end:
            (sorted_points[1] == 2)? line2.start:
            line2.end;
        const Point& point_2 =
            (sorted_points[2] == 0)? line1.start:
            (sorted_points[2] == 1)? line1.end:
            (sorted_points[2] == 2)? line2.start:
            line2.end;
        return {point_1, point_2};
    }

    if (strict) {
        if (p.second == line1.start
                || p.second == line1.end
                || p.second == line2.start
                || p.second == line2.end) {
            return {};
        }
    }

    if (line1.contains(p.second) && line2.contains(p.second))
        return {p.second};
    return {};
}

// Helper function to compute line-arc intersections
std::vector<Point> compute_line_arc_intersections(
        const ShapeElement& line,
        const ShapeElement& arc,
        bool strict)
{
    //std::cout << "line " << line.to_string() << std::endl;
    //std::cout << "arc " << arc.to_string() << std::endl;

    Point ps[2];
    if (line.start == arc.start
            || line.start == arc.end) {
        ps[0] = line.start;
        if (line.end == arc.start
                || line.end == arc.end) {
            ps[1] = line.end;
        } else {
            Point d = line.end - line.start;
            LengthDbl dd = d.x * d.x + d.y * d.y;
            LengthDbl dc = d.x * (ps[0].x - arc.center.x) + d.y * (ps[0].y - arc.center.y);
            LengthDbl t2 = -2 * dc / dd;
            //std::cout << "t2 " << t2 << std::endl;
            ps[1].x = ps[0].x + t2 * d.x;
            ps[1].y = ps[0].y + t2 * d.y;
        }
    } else if (line.end == arc.start
            || line.end == arc.end) {
        ps[0] = line.end;
        Point d = line.end - line.start;
        LengthDbl dd = d.x * d.x + d.y * d.y;
        LengthDbl dc = d.x * (ps[0].x - arc.center.x) + d.y * (ps[0].y - arc.center.y);
        LengthDbl t2 = -2 * dc / dd;
        //std::cout << "t2 " << t2 << std::endl;
        ps[1].x = ps[0].x + t2 * d.x;
        ps[1].y = ps[0].y + t2 * d.y;
    } else if (line.start.x == line.end.x) {
        LengthDbl radius = distance(arc.center, arc.start);
        LengthDbl dx = line.start.x - arc.center.x;
        LengthDbl diff = radius * radius - (dx * dx);
        if (strictly_lesser(diff, 0))
            return {};
        if (diff < 0)
            diff = 0;
        LengthDbl v = std::sqrt(diff);
        ps[0].x = line.start.x;
        ps[0].y = arc.center.y + v;
        ps[1].x = line.start.x;
        ps[1].y = arc.center.y - v;
    } else if (line.start.y == line.end.y) {
        LengthDbl radius = distance(arc.center, arc.start);
        LengthDbl dy = line.start.y - arc.center.y;
        LengthDbl diff = radius * radius - (dy * dy);
        if (strictly_lesser(diff, 0))
            return {};
        if (diff < 0)
            diff = 0;
        LengthDbl v = std::sqrt(diff);
        ps[0].x = arc.center.x + v;
        ps[0].y = line.start.y;
        ps[1].x = arc.center.x - v;
        ps[1].y = line.start.y;
    } else {

        // x (y1 - y2) + y (x2 - x1) + (x1 y2 - x2 y1) = 0
        LengthDbl xm = arc.center.x;
        LengthDbl ym = arc.center.y;
        LengthDbl a = line.start.y - line.end.y;
        LengthDbl b = line.end.x - line.start.x;
        LengthDbl c = line.end.x * line.start.y - line.start.x * line.end.y;
        LengthDbl rsq = squared_distance(arc.center, arc.start);
        LengthDbl c_prime = c - a * xm - b * ym;

        // No intersection.
        if (strictly_lesser(rsq * (a * a + b * b), c_prime * c_prime))
            return {};

        LengthDbl discriminant = rsq * (a * a + b * b) - c_prime * c_prime;
        //std::cout << "discriminant " << discriminant << std::endl;
        if (discriminant < 0)
            discriminant = 0;
        LengthDbl denom = a * a + b * b;
        LengthDbl eta_1 = (a * c_prime + b * std::sqrt(discriminant)) / denom;
        LengthDbl eta_2 = (a * c_prime - b * std::sqrt(discriminant)) / denom;
        LengthDbl teta_1 = (b * c_prime - a * std::sqrt(discriminant)) / denom;
        LengthDbl teta_2 = (b * c_prime + a * std::sqrt(discriminant)) / denom;
        ps[0].x = xm + eta_1;
        ps[0].y = ym + teta_1;
        ps[1].x = xm + eta_2;
        ps[1].y = ym + teta_2;
    }
    //std::cout << "p1 " << ps[0].to_string() << std::endl;
    //std::cout << "p2 " << ps[1].to_string() << std::endl;

    // Single intersection point.
    if (equal(ps[0], ps[1])) {
        if (strict)
            return {};

        Point p;
        p.x = (ps[0].x + ps[1].x) / 2.0;
        p.y = (ps[0].y + ps[1].y) / 2.0;
        //std::cout << "p " << p.to_string() << std::endl;

        if (equal(p, line.start)) {
            p = line.start;
        } else if (equal(p, line.end)) {
            p = line.end;
        } else if (equal(p, arc.start)) {
            p = arc.start;
        } else if (equal(p, arc.end)) {
            p = arc.end;
        }
        if (line.contains(p) && arc.contains(p)) {
            return {p};
        } else {
            return {};
        }
    }

    std::vector<Point> intersections;
    for (Point& p: ps) {
        // Check if any intersection coincides with an arc endpoint
        if (strict) {
            if (equal(p, line.start)) {
                continue;
            } else if (equal(p, line.end)) {
                continue;
            } else if (equal(p, arc.start)) {
                continue;
            } else if (equal(p, arc.end)) {
                continue;
            }
        } else {
            if (equal(p, line.start)) {
                p = line.start;
            } else if (equal(p, line.end)) {
                p = line.end;
            } else if (equal(p, arc.start)) {
                p = arc.start;
            } else if (equal(p, arc.end)) {
                p = arc.end;
            }
        }
        if (line.contains(p) && arc.contains(p))
            intersections.push_back(p);
    }

    return intersections;
}

// Helper function to compute arc-arc intersections
std::vector<Point> compute_arc_arc_intersections(
        const ShapeElement& arc,
        const ShapeElement& arc_2,
        bool strict)
{
    LengthDbl rsq = squared_distance(arc.center, arc.start);
    LengthDbl r2sq = squared_distance(arc_2.center, arc_2.start);
    if (equal(arc.center, arc_2.center)) {
        if (strict)
            return {};
        if (equal(rsq, r2sq)) {
            std::vector<Point> intersections;
            if (arc.contains(arc_2.start))
                intersections.push_back(arc_2.start);
            if (!equal(arc_2.end, arc_2.start)
                    && arc.contains(arc_2.end))
                intersections.push_back(arc_2.end);
            if (!equal(arc.start, arc_2.start)
                    && !equal(arc.start, arc_2.end)
                    && arc_2.contains(arc.start))
                intersections.push_back(arc.start);
            if (!equal(arc.end, arc_2.start)
                    && !equal(arc.end, arc_2.end)
                    && !equal(arc.end, arc.start)
                    && arc_2.contains(arc.end))
                intersections.push_back(arc.end);
            return intersections;
        } else {
            return {};
        }
    }

    LengthDbl xm = arc.center.x;
    LengthDbl ym = arc.center.y;
    LengthDbl xm2 = arc_2.center.x;
    LengthDbl ym2 = arc_2.center.y;
    LengthDbl a = 2 * (xm2 - xm);
    LengthDbl b = 2 * (ym2 - ym);
    LengthDbl c = rsq - (xm * xm) - (ym * ym)
        - r2sq + (xm2 * xm2) + (ym2 * ym2);
    //std::cout << "a " << a << " b " << b << " c " << c << std::endl;

    LengthDbl c_prime = c - a * xm - b * ym;

    // No intersection.
    if (strictly_lesser(rsq * (a * a + b * b), c_prime * c_prime))
        return {};

    std::vector<Point> intersections;
    LengthDbl discriminant = rsq * (a * a + b * b) - c_prime * c_prime;
    //std::cout << "discriminant " << discriminant << std::endl;
    if (discriminant < 0)
        discriminant = 0;
    LengthDbl denom = a * a + b * b;
    LengthDbl eta_1 = (a * c_prime + b * std::sqrt(discriminant)) / denom;
    LengthDbl eta_2 = (a * c_prime - b * std::sqrt(discriminant)) / denom;
    LengthDbl teta_1 = (b * c_prime - a * std::sqrt(discriminant)) / denom;
    LengthDbl teta_2 = (b * c_prime + a * std::sqrt(discriminant)) / denom;
    Point ps[2];
    ps[0].x = xm + eta_1;
    ps[0].y = ym + teta_1;
    ps[1].x = xm + eta_2;
    ps[1].y = ym + teta_2;
    //std::cout << "ps[0] " << ps[0].to_string() << std::endl;
    //std::cout << "ps[1] " << ps[1].to_string() << std::endl;

    // Single intersection point.
    if (equal(ps[0], ps[1])) {
        if (strict)
            return {};

        Point p;
        p.x = (ps[0].x + ps[1].x) / 2.0;
        p.y = (ps[0].y + ps[1].y) / 2.0;
        //std::cout << "p " << p.to_string() << std::endl;

        if (equal(p, arc.start)) {
            p = arc.start;
        } else if (equal(p, arc.end)) {
            p = arc.end;
        } else if (equal(p, arc_2.start)) {
            p = arc_2.start;
        } else if (equal(p, arc_2.end)) {
            p = arc_2.end;
        }
        if (arc.contains(p) && arc_2.contains(p)) {
            return {p};
        } else {
            return {};
        }
    }

    for (Point& p: ps) {
        //std::cout << "p " << p.to_string() << std::endl;
        // Check if any intersection coincides with an arc endpoint
        if (strict) {
            if (equal(p, arc.start)) {
                continue;
            } else if (equal(p, arc.end)) {
                continue;
            } else if (equal(p, arc_2.start)) {
                continue;
            } else if (equal(p, arc_2.end)) {
                continue;
            }
        } else {
            if (equal(p, arc.start)) {
                p = arc.start;
            } else if (equal(p, arc.end)) {
                p = arc.end;
            } else if (equal(p, arc_2.start)) {
                p = arc_2.start;
            } else if (equal(p, arc_2.end)) {
                p = arc_2.end;
            }
        }
        if (arc.contains(p) && arc_2.contains(p))
            intersections.push_back(p);
    }

    return intersections;
}

}

std::vector<Point> shape::compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2,
        bool strict)
{
    if (element_1.type == ShapeElementType::LineSegment
            && element_2.type == ShapeElementType::LineSegment) {
        // Line segment - Line segment intersection
        return compute_line_line_intersections(element_1, element_2, strict);
    } else if (element_1.type == ShapeElementType::LineSegment
            && element_2.type == ShapeElementType::CircularArc) {
        // Line segment - Circular arc intersection
        return compute_line_arc_intersections(element_1, element_2, strict);
    } else if (element_1.type == ShapeElementType::CircularArc
            && element_2.type == ShapeElementType::LineSegment) {
        return compute_line_arc_intersections(element_2, element_1, strict);
    } else if (element_1.type == ShapeElementType::CircularArc
            && element_2.type == ShapeElementType::CircularArc) {
        // Circular arc - Circular arc intersection
        return compute_arc_arc_intersections(element_1, element_2, strict);
    }

    throw std::invalid_argument("shape::compute_intersections: Invalid element types");
    return {};
}

bool shape::intersect(
        const Shape& shape)
{
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element_1 = shape.elements[element_pos];
        for (ElementPos element_pos_2 = element_pos + 2;
                element_pos_2 < (ElementPos)shape.elements.size();
                ++element_pos_2) {
            if (element_pos == 0
                    && element_pos_2 == shape.elements.size() - 1) {
                continue;
            }
            const ShapeElement& element_2 = shape.elements[element_pos_2];
            auto intersections = compute_intersections(
                    element_1,
                    element_2,
                    false);
            if (!intersections.empty()) {
                //write_json({{shape}}, {}, "intersect.json");
                //std::cout << "shape " << shape.to_string(2) << std::endl;
                //std::cout << "element " << element_pos << " " << element_1.to_string() << std::endl;
                //std::cout << "element " << element_pos_2 << " " << element_2.to_string() << std::endl;
                //for (const Point& intersection: intersections)
                //    std::cout << "intersection " << intersection.to_string() << std::endl;
                return true;
            }
        }
    }
    return false;
}

bool shape::intersect(
        const Shape& shape_1,
        const Shape& shape_2,
        bool strict)
{
    for (const ShapeElement& element_1: shape_1.elements) {
        for (const ShapeElement& element_2: shape_2.elements) {
            auto intersections = compute_intersections(
                    element_1,
                    element_2,
                    strict);
            if (!intersections.empty())
                return true;
        }
    }
    if (!shape_1.is_path
            && !shape_2.is_path
            && equal(shape_1, shape_2)) {
        return true;
    }
    if (!shape_2.is_path) {
        for (const ShapeElement& element_1: shape_1.elements) {
            Point middle = element_1.middle();
            if (shape_2.contains(middle, strict))
                return true;
        }
    }
    if (!shape_1.is_path) {
        for (const ShapeElement& element_2: shape_2.elements) {
            Point middle = element_2.middle();
            if (shape_1.contains(middle, strict))
                return true;
        }
    }

    return false;
}

bool shape::intersect(
        const Shape& shape,
        const ShapeElement& element,
        bool strict)
{
    for (const ShapeElement& shape_element: shape.elements) {
        auto intersections = compute_intersections(
                element,
                shape_element,
                strict);
        if (!intersections.empty())
            return true;
    }
    Point middle = element.middle();
    if (!shape.is_path)
        if (shape.contains(middle, strict))
            return true;
    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2,
        bool strict)
{
    for (const ShapeElement& element_1: shape_1.shape.elements) {
        for (const ShapeElement& element_2: shape_2.shape.elements) {
            auto intersections = compute_intersections(
                    element_1,
                    element_2,
                    strict);
            if (!intersections.empty())
                return true;
        }

        for (const Shape& hole_2: shape_2.holes) {
            for (const ShapeElement& element_2: hole_2.elements) {
                auto intersections = compute_intersections(
                        element_1,
                        element_2,
                        strict);
                if (!intersections.empty())
                    return true;
            }
        }
    }
    if (equal(shape_1, shape_2))
        return true;

    for (const Shape& hole_1: shape_1.holes) {
        for (const ShapeElement& element_1: hole_1.elements) {
            for (const ShapeElement& element_2: shape_2.shape.elements) {
                auto intersections = compute_intersections(
                        element_1,
                        element_2,
                        strict);
                if (!intersections.empty())
                    return true;
            }

            for (const Shape& hole_2: shape_2.holes) {
                for (const ShapeElement& element_2: hole_2.elements) {
                    auto intersections = compute_intersections(
                            element_1,
                            element_2,
                            strict);
                    if (!intersections.empty())
                        return true;
                }
            }
        }
    }

    // Check if shape_1 is in a hole of shape_2.
    for (const ShapeElement& element_1: shape_1.shape.elements) {
        Point middle = element_1.middle();
        for (const Shape& hole_2: shape_2.holes) {
            if (hole_2.contains(middle, strict))
                return false;
        }
    }

    // Check if shape_2 is in a hole of shape_1.
    for (const ShapeElement& element_2: shape_2.shape.elements) {
        Point middle = element_2.middle();
        for (const Shape& hole_1: shape_1.holes) {
            if (hole_1.contains(middle, strict))
                return false;
        }
    }

    for (const ShapeElement& element_1: shape_1.shape.elements) {
        Point middle = element_1.middle();
        if (shape_2.contains(middle, strict))
            return true;
    }
    for (const ShapeElement& element_2: shape_2.shape.elements) {
        Point middle = element_2.middle();
        if (shape_1.contains(middle, strict))
            return true;
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape_1,
        const Shape& shape_2,
        bool strict)
{
    //std::cout << "intersect ShapeWithHoles Shape" << std::endl;
    for (const ShapeElement& element_1: shape_1.shape.elements) {
        for (const ShapeElement& element_2: shape_2.elements) {
            auto intersections = compute_intersections(
                    element_1,
                    element_2,
                    strict);
            if (!intersections.empty()) {
                //std::cout << "element_1 " << element_1.to_string() << std::endl;
                //std::cout << "element_2 " << element_2.to_string() << std::endl;
                //for (const Point& intersection: intersections)
                //    std::cout << "intersection " << intersection.to_string() << std::endl;
                return true;
            }
        }
    }
    if (!shape_2.is_path && equal(shape_1.shape, shape_2)) {
        //std::cout << "equal" << std::endl;
        return true;
    }

    for (const Shape& hole_1: shape_1.holes) {
        for (const ShapeElement& element_1: hole_1.elements) {
            for (const ShapeElement& element_2: shape_2.elements) {
                auto intersections = compute_intersections(
                        element_1,
                        element_2,
                        strict);
                if (!intersections.empty()) {
                    //std::cout << "element_1 " << element_1.to_string() << std::endl;
                    //std::cout << "element_2 " << element_2.to_string() << std::endl;
                    //for (const Point& intersection: intersections)
                    //    std::cout << "intersection " << intersection.to_string() << std::endl;
                    return true;
                }
            }
        }
    }

    // Check if shape_2 is in a hole of shape_1.
    for (const ShapeElement& element_2: shape_2.elements) {
        Point middle = element_2.middle();
        for (const Shape& hole_1: shape_1.holes) {
            if (hole_1.contains(middle, strict)) {
                //std::cout << "shape_2 is in a hole of shape_1" << std::endl;
                return false;
            }
        }
    }

    if (!shape_2.is_path) {
        for (const ShapeElement& element_1: shape_1.shape.elements) {
            Point middle = element_1.middle();
            if (shape_2.contains(middle, strict)) {
                //std::cout << "element_1 " << element_1.to_string() << std::endl;
                //std::cout << "middle " << middle.to_string() << std::endl;
                return true;
            }
        }
    }
    for (const ShapeElement& element_2: shape_2.elements) {
        Point middle = element_2.middle();
        if (shape_1.contains(middle, strict)) {
            //std::cout << "element_2 " << element_2.to_string() << std::endl;
            //std::cout << "middle " << middle.to_string() << std::endl;
            return true;
        }
    }

    return false;
}

bool shape::intersect(
        const ShapeWithHoles& shape,
        const ShapeElement& element,
        bool strict)
{
    for (const ShapeElement& shape_element: shape.shape.elements) {
        auto intersections = compute_intersections(
                element,
                shape_element,
                strict);
        if (!intersections.empty())
            return true;
    }
    for (const Shape& hole: shape.holes) {
        for (const ShapeElement& shape_element: hole.elements) {
            auto intersections = compute_intersections(
                    element,
                    shape_element,
                    strict);
            if (!intersections.empty())
                return true;
        }
    }

    // Check if the shape element is inside a hole of the shape.
    Point middle = element.middle();
    for (const Shape& hole: shape.holes) {
        if (hole.contains(middle, strict))
            return false;
    }

    // Shape if the element is inside the shape.
    if (shape.contains(middle, strict))
        return true;

    return false;
}
