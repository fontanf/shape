#include "shape/equalize.hpp"

#include "shape/intersection_tree.hpp"

//#include <iostream>

using namespace shape;

std::vector<Point> shape::equalize_points(
        const std::vector<Point>& points)
{
    IntersectionTree intersection_tree({}, {}, points);
    std::vector<std::pair<ElementPos, ElementPos>> equal_points
        = intersection_tree.compute_equal_points();

    // Build graph.
    std::vector<std::vector<ElementPos>> graph(points.size());
    for (auto p: equal_points) {
        graph[p.first].push_back(p.second);
        graph[p.second].push_back(p.first);
    }

    // For each connected component, build a point.
    ElementPos node_id = 0;
    std::vector<uint8_t> visited(points.size(), 0);
    std::vector<Point> new_points = points;
    std::vector<ElementPos> current_component;
    Point current_component_point = {0, 0};
    for (;;) {
        while (node_id < points.size()
                && visited[node_id]) {
            node_id++;
        }
        if (node_id == points.size())
            break;

        const Point& point = points[node_id];
        visited[node_id] = 1;
        current_component = {node_id};
        current_component_point = points[node_id];
        std::vector<ElementPos> stack = {node_id};
        while (!stack.empty()) {
            ElementPos node_id_cur = stack.back();
            stack.pop_back();
            for (ElementPos neighbor: graph[node_id_cur]) {
                if (visited[neighbor])
                    continue;
                stack.push_back(neighbor);
                visited[neighbor] = 1;
                current_component.push_back(neighbor);
                current_component_point = current_component_point + points[neighbor];
                //std::cout << "new_points[neighbor] " << new_points[neighbor].to_string()
                //    << " -> " << point.to_string() << std::endl;
            }
        }
        current_component_point = 1.0 / current_component.size() * current_component_point;
        for (ElementPos point_pos: current_component)
            new_points[point_pos] = current_component_point;
    }

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
