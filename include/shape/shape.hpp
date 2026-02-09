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

namespace shape
{

using LengthDbl = double;
using AreaDbl = double;
using Angle = double;
using ElementPos = int64_t;
using ItemShapePos = int64_t;
using ShapePos = int64_t;
using Counter = int64_t;

inline bool strictly_lesser(double v1, double v2)
{
    return v2 - v1 > 1e-6;
}

inline bool strictly_greater(double v1, double v2)
{
    return v1 - v2 > 1e-6;
}

inline bool equal(double v1, double v2)
{
    return std::abs(v1 - v2) <= 1e-6;
}

LengthDbl largest_power_of_two_lesser_or_equal(LengthDbl value);

LengthDbl smallest_power_of_two_greater_or_equal(LengthDbl value);

std::string to_string(double value);

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
     */

    Point& shift(
            LengthDbl x,
            LengthDbl y);

    Point rotate(Angle angle) const;

    Point rotate(const Point& center, Angle angle) const;

    Point rotate_radians(Angle angle) const;

    Point rotate_radians(const Point& center, Angle angle) const;

    Point axial_symmetry_identity_line() const;

    Point axial_symmetry_y_axis() const;

    Point axial_symmetry_x_axis() const;

    /*
     * Export
     */

    std::string to_string() const;

    template <class basic_json>
    static Point from_json(basic_json& json_element);

    nlohmann::json to_json() const;

    std::string to_svg() const;
};

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

LengthDbl distance_point_to_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2);

inline LengthDbl signed_distance_point_to_line(
        const Point& point,
        const Point& line_point_1,
        const Point& line_point_2)
{
    return ((line_point_2.y - line_point_1.y) * point.x
            - (line_point_2.x - line_point_1.x) * point.y
            + line_point_2.x * line_point_1.y
            - line_point_2.y * line_point_1.x)
        / distance(line_point_1, line_point_2);
}

LengthDbl dot_product(
        const Point& vector_1,
        const Point& vector_2);

LengthDbl cross_product(
        const Point& vector_1,
        const Point& vector_2);

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

int counter_clockwise(
        const Point& point_1,
        const Point& point_2,
        const Point& point_3);

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// ShapeElement /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

enum class ShapeElementType
{
    LineSegment,
    CircularArc,
};

std::string element2str(ShapeElementType type);
ShapeElementType str2element(const std::string& str);

char element2char(ShapeElementType type);

enum class ShapeElementOrientation
{
    Anticlockwise,
    Clockwise,
    Full,
};

std::string orientation2str(ShapeElementOrientation type);
ShapeElementOrientation str2orientation(const std::string& str);

char orientation2char(ShapeElementOrientation type);

ShapeElementOrientation opposite(ShapeElementOrientation orientation);

/**
 * Structure for a jet of an element.
 */
struct Jet
{
    /** Angle of the tangent. */
    Angle tangent_angle = 0.0;

    /** Curvature. */
    LengthDbl curvature = 0.0;


    std::string to_string() const;
};

Jet operator-(
        const Jet& jet_1,
        const Jet& jet_2);

/**
 * Structure for the elementary elements composing a shape.
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


    /** Check if a point is on the element. */
    bool contains(const Point& point) const;

    /** Length of the element. */
    LengthDbl length() const;

    /**
     * Compute the length between the start of the element and a given point.
     *
     * The element must contain the given point.
     */
    LengthDbl length(const Point& point) const;

    Point point(LengthDbl length) const;

    Point find_point_between(
            const Point& point_1,
            const Point& point_2) const;

    /** Jet of the element. */
    Jet jet(
            const Point& point,
            bool reverse) const;

    bool same_direction(
            const ShapeElement& overlapping_part) const;

    ShapeElement& shift(
            LengthDbl x,
            LengthDbl y);

    ShapeElement rotate(Angle angle) const;

    ShapeElement axial_symmetry_identity_line() const;

    ShapeElement axial_symmetry_x_axis() const;

    ShapeElement axial_symmetry_y_axis() const;

    ShapeElement reverse() const;

    /** Get the middle point on the element. */
    Point middle() const { return middle(this->start, this->end); };

    /** Get the middle point between two points on the element. */
    Point middle(
            const Point& point_1,
            const Point& point_2) const;

    /** Compute the smallest and greatest x and y of the shape. */
    std::pair<Point, Point> min_max() const;

    /**
     * Compute furthest points according to a given direction.
     *
     * If 'angle == 0', return the lowest and highest points.
     *
     * If 'angle == 90', return the right-most and left-most points.
     */
    std::pair<Point, Point> furthest_points(Angle angle) const;

    /** Split an element at a given point. */
    std::pair<ShapeElement, ShapeElement> split(const Point& point) const;

    /** Re-compute the center of a circular arc. */
    Point recompute_center() const;

    std::string to_string() const;

    template <class basic_json>
    static ShapeElement from_json(basic_json& json_element);

    nlohmann::json to_json() const;

    std::string to_svg() const;

    void write_svg(
            const std::string& file_path) const;
};

ShapeElement build_line_segment(
        const Point& start,
        const Point& end);

ShapeElement build_circular_arc(
        const Point& start,
        const Point& end,
        const Point& center,
        const ShapeElementOrientation& orientation);

bool operator<(
        const ShapeElement& element_1,
        const ShapeElement& element_2);

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

struct ShapePoint
{
    ElementPos element_pos = -1;

    Point point = {0, 0};
};

struct ShapeWithHolesPoint
{
    ShapePos shape_pos = -2;

    ElementPos element_pos = -1;

    Point point = {0, 0};
};

/**
 * Structure for a shape.
 *
 * A shape is connected and provided in anticlockwise direction.
 */
struct Shape
{
    /**
     * List of elements.
     *
     * The end point of an element must be the start point of the next element.
     */
    std::vector<ShapeElement> elements;

    /** Boolean indicating if the shape is actually a path. */
    bool is_path = false;


    /** Return true iff the shape is a circle. */
    bool is_circle() const;

    /** Return true iff the shape is a square. */
    bool is_square() const;

    /** Return true iff the shape is a rectangle. */
    bool is_rectangle() const;

    /** Return true iff the shape is a polygon. */
    bool is_polygon() const;

    /** Compute the area of the shape. */
    AreaDbl compute_area() const;

    /** Compute the smallest and greatest x and y of the shape. */
    std::pair<Point, Point> compute_min_max(
            Angle angle = 0.0,
            bool mirror = false) const;

    /** Compute the width and length of the shape. */
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

    /** Check if the shape contains a given point. */
    bool contains(
            const Point& point,
            bool strict = false) const;

    /** Find a point strictly inside the shape. */
    Point find_point_strictly_inside() const;

    /** Determine which point is closer from the start of the path. */
    bool is_strictly_closer_to_path_start(
            const ShapePoint& point_1,
            const ShapePoint& point_2) const;

    ShapePoint find_point_between(
            const ShapePoint& point_1,
            const ShapePoint& point_2) const;

    /* Check if the shape is connected and in anticlockwise direction. */
    bool check() const;

    Shape& shift(
            LengthDbl x,
            LengthDbl y);

    Shape rotate(Angle angle) const;

    Shape axial_symmetry_identity_line() const;

    Shape axial_symmetry_y_axis() const;

    Shape axial_symmetry_x_axis() const;

    Shape reverse() const;

    /** Split a shape/path at given points. */
    std::vector<Shape> split(const std::vector<ShapePoint>& points) const;

    /*
     * Export
     */

    std::string to_string(Counter indentation) const;

    template <class basic_json>
    static Shape from_json(basic_json& json_shape);

    static Shape read_json(
            const std::string& file_path);

    nlohmann::json to_json() const;

    void write_json(
            const std::string& file_path) const;

    std::string to_svg_path() const;

    std::string to_svg(
            const std::string& fill_color = "blue") const;

    void write_svg(
            const std::string& file_path) const;
};

Shape build_triangle(
        const Point& p1,
        const Point& p2,
        const Point& p3);

Shape build_square(
        LengthDbl size_length);

Shape build_rectangle(
        const Point& p1,
        const Point& p2);

inline Shape build_rectangle(const Point& p) { return build_rectangle({0, 0}, p); }

inline Shape build_rectangle(LengthDbl x, LengthDbl y) { return build_rectangle({0, 0}, {x, y}); }

Shape build_circle(LengthDbl radius);

Shape operator*(
        LengthDbl scalar,
        const Shape& shape);

/**
 * Structure for a shape with holes.
 */
struct ShapeWithHoles
{
    Shape shape;

    std::vector<Shape> holes;


    /** Compute the area of the shape. */
    AreaDbl compute_area() const;

    /** Compute the smallest and greatest x and y of the shape. */
    std::pair<Point, Point> compute_min_max(
            Angle angle = 0.0,
            bool mirror = false) const { return this->shape.compute_min_max(angle, mirror); }

    /** Check if the shape contains a given point. */
    bool contains(
            const Point& point,
            bool strict = false) const;

    /** Find a point strictly inside the shape. */
    Point find_point_strictly_inside() const;

    /*
     * Export
     */

    std::string to_string(Counter indentation) const;

    template <class basic_json>
    static ShapeWithHoles from_json(basic_json& json_shape);

    static ShapeWithHoles read_json(
            const std::string& file_path);

    nlohmann::json to_json() const;

    void write_json(
            const std::string& file_path) const;

    std::string to_svg(
            const std::string& fill_color = "blue") const;

    void write_svg(
            const std::string& file_path) const;
};

ShapeWithHoles operator*(
        LengthDbl scalar,
        const ShapeWithHoles& shape);

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
 * Examples:
 *
 * Build a polygon:
 *
 * Build a right triangle where the hypotenuse is an inflated circular arc
 * build_shape({{0, 0}, {1, 0}, {0, 0, 1}, {1, 1}})
 *
 * Build a right triangle where the hypotenuse is a drilled circular arc
 * build_shape({{0, 0}, {1, 0}, {0, 0, -1}, {1, 1}})
 */
Shape build_shape(
        const std::vector<BuildShapeElement>& points,
        bool is_path = false);

Shape build_shape(
        const std::vector<ShapeElement>& elements);

Shape build_path(
        const std::vector<BuildShapeElement>& points);

Shape build_path(
        const std::vector<ShapeElement>& elements);

inline bool operator==(
        const Point& point_1,
        const Point& point_2)
{
    return (point_1.x == point_2.x) && (point_1.y == point_2.y);
}

inline bool equal(
        const Point& point_1,
        const Point& point_2)
{
    return equal(point_1.x, point_2.x) && equal(point_1.y, point_2.y);
}

bool operator==(
        const Jet& jet_1,
        const Jet& jet_2);

bool operator<(
        const Jet& jet_1,
        const Jet& jet_2);

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

}
