//#define ELEMENTS_INTERSECTIONS_ENABLE_DEBUG

#include "shape/elements_intersections.hpp"

#ifdef ELEMENTS_INTERSECTIONS_ENABLE_DEBUG
#include <iostream>
#endif

using namespace shape;

std::pair<bool, Point> shape::compute_line_intersection(
        const Point& p11,
        const Point& p12,
        const Point& p21,
        const Point& p22)
{
    LengthDbl dist_1start_2 = signed_distance_point_to_line(p11, p21, p22);
    LengthDbl dist_1end_2 = signed_distance_point_to_line(p12, p21, p22);
    LengthDbl dist_2start_1 = signed_distance_point_to_line(p21, p11, p12);
    LengthDbl dist_2end_1 = signed_distance_point_to_line(p22, p11, p12);
    //std::cout << "dist_1start_2 " << dist_1start_2 << std::endl;
    //std::cout << "dist_1end_2 " << dist_1end_2 << std::endl;
    //std::cout << "dist_2start_1 " << dist_2start_1 << std::endl;
    //std::cout << "dist_2end_1 " << dist_2end_1 << std::endl;
    if (equal(dist_1start_2, dist_1end_2)
            || equal(dist_2start_1, dist_2end_1)) {
        //std::cout << "a" << std::endl;
        return {false, {0, 0}};
    } else if (p11 == p21 || p11 == p22) {
        //std::cout << "b" << std::endl;
        return {true, p11};
    } else if (p12 == p21 || p12 == p22) {
        //std::cout << "c" << std::endl;
        return {true, p12};
    } else if (p11.x == p12.x) {
        //std::cout << "d" << std::endl;
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
        //std::cout << "e" << std::endl;
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
        //std::cout << "f" << std::endl;
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
        //std::cout << "g" << std::endl;
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
    } else if (equal(dist_1start_2, 0.0)) {
        //std::cout << "h" << std::endl;
        if (equal(dist_2start_1, 0.0))
            return (std::abs(dist_1start_2) < std::abs(dist_2start_1))?
                std::pair<bool, Point>{true, p11}:
                std::pair<bool, Point>{true, p21};
        if (equal(dist_2end_1, 0.0))
            return (std::abs(dist_1start_2) < std::abs(dist_2end_1))?
                std::pair<bool, Point>{true, p11}:
                std::pair<bool, Point>{true, p22};
        return {true, p11};
    } else if (equal(dist_1end_2, 0.0)) {
        //std::cout << "i" << std::endl;
        if (equal(dist_2start_1, 0.0))
            return (std::abs(dist_1end_2) < std::abs(dist_2start_1))?
                std::pair<bool, Point>{true, p12}:
                std::pair<bool, Point>{true, p21};
        if (equal(dist_2end_1, 0.0))
            return (std::abs(dist_1end_2) < std::abs(dist_2end_1))?
                std::pair<bool, Point>{true, p12}:
                std::pair<bool, Point>{true, p22};
        return {true, p12};
    } else if (equal(dist_2start_1, 0.0)) {
        //std::cout << "j" << std::endl;
        return {true, p21};
    } else if (equal(dist_2end_1, 0.0)) {
        //std::cout << "k" << std::endl;
        return {true, p22};
    } else {
        //std::cout << "l" << std::endl;
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

std::vector<Point> shape::compute_line_circle_intersections(
        const Point& line_point_1,
        const Point& line_point_2,
        const Point& circle_center,
        LengthDbl circle_radius)
{
    std::vector<Point> points;

    if (line_point_1.x == line_point_2.x) {
        LengthDbl dx = line_point_1.x - circle_center.x;
        LengthDbl diff = circle_radius * circle_radius - dx * dx;
        if (strictly_lesser(diff, 0))
            return {};
        if (diff < 0)
            diff = 0;
        LengthDbl v = std::sqrt(diff);
        points.push_back({line_point_1.x, circle_center.y + v});
        points.push_back({line_point_1.x, circle_center.y - v});
    } else if (line_point_1.y == line_point_2.y) {
        LengthDbl dy = line_point_1.y - circle_center.y;
        LengthDbl diff = circle_radius * circle_radius - dy * dy;
        if (strictly_lesser(diff, 0))
            return {};
        if (diff < 0)
            diff = 0;
        LengthDbl v = std::sqrt(diff);
        points.push_back({circle_center.x + v, line_point_1.y});
        points.push_back({circle_center.x - v, line_point_1.y});
    } else {
        LengthDbl line_a = line_point_1.y - line_point_2.y;
        LengthDbl line_b = line_point_2.x - line_point_1.x;
        LengthDbl line_c = line_point_2.x * line_point_1.y
            - line_point_1.x * line_point_2.y;
        LengthDbl c_prime = line_c
            - line_a * circle_center.x
            - line_b * circle_center.y;
        LengthDbl rsq = circle_radius * circle_radius;
        LengthDbl denom = line_a * line_a + line_b * line_b;
        if (strictly_lesser(rsq * denom, c_prime * c_prime))
            return {};
        LengthDbl discriminant = rsq * denom - c_prime * c_prime;
        if (discriminant < 0)
            discriminant = 0;
        LengthDbl sqrt_disc = std::sqrt(discriminant);
        LengthDbl eta_1 = (line_a * c_prime + line_b * sqrt_disc) / denom;
        LengthDbl eta_2 = (line_a * c_prime - line_b * sqrt_disc) / denom;
        LengthDbl teta_1 = (line_b * c_prime - line_a * sqrt_disc) / denom;
        LengthDbl teta_2 = (line_b * c_prime + line_a * sqrt_disc) / denom;
        Point point_1 = {circle_center.x + eta_1, circle_center.y + teta_1};
        Point point_2 = {circle_center.x + eta_2, circle_center.y + teta_2};
        if (equal(distance(point_1, circle_center), circle_radius))
            points.push_back(point_1);
        if (equal(distance(point_2, circle_center), circle_radius))
            points.push_back(point_2);
    }

    // Collapse to a single tangent point when the two computed points coincide.
    if (points.size() == 2) {
        Point midpoint = 0.5 * (points[0] + points[1]);
        if (equal(distance(midpoint, circle_center), circle_radius))
            return {midpoint};
    }

    return points;
}

std::vector<Point> shape::compute_circle_circle_intersections(
        const Point& center_1,
        LengthDbl radius_1,
        const Point& center_2,
        LengthDbl radius_2)
{
    if (equal(center_1, center_2))
        return {};

    LengthDbl rsq = radius_1 * radius_1;
    LengthDbl r2sq = radius_2 * radius_2;
    LengthDbl line_a = 2 * (center_2.x - center_1.x);
    LengthDbl line_b = 2 * (center_2.y - center_1.y);
    LengthDbl line_c = rsq
        - center_1.x * center_1.x - center_1.y * center_1.y
        - r2sq
        + center_2.x * center_2.x + center_2.y * center_2.y;
    LengthDbl c_prime = line_c
        - line_a * center_1.x
        - line_b * center_1.y;
    LengthDbl denom = line_a * line_a + line_b * line_b;
    if (strictly_lesser(rsq * denom, c_prime * c_prime))
        return {};
    LengthDbl discriminant = rsq * denom - c_prime * c_prime;
    if (discriminant < 0)
        discriminant = 0;
    LengthDbl sqrt_disc = std::sqrt(discriminant);
    LengthDbl eta_1 = (line_a * c_prime + line_b * sqrt_disc) / denom;
    LengthDbl eta_2 = (line_a * c_prime - line_b * sqrt_disc) / denom;
    LengthDbl teta_1 = (line_b * c_prime - line_a * sqrt_disc) / denom;
    LengthDbl teta_2 = (line_b * c_prime + line_a * sqrt_disc) / denom;
    Point point_1 = {center_1.x + eta_1, center_1.y + teta_1};
    Point point_2 = {center_1.x + eta_2, center_1.y + teta_2};
    std::vector<Point> points;
    if (equal(distance(point_1, center_1), radius_1)
            && equal(distance(point_1, center_2), radius_2)) {
        points.push_back(point_1);
    }
    if (equal(distance(point_2, center_1), radius_1)
            && equal(distance(point_2, center_2), radius_2)) {
        points.push_back(point_2);
    }

    // Collapse to a single tangent point when the two computed points coincide.
    if (points.size() == 2) {
        Point midpoint = 0.5 * (points[0] + points[1]);
        if (equal(distance(midpoint, center_1), radius_1)
                || equal(distance(midpoint, center_2), radius_2)) {
            return {midpoint};
        }
    }

    return points;
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
        if (!equal(signed_distance_point_to_line(line1.start, line2.start, line2.end), 0.0)
                && !equal(signed_distance_point_to_line(line2.start, line1.start, line1.end), 0.0)) {
            return {};
        }

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

    LengthDbl radius = distance(arc.start, arc.center);

    std::vector<Point> points = shape::compute_line_circle_intersections(
            line.start, line.end, arc.center, radius);
    if (points.empty())
        return {};

    bool circle_contains_line_start = equal(distance(line.start, arc.center), radius);
    if (circle_contains_line_start) {
        if (points.size() == 1
                || squared_distance(line.start, points[0]) < squared_distance(line.start, points[1])) {
            points[0] = line.start;
        } else {
            points[1] = line.start;
        }
    }
    bool circle_contains_line_end = equal(distance(line.end, arc.center), radius);
    if (circle_contains_line_end) {
        if (points.size() == 1
                || squared_distance(line.end, points[0]) < squared_distance(line.end, points[1])) {
            points[0] = line.end;
        } else {
            points[1] = line.end;
        }
    }
    bool line_contains_arc_start = equal(distance_point_to_line(arc.start, line.start, line.end), 0.0);
    if (line_contains_arc_start) {
        if (points.size() == 1
                || squared_distance(arc.start, points[0]) < squared_distance(arc.start, points[1])) {
            points[0] = arc.start;
        } else {
            points[1] = arc.start;
        }
    }
    bool line_contains_arc_end = equal(distance_point_to_line(arc.end, line.start, line.end), 0.0);
    if (line_contains_arc_end) {
        if (points.size() == 1
                || squared_distance(arc.end, points[0]) < squared_distance(arc.end, points[1])) {
            points[0] = arc.end;
        } else {
            points[1] = arc.end;
        }
    }

    ShapeElementIntersectionsOutput output;
    for (const Point& p: points) {
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
            if (points.size() == 1) {
                output.improper_intersections.push_back(p);
            } else {
                output.proper_intersections.push_back(p);
            }
        }
    }

    return output;
}

// Helper function to compute arc-arc intersections
ShapeElementIntersectionsOutput compute_arc_arc_intersections(
        const ShapeElement& arc,
        const ShapeElement& arc_2)
{
#ifdef ELEMENTS_INTERSECTIONS_ENABLE_DEBUG
    std::cout << "recompute_center_1 " << arc.recompute_center().to_string() << std::endl;
    std::cout << "recompute_center_2 " << arc_2.recompute_center().to_string() << std::endl;
#endif
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
#ifdef ELEMENTS_INTERSECTIONS_ENABLE_DEBUG
            std::cout << "arc1s " << arc1s.to_string() << std::endl;
            std::cout << "arc1e " << arc1e.to_string() << std::endl;
            std::cout << "arc2s " << arc2s.to_string() << std::endl;
            std::cout << "arc2e " << arc2e.to_string() << std::endl;
#endif
            Angle angle_1e = angle_radian(ref, arc1e - arc.center);
            Angle angle_2s = angle_radian(ref, arc2s - arc.center);
            Angle angle_2e = angle_radian(ref, arc2e - arc.center);
#ifdef ELEMENTS_INTERSECTIONS_ENABLE_DEBUG
            std::cout << "angle_1e " << angle_1e << std::endl;
            std::cout << "angle_2s " << angle_2s << std::endl;
            std::cout << "angle_2e " << angle_2e << std::endl;
#endif
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

    LengthDbl radius_1 = distance(arc.start, arc.center);
    LengthDbl radius_2 = distance(arc_2.start, arc_2.center);

    std::vector<Point> computed_points = shape::compute_circle_circle_intersections(
            arc.center, radius_1, arc_2.center, radius_2);
    std::vector<uint8_t> computed_points_valid(computed_points.size(), true);
    std::vector<Point> valid_points;

    // Circle 1 contains arc 2 start.
    if (equal(distance(arc_2.start, arc.center), radius_1)) {
        valid_points.push_back(arc_2.start);
        if (computed_points.size() == 0) {
        } else if (computed_points.size() == 1
                || squared_distance(arc_2.start, computed_points[0]) < squared_distance(arc_2.start, computed_points[1])) {
            computed_points_valid[0] = false;
        } else {
            computed_points_valid[1] = false;
        }
    }
    // Circle 1 contains arc 2 end.
    if (!(arc_2.end == arc_2.start)
            && equal(distance(arc_2.end, arc.center), radius_1)) {
        valid_points.push_back(arc_2.end);
        if (computed_points.size() == 0) {
        } else if (computed_points.size() == 1
                || squared_distance(arc_2.end, computed_points[0]) < squared_distance(arc_2.end, computed_points[1])) {
            computed_points_valid[0] = false;
        } else {
            computed_points_valid[1] = false;
        }
    }
    // Circle 2 contains arc 1 start.
    if (!(arc.start == arc_2.start)
            && !(arc.start == arc_2.end)
            && equal(distance(arc.start, arc_2.center), radius_2)) {
        valid_points.push_back(arc.start);
        if (computed_points.size() == 0) {
        } else if (computed_points.size() == 1
                || squared_distance(arc.start, computed_points[0]) < squared_distance(arc.start, computed_points[1])) {
            computed_points_valid[0] = false;
        } else {
            computed_points_valid[1] = false;
        }
    }
    // Circle 2 contains arc 1 end.
    if (!(arc.end == arc_2.start)
            && !(arc.end == arc_2.end)
            && !(arc.end == arc.start)
            && equal(distance(arc.end, arc_2.center), radius_2)) {
        valid_points.push_back(arc.end);
        if (computed_points.size() == 0) {
        } else if (computed_points.size() == 1
                || squared_distance(arc.end, computed_points[0]) < squared_distance(arc.end, computed_points[1])) {
            computed_points_valid[0] = false;
        } else {
            computed_points_valid[1] = false;
        }
    }

    ShapeElementIntersectionsOutput output;
    for (const Point& p: valid_points)
        if (arc.contains(p) && arc_2.contains(p))
            output.improper_intersections.push_back(p);
    for (ElementPos pos = 0; pos < (ElementPos)computed_points.size(); ++pos) {
        if (!computed_points_valid[pos])
            continue;
        const Point& p = computed_points[pos];
        // Check if any intersection coincides with an arc_2 endpoint
        if (arc.contains(p) && arc_2.contains(p)) {
            if (computed_points.size() == 1) {
                output.improper_intersections.push_back(p);
            } else {
                output.proper_intersections.push_back(p);
            }
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
    output += "\n" + indent + "improper intersections:";
    for (const Point& point: this->improper_intersections)
        output += "\n" + indent + "- " + point.to_string();
    output += "\n" + indent + "proper intersections:";
    for (const Point& point: this->proper_intersections)
        output += "\n" + indent + "- " + point.to_string();
    return output;
}
