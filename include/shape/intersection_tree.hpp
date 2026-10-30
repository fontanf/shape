#pragma once

#include "shape/shapes_intersections.hpp"

#include "optimizationtools/containers/indexed_set.hpp"

#include <functional>

namespace shape
{

/**
 * Spatial index over a set of shapes with holes, elements and points, to
 * find quickly which of them intersect a given object, or each other.
 *
 * Intersections are the ones of the functions of shapes_intersections.hpp
 * (see the conventions there), with the same 'strict' parameter, and the
 * results are positions in the vectors given to the constructor.
 */
class IntersectionTree
{

public:

    /**
     * Constructor.
     *
     * The tree keeps references to 'shapes', 'elements' and 'points': they
     * must outlive it and must not be modified while it is used.
     */
    IntersectionTree(
            const std::vector<ShapeWithHoles>& shapes,
            const std::vector<ShapeElement>& elements,
            const std::vector<Point>& points);


    /** Positions of the objects of the tree intersecting a given object. */
    struct IntersectOutput
    {
        std::vector<ShapePos> shape_ids;

        std::vector<ShapePos> element_ids;

        std::vector<ShapePos> point_ids;
    };

    /**
     * Find the shapes, elements and points of the tree intersecting a given
     * shape with holes.
     *
     * A point intersects the shape if the shape contains it (strictly if
     * 'strict' is true).
     */
    IntersectOutput intersect(
            const ShapeWithHoles& shape,
            bool strict) const;

    /**
     * Find the shapes, elements and points of the tree intersecting a given
     * shape.
     *
     * A point intersects the shape if the shape contains it (strictly if
     * 'strict' is true).
     */
    IntersectOutput intersect(
            const Shape& shape,
            bool strict) const;

    /**
     * Find the shapes, elements and points of the tree intersecting a given
     * element.
     *
     * If 'strict' is true, an element of the tree intersects the given
     * element only if they cross each other (see
     * ShapeElementIntersectionsOutput::proper_intersections), and no point
     * is returned.
     */
    IntersectOutput intersect(
            const ShapeElement& element,
            bool strict) const;

    /**
     * Find the shapes, elements and points of the tree intersecting a given
     * point: the shapes containing it (strictly if 'strict' is true), the
     * elements containing it and the points equal to it.
     *
     * If 'strict' is true, no element and no point is returned.
     */
    IntersectOutput intersect(
            const Point& point,
            bool strict) const;


    /**
     * Find a shape of the tree containing a given point (strictly if
     * 'strict' is true) and for which 'condition' is true.
     *
     * Return its position, or -1 if there is none. It stops at the first
     * shape found, so it is faster than intersect(point, strict) when many
     * shapes contain the point. 'condition' is evaluated before checking
     * that the shape contains the point, so that a cheap condition saves
     * this check.
     */
    ShapePos find_shape(
            const Point& point,
            bool strict,
            const std::function<bool(ShapePos)>& condition = nullptr) const;

    /** Get all the pairs of intersecting shapes in the tree. */
    std::vector<std::pair<ShapePos, ShapePos>> compute_intersecting_shapes(bool strict) const;

    /**
     * Get all the pairs of intersecting elements in the tree, with their
     * intersections.
     *
     * If 'strict' is true, only the pairs of elements crossing each other
     * (see ShapeElementIntersectionsOutput::proper_intersections) are
     * returned.
     */
    std::vector<ElementElementIntersection> compute_intersecting_elements(bool strict) const;

    /** Get all the pairs of equal points (within tolerance) in the tree. */
    std::vector<std::pair<ElementPos, ElementPos>> compute_equal_points() const;

private:

    using NodeId = int64_t;

    struct Node
    {
        LengthDbl l = std::numeric_limits<LengthDbl>::infinity();

        LengthDbl r = -std::numeric_limits<LengthDbl>::infinity();

        LengthDbl b = std::numeric_limits<LengthDbl>::infinity();

        LengthDbl t = -std::numeric_limits<LengthDbl>::infinity();

        char direction = 'x';

        LengthDbl position = 0.0;

        NodeId lesser_child_id = -1;

        NodeId greater_child_id = -1;

        std::vector<ShapePos> shape_ids;

        std::vector<ElementPos> element_ids;

        std::vector<ElementPos> point_ids;
    };

    struct StackElement
    {
        NodeId node_id;
        std::vector<ShapePos> shape_ids;
        std::vector<ElementPos> element_ids;
        std::vector<ElementPos> point_ids;
    };

    /**
     * Fill the potentially intersecting shapes, elements and points with
     * the ones of the leaves of the tree which may contain a given point.
     */
    void compute_potentially_intersecting(const Point& point) const;

    void build_node_rec(
            NodeId node_id,
            const std::vector<ShapePos>& shape_ids);

    /** Get the number of shapes. */
    ShapePos number_of_shapes() const { if (shapes_ == nullptr) return 0; return shapes_->size(); }

    /** Get the number of elements. */
    ShapePos number_of_elements() const { if (elements_ == nullptr) return 0; return elements_->size(); }

    /** Get the number of points. */
    ShapePos number_of_points() const { if (points_ == nullptr) return 0; return points_->size(); }

    /** Get a shape of the intersection tree. */
    const ShapeWithHoles& shape(ShapePos shape_pos) const { return (*shapes_)[shape_pos]; }

    /** Get a shape element of the intersection tree. */
    const ShapeElement& element(ElementPos element_pos) const { return (*elements_)[element_pos]; }

    /** Get a point of the intersection tree. */
    const Point& point(ElementPos point_pos) const { return (*points_)[point_pos]; }


    /** Shapes. */
    const std::vector<ShapeWithHoles>* shapes_ = nullptr;

    /** Shape elements. */
    const std::vector<ShapeElement>* elements_ = nullptr;

    /** Points. */
    const std::vector<Point>* points_ = nullptr;

    bool small_ = false;

    mutable optimizationtools::IndexedSet potentially_intersecting_shapes_;

    mutable optimizationtools::IndexedSet potentially_intersecting_elements_;

    mutable optimizationtools::IndexedSet potentially_intersecting_points_;

    mutable std::vector<NodeId> intersect_stack_;

    std::vector<Node> tree_;

};

}
