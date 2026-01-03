#include "shape/element_intersections.hpp"

#include <iostream>

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
            Point arc2s = (arc.orientation == ShapeElementOrientation::Anticlockwise)? arc_2.start: arc_2.end;
            Point arc2e = (arc.orientation == ShapeElementOrientation::Anticlockwise)? arc_2.end: arc_2.start;
            Point ref = arc1s - arc.center;
            Angle angle_1e = angle_radian(arc1e - arc.center, ref);
            Angle angle_2s = angle_radian(arc2s - arc.center, ref);
            Angle angle_2e = angle_radian(arc2e - arc.center, ref);
            if (strictly_greater(angle_2s, angle_1e)) {
                if (strictly_greater(angle_2e, angle_2s)) {
                    return {};
                } else if (strictly_greater(angle_2e, angle_1e)) {
                    return {{build_circular_arc(arc1s, arc1e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                } else {
                    if (equal(arc1s, arc2e)) {
                        return {{}, {arc1s}, {}};
                    } else {
                        return {{build_circular_arc(arc1s, arc2e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                    }
                }
            } else {
                if (strictly_greater(angle_2e, angle_1e)) {
                    if (equal(arc2s, arc1e)) {
                        return {{}, {arc2s}, {}};
                    } else {
                        return {{build_circular_arc(arc2s, arc1e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
                    }
                } else {
                    return {{build_circular_arc(arc2s, arc2e, arc.center, ShapeElementOrientation::Anticlockwise)}, {}, {}};
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
                if (strictly_greater(jet_shape_2_1, jet_shape_1)
                        && strictly_greater(jet_shape_1, jet_shape_2_2)) {
                    return true;
                }
                if (strictly_greater(jet_shape_2_2, jet_shape_1)
                        && strictly_greater(jet_shape_1, jet_shape_2_1)) {
                    return true;
                }
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
                if (strictly_greater(jet_shape_1_1, jet_shape_2)
                        && strictly_greater(jet_shape_2, jet_shape_1_2)) {
                    return true;
                }
                if (strictly_greater(jet_shape_1_2, jet_shape_2)
                        && strictly_greater(jet_shape_2, jet_shape_1_1)) {
                    return true;
                }
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
                if (strictly_greater(jet_shape_2_1, jet_shape_1)
                        && strictly_greater(jet_shape_1, jet_shape_2_2)) {
                    return true;
                }
                if (strictly_greater(jet_shape_2_2, jet_shape_1)
                        && strictly_greater(jet_shape_1, jet_shape_2_1)) {
                    return true;
                }
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
                if (strictly_greater(jet_shape_1_1, jet_shape_2)
                        && strictly_greater(jet_shape_2, jet_shape_1_2)) {
                    return true;
                }
                if (strictly_greater(jet_shape_1_2, jet_shape_2)
                        && strictly_greater(jet_shape_2, jet_shape_1_1)) {
                    return true;
                }
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
            if (strictly_greater(jet_shape_2_1, jet_shape_1)
                    && strictly_greater(jet_shape_1, jet_shape_2_2)) {
                return true;
            }
            if (strictly_greater(jet_shape_2_2, jet_shape_1)
                    && strictly_greater(jet_shape_1, jet_shape_2_1)) {
                return true;
            }
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
            if (strictly_greater(jet_shape_1_1, jet_shape_2)
                    && strictly_greater(jet_shape_2, jet_shape_1_2)) {
                return true;
            }
            if (strictly_greater(jet_shape_1_2, jet_shape_2)
                    && strictly_greater(jet_shape_2, jet_shape_1_1)) {
                return true;
            }
        }
    }
    return false;
}

}

bool shape::strictly_intersect(
        const Shape& shape)
{
    for (ElementPos element_1_pos = 0;
            element_1_pos < (ElementPos)shape.elements.size();
            ++element_1_pos) {
        for (ElementPos element_2_pos = element_1_pos + 2;
                element_2_pos < (ElementPos)shape.elements.size();
                ++element_2_pos) {
            if (::strictly_intersect(
                        shape, false, element_1_pos,
                        shape, false, element_2_pos)) {
                //std::cout << "element_1_pos " << element_1_pos
                //    << " element " << shape.elements[element_1_pos].to_string() << std::endl;
                //std::cout << "element_2_pos " << element_2_pos
                //    << " element " << shape.elements[element_2_pos].to_string() << std::endl;
                return true;
            }
        }
    }
    return false;
}

namespace
{

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
