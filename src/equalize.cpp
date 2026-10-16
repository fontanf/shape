#include "shape/equalize.hpp"

#include "shape/intersection_tree.hpp"

#include <queue>

//#include <iostream>

using namespace shape;

std::vector<Point> shape::equalize_points(
        const std::vector<Point>& points)
{
    //std::cout << "equalize_points " << points.size() << std::endl;

    std::vector<std::pair<Point, ElementPos>> sorted_points(points.size());
    for (ElementPos pos = 0; pos < (ElementPos)points.size(); ++pos)
        sorted_points[pos] = {points[pos], pos};
    std::sort(
            sorted_points.begin(),
            sorted_points.end(),
            [](const auto& pair_1, const auto pair_2)
            {
                if (pair_1.first.x != pair_2.first.x)
                    return pair_1.first.x < pair_2.first.x;
                return pair_1.first.y < pair_2.first.y;
            });
    std::vector<std::vector<ElementPos>> unique_to_orig;
    std::vector<Point> unique_points;
    for (const auto& pair: sorted_points) {
        if (unique_points.empty() || !(pair.first == unique_points.back())) {
            unique_to_orig.push_back({});
            unique_points.push_back(pair.first);
        }
        unique_to_orig.back().push_back(pair.second);
    }
    //std::cout << "unique_points " << unique_points.size() << std::endl;

    IntersectionTree intersection_tree({}, {}, unique_points);
    std::vector<std::pair<ElementPos, ElementPos>> equal_points
        = intersection_tree.compute_equal_points();

    // Build graph.
    std::vector<std::vector<ElementPos>> graph(unique_points.size());
    for (auto p: equal_points) {
        graph[p.first].push_back(p.second);
        graph[p.second].push_back(p.first);
    }

    // Greedily build clusters. Each cluster is made of a representative
    // point and all its unassigned neighbors, and all its points are moved to
    // the representative. The representative is the unassigned point with
    // the largest number of unassigned neighbors. Thus, each point is moved
    // to a point equal to it, and the representatives are pairwise not equal,
    // since a point equal to a representative is assigned to its cluster
    // before it can become a representative itself.
    // Unlike merging the connected components of the graph, this doesn't
    // move points arbitrarily far when they form chains of equal points.
    std::vector<ElementPos> degrees(unique_points.size());
    std::priority_queue<std::pair<ElementPos, ElementPos>> queue;
    for (ElementPos node_id = 0;
            node_id < (ElementPos)unique_points.size();
            ++node_id) {
        degrees[node_id] = graph[node_id].size();
        queue.push({degrees[node_id], -node_id});
    }
    std::vector<uint8_t> assigned(unique_points.size(), 0);
    std::vector<Point> new_points = points;
    while (!queue.empty()) {
        ElementPos degree = queue.top().first;
        ElementPos node_id = -queue.top().second;
        queue.pop();
        if (assigned[node_id])
            continue;
        // The degree of the node decreased since it was pushed; push it
        // again with its current degree.
        if (degree != degrees[node_id]) {
            queue.push({degrees[node_id], -node_id});
            continue;
        }

        const Point& representative = unique_points[node_id];
        std::vector<ElementPos> cluster = {node_id};
        assigned[node_id] = 1;
        for (ElementPos neighbor: graph[node_id]) {
            if (assigned[neighbor])
                continue;
            assigned[neighbor] = 1;
            cluster.push_back(neighbor);
        }
        for (ElementPos point_pos: cluster) {
            for (ElementPos neighbor: graph[point_pos])
                degrees[neighbor]--;
            for (ElementPos orig_point_pos: unique_to_orig[point_pos])
                new_points[orig_point_pos] = representative;
        }
    }

    //std::cout << "equal_points end" << std::endl;
    return new_points;
}

std::vector<LengthDbl> shape::equalize_values(
        const std::vector<LengthDbl>& values)
{
    std::vector<Point> points(values.size());
    for (ElementPos pos = 0;
            pos < (ElementPos)values.size();
            ++pos) {
        points[pos] = {values[pos], 0.0};
    }
    std::vector<Point> equalized_points = equalize_points(points);
    std::vector<LengthDbl> equalized_values(values.size());
    for (ElementPos pos = 0;
            pos < (ElementPos)values.size();
            ++pos) {
        equalized_values[pos] = equalized_points[pos].x;
    }
    return equalized_values;
}

Shape shape::equalize_shape(
        const Shape& shape_orig)
{
    Shape shape = shape_orig;
    std::vector<Point*> equalize_to_orig;
    std::vector<LengthDbl> equalize_input_x;
    std::vector<LengthDbl> equalize_input_y;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        ShapeElement& element = shape.elements[element_pos];
        equalize_input_x.push_back(element.start.x);
        equalize_input_y.push_back(element.start.y);
        equalize_to_orig.push_back(&element.start);
        equalize_input_x.push_back(element.end.x);
        equalize_input_y.push_back(element.end.y);
        equalize_to_orig.push_back(&element.end);
        if (element.type == ShapeElementType::CircularArc) {
            equalize_input_x.push_back(element.center.x);
            equalize_input_y.push_back(element.center.y);
            equalize_to_orig.push_back(&element.center);
        }
    }
    std::vector<LengthDbl> equalize_output_x = equalize_values(equalize_input_x);
    std::vector<LengthDbl> equalize_output_y = equalize_values(equalize_input_y);
    for (ElementPos pos = 0; pos < (ElementPos)equalize_output_x.size(); ++pos) {
        (*equalize_to_orig[pos]).x = equalize_output_x[pos];
        (*equalize_to_orig[pos]).y = equalize_output_y[pos];
    }
    return shape;
}

ShapeWithHoles shape::equalize_shape(
        const ShapeWithHoles& shape_orig)
{
    ShapeWithHoles shape = shape_orig;
    std::vector<Point*> equalize_to_orig;
    std::vector<LengthDbl> equalize_input_x;
    std::vector<LengthDbl> equalize_input_y;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.shape.elements.size();
            ++element_pos) {
        ShapeElement& element = shape.shape.elements[element_pos];
        equalize_input_x.push_back(element.start.x);
        equalize_input_y.push_back(element.start.y);
        equalize_to_orig.push_back(&element.start);
        equalize_input_x.push_back(element.end.x);
        equalize_input_y.push_back(element.end.y);
        equalize_to_orig.push_back(&element.end);
        if (element.type == ShapeElementType::CircularArc) {
            equalize_input_x.push_back(element.center.x);
            equalize_input_y.push_back(element.center.y);
            equalize_to_orig.push_back(&element.center);
        }
    }
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        Shape& hole = shape.holes[hole_pos];
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)hole.elements.size();
                ++element_pos) {
            ShapeElement& element = hole.elements[element_pos];
            equalize_input_x.push_back(element.start.x);
            equalize_input_y.push_back(element.start.y);
            equalize_to_orig.push_back(&element.start);
            equalize_input_x.push_back(element.end.x);
            equalize_input_y.push_back(element.end.y);
            equalize_to_orig.push_back(&element.end);
            if (element.type == ShapeElementType::CircularArc) {
                equalize_input_x.push_back(element.center.x);
                equalize_input_y.push_back(element.center.y);
                equalize_to_orig.push_back(&element.center);
            }
        }
    }
    std::vector<LengthDbl> equalize_output_x = equalize_values(equalize_input_x);
    std::vector<LengthDbl> equalize_output_y = equalize_values(equalize_input_y);
    for (ElementPos pos = 0; pos < (ElementPos)equalize_output_x.size(); ++pos) {
        (*equalize_to_orig[pos]).x = equalize_output_x[pos];
        (*equalize_to_orig[pos]).y = equalize_output_y[pos];
    }
    return shape;
}
