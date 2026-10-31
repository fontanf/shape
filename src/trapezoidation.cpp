#include "shape/trapezoidation.hpp"

#include "shape/writer.hpp"

#include <tuple>

using namespace shape;

namespace
{

enum class VertexTypeFlag
{
    LocalMaximumConvex,
    LocalMinimumConvex,
    LocalMaximumConcave,
    LocalMinimumConcave,
    Inflection,
    HorizontalLocalMaximumConvex,
    HorizontalLocalMinimumConvex,
    HorizontalLocalMaximumConcave,
    HorizontalLocalMinimumConcave,
    StrictlyHorizontal,
};

struct Vertex
{
    /** Vertex type flag. */
    VertexTypeFlag flag;
};

struct OpenTrapezoid
{
    ShapePos bottom_left_shape_pos = -1;
    ElementPos bottom_left_element_pos = -1;
    ShapePos bottom_right_shape_pos = -1;
    ElementPos bottom_right_element_pos  = -1;
    Point top_left;
    Point top_right;
};

inline ElementPos get_element_pos(
        const Shape& shape,
        ElementPos element_pos)
{
    ElementPos n = shape.elements.size();
    return ((element_pos % n) + n) % n;
}

inline Point get_vertex(
        const Shape& shape,
        ElementPos element_pos)
{
    return shape.elements[get_element_pos(shape, element_pos)].start;
}

inline Point get_vertex(
        const ShapeWithHoles& shape,
        ShapePos shape_pos,
        ElementPos element_pos)
{
    const Shape& current_shape = (shape_pos == (ShapePos)shape.holes.size())?
        shape.shape:
        shape.holes[shape_pos];
    return get_vertex(current_shape, element_pos);
}

inline Point bottom_left(
        const ShapeWithHoles& shape,
        const OpenTrapezoid& open_trapezoid)
{
    return get_vertex(
            shape,
            open_trapezoid.bottom_left_shape_pos,
            open_trapezoid.bottom_left_element_pos);
}

inline Point bottom_right(
        const ShapeWithHoles& shape,
        const OpenTrapezoid& open_trapezoid)
{
    return get_vertex(
            shape,
            open_trapezoid.bottom_right_shape_pos,
            open_trapezoid.bottom_right_element_pos);
}

inline std::ostream& operator<<(
        std::ostream& os,
        const OpenTrapezoid& open_trapezoid)
{
    os << "bl " << open_trapezoid.bottom_left_shape_pos << " " << open_trapezoid.bottom_left_element_pos
        << " br " << open_trapezoid.bottom_right_shape_pos << " " << open_trapezoid.bottom_right_element_pos
        << " tl " << open_trapezoid.top_left.to_string()
        << " tr " << open_trapezoid.top_right.to_string();
    return os;
}

LengthDbl x(
        const Point& bottom,
        const Point& top,
        LengthDbl y)
{
    //std::cout << "x bottom " << bottom.to_string() << " top " << top.to_string() << " y " << y << std::endl;
    if (bottom.y == top.y) {
        if (bottom.x != top.x) {
            throw std::runtime_error(FUNC_SIGNATURE);
        }
        return bottom.x;
    }
    if (y == bottom.y)
        return bottom.x;
    if (y == top.y)
        return top.x;
    double a = (top.x - bottom.x) / (top.y - bottom.y);
    return bottom.x + (y - bottom.y) * a;
}

std::pair<ElementPos, ElementPos> find_trapezoid_containing_vertex(
        const ShapeWithHoles& shape,
        const std::vector<OpenTrapezoid>& open_trapezoids,
        ShapePos shape_pos,
        ElementPos element_pos)
{
    Point vertex = get_vertex(shape, shape_pos, element_pos);
    // Open trapezoids containing the vertex, strictly or on one of their
    // sides. A vertex is on the side of an open trapezoid of another
    // boundary where boundaries touch.
    std::vector<std::tuple<LengthDbl, LengthDbl, ElementPos>> strict;
    std::vector<std::tuple<LengthDbl, LengthDbl, ElementPos>> on_side;
    for (ElementPos open_trapezoid_pos = 0;
            open_trapezoid_pos < (TrapezoidPos)open_trapezoids.size();
            ++open_trapezoid_pos) {
        const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];
        LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
        LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
        bool left_ok = strictly_lesser(x_left, vertex.x)
            || (shape_pos == open_trapezoid.bottom_left_shape_pos
                && element_pos == open_trapezoid.bottom_left_element_pos);
        bool right_ok = strictly_greater(x_right, vertex.x)
            || (shape_pos == open_trapezoid.bottom_right_shape_pos
                && element_pos == open_trapezoid.bottom_right_element_pos);
        if (left_ok && right_ok) {
            strict.push_back(std::make_tuple(x_left, x_right, open_trapezoid_pos));
        } else if (!strictly_greater(x_left, vertex.x)
                && !strictly_lesser(x_right, vertex.x)) {
            on_side.push_back(std::make_tuple(x_left, x_right, open_trapezoid_pos));
        }
    }
    std::vector<std::tuple<LengthDbl, LengthDbl, ElementPos>> candidates = strict;
    if (candidates.size() < 2)
        candidates.insert(candidates.end(), on_side.begin(), on_side.end());
    std::sort(candidates.begin(), candidates.end());
    std::pair<ElementPos, ElementPos> res = {-1, -1};
    if (candidates.size() >= 1)
        res.first = std::get<2>(candidates[0]);
    if (candidates.size() >= 2)
        res.second = std::get<2>(candidates[1]);
    return res;
}

}

/**
 * The outline and the holes may touch each other at points: the vertices at
 * such a point are processed in an order which keeps the open trapezoids
 * consistent, and a vertex may lie on a side of an open trapezoid of
 * another boundary.
 */
std::vector<GeneralizedTrapezoid> shape::trapezoidation(
        const ShapeWithHoles& shape_orig)
{
    if (!shape_orig.check()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "invalid input shape.");
    }
    if (!shape_orig.is_polygon()) {
        throw std::invalid_argument(
                FUNC_SIGNATURE + ": "
                "input shape must be a polygon.");
    }

    //std::cout << "polygon_trapezoidation" << std::endl;
    //std::cout << shape.to_string(0) << std::endl;
    //write_json({shape}, {}, "trapezoidation_input.json");
    // A vertex between two horizontal edges isn't supported by the sweep
    // below, which processes the horizontal edges one at a time: these
    // vertices are removed.
    auto remove_horizontally_aligned_vertices = [](const Shape& current_shape)
    {
        Shape res;
        ElementPos number_of_elements = current_shape.elements.size();
        for (ElementPos element_pos = 0;
                element_pos < number_of_elements;
                ++element_pos) {
            const ShapeElement& element_prev = current_shape.elements[(element_pos + number_of_elements - 1) % number_of_elements];
            const ShapeElement& element = current_shape.elements[element_pos];
            if (element_prev.start.y == element.start.y
                    && element.start.y == element.end.y) {
                continue;
            }
            res.elements.push_back(element);
        }
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)res.elements.size();
                ++element_pos) {
            res.elements[element_pos].end = res.elements[(element_pos + 1) % res.elements.size()].start;
        }
        return res;
    };
    ShapeWithHoles shape;
    shape.shape = remove_horizontally_aligned_vertices(shape_orig.shape);
    for (const Shape& hole: shape_orig.holes)
        shape.holes.push_back(remove_horizontally_aligned_vertices(hole));
    //std::cout << shape.to_string(0) << std::endl;

    std::vector<GeneralizedTrapezoid> trapezoids;

    // Classify the vertices.
    std::vector<std::vector<Vertex>> vertices;
    for (ShapePos shape_pos = 0;
            shape_pos <= (ShapePos)shape.holes.size();
            ++shape_pos) {
        const Shape& current_shape = (shape_pos == (ShapePos)shape.holes.size())? shape.shape: shape.holes[shape_pos];
        vertices.push_back(std::vector<Vertex>(current_shape.elements.size()));

        ElementPos element_pos_prev = current_shape.elements.size() - 2;
        ElementPos element_pos_cur = current_shape.elements.size() - 1;
        for (ElementPos element_pos_next = 0;
                element_pos_next < current_shape.elements.size();
                ++element_pos_next) {
            const ShapeElement& element_prev = current_shape.elements[element_pos_prev];
            const ShapeElement& element_cur = current_shape.elements[element_pos_cur];
            const ShapeElement& element_next = current_shape.elements[element_pos_next];

            // Convexity.
            // The convexity can be determined easily by investigating the sign of the
            // cross product of the edges meeting at the considered vertex.
            double v = cross_product(
                    element_cur.start - element_prev.start,
                    element_next.start - element_cur.start);
            bool is_convex = (v >= 0);
            if (shape_pos != shape.holes.size())
                is_convex = !is_convex;

            // Local extreme of the vertices.
            if (element_prev.start.y < element_cur.start.y
                    && element_cur.start.y < element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = VertexTypeFlag::Inflection;
            } else if (element_prev.start.y > element_cur.start.y
                    && element_cur.start.y > element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = VertexTypeFlag::Inflection;
            } else if (element_cur.start.y < element_prev.start.y
                    && element_cur.start.y < element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::LocalMinimumConvex:
                    VertexTypeFlag::LocalMinimumConcave;
            } else if (element_cur.start.y > element_prev.start.y
                    && element_cur.start.y > element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::LocalMaximumConvex:
                    VertexTypeFlag::LocalMaximumConcave;
            } else if (element_cur.start.y == element_prev.start.y
                    && element_cur.start.y < element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::HorizontalLocalMinimumConvex:
                    VertexTypeFlag::HorizontalLocalMinimumConcave;
            } else if (element_cur.start.y < element_prev.start.y
                    && element_cur.start.y == element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::HorizontalLocalMinimumConvex:
                    VertexTypeFlag::HorizontalLocalMinimumConcave;
            } else if (element_cur.start.y == element_prev.start.y
                    && element_cur.start.y > element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::HorizontalLocalMaximumConvex:
                    VertexTypeFlag::HorizontalLocalMaximumConcave;
            } else if (element_cur.start.y > element_prev.start.y
                    && element_cur.start.y == element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = (is_convex)?
                    VertexTypeFlag::HorizontalLocalMaximumConvex:
                    VertexTypeFlag::HorizontalLocalMaximumConcave;
            } else if (element_cur.start.y == element_prev.start.y
                    && element_cur.start.y == element_next.start.y) {
                vertices[shape_pos][element_pos_cur].flag = VertexTypeFlag::StrictlyHorizontal;
            }

            element_pos_prev = element_pos_cur;
            element_pos_cur = element_pos_next;
        }
    }

    // Sort vertices according to their y coordinate.
    std::vector<std::pair<ShapePos, ElementPos>> sorted_vertices;
    // Add holes.
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];
        ElementPos element_prev_pos = hole.elements.size() - 1;
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)hole.elements.size();
                ++element_pos) {
            const ShapeElement& element = hole.elements[element_pos];
            const ShapeElement& element_prev = hole.elements[element_prev_pos];
            if (!(element.start.x > element.end.x
                        && element.start.y == element.end.y)
                    && !(element.start.x > element_prev.start.x
                        && element.start.y == element_prev.start.y)) {
                sorted_vertices.push_back({hole_pos, element_pos});
            }
            element_prev_pos = element_pos;
        }
    }
    // Add shape.
    {
        ElementPos element_prev_pos = shape.shape.elements.size() - 1;
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)shape.shape.elements.size();
                ++element_pos) {
            const ShapeElement& element = shape.shape.elements[element_pos];
            const ShapeElement& element_prev = shape.shape.elements[element_prev_pos];
            if (!(element.start.x > element.end.x
                        && element.start.y == element.end.y)
                    && !(element.start.x > element_prev.start.x
                        && element.start.y == element_prev.start.y)) {
                sorted_vertices.push_back({shape.holes.size(), element_pos});
            }
            element_prev_pos = element_pos;
        }
    }
    // At a point where several boundaries meet, the vertices closing open
    // trapezoids above the point are processed first, then the vertices
    // continuing the side of an open trapezoid, then the vertices opening
    // trapezoids below the point.
    auto vertex_rank = [&shape, &vertices](
            const std::pair<ShapePos, ElementPos>& p)
    {
        const Shape& current_shape = (p.first == (ShapePos)shape.holes.size())? shape.shape: shape.holes[p.first];
        auto flag_rank = [](VertexTypeFlag flag)
        {
            switch (flag) {
            case VertexTypeFlag::LocalMinimumConvex:
            case VertexTypeFlag::LocalMinimumConcave:
            case VertexTypeFlag::HorizontalLocalMinimumConvex:
            case VertexTypeFlag::HorizontalLocalMinimumConcave:
                return 0;
            case VertexTypeFlag::LocalMaximumConvex:
            case VertexTypeFlag::LocalMaximumConcave:
            case VertexTypeFlag::HorizontalLocalMaximumConvex:
            case VertexTypeFlag::HorizontalLocalMaximumConcave:
                return 2;
            default:
                return 1;
            }
        };
        VertexTypeFlag flag = vertices[p.first][p.second].flag;
        ElementPos element_pos_next = p.second;
        if (flag == VertexTypeFlag::HorizontalLocalMaximumConvex
                || flag == VertexTypeFlag::HorizontalLocalMinimumConcave) {
            element_pos_next = (p.first == (ShapePos)shape.holes.size())?
                get_element_pos(current_shape, p.second - 1):
                get_element_pos(current_shape, p.second + 1);
        } else if (flag == VertexTypeFlag::HorizontalLocalMinimumConvex
                || flag == VertexTypeFlag::HorizontalLocalMaximumConcave) {
            element_pos_next = (p.first == (ShapePos)shape.holes.size())?
                get_element_pos(current_shape, p.second + 1):
                get_element_pos(current_shape, p.second - 1);
        }
        int rank_1 = flag_rank(flag);
        int rank_2 = flag_rank(vertices[p.first][element_pos_next].flag);
        int rank = (rank_1 == rank_2)? rank_1: 1;
        LengthDbl width = std::abs(
                current_shape.elements[element_pos_next].start.x
                - current_shape.elements[p.second].start.x);
        // The vertices closing open trapezoids above a horizontal edge are
        // processed after the vertices on this edge; the vertices opening
        // trapezoids below a horizontal edge are processed before them.
        return std::make_pair(rank, (rank == 2)? -width: width);
    };
    // Sort.
    std::sort(
            sorted_vertices.begin(),
            sorted_vertices.end(),
            [&shape, &vertex_rank](
                const std::pair<ShapePos, ElementPos>& p1,
                const std::pair<ShapePos, ElementPos>& p2)
            {
                const Shape& shape_1 = (p1.first == shape.holes.size())? shape.shape: shape.holes[p1.first];
                const Shape& shape_2 = (p2.first == shape.holes.size())? shape.shape: shape.holes[p2.first];
                ElementPos element_pos_1 = p1.second;
                ElementPos element_pos_2 = p2.second;
                if (shape_1.elements[element_pos_1].start.y
                        != shape_2.elements[element_pos_2].start.y) {
                    return shape_1.elements[element_pos_1].start.y
                        > shape_2.elements[element_pos_2].start.y;
                }
                auto rank_1 = vertex_rank(p1);
                auto rank_2 = vertex_rank(p2);
                if (rank_1 != rank_2)
                    return rank_1 < rank_2;
                return shape_1.elements[element_pos_1].start.x
                    < shape_2.elements[element_pos_2].start.x;
            });

    //for (ElementPos vertex_pos = 0;
    //        vertex_pos < (ElementPos)sorted_vertices.size();
    //        ++vertex_pos) {
    //    ShapePos shape_pos = sorted_vertices[vertex_pos].first;
    //    ElementPos element_pos = sorted_vertices[vertex_pos].second;
    //    const Shape& current_shape = (shape_pos == (ShapePos)shape.holes.size())? shape.shape: shape.holes[shape_pos];
    //    const Point& vertex = current_shape.elements[element_pos].start;
    //    std::cout << vertex.to_string() << std::endl;
    //}

    // Sweep.
    std::vector<OpenTrapezoid> open_trapezoids;
    for (ElementPos vertex_pos = 0;
            vertex_pos < (ElementPos)sorted_vertices.size();
            ++vertex_pos) {

        ShapePos shape_pos = sorted_vertices[vertex_pos].first;
        ElementPos element_pos = sorted_vertices[vertex_pos].second;

        const Shape& current_shape = (shape_pos == (ShapePos)shape.holes.size())? shape.shape: shape.holes[shape_pos];

        ElementPos element_pos_next = element_pos;
        VertexTypeFlag flag = vertices[shape_pos][element_pos].flag;
        if (flag == VertexTypeFlag::HorizontalLocalMaximumConvex
                || flag == VertexTypeFlag::HorizontalLocalMinimumConcave) {
            element_pos_next = (shape_pos == (ShapePos)shape.holes.size())?
                get_element_pos(current_shape, element_pos - 1):
                get_element_pos(current_shape, element_pos + 1);
        } else if (flag == VertexTypeFlag::HorizontalLocalMinimumConvex
                || flag == VertexTypeFlag::HorizontalLocalMaximumConcave) {
            element_pos_next = (shape_pos == (ShapePos)shape.holes.size())?
                get_element_pos(current_shape, element_pos + 1):
                get_element_pos(current_shape, element_pos - 1);
        }

        const Point& vertex = current_shape.elements[element_pos].start;
        const Point& vertex_next = current_shape.elements[element_pos_next].start;

        //std::cout << "vertex_pos " << vertex_pos << std::endl;
        //std::cout << "shape_pos " << shape_pos << " element_pos " << element_pos << std::endl;
        //std::cout << "vertex " << vertex.x << " " << vertex.y << std::endl;
        //std::cout << "vertex_next " << vertex_next.x << " " << vertex_next.y << std::endl;
        //std::cout << "open trapezoids:" << std::endl;
        //for (const OpenTrapezoid& open_trapezoid: open_trapezoids)
        //    std::cout << open_trapezoid
        //        << " " << bottom_left(shape, open_trapezoid).to_string()
        //        << " " << bottom_right(shape, open_trapezoid).to_string()
        //        << std::endl;
        if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::LocalMaximumConvex) {
            // +1 open trapezoid.
            //std::cout << "LocalMaximumConvex" << std::endl;

            // Update open_trapezoids.
            OpenTrapezoid open_trapezoid;
            open_trapezoid.top_left = vertex;
            open_trapezoid.top_right = vertex;
            open_trapezoid.bottom_left_shape_pos = shape_pos;
            open_trapezoid.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 1):
                get_element_pos(current_shape, element_pos - 1);
            open_trapezoid.bottom_right_shape_pos = shape_pos;
            open_trapezoid.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos - 1):
                get_element_pos(current_shape, element_pos + 1);
            ElementPos open_trapezoid_pos = open_trapezoids.size();

            open_trapezoids.push_back(open_trapezoid);

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::LocalMinimumConvex) {
            // -1 open trapezoid.
            //std::cout << "LocalMinimumConvex" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        vertex.x,
                        vertex.x,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::LocalMaximumConcave) {
            // -1 open trapezoid.
            // +2 open trapezoids.
            //std::cout << "LocalMaximumConcave" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
            LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid_1;
            new_open_trapezoid_1.top_left = {x_left, vertex.y};
            new_open_trapezoid_1.top_right = vertex;
            new_open_trapezoid_1.bottom_left_shape_pos = open_trapezoid.bottom_left_shape_pos;
            new_open_trapezoid_1.bottom_left_element_pos = open_trapezoid.bottom_left_element_pos;
            new_open_trapezoid_1.bottom_right_shape_pos = shape_pos;
            new_open_trapezoid_1.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos - 1):
                get_element_pos(current_shape, element_pos + 1);

            OpenTrapezoid new_open_trapezoid_2;
            new_open_trapezoid_2.top_left = vertex;
            new_open_trapezoid_2.top_right = {x_right, vertex.y};
            new_open_trapezoid_2.bottom_left_shape_pos = shape_pos;
            new_open_trapezoid_2.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 1):
                get_element_pos(current_shape, element_pos - 1);
            new_open_trapezoid_2.bottom_right_shape_pos = open_trapezoid.bottom_right_shape_pos;
            new_open_trapezoid_2.bottom_right_element_pos = open_trapezoid.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid_1);
            open_trapezoids.push_back(new_open_trapezoid_2);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::LocalMinimumConcave) {
            // -2 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "LocalMinimumConcave" << std::endl;

            auto p = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos);
            if (p.first == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'p.first' must be != -1.");
            }
            if (p.second == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'p.second' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid_1 = open_trapezoids[p.first];
            const OpenTrapezoid& open_trapezoid_2 = open_trapezoids[p.second];

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid_1), open_trapezoid_1.top_left, vertex.y);
            if (vertex.y != open_trapezoid_1.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid_1.top_left.y,
                        x_left,
                        vertex.x,
                        open_trapezoid_1.top_left.x,
                        open_trapezoid_1.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            LengthDbl x_right = x(bottom_right(shape, open_trapezoid_2), open_trapezoid_2.top_right, vertex.y);
            if (vertex.y != open_trapezoid_2.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid_2.top_left.y,
                        vertex.x,
                        x_right,
                        open_trapezoid_2.top_left.x,
                        open_trapezoid_2.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = {x_left, vertex.y};
            new_open_trapezoid.top_right = {x_right, vertex.y};
            new_open_trapezoid.bottom_left_shape_pos = open_trapezoid_1.bottom_left_shape_pos;
            new_open_trapezoid.bottom_left_element_pos = open_trapezoid_1.bottom_left_element_pos;
            new_open_trapezoid.bottom_right_shape_pos = open_trapezoid_2.bottom_right_shape_pos;
            new_open_trapezoid.bottom_right_element_pos = open_trapezoid_2.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[p.first] = open_trapezoids.back();
            open_trapezoids.pop_back();
            open_trapezoids[p.second] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::Inflection) {
            // -1 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "Inflection" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            //std::cout << "open_trapezoid_pos " << open_trapezoid_pos << std::endl;
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];
            //std::cout << "open_trapezoid"
            //    << " bl " << open_trapezoid.bottom_left_element_pos.x << " " << open_trapezoid.bottom_left_element_pos.y
            //    << " br " << open_trapezoid.bottom_right_element_pos.x << " " << open_trapezoid.bottom_right_element_pos.y
            //    << " tl " << open_trapezoid.top_left.x << " " << open_trapezoid.top_left.y
            //    << " tr " << open_trapezoid.top_right.x << " " << open_trapezoid.top_right.y
            //    << std::endl;

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
            LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                // Open trapezoids with null height might be created.
                // If that happens, don't add them to the trapezoids.
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = {x_left, vertex.y};
            new_open_trapezoid.top_right = {x_right, vertex.y};
            // Both sides of the open trapezoid may end at the vertex where
            // boundaries touch, so the side ending at this vertex is
            // identified by its element rather than by its coordinates.
            if (shape_pos == open_trapezoid.bottom_left_shape_pos
                    && element_pos == open_trapezoid.bottom_left_element_pos) {
                new_open_trapezoid.bottom_left_shape_pos = shape_pos;
                new_open_trapezoid.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                    get_element_pos(current_shape, element_pos + 1):
                    get_element_pos(current_shape, element_pos - 1);
                new_open_trapezoid.bottom_right_shape_pos = open_trapezoid.bottom_right_shape_pos;
                new_open_trapezoid.bottom_right_element_pos = open_trapezoid.bottom_right_element_pos;
            } else {
                new_open_trapezoid.bottom_left_shape_pos = open_trapezoid.bottom_left_shape_pos;
                new_open_trapezoid.bottom_left_element_pos = open_trapezoid.bottom_left_element_pos;
                new_open_trapezoid.bottom_right_shape_pos = shape_pos;
                new_open_trapezoid.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                    get_element_pos(current_shape, element_pos - 1):
                    get_element_pos(current_shape, element_pos + 1);
            }
            open_trapezoids.push_back(new_open_trapezoid);

            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();


        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMaximumConvex
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMaximumConvex) {
            // +1 open trapezoid.
            //std::cout << "HorizontalLocalMaximumConvex HorizontalLocalMaximumConvex" << std::endl;

            // Update open_trapezoids.
            OpenTrapezoid open_trapezoid;
            open_trapezoid.top_left = vertex;
            open_trapezoid.top_right = vertex_next;
            open_trapezoid.bottom_left_shape_pos = shape_pos;
            open_trapezoid.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 1):
                get_element_pos(current_shape, element_pos - 1);
            open_trapezoid.bottom_right_shape_pos = shape_pos;
            open_trapezoid.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos_next - 1):
                get_element_pos(current_shape, element_pos_next + 1);

            open_trapezoids.push_back(open_trapezoid);

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMinimumConvex
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMinimumConvex) {
            // -1 open trapezoid.
            //std::cout << "HorizontalLocalMinimumConvex HorizontalLocalMinimumConvex" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_right = ((shape_pos == shape.holes.size())?
                get_vertex(current_shape, element_pos + 1):
                get_vertex(current_shape, element_pos - 1)).x;
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        vertex.x,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMaximumConcave
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMaximumConcave) {
            // -1 open trapezoid.
            // +2 open trapezoids.
            //std::cout << "HorizontalLocalMaximumConcave HorizontalLocalMaximumConcave" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];
            //std::cout << "open trapezoid " << open_trapezoid << std::endl;

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
            LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid_1;
            new_open_trapezoid_1.top_left = {x_left, vertex.y};
            new_open_trapezoid_1.top_right = vertex;
            new_open_trapezoid_1.bottom_left_shape_pos = open_trapezoid.bottom_left_shape_pos;
            new_open_trapezoid_1.bottom_left_element_pos = open_trapezoid.bottom_left_element_pos;
            new_open_trapezoid_1.bottom_right_shape_pos = shape_pos;
            new_open_trapezoid_1.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos - 1):
                get_element_pos(current_shape, element_pos + 1);

            OpenTrapezoid new_open_trapezoid_2;
            new_open_trapezoid_2.top_left = vertex_next;
            new_open_trapezoid_2.top_right = {x_right, vertex.y};
            new_open_trapezoid_2.bottom_left_shape_pos = shape_pos;
            new_open_trapezoid_2.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 2):
                get_element_pos(current_shape, element_pos - 2);
            new_open_trapezoid_2.bottom_right_shape_pos = open_trapezoid.bottom_right_shape_pos;
            new_open_trapezoid_2.bottom_right_element_pos = open_trapezoid.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid_1);
            open_trapezoids.push_back(new_open_trapezoid_2);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMinimumConcave
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMinimumConcave) {
            // -2 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "HorizontalLocalMinimumConcave HorizontalLocalMinimumConcave" << std::endl;

            ElementPos open_trapezoid_1_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_1_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_1_pos' must be != -1.");
            }
            auto p = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos_next);
            ElementPos open_trapezoid_2_pos = (p.second != -1)? p.second: p.first;
            if (open_trapezoid_2_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_2_pos' must be != -1.");
            }
            //std::cout << "open_trapezoid_1_pos " << open_trapezoid_1_pos << " open_trapezoid_2_pos " << open_trapezoid_2_pos << std::endl;
            const OpenTrapezoid& open_trapezoid_1 = open_trapezoids[open_trapezoid_1_pos];
            const OpenTrapezoid& open_trapezoid_2 = open_trapezoids[open_trapezoid_2_pos];

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid_1), open_trapezoid_1.top_left, vertex.y);
            if (vertex.y != open_trapezoid_1.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid_1.top_left.y,
                        x_left,
                        vertex.x,
                        open_trapezoid_1.top_left.x,
                        open_trapezoid_1.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            LengthDbl x_right = x(bottom_right(shape, open_trapezoid_2), open_trapezoid_2.top_right, vertex.y);
            if (vertex.y != open_trapezoid_2.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid_2.top_left.y,
                        vertex_next.x,
                        x_right,
                        open_trapezoid_2.top_left.x,
                        open_trapezoid_2.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = {x_left, vertex.y};
            new_open_trapezoid.top_right = {x_right, vertex.y};
            new_open_trapezoid.bottom_left_shape_pos = open_trapezoid_1.bottom_left_shape_pos;
            new_open_trapezoid.bottom_left_element_pos = open_trapezoid_1.bottom_left_element_pos;
            new_open_trapezoid.bottom_right_shape_pos = open_trapezoid_2.bottom_right_shape_pos;
            new_open_trapezoid.bottom_right_element_pos = open_trapezoid_2.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[open_trapezoid_1_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();
            open_trapezoids[open_trapezoid_2_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMaximumConvex
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMinimumConcave) {
            // -1 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "HorizontalLocalMaximumConvex HorizontalLocalMinimumConcave" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos_next).first;
            //std::cout << "open_trapezoid_pos " << open_trapezoid_pos << std::endl;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
            LengthDbl x_left = ((shape_pos == shape.holes.size())?
                get_vertex(current_shape, element_pos - 1):
                get_vertex(current_shape, element_pos + 1)).x;
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = vertex;
            new_open_trapezoid.top_right = {x_right, vertex.y};
            new_open_trapezoid.bottom_left_shape_pos = shape_pos;
            new_open_trapezoid.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 1):
                get_element_pos(current_shape, element_pos - 1);
            new_open_trapezoid.bottom_right_shape_pos = open_trapezoid.bottom_right_shape_pos;
            new_open_trapezoid.bottom_right_element_pos = open_trapezoid.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMinimumConvex
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMaximumConcave) {
            // -1 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "HorizontalLocalMinimumConvex HorizontalLocalMaximumConcave" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_right = x(bottom_right(shape, open_trapezoid), open_trapezoid.top_right, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        vertex.x,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = (shape_pos == shape.holes.size())?
                get_vertex(current_shape, element_pos + 1):
                get_vertex(current_shape, element_pos - 1);
            new_open_trapezoid.top_right = {x_right, vertex.y};
            new_open_trapezoid.bottom_left_shape_pos = shape_pos;
            new_open_trapezoid.bottom_left_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos + 2):
                get_element_pos(current_shape, element_pos - 2);
            new_open_trapezoid.bottom_right_shape_pos = open_trapezoid.bottom_right_shape_pos;
            new_open_trapezoid.bottom_right_element_pos = open_trapezoid.bottom_right_element_pos;

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMaximumConcave
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMinimumConvex) {
            // -1 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "HorizontalLocalMaximumConcave HorizontalLocalMinimumConvex" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos_next).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                LengthDbl x_right = ((shape_pos == shape.holes.size())?
                        get_vertex(current_shape, element_pos + 1):
                        get_vertex(current_shape, element_pos - 1)).x;
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        x_right,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = {x_left, vertex.y};
            new_open_trapezoid.top_right = vertex;
            new_open_trapezoid.bottom_left_shape_pos = open_trapezoid.bottom_left_shape_pos;
            new_open_trapezoid.bottom_left_element_pos = open_trapezoid.bottom_left_element_pos;
            new_open_trapezoid.bottom_right_shape_pos = shape_pos;
            new_open_trapezoid.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos - 1):
                get_element_pos(current_shape, element_pos + 1);

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else if (vertices[shape_pos][element_pos].flag == VertexTypeFlag::HorizontalLocalMinimumConcave
                && vertices[shape_pos][element_pos_next].flag == VertexTypeFlag::HorizontalLocalMaximumConvex) {
            // -1 open trapezoid.
            // +1 open trapezoids.
            //std::cout << "HorizontalLocalMinimumConcave HorizontalLocalMaximumConvex" << std::endl;

            ElementPos open_trapezoid_pos = find_trapezoid_containing_vertex(shape, open_trapezoids, shape_pos, element_pos).first;
            if (open_trapezoid_pos == -1) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": "
                        "'open_trapezoid_pos' must be != -1.");
            }
            const OpenTrapezoid& open_trapezoid = open_trapezoids[open_trapezoid_pos];

            // Update trapezoids.
            LengthDbl x_left = x(bottom_left(shape, open_trapezoid), open_trapezoid.top_left, vertex.y);
            if (vertex.y != open_trapezoid.top_left.y) {
                GeneralizedTrapezoid trapezoid(
                        vertex.y,
                        open_trapezoid.top_left.y,
                        x_left,
                        vertex.x,
                        open_trapezoid.top_left.x,
                        open_trapezoid.top_right.x);
                //std::cout << "new trapezoid " << trapezoid << std::endl;
                trapezoids.push_back(trapezoid);
            }

            // Update open_trapezoids.

            OpenTrapezoid new_open_trapezoid;
            new_open_trapezoid.top_left = {x_left, vertex.y};
            new_open_trapezoid.top_right = (shape_pos == shape.holes.size())?
                get_vertex(current_shape, element_pos - 1):
                get_vertex(current_shape, element_pos + 1);
            new_open_trapezoid.bottom_left_shape_pos = open_trapezoid.bottom_left_shape_pos;
            new_open_trapezoid.bottom_left_element_pos = open_trapezoid.bottom_left_element_pos;
            new_open_trapezoid.bottom_right_shape_pos = shape_pos;
            new_open_trapezoid.bottom_right_element_pos = (shape_pos == shape.holes.size())?
                get_element_pos(current_shape, element_pos - 2):
                get_element_pos(current_shape, element_pos + 2);

            open_trapezoids.push_back(new_open_trapezoid);
            open_trapezoids[open_trapezoid_pos] = open_trapezoids.back();
            open_trapezoids.pop_back();

        } else {
            throw std::runtime_error(
                    FUNC_SIGNATURE + "; "
                    "flag: " + std::to_string((int)vertices[shape_pos][element_pos].flag) + "; "
                    "flag_next: " + std::to_string((int)vertices[shape_pos][element_pos_next].flag) + ".");

        }
        //std::cout << std::endl;
    }

    // Check area.
    AreaDbl shape_area = shape.shape.compute_area();
    for (const Shape& hole: shape.holes)
        shape_area -= hole.compute_area();
    AreaDbl trapezoidation_area = 0.0;
    for (const GeneralizedTrapezoid& trapezoid: trapezoids)
        trapezoidation_area += trapezoid.area();

    if (!equal(shape_area, trapezoidation_area)) {
        Writer writer;
        writer.add_shape_with_holes(shape_orig);
        for (const GeneralizedTrapezoid& trapezoid: trapezoids)
            writer.add_shape(trapezoid.to_shape(), "Trapezoid");
        writer.write_json("trapezoidation.json");
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "shape_area: " + std::to_string(shape_area) + "; "
                "trapezoidation_area: " + std::to_string(trapezoidation_area) + ".");
    }

    //std::cout << "polygon_trapezoidation end" << std::endl;
    return trapezoids;
}
