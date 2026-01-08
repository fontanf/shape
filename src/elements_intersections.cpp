#include "shape/elements_intersections.hpp"

//#include <iostream>

using namespace shape;

std::pair<bool, Point> shape::compute_line_intersection(
        const Point& p11,
        const Point& p12,
        const Point& p21,
        const Point& p22)
{
    if (equal(
                signed_distance_point_to_line(p11, p21, p22),
                signed_distance_point_to_line(p12, p21, p22))) {
        return {false, {0, 0}};
    } else if (p11 == p21 || p11 == p22) {
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
        if (equal(denom, 0.0))
            return {false, {0, 0}};
        return {true, p11};
    } else if (p12 == p21 || p12 == p22) {
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
        if (equal(denom, 0.0))
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
    } else if (p21.x == p22.x) {
        if (p11.x == p12.x)
            return {false, {0, 0}};
        double a = (p12.y - p11.y) / (p12.x - p11.x);
        double b = p11.y - a * p11.x;
        Point p;
        p.x = p21.x;
        p.y = a * p.x + b;

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
    } else if (p21.y == p22.y) {
        if (p11.y == p12.y)
            return {false, {0, 0}};
        double a = (p12.x - p11.x) / (p12.y - p11.y);
        double b = p11.x - a * p11.y;
        Point p;
        p.y = p21.y;
        p.x = a * p.y + b;

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
    } else if (equal(signed_distance_point_to_line(p11, p21, p22), 0.0)) {
        if (equal(signed_distance_point_to_line(p12, p21, p22), 0.0)) {
            return {false, {0, 0}};
        } else {
            return {true, p11};
        }
    } else {
        Point p;
        LengthDbl denom = (p11.x - p12.x) * (p21.y - p22.y) - (p11.y - p12.y) * (p21.x - p22.x);
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
ShapeElementIntersectionsOutput compute_line_line_intersections(
        const ShapeElement& line1,
        const ShapeElement& line2)
{
    auto p = compute_line_intersection(line1.start, line1.end, line2.start, line2.end);
    //std::cout << p.first << " " << p.second.to_string() << std::endl;

    if (!p.first) {
        // If they are colinear, check if they are aligned.
        if (!equal(signed_distance_point_to_line(line1.start, line2.start, line2.end), 0.0))
            return {};

        Point ref = line1.end - line1.start;
        std::array<LengthDbl, 4> points_values = {
            dot_product(line1.start - line1.start, ref),
            dot_product(line1.end - line1.start, ref),
            dot_product(line2.start - line1.start, ref),
            dot_product(line2.end - line1.start, ref)};

        // If they are aligned, check if they overlap.
        std::array<ElementPos, 4> sorted_points = {0, 1, 2, 3};
        std::sort(
                sorted_points.begin(),
                sorted_points.end(),
                [&points_values](
                    ElementPos point_pos_1,
                    ElementPos point_pos_2)
                {
                    return points_values[point_pos_1] < points_values[point_pos_2];
                });

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

        if (sorted_points[0] + sorted_points[1] == 1
                || sorted_points[0] + sorted_points[1] == 5) {
            if (equal(point_1, point_2)) {
                return {{}, {point_1}, {}};
            } else {
                return {};
            }
        }
        if (equal(point_1, point_2)) {
            return {{}, {point_1}, {}};
        } else {
            return {{build_line_segment(point_1, point_2)}, {}, {}};
        }
    }

    if (p.second == line1.start
            || p.second == line1.end) {
        if (line2.contains(p.second))
            return {{}, {p.second}, {}};
    }
    if (p.second == line2.start
            || p.second == line2.end) {
        if (line1.contains(p.second))
            return {{}, {p.second}, {}};
    }

    if (line1.contains(p.second) && line2.contains(p.second))
        return {{}, {}, {p.second}};
    return {};
}

// Helper function to compute line-arc intersections
ShapeElementIntersectionsOutput compute_line_arc_intersections(
        const ShapeElement& line,
        const ShapeElement& arc)
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
            return {{}, {p}, {}};
        } else {
            return {};
        }
    }

    ShapeElementIntersectionsOutput output;
    for (Point& p: ps) {
        // Check if any intersection coincides with an arc endpoint
        if (equal(p, line.start)) {
            if (arc.contains(p))
                output.improper_intersections.push_back(line.start);
        } else if (equal(p, line.end)) {
            if (arc.contains(p))
                output.improper_intersections.push_back(line.end);
        } else if (equal(p, arc.start)) {
            if (line.contains(p))
                output.improper_intersections.push_back(arc.start);
        } else if (equal(p, arc.end)) {
            if (line.contains(p))
                output.improper_intersections.push_back(arc.end);
        } else if (line.contains(p) && arc.contains(p)) {
            output.proper_intersections.push_back(p);
        }
    }

    return output;
}

// Helper function to compute arc-arc intersections
ShapeElementIntersectionsOutput compute_arc_arc_intersections(
        const ShapeElement& arc,
        const ShapeElement& arc_2)
{
    LengthDbl rsq = squared_distance(arc.center, arc.start);
    LengthDbl r2sq = squared_distance(arc_2.center, arc_2.start);
    if (equal(arc.center, arc_2.center)) {
        if (equal(rsq, r2sq)) {
            if (equal(arc.start, arc_2.start)
                    && equal(arc.end, arc_2.end)) {
                if (arc.orientation == arc_2.orientation) {
                    return {{arc}, {}, {}};
                } else {
                    return {{}, {arc.start, arc.end}, {}};
                }
            } else if (equal(arc.start, arc_2.end)
                    && equal(arc.end, arc_2.start)) {
                if (arc.orientation == arc_2.orientation) {
                    return {{}, {arc.start, arc.end}, {}};
                } else {
                    return {{arc}, {}, {}};
                }
            }

            Point arc1s = (arc.orientation == ShapeElementOrientation::Anticlockwise)? arc.start: arc.end;
            Point arc1e = (arc.orientation == ShapeElementOrientation::Anticlockwise)? arc.end: arc.start;
            Point arc2s = (arc_2.orientation == ShapeElementOrientation::Anticlockwise)? arc_2.start: arc_2.end;
            Point arc2e = (arc_2.orientation == ShapeElementOrientation::Anticlockwise)? arc_2.end: arc_2.start;
            Point ref = arc1s - arc.center;
            //std::cout << "arc1s " << arc1s.to_string() << std::endl;
            //std::cout << "arc1e " << arc1e.to_string() << std::endl;
            //std::cout << "arc2s " << arc2s.to_string() << std::endl;
            //std::cout << "arc2e " << arc2e.to_string() << std::endl;
            Angle angle_1e = angle_radian(ref, arc1e - arc.center);
            Angle angle_2s = angle_radian(ref, arc2s - arc.center);
            Angle angle_2e = angle_radian(ref, arc2e - arc.center);
            //std::cout << "angle_1e " << angle_1e << std::endl;
            //std::cout << "angle_2s " << angle_2s << std::endl;
            //std::cout << "angle_2e " << angle_2e << std::endl;
            if (strictly_greater(angle_2e, angle_2s)) {
                if (strictly_greater(angle_2s, angle_1e)) {
                    return {};
                } else if (strictly_greater(angle_2e, angle_1e)) {
                    if (equal(arc2s, arc1e)) {
                        return {{}, {arc2s}, {}};
                    } else {
                        return {{build_circular_arc(arc2s, arc1e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                    }
                } else {
                    return {{build_circular_arc(arc2s, arc2e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                }
            } else {
                if (strictly_greater(angle_2e, angle_1e)) {
                    return {{build_circular_arc(arc1s, arc1e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                } else if (strictly_greater(angle_2s, angle_1e)) {
                    if (equal(arc1s, arc2e)) {
                        return {{}, {arc1s}, {}};
                    } else {
                        return {{build_circular_arc(arc1s, arc2e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                    }
                } else {
                    ShapeElementIntersectionsOutput output;
                    if (equal(arc2s, arc1e)) {
                        output.improper_intersections.push_back(arc2s);
                    } else {
                        output.overlapping_parts.push_back(build_circular_arc(arc2s, arc1e, arc.center, ShapeElementOrientation::Anticlockwise));
                    }
                    if (equal(arc1s, arc2e)) {
                        output.improper_intersections.push_back(arc1s);
                    } else {
                        output.overlapping_parts.push_back(build_circular_arc(arc1s, arc2e, arc.center, ShapeElementOrientation::Anticlockwise));
                    }
                    return output;
                }
            }
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
            return {{}, {p}, {}};
        } else {
            return {};
        }
    }

    ShapeElementIntersectionsOutput output;
    for (Point& p: ps) {
        //std::cout << "p " << p.to_string() << std::endl;
        // Check if any intersection coincides with an arc endpoint
        if (equal(p, arc.start)) {
            if (arc_2.contains(p))
                output.improper_intersections.push_back(arc.start);
        } else if (equal(p, arc.end)) {
            if (arc_2.contains(p))
                output.improper_intersections.push_back(arc.end);
        } else if (equal(p, arc_2.start)) {
            if (arc.contains(p))
                output.improper_intersections.push_back(arc_2.start);
        } else if (equal(p, arc_2.end)) {
            if (arc.contains(p))
                output.improper_intersections.push_back(arc_2.end);
        } else if (arc.contains(p) && arc_2.contains(p)) {
            output.proper_intersections.push_back(p);
        }
    }

    return output;
}

}

ShapeElementIntersectionsOutput shape::compute_intersections(
        const ShapeElement& element_1,
        const ShapeElement& element_2)
{
    if (element_1.type == ShapeElementType::LineSegment
            && element_2.type == ShapeElementType::LineSegment) {
        // Line segment - Line segment intersection
        return compute_line_line_intersections(element_1, element_2);
    } else if (element_1.type == ShapeElementType::LineSegment
            && element_2.type == ShapeElementType::CircularArc) {
        // Line segment - Circular arc intersection
        return compute_line_arc_intersections(element_1, element_2);
    } else if (element_1.type == ShapeElementType::CircularArc
            && element_2.type == ShapeElementType::LineSegment) {
        return compute_line_arc_intersections(element_2, element_1);
    } else if (element_1.type == ShapeElementType::CircularArc
            && element_2.type == ShapeElementType::CircularArc) {
        // Circular arc - Circular arc intersection
        return compute_arc_arc_intersections(element_1, element_2);
    }

    throw std::invalid_argument(
            FUNC_SIGNATURE + ": unsupported element types.");
    return {};
}

std::string ShapeElementIntersectionsOutput::to_string(Counter indentation) const
{
    std::string s = "";
    std::string indent = std::string(indentation, ' ');

    std::string output;
    output += "overlapping parts:";
    for (const ShapeElement& overlapping_part: this->overlapping_parts)
        output += "\n" + indent + "- " + overlapping_part.to_string();
    output += "\n" + indent + "impropoer intersections:";
    for (const Point& point: this->improper_intersections)
        output += "\n" + indent + "- " + point.to_string();
    output += "\n" + indent + "propoer intersections:";
    for (const Point& point: this->proper_intersections)
        output += "\n" + indent + "- " + point.to_string();
    return output;
}
