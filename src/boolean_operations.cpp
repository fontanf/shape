#include "shape/boolean_operations.hpp"

#include "shape/equalize.hpp"
#include "shape/intersection_tree.hpp"
#include "shape/clean.hpp"

#include "optimizationtools/containers/doubly_indexed_map.hpp"

//#include <iostream>
//#include <fstream>

using namespace shape;

namespace
{

enum class BooleanOperation
{
    Union,
    Intersection,
    Difference,
    SymmetricDifference,
};

using ComponentId = int64_t;

struct SplittedElement
{
    ShapeElement element;

    ShapePos orig_shape_id = -1;

    bool original_direction = true;
};

bool operator==(
        const SplittedElement& element_1,
        const SplittedElement& element_2)
{
    if (element_1.element.type == ShapeElementType::CircularArc
            && element_1.element.orientation == ShapeElementOrientation::Full) {
        return (element_1.element == element_2.element
                && element_1.original_direction == element_2.original_direction);
    } else {
        return element_1.element == element_2.element;
    }
}

bool operator<(
        const SplittedElement& element_1,
        const SplittedElement& element_2)
{
    return element_1.element < element_2.element;
}

struct ComputeSplittedElementsOutput
{
    ComputeSplittedElementsOutput(ShapePos number_of_shapes):
        shape_component_ids(number_of_shapes, number_of_shapes) { }

    std::vector<std::vector<SplittedElement>> components_splitted_elements;

    optimizationtools::DoublyIndexedMap shape_component_ids;
};

struct ElementToSplit
{
    ShapePos orig_shape_id = -1;
};

ComputeSplittedElementsOutput compute_splitted_elements(
        const std::vector<ShapeWithHoles>& shapes)
{
    //std::cout << "compute_splitted_elements"
    //    " shapes.size() " << shapes.size() << std::endl;
    //for (const ShapeWithHoles& shape: shapes) {
    //    std::cout << shape.to_string(2)
    //        << shape.shape.compute_area()
    //        << std::endl;
    //}

    ComputeSplittedElementsOutput output(shapes.size());

    std::vector<ShapeElement> elements;
    std::vector<ElementToSplit> elements_info;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        Shape shape = shapes[shape_pos].bridge_holes();
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)shape.elements.size();
                ++element_pos) {
            const ShapeElement& element = shape.elements[element_pos];
            elements.push_back(element);
            ElementToSplit element_info;
            element_info.orig_shape_id = shape_pos;
            elements_info.push_back(element_info);
        }
    }
    //std::cout << "elements" << std::endl;
    //for (const ShapeElement& element: elements)
    //    std::cout << element.to_string() << std::endl;
    //std::cout << "elements end" << std::endl;

    IntersectionTree intersection_tree({}, elements, {});
    std::vector<IntersectionTree::ElementElementIntersection> intersections
        = intersection_tree.compute_intersecting_elements(false);
    std::vector<std::vector<Point>> element_intersections(elements.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        output.shape_component_ids.set(shape_pos, shape_pos);
    }
    for (const IntersectionTree::ElementElementIntersection& intersection: intersections) {
        // Update output.shape_component_ids.
        ShapePos shape_pos_1 = elements_info[intersection.element_id_1].orig_shape_id;
        ShapePos shape_pos_2 = elements_info[intersection.element_id_2].orig_shape_id;
        ComponentId component_id_1 = output.shape_component_ids[shape_pos_1];
        ComponentId component_id_2 = output.shape_component_ids[shape_pos_2];
        if (component_id_1 != component_id_2) {
            for (ShapePos shape_pos = 0;
                    shape_pos < (ShapePos)shapes.size();
                    ++shape_pos) {
                if (output.shape_component_ids[shape_pos] == component_id_2)
                    output.shape_component_ids.set(shape_pos, component_id_1);
            }
        }

        //if (!intersections.empty()) {
        //    std::cout << "element_1 " << intersection.element_id_1 << " " << elements[intersection.element_id_1].to_string() << std::endl;
        //    std::cout << "element_2 " << intersection.element_id_2 << " " << elements[intersection.element_id_2].to_string() << std::endl;
        //}
        for (const Point& point: intersection.intersections.points) {
            //std::cout << "intersection " << point.to_string() << std::endl;
            element_intersections[intersection.element_id_1].push_back(point);
            element_intersections[intersection.element_id_2].push_back(point);
        }
    }

    // Equalize points.
    std::vector<Point*> equalize_to_orig;
    std::vector<Point> equalize_input;
    std::vector<ShapeElement> elements_tmp = elements;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)elements.size();
            ++element_pos) {
        ShapeElement& element = elements_tmp[element_pos];
        equalize_input.push_back(element.start);
        equalize_to_orig.push_back(&element.start);
        equalize_input.push_back(element.end);
        equalize_to_orig.push_back(&element.end);
        if (element.type == ShapeElementType::CircularArc) {
            equalize_input.push_back(element.center);
            equalize_to_orig.push_back(&element.center);
        }
        for (Point& intersection: element_intersections[element_pos]) {
            equalize_input.push_back(intersection);
            equalize_to_orig.push_back(&intersection);
        }
    }
    std::vector<Point> equalize_output = equalize_points(equalize_input);
    for (ElementPos pos = 0; pos < (ElementPos)equalize_output.size(); ++pos)
        *equalize_to_orig[pos] = equalize_output[pos];

    // For each pair of connected component, check if one is strictly inside the
    // other.
    IntersectionTree intersection_tree_2(shapes, {}, {});
    for (auto it = output.shape_component_ids.values_begin();
            it != output.shape_component_ids.values_end();
            ++it) {
        ComponentId component_id = *it;

        // Draw a point strictly inside the component.
        //std::cout << "component_id " << component_id << std::endl;
        //std::cout << output.shape_component_ids.number_of_elements(component_id) << std::endl;
        //std::cout << *output.shape_component_ids.begin(component_id) << " / " << shapes.size() << std::endl;
        //std::cout << shapes[*(output.shape_component_ids.begin(component_id))].to_string(0) << std::endl;
        Point point = shapes[*(output.shape_component_ids.begin(component_id))].shape.elements.front().start;
        //std::cout << "point from component " << component_id << ": " << point.to_string() << std::endl;

        // Check if it is inside another component.
        IntersectionTree::IntersectOutput it_output = intersection_tree_2.intersect(point, true);
        if (it_output.shape_ids.empty())
            continue;
        ComponentId component_id_2 = -1;
        for (ShapePos shape_id: it_output.shape_ids) {
            if (output.shape_component_ids[shape_id] != component_id) {
                component_id_2 = output.shape_component_ids[shape_id];
                break;
            }
        }
        //std::cout << "component_id_2 " << component_id_2 << std::endl;
        if (component_id_2 != -1) {
            // Update shape_component_ids.
            if (component_id > component_id_2) {
                while (output.shape_component_ids.number_of_elements(component_id) > 0)
                    output.shape_component_ids.set(*output.shape_component_ids.begin(component_id), component_id_2);
            } else {
                while (output.shape_component_ids.number_of_elements(component_id_2) > 0)
                    output.shape_component_ids.set(*output.shape_component_ids.begin(component_id_2), component_id);
            }
        }
    }

    output.components_splitted_elements = std::vector<std::vector<SplittedElement>>(shapes.size());
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)elements.size();
            ++element_pos) {
        ShapeElement element = elements_tmp[element_pos];
        ShapePos shape_pos = elements_info[element_pos].orig_shape_id;
        ComponentId component_id = output.shape_component_ids[shape_pos];
        //std::cout << "element_pos " << element_pos
        //    << " " << element.to_string()
        //    << " shape_pos " << shape_pos
        //    << " component_id " << component_id
        //    << std::endl;
        // Sort intersection points of this element.
        std::sort(
                element_intersections[element_pos].begin(),
                element_intersections[element_pos].end(),
                [&element](
                    const Point& point_1,
                    const Point& point_2)
                {
                    switch (element.type) {
                    case ShapeElementType::LineSegment: {
                        return distance(element.start, point_1)
                            < distance(element.start, point_2);
                    } case ShapeElementType::CircularArc: {
                        return angle_radian(element.start - element.center, point_1 - element.center)
                            < angle_radian(element.start - element.center, point_2 - element.center);
                    }
                    }
                    return false;
                });
        // Create new elements.
        if (element.type == ShapeElementType::CircularArc
                && element.orientation == ShapeElementOrientation::Full
                && !element_intersections[element_pos].empty()) {
            element.start = element_intersections[element_pos].front();
            element.end = element_intersections[element_pos].front();
        }

        bool first = true;
        for (const Point& point_cur: element_intersections[element_pos]) {
            // Skip segment ends and duplicated intersections.
            if (point_cur == element.start
                    || point_cur == element.end) {
                continue;
            }
            if (equal(point_cur, element.start)
                    || equal(point_cur, element.end)) {
                throw std::logic_error(FUNC_SIGNATURE);
            }

            if (first
                    && element.type == ShapeElementType::CircularArc
                    && element.orientation == ShapeElementOrientation::Full) {
                first = false;

                ShapeElement e;
                e.type = ShapeElementType::CircularArc;
                e.start = element.start;
                e.end = point_cur;
                e.center = element.center;
                e.orientation = ShapeElementOrientation::Anticlockwise;

                element.start = point_cur;
                element.orientation = ShapeElementOrientation::Anticlockwise;

                SplittedElement new_element;
                new_element.element = e;
                new_element.orig_shape_id = shape_pos;
                new_element.original_direction = true;
                output.components_splitted_elements[component_id].push_back(new_element);

                SplittedElement new_element_reversed;
                new_element_reversed.element = e.reverse();
                new_element_reversed.orig_shape_id = shape_pos;
                new_element_reversed.original_direction = false;
                output.components_splitted_elements[component_id].push_back(new_element_reversed);

                //std::cout << "  - " << point_cur.to_string() << std::endl;
                //std::cout << "    " << new_element.element.to_string() << std::endl;
                //std::cout << "    length " << new_element.element.length() << std::endl;
            } else {

                auto p = element.split(point_cur);
                element = p.second;
                if (p.first.start == p.first.end
                        && p.first.orientation != shape::ShapeElementOrientation::Full) {
                    continue;
                }

                SplittedElement new_element;
                new_element.element = p.first;
                new_element.orig_shape_id = shape_pos;
                new_element.original_direction = true;
                output.components_splitted_elements[component_id].push_back(new_element);

                SplittedElement new_element_reversed;
                new_element_reversed.element = p.first.reverse();
                new_element_reversed.orig_shape_id = shape_pos;
                new_element_reversed.original_direction = false;
                output.components_splitted_elements[component_id].push_back(new_element_reversed);

                //std::cout << "  - " << point_cur.to_string() << std::endl;
                //std::cout << "    " << new_element.element.to_string() << std::endl;
                //std::cout << "    length " << new_element.element.length() << std::endl;
            }
        }

        if (element.orientation == ShapeElementOrientation::Full
                || !equal(element.start, element.end)) {
            SplittedElement new_element;
            new_element.element = element;
            new_element.orig_shape_id = elements_info[element_pos].orig_shape_id;
            new_element.original_direction = true;
            output.components_splitted_elements[component_id].push_back(new_element);

            SplittedElement new_element_reversed;
            new_element_reversed.element = element.reverse();
            new_element_reversed.orig_shape_id = elements_info[element_pos].orig_shape_id;
            new_element_reversed.original_direction = false;
            output.components_splitted_elements[component_id].push_back(new_element_reversed);

            //std::cout << "  - " << new_element.element.to_string() << std::endl;
            //std::cout << "    length " << new_element.element.length() << std::endl;
        }
    }

    // Remove duplicates in splitted elements.
    for (auto it = output.shape_component_ids.values_begin();
            it != output.shape_component_ids.values_end();
            ++it) {
        ComponentId component_id = *it;
        auto& vec = output.components_splitted_elements[component_id];
        std::sort(vec.begin(), vec.end());
        vec.erase(unique(vec.begin(), vec.end()), vec.end());
    }

    //std::cout << "output.shape_component_ids.number_of_values() " << output.shape_component_ids.number_of_values() << std::endl;
    //std::cout << "compute_splitted_elements end" << std::endl;
    return output;
}

using NodeId = int64_t;

struct BooleanOperationArc
{
    NodeId source_node_id = -1;

    NodeId end_node_id = -1;

    ElementPos element_pos = -1;
};

struct BooleanOperationNode
{
    std::vector<ElementPos> successors;
};

struct BooleanOperationGraph
{
    std::vector<BooleanOperationArc> arcs;

    std::vector<BooleanOperationNode> nodes;
};

BooleanOperationGraph compute_graph(
        const std::vector<SplittedElement>& splitted_elements)
{
    // Sort all element points.
    //std::cout << "compute_graph splitted_elements.size() " << splitted_elements.size() << std::endl;
    std::vector<std::pair<ElementPos, bool>> sorted_element_points;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)splitted_elements.size();
            ++element_pos) {
        sorted_element_points.push_back({element_pos, true});
        sorted_element_points.push_back({element_pos, false});
    }
    std::sort(
            sorted_element_points.begin(),
            sorted_element_points.end(),
            [&splitted_elements](
                const std::pair<ElementPos, bool>& p1,
                const std::pair<ElementPos, bool>& p2)
            {
                ElementPos element_pos_1 = p1.first;
                ElementPos element_pos_2 = p2.first;
                const ShapeElement& element_1 = splitted_elements[element_pos_1].element;
                const ShapeElement& element_2 = splitted_elements[element_pos_2].element;
                bool start_1 = p1.second;
                bool start_2 = p2.second;
                const Point& point_1 = (start_1)? element_1.start: element_1.end;
                const Point& point_2 = (start_2)? element_2.start: element_2.end;
                if (point_1.x != point_2.x)
                    return point_1.x < point_2.x;
                return point_1.y < point_2.y;
            });

    BooleanOperationGraph graph;
    graph.arcs = std::vector<BooleanOperationArc>(splitted_elements.size());
    // For each point associate a node id and build the graph arcs.
    Point point_prev = {0, 0};
    for (ElementPos pos = 0; pos < (ElementPos)sorted_element_points.size(); ++pos) {
        ElementPos element_pos = sorted_element_points[pos].first;
        const ShapeElement& element = splitted_elements[element_pos].element;
        bool start = sorted_element_points[pos].second;
        const Point& point = (start)? element.start: element.end;
        //std::cout << "pos " << pos
        //    << " element_pos " << element_pos
        //    << " start " << start
        //    << " point " << point.to_string()
        //    << std::endl;
        if (pos == 0 || !(point_prev == point))
            graph.nodes.push_back(BooleanOperationNode());
        NodeId node_id = graph.nodes.size() - 1;
        if (start) {
            graph.arcs[element_pos].source_node_id = node_id;
            graph.nodes[node_id].successors.push_back(element_pos);
        } else {
            graph.arcs[element_pos].end_node_id = node_id;
        }
        point_prev = point;
    }
    //std::cout << "compute_graph end" << std::endl;
    return graph;
}

std::vector<ShapeWithHoles> compute_boolean_operation_component(
        const std::vector<ShapeWithHoles>& shapes,
        const std::vector<SplittedElement>& splitted_elements,
        BooleanOperation boolean_operation)
{
    //std::cout << "compute_boolean_operation_component" << std::endl;
    //for (const SplittedElement& splitted_element: splitted_elements)
    //    std::cout << splitted_element.element.to_string() << std::endl;

    std::vector<ShapeWithHoles> new_shapes;
    BooleanOperationGraph graph = compute_graph(splitted_elements);

    //if (boolean_operation == BooleanOperation::Difference) {
    //    std::vector<ShapeElement> elements;
    //    for (const auto& splitted_element: splitted_elements)
    //        elements.push_back(splitted_element.element);
    //    write_json({}, {elements}, "overlay.json");
    //}

    IntersectionTree intersection_tree(shapes, {}, {});

    // Find an element from the outline.
    // To do so find, all elements from the original elements with the leftest
    // point.
    // If there are multiple such elements, then necessarily, the leftest point
    // is their start and/or their end.
    // Discard elements for which it's not the start.
    // Find the element with the smallest angle with -y.
    std::vector<ElementPos> leftest_elements_pos;
    Point p_min = {std::numeric_limits<LengthDbl>::infinity(), std::numeric_limits<LengthDbl>::infinity()};
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)splitted_elements.size();
            ++element_pos) {
        const SplittedElement& element = splitted_elements[element_pos];
        if (element.original_direction)
            continue;
        auto p = element.element.furthest_points(90);
        //std::cout << "element_pos " << element_pos
        //    << " " << element.element.to_string()
        //    << " p " << p.first.to_string() << " " << p.second.to_string()
        //    << std::endl;
        if (strictly_lesser(p.first.x, p_min.x)
                || (equal(p.first.x, p_min.x) && strictly_lesser(p.first.y, p_min.y))) {
            leftest_elements_pos = {element_pos};
            p_min = p.first;
        } else if (equal(p.first, p_min)) {
            leftest_elements_pos.push_back(element_pos);
        }
        if (strictly_lesser(p.second.x, p_min.x)
                || (equal(p.second.x, p_min.x) && strictly_lesser(p.second.y, p_min.y))) {
            leftest_elements_pos = {element_pos};
            p_min = p.second;
        } else if (equal(p.second, p_min)) {
            leftest_elements_pos.push_back(element_pos);
        }
    }
    //std::cout << "leftest_elements_pos.size() " << leftest_elements_pos.size() << std::endl;

    if (leftest_elements_pos.size() > 1) {
        std::vector<ElementPos> leftest_elements_pos_tmp;
        for (ElementPos element_pos: leftest_elements_pos) {
            const SplittedElement& element = splitted_elements[element_pos];
            //std::cout << element.element.to_string() << std::endl;
            if (!equal(element.element.start, p_min))
                continue;
            leftest_elements_pos_tmp.push_back(element_pos);
        }
        leftest_elements_pos = leftest_elements_pos_tmp;
    }
    //std::cout << "leftest_elements_pos.size() " << leftest_elements_pos.size() << std::endl;

    ElementPos element_start_pos = -1;
    Angle angle_best = 3 * M_PI;
    Point direction_0 = {0, 1};
    for (ElementPos element_pos: leftest_elements_pos) {
        const SplittedElement& splitted_element = splitted_elements[element_pos];
        const ShapeElement& element = splitted_element.element;
        // Compute angle with -y.
        Point direction = {0, 0};
        switch (element.type) {
        case ShapeElementType::LineSegment: {
            direction = element.end - element.start;
            break;
        } case ShapeElementType::CircularArc: {
            if (element.orientation == ShapeElementOrientation::Anticlockwise) {
                direction = {
                    element.center.y - element.start.y,
                    element.start.x - element.center.x};
            } else {
                direction = {
                    element.start.y - element.center.y,
                    element.center.x - element.start.x};
            }
            break;
        }
        }

        Angle angle = angle_radian(
                direction,
                direction_0);
        //std::cout << "element_pos " << element_pos
        //    << " " << element.to_string()
        //    << " angle " << angle
        //    << std::endl;
        if (angle_best > angle
                || equal(angle_best, angle) && element.type == ShapeElementType::LineSegment) {
            element_start_pos = element_pos;
            angle_best = angle;
        }
    }
    //std::cout << "element_start_pos " << element_start_pos << std::endl;
    if (element_start_pos == -1) {
        throw std::logic_error(
                FUNC_SIGNATURE + ": element_start_pos is '-1'.");
    }

    std::vector<uint8_t> element_is_processed(splitted_elements.size(), 0);

    // Find outer loop.
    //std::cout << "find outer loop..." << std::endl;
    ElementPos element_cur_pos = element_start_pos;
    Shape outline;
    for (int i = 0;; ++i) {
        // Check infinite loop.
        if (i >= 2 * splitted_elements.size()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": infinite loop in outline.");
        }

        const ShapeElement& element_cur = splitted_elements[element_cur_pos].element;
        ShapeElement element_cur_rev = element_cur.reverse();
        const BooleanOperationArc& arc = graph.arcs[element_cur_pos];
        const BooleanOperationNode& node = graph.nodes[arc.end_node_id];
        element_is_processed[element_cur_pos] = 1;
        outline.elements.push_back(element_cur);

        if (element_cur.orientation == ShapeElementOrientation::Full)
            break;

        // Find the next element with the smallest angle.
        //std::cout
        //    << "element_cur_pos " << element_cur_pos
        //    << " " << element_cur.to_string()
        //    << " node_id " << arc.end_node_id << " / " << graph.nodes.size()
        //    << std::endl;
        ElementPos smallest_angle_element_pos = -1;
        Angle smallest_angle = 0.0;

        Point direction_cur = {0, 0};
        switch (element_cur.type) {
        case ShapeElementType::LineSegment: {
            direction_cur = element_cur.end - element_cur.start;
            break;
        } case ShapeElementType::CircularArc: {
            if (element_cur.orientation == ShapeElementOrientation::Anticlockwise) {
                direction_cur = {
                    element_cur.center.y - element_cur.end.y,
                    element_cur.end.x - element_cur.center.x};
            } else {
                direction_cur = {
                    element_cur.end.y - element_cur.center.y,
                    element_cur.center.x - element_cur.end.x};
            }
            break;
        }
        }
        direction_cur = {-direction_cur.x, -direction_cur.y};

        for (ElementPos element_pos_next: node.successors) {
            const ShapeElement& element_next = splitted_elements[element_pos_next].element;
            if (element_next == element_cur_rev)
                continue;

            Point direction_next = {0, 0};
            switch (element_next.type) {
            case ShapeElementType::LineSegment: {
                direction_next = element_next.end - element_next.start;
                break;
            } case ShapeElementType::CircularArc: {
                if (element_next.orientation == ShapeElementOrientation::Anticlockwise) {
                    direction_next = {
                        element_next.center.y - element_next.start.y,
                        element_next.start.x - element_next.center.x};
                } else {
                    direction_next = {
                        element_next.start.y - element_next.center.y,
                        element_next.center.x - element_next.start.x};
                }
                break;
            }
            }

            Angle angle = angle_radian(
                    direction_next,
                    direction_cur);
            //std::cout << "- element_pos_next " << element_pos_next
            //    << " " << element_next.to_string()
            //    << " angle " << angle
            //    << std::endl;
            bool update = false;
            if (smallest_angle_element_pos == -1) {
                update = true;
            } else if (strictly_greater(smallest_angle, angle)) {
                //std::cout << "strictly_lesser" << std::endl;
                update = true;
            } else if (equal(smallest_angle, angle)) {
                const SplittedElement& smallest_angle_splitted_element = splitted_elements[smallest_angle_element_pos];
                const ShapeElement& smallest_angle_element = smallest_angle_splitted_element.element;
                if (smallest_angle_element.type == ShapeElementType::CircularArc
                        && smallest_angle_element.orientation == ShapeElementOrientation::Clockwise) {
                    if (element_next.type == ShapeElementType::LineSegment) {
                        update = true;
                    } else if (element_next.type == ShapeElementType::CircularArc
                            && element_next.orientation == ShapeElementOrientation::Anticlockwise) {
                        update = true;
                    }
                } else if (smallest_angle_element.type == ShapeElementType::LineSegment) {
                    if (element_next.type == ShapeElementType::CircularArc
                            && element_next.orientation == ShapeElementOrientation::Anticlockwise) {
                        update = true;
                    }
                }
            }
            //std::cout << "update " << update << std::endl;
            if (update) {
                smallest_angle_element_pos = element_pos_next;
                smallest_angle = angle;
            }
        }
        if (smallest_angle_element_pos == -1) {
            //nlohmann::json json;
            //for (ElementPos pos = 0;
            //        pos < (ElementPos)splitted_elements.size();
            //        ++pos) {
            //    json["elements"][pos] = splitted_elements[pos].element.to_json();
            //}
            //std::string file_path = "overlay.json";
            //std::ofstream file{file_path};
            //if (!file.good()) {
            //    throw std::runtime_error(
            //            "Unable to open file \"" + file_path + "\".");
            //}
            //file << std::setw(4) << json << std::endl;
            throw std::logic_error(
                    FUNC_SIGNATURE + ": "
                    "smallest_angle_element_pos is '-1' in outline.");
        }

        // Update current element.
        element_cur_pos = smallest_angle_element_pos;
        if (element_cur_pos == element_start_pos)
            break;
    }
    //std::cout << "shape " << outline.to_string(0) << std::endl;
    switch (boolean_operation) {
    case BooleanOperation::Union: {
        ShapeWithHoles new_shape;
        new_shape.shape = outline.reverse();
        new_shape.shape = remove_redundant_vertices(new_shape.shape).second;
        new_shape.shape = remove_aligned_vertices(new_shape.shape).second;
        new_shapes.push_back(new_shape);
        break;
    } case BooleanOperation::Intersection: {
        break;
    } case BooleanOperation::Difference: {
        break;
    } case BooleanOperation::SymmetricDifference: {
        break;
    }
    }

    // Process each face.
    element_start_pos = -1;
    for (;;) {
        // Find an unprocessed element.
        NodeId node_start_id = -1;
        for (ElementPos element_pos = element_start_pos + 1;
                element_pos < (ElementPos)splitted_elements.size();
                ++element_pos) {
            const BooleanOperationArc& arc = graph.arcs[element_pos];
            const ShapeElement& element = splitted_elements[element_pos].element;
            //std::cout << "element " << element_pos
            //    << " " << element.to_string()
            //    << " proc " << element_is_processed[element_pos]
            //    << std::endl;
            if (!element_is_processed[element_pos]) {
                element_start_pos = element_pos;
                node_start_id = arc.end_node_id;
                break;
            }
        }
        // If all elements have already been processed, stop.
        if (node_start_id == -1)
            break;

        //std::cout << "find faces..." << std::endl;
        Shape face;
        std::vector<uint8_t> is_inside(shapes.size());
        ElementPos element_cur_pos = element_start_pos;
        for (int i = 0;; ++i) {
            // Check infinite loop.
            if (i >= 2 * splitted_elements.size()) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + ": infinite loop in faces.");
            }

            const SplittedElement& splitted_element_cur = splitted_elements[element_cur_pos];
            const ShapeElement& element_cur = splitted_element_cur.element;
            ShapeElement element_cur_rev = element_cur.reverse();
            const BooleanOperationArc& arc = graph.arcs[element_cur_pos];
            const BooleanOperationNode& node = graph.nodes[arc.end_node_id];
            face.elements.push_back(element_cur);
            element_is_processed[element_cur_pos] = 1;
            if (splitted_element_cur.original_direction)
                is_inside[splitted_element_cur.orig_shape_id] = 1;
            if (element_cur.orientation == ShapeElementOrientation::Full)
                break;

            // Find the next element with the smallest angle.
            //std::cout
            //    << "element_cur_pos " << element_cur_pos
            //    << " " << element_cur.to_string()
            //    << " node_id " << arc.end_node_id << " / " << graph.nodes.size()
            //    << " original_direction " << splitted_element_cur.original_direction
            //    << std::endl;
            ElementPos smallest_angle_element_pos = -1;
            Angle smallest_angle = 0.0;

            Point direction_cur = {0, 0};
            switch (element_cur.type) {
            case ShapeElementType::LineSegment: {
                direction_cur = element_cur.end - element_cur.start;
                break;
            } case ShapeElementType::CircularArc: {
                if (element_cur.orientation == ShapeElementOrientation::Anticlockwise) {
                    direction_cur = {
                        element_cur.center.y - element_cur.end.y,
                        element_cur.end.x - element_cur.center.x};
                } else {
                    direction_cur = {
                        element_cur.end.y - element_cur.center.y,
                        element_cur.center.x - element_cur.end.x};
                }
                break;
            }
            }
            direction_cur = {-direction_cur.x, -direction_cur.y};

            for (ElementPos element_pos_next: node.successors) {
                const SplittedElement& splitted_element_next = splitted_elements[element_pos_next];
                const ShapeElement& element_next = splitted_element_next.element;
                if (element_next == element_cur_rev)
                    continue;

                Point direction_next = {0, 0};
                switch (element_next.type) {
                case ShapeElementType::LineSegment: {
                    direction_next = element_next.end - element_next.start;
                    break;
                } case ShapeElementType::CircularArc: {
                    if (element_next.orientation == ShapeElementOrientation::Anticlockwise) {
                        direction_next = {
                            element_next.center.y - element_next.start.y,
                            element_next.start.x - element_next.center.x};
                    } else {
                        direction_next = {
                            element_next.start.y - element_next.center.y,
                            element_next.center.x - element_next.start.x};
                    }
                    break;
                }
                }

                Angle angle = angle_radian(
                        direction_next,
                        direction_cur);
                //std::cout << "* element_pos_next " << element_pos_next
                //    << " " << element_next.to_string()
                //    << " angle " << angle
                //    << std::endl;
                bool update = false;
                if (smallest_angle_element_pos == -1) {
                    update = true;
                } else if (strictly_greater(smallest_angle, angle)) {
                    //std::cout << "strictly_lesser" << std::endl;
                    update = true;
                } else if (equal(smallest_angle, angle)) {
                    const SplittedElement& smallest_angle_splitted_element = splitted_elements[smallest_angle_element_pos];
                    const ShapeElement& smallest_angle_element = smallest_angle_splitted_element.element;
                    if (smallest_angle_element.type == ShapeElementType::CircularArc
                            && smallest_angle_element.orientation == ShapeElementOrientation::Anticlockwise) {
                        if (element_next.type == ShapeElementType::LineSegment) {
                            update = true;
                        } else if (element_next.type == ShapeElementType::CircularArc
                                && element_next.orientation == ShapeElementOrientation::Clockwise) {
                            update = true;
                        }
                    } else if (smallest_angle_element.type == ShapeElementType::LineSegment) {
                        if (element_next.type == ShapeElementType::CircularArc
                                && element_next.orientation == ShapeElementOrientation::Clockwise) {
                            update = true;
                        }
                    }
                }
                //std::cout << "update " << update << std::endl;
                if (update) {
                    smallest_angle_element_pos = element_pos_next;
                    smallest_angle = angle;
                }
            }
            //std::cout << "smallest_angle_element_pos " << smallest_angle_element_pos << std::endl;
            if (smallest_angle_element_pos == -1) {
                throw std::logic_error(
                        FUNC_SIGNATURE + ": "
                        "smallest_angle_element_pos is '-1'");
            }

            // Update current element.
            element_cur_pos = smallest_angle_element_pos;

            // Check if hole is finished.
            if (element_is_processed[element_cur_pos])
                break;
        }

        //std::cout << "face finished size " << face.elements.size() << std::endl;
        //std::cout << face.to_string(0) << std::endl;
        switch (boolean_operation) {
        case BooleanOperation::Union: {
            // Fast check.
            bool ok = false;
            for (ShapePos shape_pos = 0;
                    shape_pos < (ShapePos)shapes.size();
                    ++shape_pos) {
                if (is_inside[shape_pos]) {
                    ok = true;
                    break;
                }
            }
            //std::cout << "ok " << ok << std::endl;
            if (ok)
                break;

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(face, true);
            //std::cout << "intersection_output.shape_ids.size() " << intersection_output.shape_ids.size() << std::endl;
            if (intersection_output.shape_ids.empty()) {
                //std::cout << "add hole" << std::endl;
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes[0].holes.push_back(face);
            }

            break;
        } case BooleanOperation::Intersection: {
            // Fast check.
            bool ok = true;
            for (ShapePos shape_pos = 0;
                    shape_pos < (ShapePos)shapes.size();
                    ++shape_pos) {
                if (!is_inside[shape_pos]) {
                    ok = false;
                    break;
                }
            }
            //std::cout << "ok " << ok << std::endl;
            if (ok) {
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes.push_back({face});
                break;
            }

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(face, true);
            if (intersection_output.shape_ids.size() == shapes.size()) {
                //std::cout << "add face" << std::endl;
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes.push_back({face});
            }

            break;
        } case BooleanOperation::Difference: {
            // Fast check.
            bool ok = true;
            for (ShapePos shape_pos = 1;
                    shape_pos < (ShapePos)shapes.size();
                    ++shape_pos) {
                if (is_inside[shape_pos]) {
                    ok = false;
                    break;
                }
            }
            //std::cout << "ok " << ok << std::endl;
            if (!ok)
                break;

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(face, true);
            if (intersection_output.shape_ids.size() == 1
                    && intersection_output.shape_ids[0] == 0) {
                //std::cout << "add face" << std::endl;
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes.push_back({face});
            }

            break;
        } case BooleanOperation::SymmetricDifference: {
            // Fast check.
            if (is_inside[0] && is_inside[1])
                break;

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(face, true);
            if (intersection_output.shape_ids.size() == 1) {
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes.push_back({face});
            }

            break;
        }
        }
    }

    return new_shapes;
}

std::vector<ShapeWithHoles> compute_boolean_operation(
        const std::vector<ShapeWithHoles>& shapes,
        BooleanOperation boolean_operation)
{
    //std::cout << "compute_boolean_operation " << (int)boolean_operation << " shapes.size() " << shapes.size() << std::endl;
    //if (boolean_operation == BooleanOperation::Difference)
    //    write_json(shapes, {}, "boolean_operation_input.json");
    //for (const ShapeWithHoles& shape: shapes)
    //    std::cout << shape.to_string(2) << std::endl;

    std::vector<ShapeWithHoles> output;

    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const ShapeWithHoles& shape = shapes[shape_pos];
        if (shape.shape.elements.empty())
            throw std::invalid_argument(FUNC_SIGNATURE);
        for (const Shape& hole: shape.holes)
            if (hole.elements.empty())
                throw std::invalid_argument(FUNC_SIGNATURE);
    }

    // Union
    // - outline
    // - faces that are in no shapes as holes
    // Intersection
    // - no outline
    // - faces that are in all shapes are shapes
    // - holes?
    // Difference
    // - no outline
    // - faces that are in the first shape and not in other shapes are shapes
    // Symmetric difference
    // - no outline
    // - faces that are in exactly a single shape are shapes

    // Compute intersections and update the connected component of each input
    // shape.
    ComputeSplittedElementsOutput cse_output = compute_splitted_elements(shapes);

    if (boolean_operation == BooleanOperation::Intersection
            || boolean_operation == BooleanOperation::SymmetricDifference) {
        if (cse_output.shape_component_ids.number_of_values() > 1)
            return output;
    }

    // For each connected component.
    for (ComponentId component_id = 0;
            component_id < (ShapePos)shapes.size();
            ++component_id) {
        if (cse_output.shape_component_ids.number_of_elements(component_id) == 0)
            continue;
        if (boolean_operation == BooleanOperation::Difference) {
            if (cse_output.shape_component_ids[0] != component_id)
                continue;
        }
        // Compute the union of the shapes from this component.
        //std::cout << "component_id " << component_id
        //    << " " << cse_output.shape_component_ids.number_of_elements(component_id)
        //    << std::endl;
        std::vector<ShapeWithHoles> new_shapes = compute_boolean_operation_component(
                shapes,
                cse_output.components_splitted_elements[component_id],
                boolean_operation);
        if (boolean_operation != BooleanOperation::Union) {
            //write_json(new_shapes, {}, "union_input.json");
            new_shapes = compute_union(new_shapes);
        }
        for (const ShapeWithHoles& new_shape: new_shapes)
            output.push_back(new_shape);
    }

    //write_json(output, {}, "output.json");
    return output;
}

}

std::vector<ShapeWithHoles> shape::compute_union(
        const std::vector<ShapeWithHoles>& shapes)
{
    // Write input to json for tests.
    //{
    //    std::string file_path = "union_input.json";
    //    std::ofstream file{file_path};
    //    nlohmann::json json;
    //    for (ShapePos shape_pos = 0;
    //            shape_pos < (ShapePos)shapes.size();
    //            ++shape_pos) {
    //        json["shapes"][shape_pos] = shapes[shape_pos].shape.to_json();
    //    }
    //    file << std::setw(4) << json << std::endl;
    //}

    return compute_boolean_operation(
            shapes,
            BooleanOperation::Union);
}

std::vector<ShapeWithHoles> shape::compute_intersection(
        const std::vector<ShapeWithHoles>& shapes)
{
    return compute_boolean_operation(
            shapes,
            BooleanOperation::Intersection);
}

std::vector<ShapeWithHoles> shape::compute_difference(
        const ShapeWithHoles& shape,
        const std::vector<ShapeWithHoles>& shapes)
{
    std::vector<ShapeWithHoles> v;
    v.push_back(shape);
    for (const ShapeWithHoles& shape: shapes)
        v.push_back(shape);
    return compute_boolean_operation(
            v,
            BooleanOperation::Difference);
}

std::vector<ShapeWithHoles> shape::compute_symmetric_difference(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2)
{
    std::vector<ShapeWithHoles> v;
    v.push_back(shape_1);
    v.push_back(shape_2);
    return compute_boolean_operation(
            v,
            BooleanOperation::SymmetricDifference);
}
