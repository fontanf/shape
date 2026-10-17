#pragma once

#define _USE_MATH_DEFINES

#include "nlohmann/json.hpp"

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define FUNC_SIGNATURE std::string(__PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
#  define FUNC_SIGNATURE std::string(__FUNCSIG__)
#else
#  define FUNC_SIGNATURE std::string(__func__)
#endif

/*
 * See docs/conventions.md for the conventions shared by the whole library:
 * orientation of the outlines and of the holes, tolerance and scaling of the
 * coordinates, consistency of the circular arcs, and validity of the shapes.
 */

namespace shape
{

using LengthDbl = double;
using AreaDbl = double;
using Angle = double;
using ElementPos = int64_t;
using ItemShapePos = int64_t;
using ShapePos = int64_t;
using Counter = int64_t;

/*
 * Comparisons with the fixed absolute tolerance of 1e-6 used by the whole
 * library (see docs/conventions.md).
 */

/** Return true iff 'v1' is smaller than 'v2' by more than the tolerance. */
inline bool strictly_lesser(double v1, double v2)
{
    return v2 - v1 > 1e-6;
}

/** Return true iff 'v1' is greater than 'v2' by more than the tolerance. */
inline bool strictly_greater(double v1, double v2)
{
    return v1 - v2 > 1e-6;
}

/** Return true iff 'v1' and 'v2' differ by at most the tolerance. */
inline bool equal(double v1, double v2)
{
    return std::abs(v1 - v2) <= 1e-6;
}

/**
 * Return the largest power of two smaller than or equal to a positive value,
 * e.g. to scale coordinates (see docs/conventions.md).
 */
LengthDbl largest_power_of_two_lesser_or_equal(LengthDbl value);

/**
 * Return the smallest power of two greater than or equal to a positive
 * value, e.g. to scale coordinates (see docs/conventions.md).
 */
LengthDbl smallest_power_of_two_greater_or_equal(LengthDbl value);

/** Convert a value to a string with enough digits to read it back exactly. */
std::string to_string(double value);

/**
 * Return the angle in degrees in [0, 360[ equal to 'angle' modulo 360.
 *
 * An angle within the tolerance below a multiple of 360 gives a slightly
 * negative result.
 */
Angle normalize_angle(Angle angle);

/** Same as normalize_angle, in radians, in [0, 2 * pi[. */
Angle normalize_angle_radian(Angle angle);

/** Return the distance between two angles in degrees, in [0, 180]. */
inline Angle angular_distance(
        Angle angle_1,
        Angle angle_2)
{
    return std::abs(std::remainder(angle_2 - angle_1, 360));
}

/** Return the distance between two angles in radians, in [0, pi]. */
inline Angle angular_distance_radian(
        Angle angle_1,
        Angle angle_2)
{
    return std::abs(std::remainder(angle_2 - angle_1, 2.0 * M_PI));
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Point /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * Structure for a point.
 */
struct Point
{
    /** x-coordinate. */
    LengthDbl x;

    /** y-coordiante. */
    LengthDbl y;

    /*
     * Transformations
     *
     * The rotations are anticlockwise. The angles are in degrees, except for
     * rotate_radians. A rotation by a multiple of 90 degrees is exact.
     */

    /** Translate the point by (x, y). */
    Point& shift(
            LengthDbl x,
            LengthDbl y);

    /** Translate the point by 'vector'. */
    Point& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }

    /** Rotate the point around the origin. */
    Point rotate(Angle angle) const;

    /** Rotate the point around 'center'. */
    Point rotate(const Point& center, Angle angle) const;

    /** Rotate the point around the origin, 'angle' being in radians. */
    Point rotate_radians(Angle angle) const;

    /** Rotate the point around 'center', 'angle' being in radians. */
    Point rotate_radians(const Point& center, Angle angle) const;

    /** Mirror the point through the line y = x, i.e. swap x and y. */
    Point axial_symmetry_identity_line() const;

    /** Mirror the point through the y-axis, i.e. negate x. */
    Point axial_symmetry_y_axis() const;

    /** Mirror the point through the x-axis, i.e. negate y. */
    Point axial_symmetry_x_axis() const;

    /*
     * Export
     */

    /** Return "(x, y)", with enough digits to read the values back exactly. */
    std::string to_string() const;

    /** Read a point from a JSON object {"x": ..., "y": ...}. */
    template <class basic_json>
    static Point from_json(basic_json& json_element);

    /** Write the point to a JSON object {"x": ..., "y": ...}. */
    nlohmann::json to_json() const;

    /**
     * Return the SVG of a disk of radius 'radius' centered at the point,
     * with the y-axis going up.
     */
    std::string to_svg(LengthDbl radius = 1) const;
};

/** Return true iff two points are exactly equal. */
inline bool operator==(
        const Point& point_1,
        const Point& point_2)
{
    return (point_1.x == point_2.x) && (point_1.y == point_2.y);
}

/**
 * Return true iff two points are equal within tolerance, coordinate by
 * coordinate: the tolerance region around a point is a square, not a disk.
 */
inline bool equal(
        const Point& point_1,
        const Point& point_2)
{
    return equal(point_1.x, point_2.x) && equal(point_1.y, point_2.y);
}

Point operator+(
        const Point& point_1,
        const Point& point_2);

inline Point operator-(
        const Point& point_1,
        const Point& point_2)
{
    return {point_1.x - point_2.x, point_1.y - point_2.y};
}

Point operator*(
        LengthDbl scalar,
        const Point& point);

Point operator/(
        const Point& point,
        LengthDbl scalar);

inline LengthDbl norm(
        const Point& vector)
{
    return std::hypot(vector.x, vector.y);
}

/** Return the vector of norm 1 with the direction of 'vector', non-null. */
Point normalize(
        const Point& vector);

LengthDbl squared_norm(
        const Point& vector);

inline LengthDbl distance(
        const Point& point_1,
        const Point& point_2)
{
    return norm(point_2 - point_1);
}

LengthDbl squared_distance(
        const Point& point_1,
        const Point& point_2);

/**
 * Ratio t such that line_point_1 + t * (line_point_2 - line_point_1) is the
 * projection of 'point' onto the (infinite) line through line_point_1 and
 * line_point_2. t is unclamped: 0 at line_point_1, 1 at line_point_2, and
 * outside [0, 1] if the projection falls beyond either point.
 */
inline LengthDbl project_point_on_line_ratio(
        const Point& line_point_1,
        const Point& line_point_2,
        const Point& point)
{
    LengthDbl bx = line_point_2.x - line_point_1.x;
    LengthDbl by = line_point_2.y - line_point_1.y;
    LengthDbl px = point.x - line_point_1.x;
    LengthDbl py = point.y - line_point_1.y;
    LengthDbl denom = std::fma(bx, bx, by * by);
    return std::fma(px, bx, py * by) / denom;
}

/**
 * Return the projection of 'point' onto the (infinite) line through
 * line_point_1 and line_point_2 (see project_point_on_line_ratio).
 */
inline Point project_point_on_line(
        const Point& line_point_1,
        const Point& line_point_2,
        const Point& point)
{
    LengthDbl t = project_point_on_line_ratio(line_point_1, line_point_2, point);
    Point q;
    q.x = line_point_1.x + t * (line_point_2.x - line_point_1.x);
    q.y = line_point_1.y + t * (line_point_2.y - line_point_1.y);
    return q;
}

/**
 * Return true iff 'point' lies (within tolerance) on the infinite line
 * through line_point_1 and line_point_2.
 *
 * Project 'point' onto the line and compare the projection to 'point' itself
 * with the standard point tolerance, rather than checking
 * distance_point_to_line(...) against 0 directly: the same rationale as
 * ShapeElement::contains applies here too, and project_point_on_line_ratio's
 * fma-based, line_point_1-relative computation additionally avoids the
 * catastrophic cancellation that distance_point_to_line's raw absolute-
 * coordinate formula is prone to far from the origin (see
 * fontanf/packingsolver#574).
 */
inline bool point_on_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2)
{
    Point proj = project_point_on_line(line_point_1, line_point_2, point);
    return shape::equal(proj, point);
}

/**
 * Return true iff 'point' lies (within tolerance) on the circle of the given
 * center and radius.
 *
 * Same rationale as point_on_line/ShapeElement::contains: radially project
 * 'point' onto the circle and compare the projection to 'point' itself with
 * the standard (anisotropic, per-coordinate) point tolerance, rather than
 * comparing distance(point, center) to radius directly (an isotropic
 * tolerance on the radial distance) -- consistent with how every other
 * on-element check in this codebase measures closeness. 'point' at 'center'
 * has no well-defined radial direction to project along, so it is handled
 * separately: it is on the circle only in the degenerate radius-0 case.
 */
inline bool point_on_circle(
        const Point& point,
        const Point& center,
        LengthDbl radius)
{
    LengthDbl point_distance = distance(point, center);
    if (point_distance == 0.0)
        return shape::equal(radius, 0.0);
    Point projection = center + (radius / point_distance) * (point - center);
    return shape::equal(projection, point);
}

/**
 * Return the distance from 'point' to the (infinite) line through
 * line_point_1 and line_point_2.
 *
 * Prefer point_on_line to check whether a point is on a line: this formula
 * uses absolute coordinates and loses precision far from the origin.
 */
LengthDbl distance_point_to_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2);

/**
 * Same as distance_point_to_line, but positive if 'point' is on the right of
 * the line going from line_point_1 to line_point_2, and negative if it is on
 * its left.
 */
inline LengthDbl signed_distance_point_to_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2)
{
    const LengthDbl dx = line_point_2.x - line_point_1.x;
    const LengthDbl dy = line_point_2.y - line_point_1.y;
    const LengthDbl px = point.x - line_point_1.x;
    const LengthDbl py = point.y - line_point_1.y;
    const LengthDbl cross = std::fma(px, dy, -py * dx);
    return cross / std::hypot(dx, dy);
}

LengthDbl dot_product(
        const Point& vector_1,
        const Point& vector_2);

/**
 * Return vector_1.x * vector_2.y - vector_1.y * vector_2.x, which is
 * positive iff 'vector_2' is anticlockwise from 'vector_1' by less than a
 * half turn.
 */
LengthDbl cross_product(
        const Point& vector_1,
        const Point& vector_2);

/**
 * Return the angle of a vector, measured anticlockwise from the x-axis, in
 * [0, 2 * pi[.
 *
 * A vector within the tolerance below the x-axis gives a slightly negative
 * angle.
 */
Angle angle_radian(
        const Point& vector);

/**
 * Return the angle between two vectors.
 *
 * The angle is measured anticlockwise and always belongs to [0, 2 * pi[.
 */
Angle angle_radian(
        const Point& vector_1,
        const Point& vector_2);

/**
 * Return the signed area of the triangle (point_1, point_2, point_3):
 * positive iff the points are in anticlockwise order.
 */
inline AreaDbl compute_area(
        const Point& point_1,
        const Point& point_2,
        const Point& point_3)
{
    // https://en.wikipedia.org/wiki/Area_of_a_triangle#Using_coordinates
    return 0.5 * (
            (point_1.x - point_3.x) * (point_2.y - point_1.y)
            - (point_1.x - point_2.x) * (point_3.y - point_1.y));
}

/**
 * Return the orientation of the triangle (point_1, point_2, point_3):
 * - -1 if the points are in anticlockwise order;
 * - 1 if they are in clockwise order;
 * - 0 if point_3 is on the line through point_1 and point_2 (within
 *   tolerance, see point_on_line).
 *
 * Note that, despite the name, the anticlockwise order gives -1.
 */
int counter_clockwise(
        const Point& point_1,
        const Point& point_2,
        const Point& point_3);

/**
 * Return true iff the angle of 'vector_1' is strictly smaller than the
 * angle of 'vector_2', the angles being measured anticlockwise from the
 * x-axis in [0, 2 * pi[.
 *
 * The comparison is exact, without tolerance, and doesn't depend on the
 * lengths of the vectors.
 */
bool strictly_lesser_angle(
        const Point& vector_1,
        const Point& vector_2);

/** Same as strictly_lesser_angle with the vectors swapped. */
bool strictly_greater_angle(
        const Point& vector_1,
        const Point& vector_2);

/**
 * Axis-aligned bounding box.
 *
 * The default box is empty: its bounds are infinite, so that merging it with
 * another box gives the other box.
 */
struct AxisAlignedBoundingBox
{
    LengthDbl x_min = +std::numeric_limits<LengthDbl>::infinity();
    LengthDbl x_max = -std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_min = +std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_max = -std::numeric_limits<LengthDbl>::infinity();

    inline AxisAlignedBoundingBox& shift(
            LengthDbl x,
            LengthDbl y)
    {
        x_min += x;
        x_max += x;
        y_min += y;
        y_max += y;
        return *this;
    }

    inline AxisAlignedBoundingBox& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }
};

inline bool equal(
        const AxisAlignedBoundingBox& aabb_1,
        const AxisAlignedBoundingBox& aabb_2)
{
    if (!equal(aabb_1.x_min, aabb_2.x_min))
        return false;
    if (!equal(aabb_1.x_max, aabb_2.x_max))
        return false;
    if (!equal(aabb_1.y_min, aabb_2.y_min))
        return false;
    if (!equal(aabb_1.y_max, aabb_2.y_max))
        return false;
    return true;
}

/** Return the smallest box containing both boxes. */
inline AxisAlignedBoundingBox merge(
        const AxisAlignedBoundingBox& aabb_1,
        const AxisAlignedBoundingBox& aabb_2)
{
    AxisAlignedBoundingBox output;
    output.x_min = std::min(aabb_1.x_min, aabb_2.x_min);
    output.x_max = std::max(aabb_1.x_max, aabb_2.x_max);
    output.y_min = std::min(aabb_1.y_min, aabb_2.y_min);
    output.y_max = std::max(aabb_1.y_max, aabb_2.y_max);
    return output;
}

/**
 * Return true iff the interiors of two boxes overlap. The comparison is
 * exact: boxes only touching each other don't intersect.
 */
inline bool intersect(
        const AxisAlignedBoundingBox& aabb_1,
        const AxisAlignedBoundingBox& aabb_2)
{
    return aabb_1.x_max > aabb_2.x_min && aabb_2.x_max > aabb_1.x_min
        && aabb_1.y_max > aabb_2.y_min && aabb_2.y_max > aabb_1.y_min;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ShapeElement /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum class ShapeElementType
{
    LineSegment,
    CircularArc,
};

/**
 * Conversions between an element type and its name ("LineSegment",
 * "CircularArc"), as written in JSON files, or its first letter.
 */
std::string element2str(ShapeElementType type);
ShapeElementType str2element(const std::string& str);

char element2char(ShapeElementType type);

/**
 * Orientation of a circular arc. A Full arc is a whole circle, going
 * anticlockwise from its start, which is also its end.
 */
enum class ShapeElementOrientation
{
    Anticlockwise,
    Clockwise,
    Full,
};

/**
 * Conversions between an orientation and its name ("Anticlockwise",
 * "Clockwise", "Full"), as written in JSON files, or its first letter.
 */
std::string orientation2str(ShapeElementOrientation type);
ShapeElementOrientation str2orientation(const std::string& str);

char orientation2char(ShapeElementOrientation type);

/** Return the opposite orientation; the opposite of Full is Full. */
ShapeElementOrientation opposite(ShapeElementOrientation orientation);

/**
 * A point at which an element touches a horizontal ray-casting line at
 * some height 'y' (see ShapeElement::ray_crossings).
 */
struct RayCrossing
{
    /** x-coordinate of the touch point. */
    LengthDbl x;

    /**
     * Whether this touch is a genuine, parity-flipping ray-casting
     * crossing (see Shape::contains()), as opposed to a mere touch (a
     * vertex the ray grazes without the boundary actually switching sides,
     * or a CircularArc tangent to the ray at a local y-extremum strictly
     * between its own endpoints) that still lies exactly on the boundary
     * but does not affect ray-casting parity.
     */
    bool is_crossing;
};

/**
 * Structure for the elementary elements composing a shape.
 *
 * The center of a circular arc isn't required to be exactly at the same
 * distance from its start and its end (see docs/conventions.md and
 * recompute_center).
 */
struct ShapeElement
{
    /** Type of element. */
    ShapeElementType type;

    /** Start point of the element. */
    Point start;

    /** End point of the element. */
    Point end;

    /** If the element is a CircularArc, center of the circle. */
    Point center = {0, 0};

    /** If the element is a CircularArc, direction of the rotation. */
    ShapeElementOrientation orientation = ShapeElementOrientation::Anticlockwise;

    /**
     * If the element is a CircularArc, check if a point lies within its
     * angular cone, i.e. if its direction from the center is within the arc.
     * The center itself, the apex of the cone, is within it.
     */
    bool in_circular_arc_cone(const Point& point) const;

    /** Check if a point is on the element. */
    bool contains(const Point& point) const;

    /**
     * Every point at which this element touches a horizontal line at
     * height 'y', each tagged with whether it is a genuine, parity-flipping
     * ray-casting crossing (see RayCrossing) -- 0, 1 or 2 points for a
     * CircularArc; 0 or 1 for a non-horizontal LineSegment. A horizontal
     * LineSegment exactly at height 'y' returns no point here; see
     * ray_horizontal_extent for that case instead.
     *
     * This is the same up/down, include-start / exclude-end classification
     * used by Shape::contains(), extracted so it can be reused
     * independently of any specific query point.
     */
    std::vector<RayCrossing> ray_crossings(LengthDbl y) const;

    /**
     * If this element is a LineSegment exactly horizontal at height 'y',
     * sets 'x_min' and 'x_max' to its extent (every point in
     * ['x_min', 'x_max'] lies exactly on this element, regardless of
     * ray-casting parity) and returns true. Returns false otherwise.
     */
    bool ray_horizontal_extent(
            LengthDbl y,
            LengthDbl& x_min,
            LengthDbl& x_max) const;

    /** Radius of the element. The element must be a CircularArc. */
    LengthDbl radius() const
    {
        if (this->type != ShapeElementType::CircularArc)
            throw std::invalid_argument(FUNC_SIGNATURE);
        return distance(this->center, this->start);
    }

    /** Length of the element. */
    LengthDbl length() const;

    /**
     * Compute the length between the start of the element and a given point.
     *
     * The element must contain the given point.
     */
    LengthDbl length(const Point& point) const;

    /**
     * Return the point of the element at a given length from its start,
     * going in the direction of the element.
     */
    Point point(LengthDbl length) const;

    /**
     * Return the point of the element halfway, along the element, between
     * two points of the element.
     */
    Point find_point_between(
            const Point& point_1,
            const Point& point_2) const;

    /**
     * Return the unit tangent of the element at a given point of it, in the
     * direction of the element.
     */
    Point tangent(
            const Point& point) const;

    /**
     * Get the normal at a given point of the element.
     *
     * The normal is pointing towards the exterior, that is, to the right of
     * the element, which is the exterior for an anticlockwise boundary.
     */
    Point normal(
            const Point& point) const;

    /**
     * Return true iff 'overlapping_part', a part of this element (e.g. from
     * ShapeElementIntersectionsOutput::overlapping_parts), goes in the same
     * direction as this element.
     */
    bool same_direction(
            const ShapeElement& overlapping_part) const;

    /** Translate the element by (x, y). */
    ShapeElement& shift(
            LengthDbl x,
            LengthDbl y);

    /** Translate the element by 'vector'. */
    ShapeElement& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }

    /** Rotate the element anticlockwise around the origin, in degrees. */
    ShapeElement rotate(Angle angle) const;

    /*
     * Mirror the element (see the symmetries of Point). The start and the
     * end are swapped, so that mirroring an anticlockwise boundary element by
     * element gives an anticlockwise boundary.
     */

    ShapeElement axial_symmetry_identity_line() const;

    ShapeElement axial_symmetry_x_axis() const;

    ShapeElement axial_symmetry_y_axis() const;

    /**
     * Return the same element going in the opposite direction: start and end
     * swapped, and the opposite orientation for a circular arc.
     */
    ShapeElement reverse() const;

    /** Get the middle point on the element. */
    Point middle() const { return middle(this->start, this->end); };

    /** Get the middle point between two points on the element. */
    Point middle(
            const Point& point_1,
            const Point& point_2) const;

    /** Compute the smallest and greatest x and y of the shape. */
    AxisAlignedBoundingBox min_max() const;

    /**
     * Compute furthest points according to a given direction.
     *
     * If 'angle == 0', return the lowest and highest points.
     *
     * If 'angle == 90', return the right-most and left-most points.
     */
    std::pair<Point, Point> furthest_points(Angle angle) const;

    /**
     * Extract the sub-element between two points of the element, going in
     * the direction of the element from 'start_point' to 'end_point'.
     */
    ShapeElement extract(const Point& start_point, const Point& end_point) const;

    /**
     * Re-compute the center of a circular arc: return the projection of its
     * center on the perpendicular bisector of its start and its end, which is
     * at the same distance from both.
     *
     * The element must be a CircularArc; it must not be a full circle, whose
     * start and end are the same point.
     */
    Point recompute_center() const;

    std::string to_string() const;

    /**
     * Read an element from a JSON object with "type", "start" and "end", and,
     * for a circular arc, "center" and "orientation".
     */
    template <class basic_json>
    static ShapeElement from_json(basic_json& json_element);

    /** Write the element to a JSON object read by from_json. */
    nlohmann::json to_json() const;

    /** Return the SVG of the element, with the y-axis going up. */
    std::string to_svg() const;

    /** Write the SVG of the element to a file. */
    void write_svg(
            const std::string& file_path) const;
};

/** Build a line segment. */
ShapeElement build_line_segment(
        const Point& start,
        const Point& end);

/** Build a circular arc; its center is used as given. */
ShapeElement build_circular_arc(
        const Point& start,
        const Point& end,
        const Point& center,
        const ShapeElementOrientation& orientation);

/** Scale an element, i.e. its start, end and center, around the origin. */
ShapeElement operator*(
        LengthDbl scalar,
        const ShapeElement& element);

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Shape /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum class ShapeType
{
    Circle,
    Square,
    Rectangle,
    Polygon,
    MultiPolygon,
    PolygonWithHoles,
    MultiPolygonWithHoles,
    GeneralShape,
};

std::string shape2str(ShapeType type);

/** A point of the boundary of a shape, with the position of its element. */
struct ShapePoint
{
    ElementPos element_pos = -1;

    Point point = {0, 0};
};

/**
 * A point of the boundary of a shape with holes: 'shape_pos' is the position
 * of the hole, or -1 for the outline.
 */
struct ShapeWithHolesPoint
{
    ShapePos shape_pos = -2;

    ElementPos element_pos = -1;

    Point point = {0, 0};
};

/**
 * Structure for a shape.
 *
 * A shape is connected and provided in anticlockwise direction. This also
 * holds for the holes of a ShapeWithHoles (see docs/conventions.md).
 */
struct Shape
{
    /**
     * List of elements.
     *
     * The end point of an element must be the start point of the next element.
     */
    std::vector<ShapeElement> elements;

    /**
     * Boolean indicating if the shape is actually a path: an open chain of
     * elements, from the start of the first one to the end of the last one,
     * without interior.
     */
    bool is_path = false;


    /** Return true iff the shape is a circle: a single full circle element. */
    bool is_circle() const;

    /**
     * Return true iff the shape is a square: 4 line segments of equal
     * lengths, turning anticlockwise by right angles (within tolerance).
     */
    bool is_square() const;

    /**
     * Return true iff the shape is a rectangle: 4 line segments turning
     * anticlockwise by right angles (within tolerance).
     */
    bool is_rectangle() const;

    /** Return true iff the shape is closed and made of line segments only. */
    bool is_polygon() const;

    /** Return true iff the shape is a path made of line segments only. */
    bool is_polyline() const;

    /**
     * Return true iff the shape, assumed anticlockwise, is convex: it has no
     * clockwise circular arc and it never turns clockwise (within tolerance)
     * at a vertex. A circle is convex.
     */
    bool is_convex() const;

    /**
     * Compute the signed area of the shape: positive iff it is
     * anticlockwise.
     *
     * Throw std::invalid_argument if the shape is a path.
     */
    AreaDbl compute_area() const;

    /** Compute the length of the boundary of the shape. */
    LengthDbl compute_length() const;

    /**
     * Compute the smallest and greatest x and y of the shape, after
     * mirroring it through the y-axis if 'mirror' is true, then rotating it
     * anticlockwise by 'angle' degrees around the origin.
     */
    AxisAlignedBoundingBox compute_min_max(
            Angle angle = 0.0,
            bool mirror = false) const;

    /**
     * Compute the smallest and greatest x and y of the part of the boundary
     * going forward from 'point_1' to 'point_2'.
     *
     * Throw std::invalid_argument if the element position of a point is out
     * of range.
     */
    AxisAlignedBoundingBox compute_min_max(
            const ShapePoint& point_1,
            const ShapePoint& point_2) const;

    /**
     * Compute the width and height of the bounding box of the shape, mirrored
     * and rotated as in compute_min_max.
     */
    std::pair<LengthDbl, LengthDbl> compute_width_and_height(
            Angle angle = 0.0,
            bool mirror = false) const;

    struct FurthestPoint
    {
        Point point;

        ElementPos element_pos;
    };

    /** Compute furthest points according to a given direction. */
    std::pair<FurthestPoint, FurthestPoint> compute_furthest_points(
            Angle angle) const;

    /**
     * Check if the shape contains a given point.
     *
     * A point on the boundary (within tolerance) is contained iff 'strict' is
     * false.
     */
    bool contains(
            const Point& point,
            bool strict = false) const;

    /** Write the inputs of contains to a JSON file. */
    void contains_export_inputs(
            const std::string& file_path,
            const Point& point,
            bool strict) const;

    /** Find a point strictly inside the shape. */
    Point find_point_strictly_inside() const;

    /**
     * Return true iff 'point_1' comes strictly before 'point_2' (within
     * tolerance) going along the elements from the first one.
     */
    bool is_strictly_closer_to_path_start(
            const ShapePoint& point_1,
            const ShapePoint& point_2) const;

    /**
     * Return a point of the boundary strictly between 'point_1' and
     * 'point_2', going forward from 'point_1' (around the start of the shape
     * if needed, for a closed shape).
     *
     * Throw std::invalid_argument if the element position of a point is out
     * of range.
     */
    ShapePoint find_point_between(
            const ShapePoint& point_1,
            const ShapePoint& point_2) const;

    /**
     * Check if the shape is valid: it has at least one element, the end of
     * each element is the start of the next one (within tolerance), it is
     * closed (within tolerance) unless it is a path, its boundary doesn't
     * intersect itself (see intersect(const Shape&)), and its area isn't
     * negative (within tolerance), i.e. it is anticlockwise.
     *
     * The reason why the shape isn't valid is written to the standard output.
     */
    bool check() const;

    /** Translate the shape by (x, y). */
    Shape& shift(
            LengthDbl x,
            LengthDbl y);

    /** Translate the shape by 'vector'. */
    Shape& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }

    /** Rotate the shape anticlockwise around the origin, in degrees. */
    Shape rotate(Angle angle) const;

    /*
     * Mirror the shape (see the symmetries of Point). The order of the
     * elements is reversed, so that an anticlockwise shape stays
     * anticlockwise.
     */

    Shape axial_symmetry_identity_line() const;

    Shape axial_symmetry_y_axis() const;

    Shape axial_symmetry_x_axis() const;

    /**
     * Return the same shape going in the opposite direction, e.g. clockwise
     * for an anticlockwise shape.
     */
    Shape reverse() const;

    /**
     * Extract the sub-path going forward from point_start to point_end.
     *
     * Throw std::invalid_argument if the element position of a point is out
     * of range, or if the two points are equal (within tolerance).
     */
    Shape extract_path(const ShapePoint& point_start, const ShapePoint& point_end) const;

    struct PathReplacement
    {
        ShapePoint start;
        ShapePoint end;
        std::vector<ShapeElement> path;
    };

    /**
     * Replace parts of the shape by other given paths: for each replacement,
     * the part of the boundary going forward from 'start' to 'end' is
     * replaced by 'path'. The replaced parts must not overlap.
     *
     * Throw std::invalid_argument if a path doesn't start at its 'start' or
     * end at its 'end' (within tolerance).
     */
    Shape replace(const std::vector<PathReplacement>& paths) const;

    /*
     * Export
     */

    std::string to_string(Counter indentation) const;

    /**
     * Read a shape from a JSON object whose "type" is:
     * - "circle", with "radius" and optionally "x" and "y" (its center);
     * - "rectangle", with "width", "height" and optionally "x" and "y" (its
     *   bottom-left corner);
     * - "polygon", with the list of its "vertices";
     * - "general", with the list of its "elements" (see
     *   ShapeElement::from_json) and optionally "is_path".
     *
     * Throw std::invalid_argument for another type.
     */
    template <class basic_json>
    static Shape from_json(basic_json& json_shape);

    /**
     * Read a shape from a JSON file (see from_json).
     *
     * Throw std::runtime_error if the file can't be opened.
     */
    static Shape read_json(
            const std::string& file_path);

    /** Write the shape to a JSON object of type "general". */
    nlohmann::json to_json() const;

    /** Write the shape to a JSON file (see to_json). */
    void write_json(
            const std::string& file_path) const;

    /** Return the SVG path data of the shape, with the y-axis going up. */
    std::string to_svg_path() const;

    /** Return the SVG of the shape, with the y-axis going up. */
    std::string to_svg(
            const std::string& fill_color = "blue") const;

    /** Write the SVG of the shape to a file. */
    void write_svg(
            const std::string& file_path) const;
};

/** Build the triangle (p1, p2, p3); it is anticlockwise iff they are. */
Shape build_triangle(
        const Point& p1,
        const Point& p2,
        const Point& p3);

/**
 * Build the anticlockwise square of side 'size_length' whose bottom-left
 * corner is the origin.
 */
Shape build_square(
        LengthDbl size_length);

/** Build an anticlockwise rectangle. */
Shape build_rectangle(
        LengthDbl x_min,
        LengthDbl x_max,
        LengthDbl y_min,
        LengthDbl y_max);

inline Shape build_rectangle(const Point& bottom_left, const Point& top_right) { return build_rectangle(bottom_left.x, top_right.x, bottom_left.y, top_right.y); }

inline Shape build_rectangle(const Point& p) { return build_rectangle(0, p.x, 0, p.y); }

inline Shape build_rectangle(LengthDbl x, LengthDbl y) { return build_rectangle(0, x, 0, y); }

inline Shape build_rectangle(const AxisAlignedBoundingBox& aabb) { return build_rectangle(aabb.x_min, aabb.x_max, aabb.y_min, aabb.y_max); }

/** Build a circle of radius 'radius' centered at the origin. */
Shape build_circle(LengthDbl radius);

/** Scale a shape around the origin. */
Shape operator*(
        LengthDbl scalar,
        const Shape& shape);

/**
 * Structure for a shape with holes.
 *
 * The outline and the holes are all anticlockwise. A hole may touch the
 * outline or another hole at a single point at most (see
 * docs/conventions.md).
 */
struct ShapeWithHoles
{
    /** Outline. */
    Shape shape;

    /** Holes, each of them anticlockwise. */
    std::vector<Shape> holes;


    /**
     * Check if the shape with holes is valid: the outline isn't a path and
     * is valid, each hole is valid (see Shape::check) and inside the
     * outline.
     *
     * The contacts between the holes and the outline, or between two holes,
     * aren't checked. The reason why the shape isn't valid is written to the
     * standard output.
     */
    bool check() const;

    /** Return true iff the outline and the holes are polygons. */
    bool is_polygon() const;

    /** Translate the shape by (x, y). */
    ShapeWithHoles& shift(
            LengthDbl x,
            LengthDbl y);

    /** Translate the shape by 'vector'. */
    ShapeWithHoles& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }

    /** Rotate the shape anticlockwise around the origin, in degrees. */
    ShapeWithHoles rotate(Angle angle) const;

    /*
     * Mirror the shape (see the symmetries of Shape); the outline and the
     * holes stay anticlockwise.
     */

    ShapeWithHoles axial_symmetry_identity_line() const;

    ShapeWithHoles axial_symmetry_y_axis() const;

    ShapeWithHoles axial_symmetry_x_axis() const;

    /**
     * Compute the area of the shape: the area of the outline minus the areas
     * of the holes.
     */
    AreaDbl compute_area() const;

    /** Compute the smallest and greatest x and y of the shape. */
    AxisAlignedBoundingBox compute_min_max(
            Angle angle = 0.0,
            bool mirror = false) const { return this->shape.compute_min_max(angle, mirror); }

    /**
     * Check if the shape contains a given point: inside the outline and not
     * strictly inside a hole.
     *
     * A point on the boundary of the outline or of a hole (within tolerance)
     * is contained iff 'strict' is false.
     */
    bool contains(
            const Point& point,
            bool strict = false) const;

    /** Find a point strictly inside the shape, i.e. not in a hole. */
    Point find_point_strictly_inside() const;

    /*
     * Export
     */

    std::string to_string(Counter indentation) const;

    /**
     * Read a shape with holes from a JSON object: the outline (see
     * Shape::from_json), and optionally the list of its "holes", each in the
     * same format.
     */
    template <class basic_json>
    static ShapeWithHoles from_json(basic_json& json_shape);

    /**
     * Read a shape with holes from a JSON file (see from_json).
     *
     * Throw std::runtime_error if the file can't be opened.
     */
    static ShapeWithHoles read_json(
            const std::string& file_path);

    /** Write the shape with holes to a JSON object read by from_json. */
    nlohmann::json to_json() const;

    /** Write the shape with holes to a JSON file (see to_json). */
    void write_json(
            const std::string& file_path) const;

    /** Return the SVG of the shape with holes, with the y-axis going up. */
    std::string to_svg(
            const std::string& fill_color = "blue") const;

    /** Write the SVG of the shape with holes to a file. */
    void write_svg(
            const std::string& file_path) const;
};

/** Scale a shape with holes around the origin. */
ShapeWithHoles operator*(
        LengthDbl scalar,
        const ShapeWithHoles& shape);

/**
 * Structure for a set of possibly disjoint shapes with holes, i.e. a
 * multi-shape, the union of several `ShapeWithHoles`.
 */
struct MultiShapeWithHoles
{
    std::vector<ShapeWithHoles> shapes_with_holes;

    /** Translate the shapes by (x, y). */
    MultiShapeWithHoles& shift(
            LengthDbl x,
            LengthDbl y);

    /** Translate the shapes by 'vector'. */
    MultiShapeWithHoles& shift(
            const Point& vector)
    {
        return this->shift(vector.x, vector.y);
    }

    /*
     * Export
     */

    /**
     * Read a multi-shape from a JSON array of shapes with holes (see
     * ShapeWithHoles::from_json).
     */
    template <class basic_json>
    static MultiShapeWithHoles from_json(basic_json& json_shape);

    /**
     * Read a multi-shape from a JSON file (see from_json).
     *
     * Throw std::runtime_error if the file can't be opened.
     */
    static MultiShapeWithHoles read_json(
            const std::string& file_path);

    /** Write the multi-shape to a JSON array read by from_json. */
    nlohmann::json to_json() const;

    /** Write the multi-shape to a JSON file (see to_json). */
    void write_json(
            const std::string& file_path) const;

    /** Return the SVG of the shapes, with the y-axis going up. */
    std::string to_svg(
            const std::string& fill_color = "blue") const;
};

struct BuildShapeElement
{
    LengthDbl x = 0;
    LengthDbl y = 0;

    /**
     * Type of point:
     * - 0: point of the shape
     * - 1: center of an anticlockwise circular arc
     * - -1: center of a clockwise circular arc
     */
    int type = 0;
};

/**
 * Function to help build a shape easily.
 *
 * Espacially useful to write tests.
 *
 * Each point of type 0 is a vertex, linked to the next vertex by a line
 * segment, or by a circular arc if a center (a point of type 1 or -1) is
 * between them. The last vertex is linked back to the first one unless
 * 'is_path' is true.
 *
 * Examples:
 *
 * Build a unit square:
 * build_shape({{0, 0}, {1, 0}, {1, 1}, {0, 1}})
 *
 * Build a right triangle where the hypotenuse is an inflated circular arc
 * (a quarter of the disk of radius 1 centered at the origin)
 * build_shape({{0, 0}, {1, 0}, {0, 0, 1}, {0, 1}})
 *
 * Build a unit square where the side between (1, 0) and (0, 1) is a drilled
 * circular arc centered at (1, 1)
 * build_shape({{0, 0}, {1, 0}, {1, 1, -1}, {0, 1}})
 */
Shape build_shape(
        const std::vector<BuildShapeElement>& points,
        bool is_path = false);

/** Build a closed shape from its elements. */
Shape build_shape(
        const std::vector<ShapeElement>& elements);

/** Same as build_shape with 'is_path' true. */
Shape build_path(
        const std::vector<BuildShapeElement>& points);

/** Build a path from its elements. */
Shape build_path(
        const std::vector<ShapeElement>& elements);

/*
 * Equality of elements and shapes. operator== compares the coordinates
 * exactly, and equal compares them within tolerance (see equal(const Point&,
 * const Point&)).
 *
 * Two elements are equal if they have the same type, start, end and, for
 * circular arcs, center and orientation. With equal, two full circles are
 * equal if they have the same center and radius, whatever their starts.
 *
 * Two shapes are equal if they have the same elements in the same order. For
 * closed shapes, they may start at different elements; paths must start at
 * the same element. Two shapes with holes are equal if their outlines are
 * equal and their holes are equal, in any order.
 */

bool operator==(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

bool equal(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

bool operator==(
        const Shape& shape_1,
        const Shape& shape_2);

bool equal(
        const Shape& shape_1,
        const Shape& shape_2);

bool operator==(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

bool equal(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2);

template <class basic_json>
Point Point::from_json(basic_json& json_element)
{
    Point point;
    point.x = json_element["x"];
    point.y = json_element["y"];
    return point;
}

template <class basic_json>
ShapeElement ShapeElement::from_json(basic_json& json_element)
{
    ShapeElement element;
    element.type = str2element(json_element["type"]);
    element.start = Point::from_json(json_element["start"]);
    element.end = Point::from_json(json_element["end"]);
    if (element.type == ShapeElementType::CircularArc) {
        element.center = Point::from_json(json_element["center"]);
        element.orientation = str2orientation(json_element["orientation"]);
    }
    return element;
}

template <class basic_json>
Shape Shape::from_json(basic_json& json_shape)
{
    Shape shape;
    if (json_shape["type"] == "circle") {
        ShapeElement element;
        LengthDbl x = json_shape.value("x", 0.0);
        LengthDbl y = json_shape.value("y", 0.0);
        LengthDbl radius = json_shape["radius"];
        element.type = ShapeElementType::CircularArc;
        element.orientation = ShapeElementOrientation::Full;
        element.center = {x, y};
        element.start = {x + radius, y};
        element.end = element.start;
        shape.elements.push_back(element);
    } else if (json_shape["type"] == "rectangle") {
        LengthDbl x = json_shape.value("x", 0.0);
        LengthDbl y = json_shape.value("y", 0.0);
        LengthDbl width = json_shape["width"];
        LengthDbl height = json_shape["height"];
        ShapeElement element_1;
        ShapeElement element_2;
        ShapeElement element_3;
        ShapeElement element_4;
        element_1.type = ShapeElementType::LineSegment;
        element_2.type = ShapeElementType::LineSegment;
        element_3.type = ShapeElementType::LineSegment;
        element_4.type = ShapeElementType::LineSegment;
        element_1.start = {x, y};
        element_1.end = {x + width, y};
        element_2.start = {x + width, y};
        element_2.end = {x + width, y + height};
        element_3.start = {x + width, y + height};
        element_3.end = {x, y + height};
        element_4.start = {x, y + height};
        element_4.end = {x, y};
        shape.elements.push_back(element_1);
        shape.elements.push_back(element_2);
        shape.elements.push_back(element_3);
        shape.elements.push_back(element_4);
    } else if (json_shape["type"] == "polygon") {
        for (auto it = json_shape["vertices"].begin();
                it != json_shape["vertices"].end();
                ++it) {
            auto it_next = it + 1;
            if (it_next == json_shape["vertices"].end())
                it_next = json_shape["vertices"].begin();
            ShapeElement element;
            element.type = ShapeElementType::LineSegment;
            element.start = {(*it)["x"], (*it)["y"]};
            element.end = {(*it_next)["x"], (*it_next)["y"]};
            shape.elements.push_back(element);
        }
    } else if (json_shape["type"] == "general") {
        for (auto it = json_shape["elements"].begin();
                it != json_shape["elements"].end();
                ++it) {
            ShapeElement element = ShapeElement::from_json(*it);
            shape.elements.push_back(element);
        }
        if (json_shape.contains("is_path"))
            shape.is_path = json_shape["is_path"];
    } else {
        throw std::invalid_argument(FUNC_SIGNATURE);
    }
    return shape;
}

template <class basic_json>
ShapeWithHoles ShapeWithHoles::from_json(basic_json& json_shape)
{
    ShapeWithHoles shape;
    shape.shape = Shape::from_json(json_shape);

    if (json_shape.contains("holes")) {
        for (auto it_hole = json_shape["holes"].begin();
                it_hole != json_shape["holes"].end();
                ++it_hole) {
            auto json_hole = *it_hole;
            Shape hole = Shape::from_json(json_hole);
            shape.holes.push_back(hole);
        }
    }

    return shape;
}

template <class basic_json>
MultiShapeWithHoles MultiShapeWithHoles::from_json(basic_json& json_shape)
{
    MultiShapeWithHoles multi_shape;
    for (auto& json_shape_with_holes: json_shape.items())
        multi_shape.shapes_with_holes.push_back(
                ShapeWithHoles::from_json(json_shape_with_holes.value()));
    return multi_shape;
}

}
