#include "shape/no_fit_polygon.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
#include "shape/convex_partition.hpp"

using namespace shape;

Shape shape::no_fit_polygon(
        const Shape& fixed_shape,
        const Shape& orbiting_shape)
{
    if (!fixed_shape.is_polygon()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "fixed_shape is not a polygon.");
    }
    if (!orbiting_shape.is_polygon()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "orbiting_shape is not a polygon.");
    }
    if (!fixed_shape.is_convex()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "fixed_shape is not convex.");
    }
    if (!orbiting_shape.is_convex()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "orbiting_shape is not convex.");
    }

    // Find the bottom-most vertex of fixed_shape (min y, then min x for ties).
    // Starting the edge sequence here ensures the first outgoing edge
    // has angle in [0°, 180°), suitable for the angular merge below.
    ElementPos fixed_start_pos = 0;
    for (ElementPos fixed_pos = 1;
            fixed_pos < (ElementPos)fixed_shape.elements.size();
            ++fixed_pos) {
        const Point& point = fixed_shape.elements[fixed_pos].start;
        const Point& best = fixed_shape.elements[fixed_start_pos].start;
        if (strictly_lesser(point.y, best.y)
                || (equal(point.y, best.y)
                    && strictly_lesser(point.x, best.x))) {
            fixed_start_pos = fixed_pos;
        }
    }

    // Find the top-most vertex of orbiting_shape (max y, then max x for ties).
    // Negating it gives the bottom-most vertex of −orbiting_shape
    // (= orbiting_shape rotated 180°), which also has its first outgoing
    // edge in [0°, 180°).
    ElementPos orbiting_start_pos = 0;
    for (ElementPos orbiting_pos = 1;
            orbiting_pos < (ElementPos)orbiting_shape.elements.size();
            ++orbiting_pos) {
        const Point& point = orbiting_shape.elements[orbiting_pos].start;
        const Point& best = orbiting_shape.elements[orbiting_start_pos].start;
        if (strictly_greater(point.y, best.y)
                || (equal(point.y, best.y)
                    && strictly_greater(point.x, best.x))) {
            orbiting_start_pos = orbiting_pos;
        }
    }

    // Starting vertex of the NFP: bottom of fixed minus top of orbiting.
    const Point nfp_start = fixed_shape.elements[fixed_start_pos].start
        - orbiting_shape.elements[orbiting_start_pos].start;

    // Build the edge-vector sequences for the angular merge.
    //
    // Edges of fixed_shape in CCW order starting from fixed_start_pos:
    //   edges_fixed[edge_index] =
    //     fixed_shape.elements[(fixed_start_pos + edge_index) % m].end
    //   − fixed_shape.elements[(fixed_start_pos + edge_index) % m].start
    //
    // Edges of −orbiting_shape in CCW order starting from the negated top vertex:
    //   Since −orbiting_shape = rotate(orbiting_shape, 180°) it is also CCW.
    //   Going CCW from the negated top vertex follows the same index direction,
    //   so each edge of −orbiting_shape is the negation of the corresponding
    //   edge of orbiting_shape (offset by orbiting_start_pos):
    //   edges_neg_orbiting[edge_index] =
    //     −(orbiting_shape.elements[(orbiting_start_pos + edge_index) % n].end
    //      − orbiting_shape.elements[(orbiting_start_pos + edge_index) % n].start)
    const ElementPos fixed_num_elements
        = (ElementPos)fixed_shape.elements.size();
    const ElementPos orbiting_num_elements
        = (ElementPos)orbiting_shape.elements.size();

    std::vector<Point> edges_fixed(fixed_num_elements);
    for (ElementPos edge_index = 0; edge_index < fixed_num_elements; ++edge_index) {
        const ElementPos element_pos = (fixed_start_pos + edge_index) % fixed_num_elements;
        edges_fixed[edge_index] = fixed_shape.elements[element_pos].end
            - fixed_shape.elements[element_pos].start;
    }

    std::vector<Point> edges_neg_orbiting(orbiting_num_elements);
    for (ElementPos edge_index = 0; edge_index < orbiting_num_elements; ++edge_index) {
        const ElementPos element_pos
            = (orbiting_start_pos + edge_index) % orbiting_num_elements;
        const Point edge = orbiting_shape.elements[element_pos].end
            - orbiting_shape.elements[element_pos].start;
        edges_neg_orbiting[edge_index] = {-edge.x, -edge.y};
    }

    // Merge both edge sequences in non-decreasing angular order (rotating
    // calipers) and trace the Minkowski-sum polygon.
    //
    // When two edges have the same direction (parallel) they are both added
    // in sequence, producing collinear intermediate vertices that are removed
    // by remove_aligned_vertices at the end.
    std::vector<Point> nfp_vertices;
    nfp_vertices.reserve(fixed_num_elements + orbiting_num_elements);
    nfp_vertices.push_back(nfp_start);

    Point current_vertex = nfp_start;
    ElementPos fixed_edge_pos = 0;
    ElementPos orbiting_edge_pos = 0;

    while (fixed_edge_pos < fixed_num_elements
            || orbiting_edge_pos < orbiting_num_elements) {
        if (fixed_edge_pos >= fixed_num_elements) {
            current_vertex = current_vertex + edges_neg_orbiting[orbiting_edge_pos++];
        } else if (orbiting_edge_pos >= orbiting_num_elements) {
            current_vertex = current_vertex + edges_fixed[fixed_edge_pos++];
        } else if (strictly_lesser_angle(
                edges_fixed[fixed_edge_pos],
                edges_neg_orbiting[orbiting_edge_pos])) {
            current_vertex = current_vertex + edges_fixed[fixed_edge_pos++];
        } else if (strictly_lesser_angle(
                edges_neg_orbiting[orbiting_edge_pos],
                edges_fixed[fixed_edge_pos])) {
            current_vertex = current_vertex + edges_neg_orbiting[orbiting_edge_pos++];
        } else {
            // Parallel edges: advance fixed first, then negated orbiting.
            current_vertex = current_vertex + edges_fixed[fixed_edge_pos++];
            nfp_vertices.push_back(current_vertex);
            current_vertex = current_vertex + edges_neg_orbiting[orbiting_edge_pos++];
        }

        // Only push if we have not yet closed the polygon.
        if (fixed_edge_pos < fixed_num_elements
                || orbiting_edge_pos < orbiting_num_elements)
            nfp_vertices.push_back(current_vertex);
    }
    // After all edges, current_vertex == nfp_start (the polygon is closed).

    // Build the result Shape from consecutive vertex pairs.
    Shape result;
    result.elements.reserve(nfp_vertices.size());
    for (ElementPos vertex_pos = 0;
            vertex_pos < (ElementPos)nfp_vertices.size();
            ++vertex_pos) {
        ShapeElement element;
        element.type = ShapeElementType::LineSegment;
        element.start = nfp_vertices[vertex_pos];
        element.end = nfp_vertices[(vertex_pos + 1) % nfp_vertices.size()];
        result.elements.push_back(element);
    }

    // Remove collinear vertices introduced by parallel-edge pairs.
    return remove_aligned_vertices(result).second;
}

std::vector<ShapeWithHoles> shape::no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape)
{
    std::vector<Shape> fixed_parts = compute_convex_partition(fixed_shape);
    std::vector<Shape> orbiting_parts = compute_convex_partition(orbiting_shape);

    std::vector<ShapeWithHoles> nfp_parts;
    nfp_parts.reserve(fixed_parts.size() * orbiting_parts.size());

    for (const Shape& fixed_part: fixed_parts) {
        for (const Shape& orbiting_part: orbiting_parts) {
            Shape convex_nfp = no_fit_polygon(fixed_part, orbiting_part);
            nfp_parts.push_back({convex_nfp, {}});
        }
    }

    return compute_union(nfp_parts);
}
