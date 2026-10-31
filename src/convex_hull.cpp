//#define CONVEX_HULL_DEBUG

#include "shape/convex_hull.hpp"

#ifdef CONVEX_HULL_DEBUG
#include "shape/writer.hpp"
#endif

//#include <iostream>

using namespace shape;

Shape shape::convex_hull(
        const Shape& shape)
{
    // The algorithm below only looks at each element's start point, so a
    // circular arc bulging past the chord between its endpoints would be
    // silently ignored, producing a hull that doesn't actually contain the
    // input shape (only caught after the fact by the area check below,
    // with a much less helpful error). Reject arcs up front instead; for
    // shapes with circular arcs, decompose into arc-free convex pieces
    // first (see decompose_into_basic_shapes -- as of this writing it does
    // not yet feed into a convex-hull-of-those-pieces computation, but that
    // is the intended path forward).
    for (const ShapeElement& element: shape.elements) {
        if (element.type != ShapeElementType::LineSegment) {
            throw std::invalid_argument(
                    FUNC_SIGNATURE + ": "
                    "convex_hull only supports polygons (shapes made of line "
                    "segments); shape contains at least one circular arc.");
        }
    }

    //std::cout << "shape.elements.size() " << shape.elements.size() << std::endl;
    if (shape.elements.size() < 3)
        return shape;

    // The pivot is the leftmost of the points with the least y, compared
    // exactly, so that the directions from the pivot to all the other points
    // are in the half-plane in which 'strictly_lesser_angle' orders them by
    // angle.
    Point point_least_y = shape.elements.front().start;
    for (const ShapeElement& shape_element: shape.elements) {
        if (shape_element.start.y < point_least_y.y
                || (shape_element.start.y == point_least_y.y
                    && shape_element.start.x < point_least_y.x)) {
            point_least_y = shape_element.start;
        }
    }

    // swap the pivot with the first point
    std::vector<Point> points;
    for (const ShapeElement& shape_element: shape.elements) {
        if (shape_element.start == point_least_y)
            continue;
        points.push_back(shape_element.start);
    }

    std::sort(
            points.begin(),
            points.end(),
            [&point_least_y](
                const Point& point_1,
                const Point& point_2)
            {
                // Compare the angles exactly rather than with
                // 'counter_clockwise', whose collinearity check is not
                // symmetric: a far point can be off the line through the
                // pivot and a near point while the near point is on the line
                // through the pivot and the far point. The comparison would
                // then not be a strict weak ordering, and the points would be
                // sorted wrongly.
                Point vector_1 = point_1 - point_least_y;
                Point vector_2 = point_2 - point_least_y;
                if (strictly_lesser_angle(vector_1, vector_2))
                    return true;
                if (strictly_lesser_angle(vector_2, vector_1))
                    return false;
                return squared_distance(point_least_y, point_1)
                    < squared_distance(point_least_y, point_2);
            });

    // Graham scan. The turns are checked exactly, consistently with the
    // order of the points: a check with a tolerance can find that three
    // points are collinear while the middle one in the angular order isn't
    // the middle one along the line, and remove a point of the hull.
    std::vector<Point> convex_hull;
    convex_hull.push_back(point_least_y);
    for (const Point& point: points) {
        while (convex_hull.size() > 1) {
            const Point& point_prev = convex_hull[convex_hull.size() - 2];
            const Point& point_top = convex_hull.back();
            Point vector_1 = point_top - point_prev;
            Point vector_2 = point - point_top;
            if (vector_1.x * vector_2.y > vector_1.y * vector_2.x)
                break;
            convex_hull.pop_back();
        }
        convex_hull.push_back(point);
    }

    Shape convex_hull_shape;
    for (ElementPos pos = 0;
            pos < convex_hull.size();
            ++pos) {
        ShapeElement element;
        element.type = ShapeElementType::LineSegment;
        element.start = convex_hull[pos];
        element.end = convex_hull[(pos + 1) % convex_hull.size()];
        convex_hull_shape.elements.push_back(element);
    }

    if (strictly_lesser(
                std::sqrt(convex_hull_shape.compute_area()),
                std::sqrt(shape.compute_area()))) {
        //std::cout << shape.to_string(0) << std::endl;
        //std::cout << convex_hull_shape.to_string(0) << std::endl;
#ifdef CONVEX_HULL_DEBUG
        Writer()
            .add_shape(shape)
            .add_shape(convex_hull_shape)
            .write_json("convex_hull.json");
#endif
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "shape.compute_area(): " + std::to_string(shape.compute_area()) + "; "
                "convex_hull_shape.compute_area(): " + std::to_string(convex_hull_shape.compute_area()) + ".");

    }

    return convex_hull_shape;
}
