#pragma once

#include "shape/shape.hpp"

#include <iostream>

namespace shape
{

using TrapezoidPos = int64_t;

/**
 * Class for a generalized trapezoid which parallel edges are parallel to the
 * x-axis.
 */
class GeneralizedTrapezoid
{

public:

    /** Standard constructor. */
    inline GeneralizedTrapezoid(
            LengthDbl yb,
            LengthDbl yt,
            LengthDbl xbl,
            LengthDbl xbr,
            LengthDbl xtl,
            LengthDbl xtr):
        yb_(yb),
        yt_(yt),
        xbl_(xbl),
        xbr_(xbr),
        xtl_(xtl),
        xtr_(xtr)
    {
        // Checks.
        if (yb_ >= yt_) {
            throw std::logic_error(
                    FUNC_SIGNATURE + "; "
                    "yb: " + std::to_string(yb) + "; "
                    "yt: " + std::to_string(yt) + ".");
        }
        if (strictly_greater(xbl_, xbr_)) {
            throw std::logic_error(
                    FUNC_SIGNATURE + "; "
                    "xbl: " + std::to_string(xbl) + "; "
                    "xbr: " + std::to_string(xbr) + ".");
        }
        if (strictly_greater(xtl_, xtr_)) {
            throw std::logic_error(
                    FUNC_SIGNATURE + "; "
                    "xtl: " + std::to_string(xtl) + "; "
                    "xtr: " + std::to_string(xtr) + ".");
        }

        width_top_ = x_top_right() - x_top_left();
        width_bottom_ = x_bottom_right() - x_bottom_left();
        height_ = y_top() - y_bottom();
        area_ = (width_top() + width_bottom()) * height() / 2.0;

        a_left_ = (x_top_left() - x_bottom_left()) / (y_top() - y_bottom());
        a_right_ = (x_top_right() - x_bottom_right()) / (y_top() - y_bottom());

        x_min_ = std::min(x_bottom_left(), x_top_left());
        x_max_ = std::max(x_bottom_right(), x_top_right());

        left_side_increasing_not_vertical_ = (a_left() > 0);
        left_side_decreasing_not_vertical_ = (a_left() < -0);
        right_side_increasing_not_vertical_ = (a_right() > 0);
        right_side_decreasing_not_vertical_ = (a_right() < -0);
    }

    /** Translate the generalized trapezoid by 'l' along the y-axis. */
    void shift_top(LengthDbl l)
    {
        yb_ += l;
        yt_ += l;
    }

    /** Translate the generalized trapezoid by 'l' along the x-axis. */
    void shift_right(LengthDbl l)
    {
        xbl_ += l;
        xbr_ += l;
        xtl_ += l;
        xtr_ += l;
        x_min_ += l;
        x_max_ += l;
    }

    /**
     * Replace the left side of the generalized trapezoid by the vertical
     * side at x = 'l', which must be on the left of both left corners.
     */
    void extend_left(LengthDbl l)
    {
        xbl_ = l;
        xtl_ = l;
        x_min_ = l;
        left_side_decreasing_not_vertical_ = false;
        left_side_increasing_not_vertical_ = false;
        a_left_ = 0;
    }

    /**
     * Return true iff two generalized trapezoids are equal: the
     * y-coordinates are compared exactly, and the x-coordinates within
     * tolerance.
     */
    bool operator==(const GeneralizedTrapezoid& trapezoid) const
    {
        return (y_bottom() == trapezoid.y_bottom()
                && y_top() == trapezoid.y_top()
                && equal(x_bottom_left(), trapezoid.x_bottom_left())
                && equal(x_bottom_right(), trapezoid.x_bottom_right())
                && equal(x_top_left(), trapezoid.x_top_left())
                && equal(x_top_right(), trapezoid.x_top_right()));
    }

    /*
     * Getters
     */

    /** Get the y-coordinate of the bottom side of the generalized trapezoid. */
    inline LengthDbl y_bottom() const { return yb_; }

    /** Get the y-coordinate of the top side of the generalized trapezoid. */
    inline LengthDbl y_top() const { return yt_; }

    /** Get the x-coordinate of the bottom-left corner of the trapezoid. */
    inline LengthDbl x_bottom_left() const { return xbl_; }

    /** Get the x-coordinate of the bottom-right corner of the trapezoid. */
    inline LengthDbl x_bottom_right() const { return xbr_; }

    /** Get the x-coordinate of the top-left corner of the trapezoid. */
    inline LengthDbl x_top_left() const { return xtl_; }

    /** Get the x-coordinate of the top-right corner of the trapezoid. */
    inline LengthDbl x_top_right() const { return xtr_; }

    /** Get the height of the generalized trapezoid. */
    inline LengthDbl height() const { return height_; }

    /** Get the bottom width of the generalized trapezoid. */
    inline LengthDbl width_bottom() const { return width_bottom_; }

    /** Get the top width of the generalized trapezoid. */
    inline LengthDbl width_top() const { return width_top_; }

    /** Get the greatest x of the generalized trapezoid. */
    inline LengthDbl x_max() const { return x_max_; }

    /** Get the smallest x of the generalized trapezoid. */
    inline LengthDbl x_min() const { return x_min_; }

    /** Get the area of the generalized trapezoid. */
    inline AreaDbl area() const { return area_; }

    /**
     * Get the slope of the left side, as the variation of x for a unit
     * increase of y: 0 if the side is vertical, positive if it goes to the
     * right going up.
     */
    inline double a_left() const { return a_left_; }

    /** Get the slope of the right side, in the same way as a_left. */
    inline double a_right() const { return a_right_; }

    /*
     * Directions of the sides going up: x increasing (to the right) or
     * decreasing (to the left), the side not being vertical.
     */

    inline bool left_side_increasing_not_vertical() const { return left_side_increasing_not_vertical_; }

    inline bool left_side_decreasing_not_vertical() const { return left_side_decreasing_not_vertical_; }

    inline bool right_side_increasing_not_vertical() const { return right_side_increasing_not_vertical_; }

    inline bool right_side_decreasing_not_vertical() const { return right_side_decreasing_not_vertical_; }


    /** Get the x-coordinate of a point on the left side of the trapezoid. */
    inline LengthDbl x_left(LengthDbl y) const
    {
        if (equal(y, y_bottom()))
            return x_bottom_left();
        if (equal(y, y_top()))
            return x_top_left();
        if (equal(x_bottom_left(), x_top_left()))
            return x_bottom_left();
        return x_bottom_left() + (y - y_bottom()) * a_left_;
    }

    /** Get the x-coordinate of a point on the right side of the trapezoid. */
    inline LengthDbl x_right(LengthDbl y) const
    {
        if (equal(y, y_bottom()))
            return x_bottom_right();
        if (equal(y, y_top()))
            return x_top_right();
        if (equal(x_bottom_right(), x_top_right()))
            return x_bottom_right();
        return x_bottom_right() + (y - y_bottom()) * a_right_;
    }

    /**
     * Get the area of the part of the generalized trapezoid on the right of
     * the vertical line x = 'x_left', which must be on the right of both left
     * corners.
     *
     * Throw std::invalid_argument if 'x_left' is on the right of both right
     * corners.
     */
    AreaDbl area(LengthDbl x_left) const
    {
        if (strictly_greater(x_left, x_bottom_right())) {
            if (strictly_greater(x_left, x_top_right())) {
                throw std::invalid_argument(
                        FUNC_SIGNATURE + "; "
                        "x_left: " + std::to_string(x_left) + "; "
                        "x_top_right: " + std::to_string(x_top_right()) + "; "
                        "x_bottom_right: " + std::to_string(x_bottom_right()) + ".");
            }
            double k = (x_top_right() - x_bottom_right()) / (x_top_right() - x_left);
            return (x_top_right() - x_bottom_right()) * height() / 2.0 / k / k;
        } else if (strictly_greater(x_left, x_top_right())) {
            double k = (x_bottom_right() - x_top_right()) / (x_bottom_right() - x_left);
            return (x_bottom_right() - x_top_right()) * height() / 2.0 / k / k;
        }
        LengthDbl width_top = x_top_right() - x_left;
        LengthDbl width_bottom = x_bottom_right() - x_left;
        return (width_top + width_bottom) * height() / 2.0;
    }

    /** Check of the generalized trapezoid intersects another generalized trapezoid. */
    inline bool intersect(
            const GeneralizedTrapezoid& trapezoid) const
    {
        if (!strictly_lesser(y_bottom(), trapezoid.y_top()))
            return false;
        if (!strictly_greater(y_top(), trapezoid.y_bottom()))
            return false;

        LengthDbl yb = std::max(y_bottom(), trapezoid.y_bottom());
        LengthDbl yt = std::min(y_top(), trapezoid.y_top());

        LengthDbl x1br = x_right(yb);
        LengthDbl x1tr = x_right(yt);
        LengthDbl x2bl = trapezoid.x_left(yb);
        LengthDbl x2tl = trapezoid.x_left(yt);
        if (!strictly_greater(x1br, x2bl)
                && !strictly_greater(x1tr, x2tl))
            return false;

        LengthDbl x1bl = x_left(yb);
        LengthDbl x1tl = x_left(yt);
        LengthDbl x2br = trapezoid.x_right(yb);
        LengthDbl x2tr = trapezoid.x_right(yt);
        if (!strictly_lesser(x1bl, x2br)
                && !strictly_lesser(x1tl, x2tr))
            return false;

        return true;
    }

    /**
     * Compute by how much the generalized trapezoid must be shifted to be right
     * next to another generalized trapezoid.
     */
    inline LengthDbl compute_right_shift(
            const GeneralizedTrapezoid& trapezoid) const
    {
        if (!strictly_lesser(y_bottom(), trapezoid.y_top()))
            return 0.0;
        if (!strictly_greater(y_top(), trapezoid.y_bottom()))
            return 0.0;

        LengthDbl yb = std::max(y_bottom(), trapezoid.y_bottom());
        LengthDbl yt = std::min(y_top(), trapezoid.y_top());
        //std::cout << "yb " << yb << " yt " << yt << std::endl;

        LengthDbl x1bl = x_left(yb);
        LengthDbl x1tl = x_left(yt);
        LengthDbl x2br = trapezoid.x_right(yb);
        LengthDbl x2tr = trapezoid.x_right(yt);
        //std::cout << "x1bl " << x1bl << " x1tl " << x1tl << std::endl;
        //std::cout << "x2br " << x2br << " x2tr " << x2tr << std::endl;
        if (!strictly_lesser(x1bl, x2br)
                && !strictly_lesser(x1tl, x2tr))
            return 0.0;

        return std::max(x2br - x1bl, x2tr - x1tl);
    }

    /**
     * Compute by how much the generalized trapezoid must be shifted to be right
     * next to another generalized trapezoid, only if they already intersect.
     */
    inline LengthDbl compute_right_shift_if_intersects(
            const GeneralizedTrapezoid& trapezoid) const
    {
        if (!strictly_lesser(y_bottom(), trapezoid.y_top()))
            return 0.0;
        if (!strictly_greater(y_top(), trapezoid.y_bottom()))
            return 0.0;

        LengthDbl yb = std::max(y_bottom(), trapezoid.y_bottom());
        LengthDbl yt = std::min(y_top(), trapezoid.y_top());

        LengthDbl x1br = x_right(yb);
        LengthDbl x1tr = x_right(yt);
        LengthDbl x2bl = trapezoid.x_left(yb);
        LengthDbl x2tl = trapezoid.x_left(yt);
        if (!strictly_greater(x1br, x2bl)
                && !strictly_greater(x1tr, x2tl))
            return 0.0;

        LengthDbl x1bl = x_left(yb);
        LengthDbl x1tl = x_left(yt);
        LengthDbl x2br = trapezoid.x_right(yb);
        LengthDbl x2tr = trapezoid.x_right(yt);
        if (!strictly_lesser(x1bl, x2br)
                && !strictly_lesser(x1tl, x2tr))
            return 0.0;

        return std::max(x2br - x1bl, x2tr - x1tl);
    }

    /**
     * Same as compute_right_shift, but moving the generalized trapezoid along
     * the direction of slope 'a', towards increasing x: return by how much
     * its x must increase (its y increasing by 'a' times as much) for it to be
     * right next to the other generalized trapezoid, that is, the largest
     * shift at which a vertex of one of them meets a side of the other.
     *
     * Return 0 if no such shift exists. 'a' must be non-zero; use
     * compute_right_shift for a horizontal move.
     */
    inline LengthDbl compute_top_right_shift(
            const GeneralizedTrapezoid& trapezoid,
            double a) const
    {
        LengthDbl x_shift = 0.0;

        for (const Point& p: std::array<Point, 4>{
                Point{x_bottom_left(), y_bottom()},
                Point{x_bottom_right(), y_bottom()},
                Point{x_top_left(), y_top()},
                Point{x_top_right(), y_top()}}) {
            //std::cout << "p1 " << p.to_string() << std::endl;
            LengthDbl b = p.y - p.x * a;

            // Bottom side of second trapezoid.
            {
                LengthDbl x = (trapezoid.y_bottom() - b) / a;
                //std::cout << "bottom:"
                //    << " x " << x
                //    << " x_shift " << x - p.x
                //    << " y_shift " << trapezoid.y_bottom() - p.y
                //    << std::endl;
                if (strictly_greater(x, p.x)
                        && !strictly_lesser(x, trapezoid.x_bottom_left())
                        && !strictly_greater(x, trapezoid.x_bottom_right())) {
                    x_shift = std::max(x_shift, x - p.x);
                }
            }

            // Top side of second trapezoid.
            {
                LengthDbl x = (trapezoid.y_top() - b) / a;
                //std::cout << "top:"
                //    << " x " << x
                //    << " x_shift " << x - p.x
                //    << " y_shift " << trapezoid.y_top() - p.y
                //    << std::endl;
                if (strictly_greater(x, p.x)
                        && !strictly_lesser(x, trapezoid.x_top_left())
                        && !strictly_greater(x, trapezoid.x_top_right())) {
                    x_shift = std::max(x_shift, x - p.x);
                }
            }

            // Left side of second trapezoid.
            {
                double a_left = 1.0 / trapezoid.a_left_;
                LengthDbl b_left = trapezoid.y_bottom() - a_left * trapezoid.x_bottom_left();
                bool intersects = true;
                LengthDbl x = 0;
                LengthDbl y = 0;
                if (trapezoid.a_left() == 0) {
                    x = trapezoid.x_top_left();
                    y = a * x + b;
                } else if (!equal(a, a_left)) {
                    x = (b_left - b) / (a - a_left);
                    y = a * x + b;
                } else if (equal(b, b_left)) {
                    x = trapezoid.x_top_left();
                    y = trapezoid.y_top();
                } else {
                    // Parallel sides which are not on the same line.
                    intersects = false;
                }
                if (intersects
                        && strictly_greater(x, p.x)
                        && !strictly_lesser(y, trapezoid.y_bottom())
                        && !strictly_greater(y, trapezoid.y_top())) {
                    //std::cout << "left:"
                    //    << " x_shift " << x - p.x
                    //    << " y_shift " << y - p.y
                    //    << std::endl;
                    x_shift = std::max(x_shift, x - p.x);
                }
            }

            // Right side of second trapezoid.
            {
                double a_right = 1.0 / trapezoid.a_right_;
                LengthDbl b_right = trapezoid.y_bottom() - a_right * trapezoid.x_bottom_right();
                bool intersects = true;
                LengthDbl x = 0;
                LengthDbl y = 0;
                if (trapezoid.a_right() == 0) {
                    x = trapezoid.x_top_right();
                    y = a * x + b;
                } else if (!equal(a, a_right)) {
                    x = (b_right - b) / (a - a_right);
                    y = a * x + b;
                } else if (equal(b, b_right)) {
                    x = trapezoid.x_top_right();
                    y = trapezoid.y_top();
                } else {
                    // Parallel sides which are not on the same line.
                    intersects = false;
                }
                //std::cout << "right:"
                //    << " b " << b
                //    << " b_right " << b_right
                //    << " a " << a
                //    << " a_right " << a_right
                //    << " x " << x
                //    << " y " << y
                //    << " x_shift " << x - p.x
                //    << " y_shift " << y - p.y
                //    << std::endl;
                if (intersects
                        && strictly_greater(x, p.x)
                        && !strictly_lesser(y, trapezoid.y_bottom())
                        && !strictly_greater(y, trapezoid.y_top())) {
                    x_shift = std::max(x_shift, x - p.x);
                }
            }
        }

        for (const Point& p: std::array<Point, 4>{
                Point{trapezoid.x_bottom_left(), trapezoid.y_bottom()},
                Point{trapezoid.x_bottom_right(), trapezoid.y_bottom()},
                Point{trapezoid.x_top_left(), trapezoid.y_top()},
                Point{trapezoid.x_top_right(), trapezoid.y_top()}}) {
            //std::cout << "p2 " << p.to_string() << std::endl;
            LengthDbl b = p.y - p.x * a;

            // Bottom side of second trapezoid.
            {
                LengthDbl x = (y_bottom() - b) / a;
                //std::cout << "bottom:"
                //    << " x " << x
                //    << " x_shift " << p.x - x
                //    << " y_shift " << p.y - y_bottom()
                //    << std::endl;
                if (strictly_lesser(x, p.x)
                        && !strictly_lesser(x, x_bottom_left())
                        && !strictly_greater(x, x_bottom_right())) {
                    x_shift = std::max(x_shift, p.x - x);
                }
            }

            // Top side of second trapezoid.
            {
                LengthDbl x = (y_top() - b) / a;
                //std::cout << "top:"
                //    << " x " << x
                //    << " x_shift " << p.x - x
                //    << " y_shift " << p.y - y_top()
                //    << std::endl;
                if (strictly_lesser(x, p.x)
                        && !strictly_lesser(x, x_top_left())
                        && !strictly_greater(x, x_top_right())) {
                    x_shift = std::max(x_shift, p.x - x);
                }
            }

            // Left side of second trapezoid.
            {
                double a_left = 1.0 / a_left_;
                LengthDbl b_left = y_bottom() - a_left * x_bottom_left();
                bool intersects = true;
                LengthDbl x = 0;
                LengthDbl y = 0;
                if (a_left_ == 0) {
                    x = x_top_left();
                    y = a * x + b;
                } else if (!equal(a, a_left)) {
                    x = (b_left - b) / (a - a_left);
                    y = a * x + b;
                } else if (equal(b, b_left)) {
                    x = x_top_left();
                    y = y_top();
                } else {
                    // Parallel sides which are not on the same line.
                    intersects = false;
                }
                //std::cout << "left:"
                //    << " x " << x
                //    << " x_shift " << p.x - x
                //    << " y_shift " << p.y - y
                //    << std::endl;
                if (intersects
                        && strictly_lesser(x, p.x)
                        && !strictly_lesser(y, y_bottom())
                        && !strictly_greater(y, y_top())) {
                    x_shift = std::max(x_shift, p.x - x);
                }
            }

            // Right side of second trapezoid.
            {
                double a_right = 1.0 / a_right_;
                LengthDbl b_right = y_bottom() - a_right * x_bottom_right();
                bool intersects = true;
                LengthDbl x = 0;
                LengthDbl y = 0;
                if (a_right_ == 0) {
                    x = x_top_right();
                    y = a * x + b;
                } else if (!equal(a, a_right)) {
                    x = (b_right - b) / (a - a_right);
                    y = a * x + b;
                } else if (equal(b, b_right)) {
                    x = x_top_right();
                    y = y_top();
                } else {
                    // Parallel sides which are not on the same line.
                    intersects = false;
                }
                //std::cout << "right:"
                //    << " x " << x
                //    << " x_shift " << p.x - x
                //    << " y_shift " << p.y - y
                //    << std::endl;
                if (intersects
                        && strictly_lesser(x, p.x)
                        && !strictly_lesser(y, y_bottom())
                        && !strictly_greater(y, y_top())) {
                    x_shift = std::max(x_shift, p.x - x);
                }
            }
        }

        return x_shift;
    }

    /** Mirror the generalized trapezoid through the x-axis. */
    GeneralizedTrapezoid axial_symmetry_x_axis() const
    {
        return GeneralizedTrapezoid(
                -y_top(),
                -y_bottom(),
                x_top_left(),
                x_top_right(),
                x_bottom_left(),
                x_bottom_right());
    }

    /** Mirror the generalized trapezoid through the y-axis. */
    GeneralizedTrapezoid axial_symmetry_y_axis() const
    {
        return GeneralizedTrapezoid(
                y_bottom(),
                y_top(),
                -x_bottom_right(),
                -x_bottom_left(),
                -x_top_right(),
                -x_top_left());
    }

    /**
     * Return a copy of the generalized trapezoid whose almost vertical sides
     * are vertical: each side whose slope (see a_left) is smaller than 1e-2
     * or greater than 1e2 in absolute value is replaced by the vertical side
     * through its outermost corner, so that the result contains the
     * generalized trapezoid.
     *
     * The corners of a side are also swapped if its left one is on the right
     * of its right one.
     */
    GeneralizedTrapezoid clean() const;

    /*
     * Export
     */

    /**
     * Convert the generalized trapezoid to an anticlockwise polygon of 4
     * line segments; for a triangle, one of them has a null length.
     */
    Shape to_shape() const
    {
        Shape shape;
        {
            ShapeElement element;
            element.type = ShapeElementType::LineSegment;
            element.start = {x_bottom_left(), y_bottom()};
            element.end = {x_bottom_right(), y_bottom()};
            shape.elements.push_back(element);
        }
        {
            ShapeElement element;
            element.type = ShapeElementType::LineSegment;
            element.start = {x_bottom_right(), y_bottom()};
            element.end = {x_top_right(), y_top()};
            shape.elements.push_back(element);
        }
        {
            ShapeElement element;
            element.type = ShapeElementType::LineSegment;
            element.start = {x_top_right(), y_top()};
            element.end = {x_top_left(), y_top()};
            shape.elements.push_back(element);
        }
        {
            ShapeElement element;
            element.type = ShapeElementType::LineSegment;
            element.start = {x_top_left(), y_top()};
            element.end = {x_bottom_left(), y_bottom()};
            shape.elements.push_back(element);
        }
        return shape;
    }

    /**
     * Return the SVG of the generalized trapezoid, with the y-axis going up,
     * its coordinates multiplied by 'factor', filled with 'color'.
     */
    std::string to_svg(
            const std::string color = "purple",
            double factor = 10) const
    {
        return std::string("<path d=\"")
            + "M" + std::to_string(x_bottom_left() * factor) + "," + std::to_string(-y_bottom() * factor)
            + "L" + std::to_string(x_bottom_right() * factor) + "," + std::to_string(-y_bottom() * factor)
            + "L" + std::to_string(x_top_right() * factor) + "," + std::to_string(-y_top() * factor)
            + "L" + std::to_string(x_top_left() * factor) + "," + std::to_string(-y_top() * factor)
            + "Z\""
            + " stroke=\"black\""
            + " stroke-width=\"1\""
            + " fill=\"" + color + "\""
            + " fill-opacity=\"0.2\""
            + "/>\n";
    }

    /**
     * Read a generalized trapezoid from a JSON object with "y_bottom",
     * "y_top", "x_bottom_left", "x_bottom_right", "x_top_left" and
     * "x_top_right".
     */
    template <class basic_json>
    static GeneralizedTrapezoid from_json(basic_json& json_element)
    {
        return GeneralizedTrapezoid(
                json_element["y_bottom"],
                json_element["y_top"],
                json_element["x_bottom_left"],
                json_element["x_bottom_right"],
                json_element["x_top_left"],
                json_element["x_top_right"]);
    }

    /** Write the generalized trapezoid to a JSON object read by from_json. */
    nlohmann::json to_json() const
    {
        nlohmann::json json;
        json["y_bottom"] = y_bottom();
        json["y_top"] = y_top();
        json["x_bottom_left"] = x_bottom_left();
        json["x_bottom_right"] = x_bottom_right();
        json["x_top_left"] = x_top_left();
        json["x_top_right"] = x_top_right();
        return json;
    }

private:

    /** x-coordinate of the bottom. */
    LengthDbl yb_;

    /** y-coordinate of the top. */
    LengthDbl yt_;

    /** x-coordinate of the bottom-left point. */
    LengthDbl xbl_;

    /** x-coordinate of the bottom-right point. */
    LengthDbl xbr_;

    /** x-coordinate of the top-left point. */
    LengthDbl xtl_;

    /** x-coordinate of the top-right point. */
    LengthDbl xtr_;

    /*
     * Computed attributes
     */

    /** height of the generalized trapezoid. */
    LengthDbl height_;

    /** Bottom width of the generalized trapezoid. */
    LengthDbl width_bottom_;

    /** Top width of the generalized trapezoid. */
    LengthDbl width_top_;

    /** Smallest x of the generalized trapezoid. */
    LengthDbl x_min_;

    /** Greatest x of the generalized trapezoid. */
    LengthDbl x_max_;

    /** Area of the generalized trapezoid. */
    AreaDbl area_;

    /** Left slope. */
    double a_left_;

    /** Right slope. */
    double a_right_;

    bool left_side_increasing_not_vertical_ = false;

    bool left_side_decreasing_not_vertical_ = false;

    bool right_side_increasing_not_vertical_ = false;

    bool right_side_decreasing_not_vertical_ = false;

};

inline std::ostream& operator<<(
        std::ostream& os,
        const GeneralizedTrapezoid& trapezoid)
{
    std::streamsize precision = std::cout.precision();
    os << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        << "yb " << trapezoid.y_bottom()
        << " yt " << trapezoid.y_top()
        << " xbl " << trapezoid.x_bottom_left()
        << " xbr " << trapezoid.x_bottom_right()
        << " xtl " << trapezoid.x_top_left()
        << " xtr " << trapezoid.x_top_right()
        << std::setprecision(precision)
        ;
    return os;
}

inline GeneralizedTrapezoid GeneralizedTrapezoid::clean() const
{
    LengthDbl yb = yb_;
    LengthDbl yt = yt_;
    LengthDbl xbl = xbl_;
    LengthDbl xbr = xbr_;
    LengthDbl xtl = xtl_;
    LengthDbl xtr = xtr_;

    bool is_left_vertical = (a_left() > 1e2
            || a_left() < -1e2
            || (a_left() >= 0 && a_left() < 1e-2)
            || (a_left() <= 0 && a_left() > -1e-2));
    bool is_right_vertical = (a_right() > 1e2
            || a_right() < -1e2
            || (a_right() >= 0 && a_right() < 1e-2)
            || (a_right() <= 0 && a_right() > -1e-2));

    if (xtl > xtr)
        std::swap(xtl, xtr);
    if (xbl > xbr)
        std::swap(xbl, xbr);

    if (is_left_vertical) {
        if (xtl > xbl) {
            xtl = xbl;
        } else if (xbl > xtl) {
            xbl = xtl;
        }
    }

    if (is_right_vertical) {
        if (xbr < xtr) {
            xbr = xtr;
        } else if (xtr < xbr) {
            xtr = xbr;
        }
    }

    GeneralizedTrapezoid new_trapezoid(
            yb,
            yt,
            xbl,
            xbr,
            xtl,
            xtr);

    //std::cout << "is_top_point " << is_top_point << " is_bottom_point " << is_bottom_point << std::endl;
    //std::cout << "is_left_vertical " << is_left_vertical << " is_right_vertical " << is_right_vertical << std::endl;
    //std::cout << *this << std::endl;
    //std::cout << "a_left " << a_left() << " a_right " << a_right() << std::endl;
    //std::cout << new_trapezoid << std::endl;
    //std::cout << new_trapezoid.a_left() << " " << new_trapezoid.a_right() << std::endl;
    return new_trapezoid;
}

}
