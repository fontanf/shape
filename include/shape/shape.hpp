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

    Point rotate_radians(Angle angle) const;

    Point rotate_radians(const Point& center, Angle angle) const;

    Point axial_symmetry_identity_line() const;

    Point axial_symmetry_y_axis() const;

    Point axial_symmetry_x_axis() const;

    /*
     * Export
     */

    std::string to_string() const;
};

Point operator+(
        const Point& point_1,
        const Point& point_2);

Point operator-(
        const Point& point_1,
        const Point& point_2);

Point operator*(
        LengthDbl scalar,
        const Point& point);

LengthDbl norm(
        const Point& vector);

LengthDbl squared_norm(
        const Point& vector);

LengthDbl distance(
        const Point& point_1,
        const Point& point_2);

LengthDbl squared_distance(
        const Point& point_1,
        const Point& point_2);

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
    bool anticlockwise = true;

    /** Length of the element. */
    LengthDbl length() const;

    /** Check if a point is on the element. */
    bool contains(const Point& point) const;

    ShapeElement& shift(
            LengthDbl x,
            LengthDbl y);

    ShapeElement rotate(Angle angle) const;

    ShapeElement axial_symmetry_identity_line() const;

    ShapeElement axial_symmetry_x_axis() const;

    ShapeElement axial_symmetry_y_axis() const;

    ShapeElement reverse() const;

    /** Get the middle point on the element. */
    Point middle() const;

    /** Compute the smallest and greatest x and y of the shape. */
    std::pair<Point, Point> min_max() const;

    std::string to_string() const;

    nlohmann::json to_json() const;
};

ShapeElement operator*(
        LengthDbl scalar,
        const ShapeElement& element);

/**
 * Convert a shape element of type CircularArc into multiple shape elements
 * of type LineSegment.
 */
std::vector<ShapeElement> approximate_circular_arc_by_line_segments(
        const ShapeElement& circular_arc,
        ElementPos number_of_line_segments,
        bool outer);

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

    /** Check if the shape contains a given point. */
    bool contains(
            const Point& point,
            bool strict = false) const;

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

    std::string to_string(Counter indentation) const;

    nlohmann::json to_json() const;

    std::string to_svg(double factor) const;

    void write_svg(
            const std::string& file_path) const;
};

Shape operator*(
        LengthDbl scalar,
        const Shape& shape);

Shape approximate_by_line_segments(
        const Shape& shape,
        ElementPos number_of_line_segments,
        bool outer);

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
        bool path = false);

double compute_svg_factor(double width);

std::string to_svg(
        const Shape& shape,
        const std::vector<Shape>& holes,
        double factor,
        const std::string& fill_color = "blue");

void write_svg(
        const Shape& shape,
        const std::vector<Shape>& holes,
        const std::string& file_path);

std::pair<bool, Shape> remove_redundant_vertices(
        const Shape& shape);

std::pair<bool, Shape> remove_aligned_vertices(
        const Shape& shape);

std::pair<bool, Shape> clean_extreme_slopes(
        const Shape& shape,
        bool outer);

Shape clean_shape(
        const Shape& shape,
        bool outer);

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

}
