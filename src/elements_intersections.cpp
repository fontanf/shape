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
    // Check whether an endpoint of one line already lies (within tolerance)
    // on the other line *before* the axis-aligned special cases below. Those
    // special cases each compute their result by projecting along one
    // line's own direction, which -- for a line that meets the tolerance
    // band at a shallow angle -- can amplify a sub-tolerance perpendicular
    // gap (e.g. two near-duplicate near-parallel lines, both within 1e-6 of
    // each other) into a displacement well outside the point-equality
    // tolerance once projected along the shallow line. Checking the
    // perpendicular distances first (unaffected by that amplification)
    // ensures such a case snaps to the existing endpoint instead of
    // computing a slightly-off, spuriously "new" point next to it.
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
    // Each branch below first computes the foot of the perpendicular from
    // the circle's center to the line: the line's closest point to the
    // circle, and the midpoint of the two intersections whenever there are
    // two. If it lies on the circle (point_on_circle, i.e. at the
    // point-equality tolerance), the line stays within that tolerance of
    // the circle everywhere between the two intersections: they are the same
    // point, the line is tangent to the circle, and the foot is returned as
    // the single intersection.
    //
    // Comparing the two computed intersections directly would not work: at
    // a tangency, they are a double root, and the rounding error of the
    // coordinates alone (a few 'epsilon' times their magnitude) moves the
    // line enough to separate them by several times the point-equality
    // tolerance once the radius exceeds ~30 (e.g. ~2e-6 for a radius of ~46
    // at coordinates of ~100, see fontanf/packingsolver#595). Reported as
    // two distinct proper intersections, they make the arc and the line
    // share a sub-tolerance edge between them instead of touching at a
    // point, which corrupts boolean operations downstream.
    //
    // Conversely, a line within the tolerance of the circle between two
    // genuinely distinct crossings (a short chord of a large circle, or a
    // longer one of a nearly tangent line, e.g. a crossing 0.0023 from the
    // tangent point of a radius-4 arc, see fontanf/packingsolver#574) is
    // treated as tangent too: at the library's precision, the two elements
    // coincide between the two crossings.
    std::vector<Point> points;

    if (line_point_1.x == line_point_2.x) {
        Point foot = {line_point_1.x, circle_center.y};
        if (point_on_circle(foot, circle_center, circle_radius))
            return {foot};
        LengthDbl dx = line_point_1.x - circle_center.x;
        LengthDbl diff = circle_radius * circle_radius - dx * dx;
        if (diff < 0)
            return {};
        LengthDbl v = std::sqrt(diff);
        points.push_back({line_point_1.x, circle_center.y + v});
        points.push_back({line_point_1.x, circle_center.y - v});
    } else if (line_point_1.y == line_point_2.y) {
        Point foot = {circle_center.x, line_point_1.y};
        if (point_on_circle(foot, circle_center, circle_radius))
            return {foot};
        LengthDbl dy = line_point_1.y - circle_center.y;
        LengthDbl diff = circle_radius * circle_radius - dy * dy;
        if (diff < 0)
            return {};
        LengthDbl v = std::sqrt(diff);
        points.push_back({circle_center.x + v, line_point_1.y});
        points.push_back({circle_center.x - v, line_point_1.y});
    } else {
        // Work in coordinates relative to the circle's own center, rather
        // than computing the line's implicit-form coefficients from the
        // raw input points and only re-centering afterwards (via
        // 'c_prime'). For a line and circle far from the world origin,
        // 'line_point_2.x * line_point_1.y - line_point_1.x * line_point_2.y'
        // subtracts two products of similar, large magnitude to get a much
        // smaller result -- and the discriminant below repeats the same
        // cancellation one level up, squaring the effect. E.g. for a
        // tangent line to a radius-4 circle at coordinates ~(700, 1150),
        // this produced two computed roots ~1.7e-6 apart instead of the
        // true, exact double root (see fontanf/packingsolver#574).
        // Translating first keeps every intermediate term at the scale of
        // the circle itself (~radius), not the scale of its position in
        // the world.
        Point p1 = {line_point_1.x - circle_center.x, line_point_1.y - circle_center.y};
        Point p2 = {line_point_2.x - circle_center.x, line_point_2.y - circle_center.y};
        LengthDbl line_a = p1.y - p2.y;
        LengthDbl line_b = p2.x - p1.x;
        LengthDbl c_prime = p2.x * p1.y - p1.x * p2.y;
        LengthDbl rsq = circle_radius * circle_radius;
        LengthDbl denom = line_a * line_a + line_b * line_b;
        Point foot = {
            circle_center.x + line_a * c_prime / denom,
            circle_center.y + line_b * c_prime / denom};
        if (point_on_circle(foot, circle_center, circle_radius))
            return {foot};
        LengthDbl discriminant = rsq * denom - c_prime * c_prime;
        if (discriminant < 0)
            return {};
        LengthDbl sqrt_disc = std::sqrt(discriminant);
        LengthDbl eta_1 = (line_a * c_prime + line_b * sqrt_disc) / denom;
        LengthDbl eta_2 = (line_a * c_prime - line_b * sqrt_disc) / denom;
        LengthDbl teta_1 = (line_b * c_prime - line_a * sqrt_disc) / denom;
        LengthDbl teta_2 = (line_b * c_prime + line_a * sqrt_disc) / denom;
        Point point_1 = {circle_center.x + eta_1, circle_center.y + teta_1};
        Point point_2 = {circle_center.x + eta_2, circle_center.y + teta_2};
        if (point_on_circle(point_1, circle_center, circle_radius))
            points.push_back(point_1);
        if (point_on_circle(point_2, circle_center, circle_radius))
            points.push_back(point_2);
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

    // Work relative to center_1 rather than computing the radical line's
    // coefficients from the raw world coordinates of both centers and only
    // re-centering afterwards -- see the identical fix (and why) in
    // compute_line_circle_intersections (fontanf/packingsolver#574): the
    // 'center_1.x*center_1.x + ... + center_2.x*center_2.x + ...' form
    // subtracts large squared terms to get a much smaller result, then the
    // discriminant repeats the cancellation one level up. d = center_2 -
    // center_1 keeps every term at the scale of the two circles' actual
    // separation instead of their absolute position in the world.
    Point d = {center_2.x - center_1.x, center_2.y - center_1.y};
    LengthDbl rsq = radius_1 * radius_1;
    LengthDbl r2sq = radius_2 * radius_2;
    LengthDbl line_a = 2 * d.x;
    LengthDbl line_b = 2 * d.y;
    LengthDbl c_prime = rsq - r2sq + d.x * d.x + d.y * d.y;
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
    if (point_on_circle(point_1, center_1, radius_1)
            && point_on_circle(point_1, center_2, radius_2)) {
        points.push_back(point_1);
    }
    if (point_on_circle(point_2, center_1, radius_1)
            && point_on_circle(point_2, center_2, radius_2)) {
        points.push_back(point_2);
    }

    // Collapse to a single tangent point when the two computed points
    // coincide, since callers use the number of returned points to tell a
    // tangency from a crossing (fontanf/packingsolver#574). See
    // compute_line_circle_intersections for the tangency criterion used
    // there instead (fontanf/packingsolver#595).
    if (points.size() == 2 && equal(points[0], points[1]))
        return {0.5 * (points[0] + points[1])};

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
        if (!point_on_line(line1.start, line2.start, line2.end)
                && !point_on_line(line2.start, line1.start, line1.end)) {
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

    LengthDbl radius = arc.radius();

    std::vector<Point> computed_points = shape::compute_line_circle_intersections(
            line.start, line.end, arc.center, radius);
    std::vector<uint8_t> computed_points_valid(computed_points.size(), true);
    std::vector<Point> end_points;

    // Circle contains line start.
    if (point_on_circle(line.start, arc.center, radius)) {
        end_points.push_back(line.start);
        // Invalidate every computed point that duplicates this endpoint,
        // not just whichever one happens to be nominally closest to it.
        // With a genuine tangency at this endpoint, both roots of
        // compute_line_circle_intersections can independently land within
        // point-equality tolerance of it (see fontanf/packingsolver#574):
        // the old mutually-exclusive if/else-if/else always discarded
        // exactly one of the two, leaving the other to survive as a
        // spurious 'proper' intersection a fraction of a micro-unit away.
        if (computed_points.size() >= 1 && equal(computed_points[0], line.start))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], line.start))
            computed_points_valid[1] = false;
    }
    // Circle contains line end.
    if (!(line.end == line.start)
            && point_on_circle(line.end, arc.center, radius)) {
        end_points.push_back(line.end);
        if (computed_points.size() >= 1 && equal(computed_points[0], line.end))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], line.end))
            computed_points_valid[1] = false;
    }
    // Line contains arc start.
    if (!(arc.start == line.start)
            && !(arc.start == line.end)
            && point_on_line(arc.start, line.start, line.end)) {
        end_points.push_back(arc.start);
        if (computed_points.size() >= 1 && equal(computed_points[0], arc.start))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc.start))
            computed_points_valid[1] = false;
    }
    // Line contains arc end.
    if (!(arc.end == line.start)
            && !(arc.end == line.end)
            && !(arc.end == arc.start)
            && point_on_line(arc.end, line.start, line.end)) {
        end_points.push_back(arc.end);
        if (computed_points.size() >= 1 && equal(computed_points[0], arc.end))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc.end))
            computed_points_valid[1] = false;
    }

    ShapeElementIntersectionsOutput output;
    for (const Point& p: end_points)
        if (arc.contains(p) && line.contains(p))
            output.improper_intersections.push_back(p);
    for (ElementPos pos = 0; pos < (ElementPos)computed_points.size(); ++pos) {
        if (!computed_points_valid[pos])
            continue;
        const Point& p = computed_points[pos];
        // Check if any intersection coincides with an arc_2 endpoint
        if (arc.contains(p) && line.contains(p)) {
            if (computed_points.size() == 1) {
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

    LengthDbl radius_1 = arc.radius();
    LengthDbl radius_2 = arc_2.radius();

    std::vector<Point> computed_points = shape::compute_circle_circle_intersections(
            arc.center, radius_1, arc_2.center, radius_2);
    std::vector<uint8_t> computed_points_valid(computed_points.size(), true);
    std::vector<Point> end_points;

    // Circle 1 contains arc 2 start.
    if (point_on_circle(arc_2.start, arc.center, radius_1)) {
        end_points.push_back(arc_2.start);
        // Invalidate every computed point that duplicates this endpoint,
        // not just whichever one happens to be nominally closest to it --
        // see the identical fix (and why) in compute_line_arc_intersections
        // (fontanf/packingsolver#574).
        if (computed_points.size() >= 1 && equal(computed_points[0], arc_2.start))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc_2.start))
            computed_points_valid[1] = false;
    }
    // Circle 1 contains arc 2 end.
    if (!(arc_2.end == arc_2.start)
            && point_on_circle(arc_2.end, arc.center, radius_1)) {
        end_points.push_back(arc_2.end);
        if (computed_points.size() >= 1 && equal(computed_points[0], arc_2.end))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc_2.end))
            computed_points_valid[1] = false;
    }
    // Circle 2 contains arc 1 start.
    if (!(arc.start == arc_2.start)
            && !(arc.start == arc_2.end)
            && point_on_circle(arc.start, arc_2.center, radius_2)) {
        end_points.push_back(arc.start);
        if (computed_points.size() >= 1 && equal(computed_points[0], arc.start))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc.start))
            computed_points_valid[1] = false;
    }
    // Circle 2 contains arc 1 end.
    if (!(arc.end == arc_2.start)
            && !(arc.end == arc_2.end)
            && !(arc.end == arc.start)
            && point_on_circle(arc.end, arc_2.center, radius_2)) {
        end_points.push_back(arc.end);
        if (computed_points.size() >= 1 && equal(computed_points[0], arc.end))
            computed_points_valid[0] = false;
        if (computed_points.size() >= 2 && equal(computed_points[1], arc.end))
            computed_points_valid[1] = false;
    }

    ShapeElementIntersectionsOutput output;
    for (const Point& p: end_points)
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
