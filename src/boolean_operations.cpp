#include "shape/boolean_operations.hpp"

#include "shape/equalize.hpp"
#include "shape/intersection_tree.hpp"
#include "shape/clean.hpp"
//#include "shape/writer.hpp"

#include "optimizationtools/containers/doubly_indexed_map.hpp"

//#include <iostream>
#include <fstream>

using namespace shape;

namespace
{

enum class BooleanOperation
{
    Union,
    Intersection,
    Difference,
    SymmetricDifference,
    FaceExtraction,
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
    if (element_1.element.type != element_2.element.type)
        return element_1.element.type < element_2.element.type;
    if (element_1.element.start.x != element_2.element.start.x)
        return element_1.element.start.x < element_2.element.start.x;
    if (element_1.element.start.y != element_2.element.start.y)
        return element_1.element.start.y < element_2.element.start.y;
    if (element_1.element.end.x != element_2.element.end.x)
        return element_1.element.end.x < element_2.element.end.x;
    if (element_1.element.end.y != element_2.element.end.y)
        return element_1.element.end.y < element_2.element.end.y;
    if (element_1.element.center.x != element_2.element.center.x)
        return element_1.element.center.x < element_2.element.center.x;
    if (element_1.element.center.y != element_2.element.center.y)
        return element_1.element.center.y < element_2.element.center.y;
    if (element_1.element.orientation != element_2.element.orientation)
        return element_1.element.orientation < element_2.element.orientation;
    return false;
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
        const std::vector<ShapeWithHoles>& shapes,
        BooleanOperation boolean_operation)
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
        const ShapeWithHoles& shape = shapes[shape_pos];
        for (ShapePos hole_pos = 0;
                hole_pos <= shape.holes.size();
                ++hole_pos) {
            const Shape& hole = (hole_pos == (ShapePos)shape.holes.size())?
                shape.shape:
                shape.holes[hole_pos];
            for (ElementPos element_pos = 0;
                    element_pos < (ElementPos)hole.elements.size();
                    ++element_pos) {
                ShapeElement element = hole.elements[element_pos];
                if (element.type == shape::ShapeElementType::CircularArc
                        && !(element.start == element.end)) {
                    element.center = element.recompute_center();
                }
                if (hole_pos == (ShapePos)shape.holes.size()) {
                    elements.push_back(element);
                } else {
                    elements.push_back(element.reverse());
                }
                ElementToSplit element_info;
                element_info.orig_shape_id = shape_pos;
                elements_info.push_back(element_info);
            }
        }
        std::vector<ShapeElement> bridges = find_holes_bridges(shape);
        for (const ShapeElement& element: bridges) {
            {
                elements.push_back(element);
                ElementToSplit element_info;
                element_info.orig_shape_id = shape_pos;
                elements_info.push_back(element_info);
            }
            {
                elements.push_back(element.reverse());
                ElementToSplit element_info;
                element_info.orig_shape_id = shape_pos;
                elements_info.push_back(element_info);
            }
        }
    }
    //std::cout << "elements" << std::endl;
    //for (const ShapeElement& element: elements)
    //    std::cout << element.to_string() << std::endl;
    //std::cout << "elements end" << std::endl;

    IntersectionTree intersection_tree({}, elements, {});
    std::vector<ElementElementIntersection> intersections
        = intersection_tree.compute_intersecting_elements(false);
    std::vector<std::vector<Point>> elements_intersections(elements.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        output.shape_component_ids.set(shape_pos, shape_pos);
    }
    for (const ElementElementIntersection& intersection: intersections) {
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
        for (const ShapeElement& overlapping_part: intersection.intersections.overlapping_parts) {
            elements_intersections[intersection.element_id_1].push_back(overlapping_part.start);
            elements_intersections[intersection.element_id_1].push_back(overlapping_part.end);
            elements_intersections[intersection.element_id_2].push_back(overlapping_part.start);
            elements_intersections[intersection.element_id_2].push_back(overlapping_part.end);
        }
        for (const Point& point: intersection.intersections.improper_intersections) {
            elements_intersections[intersection.element_id_1].push_back(point);
            elements_intersections[intersection.element_id_2].push_back(point);
        }
        for (const Point& point: intersection.intersections.proper_intersections) {
            elements_intersections[intersection.element_id_1].push_back(point);
            elements_intersections[intersection.element_id_2].push_back(point);
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
        for (Point& intersection: elements_intersections[element_pos]) {
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
    optimizationtools::IndexedSet intersecting_compooents(shapes.size());
    for (ComponentId component_id = 0;
            component_id < (ComponentId)shapes.size();
            ++component_id) {
        if (output.shape_component_ids.number_of_elements(component_id) == 0)
            continue;

        //std::cout << "component_id " << component_id << std::endl;
        //for (auto it_shape = output.shape_component_ids.begin(component_id);
        //        it_shape != output.shape_component_ids.end(component_id);
        //        ++it_shape) {
        //    std::cout << " " << *it_shape;
        //}
        //std::cout << std::endl;

        // Check if it is inside another component.
        IntersectionTree::IntersectOutput it_output = intersection_tree_2.intersect(
                shapes[*(output.shape_component_ids.begin(component_id))].find_point_strictly_inside(),
                false);
        intersecting_compooents.clear();
        for (ShapePos shape_id: it_output.shape_ids) {
            if (!output.shape_component_ids.contains(shape_id))
                continue;
            ComponentId component_2_id = output.shape_component_ids[shape_id];
            //std::cout << "shape_id " << shape_id
            //    << " component_2_id " << component_2_id
            //    << std::endl;
            if (component_2_id != component_id)
                intersecting_compooents.add(component_2_id);
        }
        for (ComponentId component_2_id: intersecting_compooents) {
            // Find a point on the outline of the component.
            Point point = shapes[*(output.shape_component_ids.begin(component_2_id))].shape.elements.front().start;
            // Check if the point is contained inside the first component.
            IntersectionTree::IntersectOutput it_output_2 = intersection_tree_2.intersect(point, false);
            bool contained = false;
            for (ShapePos shape_id: it_output_2.shape_ids) {
                ComponentId component_3_id = output.shape_component_ids[shape_id];
                if (component_3_id == component_id)
                    contained = true;
            }
            // If the point is contained, then the first component contains the
            // second one.
            // We do nothing here since the first component might contain the
            // second one while we don't get here, since the point drawn inside
            // the first component could not be inside the second component in
            // this case.
            if (contained)
                continue;
            // The first component is contained inside the second component.
            // - union: remove first component
            // - intersection: remove second component
            // - difference: replace second component by first and remove first
            // - symmetric difference: bridge the two components
            switch (boolean_operation) {
            case BooleanOperation::Union: {
                while (output.shape_component_ids.number_of_elements(component_id) > 0) {
                    output.shape_component_ids.set(
                            *output.shape_component_ids.begin(component_id),
                            shapes.size());
                }
                break;
            } case BooleanOperation::Intersection: {
                while (output.shape_component_ids.number_of_elements(component_2_id) > 0) {
                    output.shape_component_ids.set(
                            *output.shape_component_ids.begin(component_2_id),
                            shapes.size());
                }
                break;
            } case BooleanOperation::Difference: {
                throw std::logic_error(
                        FUNC_SIGNATURE + ": "
                        "not implemented.");
                break;
            } case BooleanOperation::SymmetricDifference: {
                throw std::logic_error(
                        FUNC_SIGNATURE + ": "
                        "not implemented.");
                break;
            } case BooleanOperation::FaceExtraction: {
                throw std::logic_error(
                        FUNC_SIGNATURE + ": "
                        "not implemented.");
                break;
            }
            }
        }
    }

    // Build splitted elements.
    output.components_splitted_elements = std::vector<std::vector<SplittedElement>>(shapes.size());
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)elements.size();
            ++element_pos) {
        ShapeElement element = elements_tmp[element_pos];
        ShapePos shape_pos = elements_info[element_pos].orig_shape_id;
        if (!output.shape_component_ids.contains(shape_pos))
            continue;
        ComponentId component_id = output.shape_component_ids[shape_pos];
        //std::cout << "element_pos " << element_pos
        //    << " " << element.to_string()
        //    << " shape_pos " << shape_pos
        //    << " component_id " << component_id
        //    << std::endl;
        // Sort intersection points of this element.
        std::sort(
                elements_intersections[element_pos].begin(),
                elements_intersections[element_pos].end(),
                [&element](
                    const Point& point_1,
                    const Point& point_2)
                {
                    return element.length(point_1) < element.length(point_2);
                });
        // Create new elements.
        if (element.type == ShapeElementType::CircularArc
                && element.orientation == ShapeElementOrientation::Full
                && !elements_intersections[element_pos].empty()) {
            element.start = elements_intersections[element_pos].front();
            element.end = elements_intersections[element_pos].front();
        }

        bool first = true;
        for (const Point& point_cur: elements_intersections[element_pos]) {
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

                if (p.first.type == shape::ShapeElementType::CircularArc
                        && p.first.contains((p.first.start + p.first.end) / 2)) {
                    //std::cout << "change to line segment" << std::endl;
                    p.first.type = shape::ShapeElementType::LineSegment;
                }

                SplittedElement new_element;
                new_element.element = p.first;
                new_element.orig_shape_id = shape_pos;
                new_element.original_direction = true;
                output.components_splitted_elements[component_id].push_back(new_element);

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

            //std::cout << "  - " << new_element.element.to_string() << std::endl;
            //std::cout << "    length " << new_element.element.length() << std::endl;
        }
    }

    // Reverse some elements to help detecting duplicates.
    for (auto it = output.shape_component_ids.values_begin();
            it != output.shape_component_ids.values_end();
            ++it) {
        ComponentId component_id = *it;
        auto& splitted_elements = output.components_splitted_elements[component_id];
        for (SplittedElement& splitted_element: splitted_elements) {
            if (splitted_element.element.start.x < splitted_element.element.end.x)
                continue;
            if (splitted_element.element.start.x == splitted_element.element.end.x
                    && splitted_element.element.start.y < splitted_element.element.end.y) {
                continue;
            }
            splitted_element.element = splitted_element.element.reverse();
            splitted_element.original_direction = !splitted_element.original_direction;
        }
    }

    // Remove duplicates in splitted elements.
    for (auto it = output.shape_component_ids.values_begin();
            it != output.shape_component_ids.values_end();
            ++it) {
        ComponentId component_id = *it;
        auto& splitted_elements = output.components_splitted_elements[component_id];
        std::sort(splitted_elements.begin(), splitted_elements.end());
        splitted_elements.erase(unique(splitted_elements.begin(), splitted_elements.end()), splitted_elements.end());
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

    ElementPos reverse_arc_id = -1;
};

struct BooleanOperationNode
{
    std::vector<ElementPos> predecessors;

    std::vector<ElementPos> successors;
};

struct BooleanOperationGraph
{
    std::vector<BooleanOperationArc> arcs;

    std::vector<BooleanOperationNode> nodes;
};

BooleanOperationGraph compute_graph(
        std::vector<SplittedElement>& splitted_elements)
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
            graph.nodes[node_id].predecessors.push_back(element_pos);
        }
        point_prev = point;
    }

    // Add reverse arcs.
    ElementPos initial_size = graph.arcs.size();
    for (ElementPos arc_id = 0; arc_id < initial_size; ++arc_id) {
        BooleanOperationArc& arc = graph.arcs[arc_id];
        const SplittedElement& splitted_element = splitted_elements[arc_id];
        ElementPos arc_reversed_id = graph.arcs.size();
        BooleanOperationArc arc_reversed;
        arc_reversed.source_node_id = arc.end_node_id;
        arc_reversed.end_node_id = arc.source_node_id;
        arc_reversed.reverse_arc_id = arc_id;
        arc.reverse_arc_id = arc_reversed_id;
        SplittedElement splitted_element_reversed;
        splitted_element_reversed.element = splitted_element.element.reverse();
        splitted_element_reversed.orig_shape_id = splitted_element.orig_shape_id;
        splitted_element_reversed.original_direction = !splitted_element.original_direction;
        splitted_elements.push_back(splitted_element_reversed);
        graph.arcs.push_back(arc_reversed);
        graph.nodes[arc_reversed.source_node_id].successors.push_back(arc_reversed_id);
    }

    //std::cout << "compute_graph end" << std::endl;
    return graph;
}

std::vector<ShapeWithHoles> compute_boolean_operation_component(
        const std::vector<ShapeWithHoles>& shapes,
        std::vector<SplittedElement>& splitted_elements,
        BooleanOperation boolean_operation)
{
    //std::cout << "compute_boolean_operation_component" << std::endl;
    //for (const SplittedElement& splitted_element: splitted_elements)
    //    std::cout << splitted_element.element.to_string() << std::endl;
    //Writer().add_shapes_with_holes(shapes).write_json("compute_boolean_operation_component_input.json");
    //Writer writer;
    //for (const auto& splitted_element: splitted_elements)
    //    writer.add_element(splitted_element.element);
    //writer.write_json("overlay.json");

    std::vector<ShapeWithHoles> new_shapes;
    BooleanOperationGraph graph = compute_graph(splitted_elements);

    std::vector<ElementPos> arcs_next(graph.arcs.size(), -1);
    for (NodeId node_id = 0; node_id < (NodeId)graph.nodes.size(); ++node_id) {
        BooleanOperationNode& node = graph.nodes[node_id];
        LengthDbl l = std::numeric_limits<LengthDbl>::infinity();
        for (ElementPos arc_id: node.successors) {
            const SplittedElement& splitted_element = splitted_elements[arc_id];
            l = (std::min)(l, splitted_element.element.length());
        }
        // Sort the arcs at this node.
        std::sort(
                node.successors.begin(),
                node.successors.end(),
                [&l, &splitted_elements](
                    ElementPos arc_1_id,
                    ElementPos arc_2_id)
                {
                    const SplittedElement& splitted_element_1 = splitted_elements[arc_1_id];
                    const SplittedElement& splitted_element_2 = splitted_elements[arc_2_id];
                    //std::cout << "element_1 " << splitted_element_1.element.to_string() << std::endl;
                    //std::cout << "element_2 " << splitted_element_2.element.to_string() << std::endl;
                    Point p1 = splitted_element_1.element.point(l);
                    Point p2 = splitted_element_2.element.point(l);
                    //std::cout << "p1 " << p1.to_string() << std::endl;
                    //std::cout << "p2 " << p2.to_string() << std::endl;
                    p1 = p1 - splitted_element_1.element.start;
                    p2 = p2 - splitted_element_2.element.start;
                    if (p1.y >= 0 && p2.y < 0) {
                        //std::cout << "return " << "true" << std::endl;
                        return true;
                    }
                    if (p1.y < 0 && p2.y >= 0) {
                        //std::cout << "return " << "false" << std::endl;
                        return false;
                    }
                    if (p1.y == 0 && p2.y == 0) {
                        //std::cout << "return " << (p1.x < p2.x) << std::endl;
                        return p1.x > p2.x;
                    }
                    LengthDbl v = p1.x * p2.y - p1.y * p2.x;
                    //std::cout << "p1 " << p1.to_string() << std::endl;
                    //std::cout << "p2 " << p2.to_string() << std::endl;
                    //std::cout << "v " << v << std::endl;
                    //std::cout << "return " << (v > 0) << std::endl;
                    return v > 0;
                });
        // Update arcs_next.
        ElementPos arc_prev_id = node.successors.back();
        //std::cout << "node_id " << node_id << std::endl;
        for (ElementPos arc_id: node.successors) {
            const BooleanOperationArc& arc = graph.arcs[arc_id];
            arcs_next[arc.reverse_arc_id] = arc_prev_id;
            //std::cout << arc_id
            //    << " " << splitted_elements[arc_id].element.to_string() << std::endl;
            //std::cout << arc.reverse_arc_id << " -> " << arc_prev_id << std::endl;
            //std::cout << arc.reverse_arc_id
            //    << " " << splitted_elements[arc.reverse_arc_id].element.to_string() << std::endl;
            //std::cout << arc_prev_id
            //    << " " << splitted_elements[arc_prev_id].element.to_string() << std::endl;
            arc_prev_id = arc_id;
        }
    }

    //if (boolean_operation == BooleanOperation::Difference) {
    //    std::vector<ShapeElement> elements;
    //    Writer writer;
    //    for (const auto& splitted_element: splitted_elements)
    //        writer.add_element(splitted_element.element);
    //    writer.write_json("overlay.json");
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
        auto p = element.element.furthest_points(90);
        //std::cout << "element_pos " << element_pos << std::endl
        //    << "    " << element.element.to_string() << std::endl
        //    << "    p " << p.first.to_string() << " " << p.second.to_string() << std::endl;
        if (element.element.contains(p_min))
            leftest_elements_pos.push_back(element_pos);
        if (strictly_lesser(p.first.x, p_min.x)
                || (equal(p.first.x, p_min.x) && strictly_lesser(p.first.y, p_min.y))) {
            leftest_elements_pos = {element_pos};
            p_min = p.first;
        }
        if (strictly_lesser(p.second.x, p_min.x)
                || (equal(p.second.x, p_min.x) && strictly_lesser(p.second.y, p_min.y))) {
            leftest_elements_pos = {element_pos};
            p_min = p.second;
        }
    }
    //std::cout << "leftest_elements_pos.size() " << leftest_elements_pos.size() << std::endl;

    if (leftest_elements_pos.size() > 1) {
        std::vector<ElementPos> leftest_elements_pos_tmp;
        for (ElementPos element_pos: leftest_elements_pos) {
            const SplittedElement& element = splitted_elements[element_pos];
            //std::cout << element.element.to_string() << std::endl;
            if (element.element.end == p_min)
                continue;
            leftest_elements_pos_tmp.push_back(element_pos);
        }
        leftest_elements_pos = leftest_elements_pos_tmp;
    }
    //std::cout << "leftest_elements_pos.size() " << leftest_elements_pos.size() << std::endl;

    ElementPos element_start_pos = -1;
    Jet largest_jet;
    ShapeElement ref = build_line_segment(p_min, p_min + Point{-1, 0});
    Jet current_jet = ref.jet(p_min, false);
    for (ElementPos element_pos: leftest_elements_pos) {
        const SplittedElement& splitted_element = splitted_elements[element_pos];
        const ShapeElement& element = splitted_element.element;
        Jet jet = element.jet(p_min, false) - current_jet;
        //std::cout << "element_pos " << element_pos << std::endl
        //    << "    " << element.to_string() << std::endl
        //    << "    jet " << jet.to_string() << std::endl;
        if (element_start_pos == -1
                || largest_jet < jet) {
            element_start_pos = element_pos;
            largest_jet = jet;
        }
    }
    //std::cout << "element_start_pos " << element_start_pos << std::endl
    //    << "    " << splitted_elements[element_start_pos].element.to_string() << std::endl;
    if (element_start_pos == -1) {
        throw std::logic_error(
                FUNC_SIGNATURE + ": element_start_pos is '-1'.");
    }

    // Find outer loop.
    //std::cout << "find outer loop..." << std::endl;
    std::vector<uint8_t> element_is_processed(splitted_elements.size(), 0);
    ElementPos element_cur_pos = element_start_pos;
    Shape outline;
    while (!element_is_processed[element_cur_pos]) {
        const ShapeElement& element_cur = splitted_elements[element_cur_pos].element;
        //std::cout << "element_cur " << element_cur_pos
        //    << " " << element_cur.to_string() << std::endl;
        element_is_processed[element_cur_pos] = 1;
        outline.elements.push_back(element_cur);
        if (element_cur.orientation == ShapeElementOrientation::Full)
            break;
        element_cur_pos = arcs_next[element_cur_pos];
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
    } case BooleanOperation::FaceExtraction: {
        outline = remove_redundant_vertices(outline).second;
        outline = remove_aligned_vertices(outline).second;
        if (outline.compute_area() > 0)
            new_shapes.push_back({outline});
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
        while (!element_is_processed[element_cur_pos]) {
            const SplittedElement& splitted_element_cur = splitted_elements[element_cur_pos];
            const ShapeElement& element_cur = splitted_element_cur.element;
            //std::cout << "element_cur " << element_cur_pos
            //    << " " << element_cur.to_string() << std::endl;
            face.elements.push_back(element_cur);
            element_is_processed[element_cur_pos] = 1;
            if (splitted_element_cur.original_direction)
                is_inside[splitted_element_cur.orig_shape_id] = 1;
            if (element_cur.orientation == ShapeElementOrientation::Full)
                break;
            element_cur_pos = arcs_next[element_cur_pos];
        }

        //std::cout << "face finished size " << face.elements.size() << std::endl;
        //std::cout << face.to_string(0) << std::endl;

        // Check if the face is valid.
        if (!equal(face.elements.back().end, face.elements.front().start)) {
            //std::cout << face.to_string(0) << std::endl;
            //Writer writer;
            //for (const auto& splitted_element: splitted_elements)
            //    writer.add_element(splitted_element.element);
            //writer.write_json("overlay.json");
            //Writer().add_shapes_with_holes(shapes).write_json("shape.json");
            //compute_union_export_inputs(
            //        "compute_union_inputs.json",
            //        shapes);
            throw std::logic_error(
                    FUNC_SIGNATURE + ": "
                    "face is not closed.");
        }

        // Check if the face is valid.
        if (strictly_lesser(face.compute_area(), 0.0)) {
            //std::cout << face.to_string(0) << std::endl;
            //std::vector<ShapeElement> elements;
            //Writer writer;
            //for (const auto& splitted_element: splitted_elements)
            //    writer.add_element(splitted_element.element);
            //writer.write_json("overlay.json");
            //Writer().add_shapes_with_holes(shapes).write_json("shape.json");
            throw std::logic_error(
                    FUNC_SIGNATURE + ": "
                    "face area is not positive.");
        }

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

            // Fix backtracks caused by numerical issues.
            face = remove_backtracks(face);

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(
                    face.find_point_strictly_inside(),
                    false);
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
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(
                    face.find_point_strictly_inside(),
                    false);
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

            //std::cout << face.find_point_strictly_inside().to_string() << std::endl;

            // Real check.
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(
                    face.find_point_strictly_inside(),
                    false);
            //std::cout << "intersection_output.shape_ids.size() " << intersection_output.shape_ids.size() << std::endl;
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
            IntersectionTree::IntersectOutput intersection_output = intersection_tree.intersect(
                    face.find_point_strictly_inside(),
                    false);
            if (intersection_output.shape_ids.size() == 1) {
                face = remove_redundant_vertices(face).second;
                face = remove_aligned_vertices(face).second;
                new_shapes.push_back({face});
            }

            break;
        } case BooleanOperation::FaceExtraction: {
            face = remove_redundant_vertices(face).second;
            face = remove_aligned_vertices(face).second;
            if (face.compute_area() > 0)
                new_shapes.push_back({face});
            break;
        }
        }
    }

    if (boolean_operation == BooleanOperation::Union)
        new_shapes = fix_self_intersections(new_shapes[0]);

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
    ComputeSplittedElementsOutput cse_output = compute_splitted_elements(shapes, boolean_operation);

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
    //std::cout << "compute_union " << shapes.size() << std::endl;
    //compute_union_export_inputs(
    //        "compute_union_inputs.json",
    //        shapes);

    return compute_boolean_operation(
            shapes,
            BooleanOperation::Union);
}

void shape::compute_union_export_inputs(
        const std::string& file_path,
        const std::vector<ShapeWithHoles>& shapes)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        json["shapes"][shape_pos] = shapes[shape_pos].to_json();
    }
    file << std::setw(4) << json << std::endl;
}

std::vector<ShapeWithHoles> shape::compute_intersection(
        const std::vector<ShapeWithHoles>& shapes)
{
    std::vector<ShapeWithHoles> faces = compute_boolean_operation(
            shapes,
            BooleanOperation::Intersection);
    return compute_union(faces);
}

std::vector<ShapeWithHoles> shape::compute_difference(
        const ShapeWithHoles& shape,
        const std::vector<ShapeWithHoles>& shapes)
{
    std::vector<ShapeWithHoles> v;
    v.push_back(shape);
    for (const ShapeWithHoles& shape: shapes)
        v.push_back(shape);
    std::vector<ShapeWithHoles> faces = compute_boolean_operation(
            v,
            BooleanOperation::Difference);
    return compute_union(faces);
}

std::vector<ShapeWithHoles> shape::compute_symmetric_difference(
        const ShapeWithHoles& shape_1,
        const ShapeWithHoles& shape_2)
{
    std::vector<ShapeWithHoles> v;
    v.push_back(shape_1);
    v.push_back(shape_2);
    std::vector<ShapeWithHoles> faces = compute_boolean_operation(
            v,
            BooleanOperation::SymmetricDifference);
    return compute_union(faces);
}

std::vector<Shape> shape::extract_faces(
        const Shape& shape)
{
    auto output = compute_boolean_operation(
            {{shape}},
            BooleanOperation::FaceExtraction);
    std::vector<Shape> faces;
    for (const ShapeWithHoles& shape: output)
        faces.push_back(shape.shape);
    return faces;
}

struct HoleGraph
{
    std::vector<std::vector<ShapePos>> components;
    ShapePos outline_component = -1;
    std::vector<ShapePos> holes_components;
};

HoleGraph compute_hole_graph(
        const ShapeWithHoles& shape)
{
    std::vector<ShapeWithHoles> intersection_tree_shapes;
    for (const Shape& hole: shape.holes)
        intersection_tree_shapes.push_back({hole});
    IntersectionTree intersection_tree({intersection_tree_shapes}, {}, {});
    std::vector<std::pair<ShapePos, ShapePos>> touching_shapes
        = intersection_tree.compute_intersecting_shapes(false);

    // Identify connected holes.
    std::vector<std::vector<ShapePos>> graph(shape.holes.size() + 1);
    for (auto p: touching_shapes) {
        //std::cout << "edge " << p.first << " " << p.second << std::endl;
        graph[p.first].push_back(p.second);
        graph[p.second].push_back(p.first);
    }

    IntersectionTree intersection_tree_2({}, {shape.shape.elements}, {});
    for (ShapePos hole_pos = 0;
            hole_pos < (ShapePos)shape.holes.size();
            ++hole_pos) {
        const Shape& hole = shape.holes[hole_pos];
        IntersectionTree::IntersectOutput intersections = intersection_tree_2.intersect(hole, false);
        if (intersections.element_ids.empty())
            continue;
        graph[shape.holes.size()].push_back(hole_pos);
        graph[hole_pos].push_back(shape.holes.size());
    }

    // Find connected components.
    HoleGraph output;
    output.holes_components = std::vector<ShapePos>(shape.holes.size() + 1, -1);
    ShapePos hole_pos = 0;
    for (;;) {
        while (hole_pos <= shape.holes.size()
                && output.holes_components[hole_pos] != -1) {
            hole_pos++;
        }
        if (hole_pos == shape.holes.size() + 1)
            break;

        ShapePos component_id = output.components.size();
        std::vector<ShapePos> component;
        bool component_contains_outline = false;
        if (hole_pos == shape.holes.size())
            component_contains_outline = true;
        output.holes_components[hole_pos] = component_id;
        //std::cout << "hole_pos " << hole_pos << std::endl;
        component.push_back(hole_pos);
        std::vector<ShapePos> stack = {hole_pos};
        while (!stack.empty()) {
            ShapePos shape_cur_pos = stack.back();
            stack.pop_back();
            for (ShapePos neighbor: graph[shape_cur_pos]) {
                if (output.holes_components[neighbor] != -1)
                    continue;
                if (neighbor == shape.holes.size())
                    component_contains_outline = true;
                stack.push_back(neighbor);
                //std::cout << "neighbor " << neighbor << std::endl;
                output.holes_components[neighbor] = component_id;
                component.push_back(neighbor);
            }
        }

        output.components.push_back(component);
        if (component_contains_outline)
            output.outline_component = component_id;
    }

    return output;
}

std::vector<ShapeElement> shape::find_holes_bridges(
        const ShapeWithHoles& shape)
{
    HoleGraph hole_graph = compute_hole_graph(shape);

    std::vector<ShapeElement> bridges;
    auto mm = shape.compute_min_max();
    for (ShapePos component_id = 0;
            component_id < (ShapePos)hole_graph.components.size();
            ++component_id) {
        //std::cout << "component_id " << component_id
        //    << " / " << components.size() << std::endl;
        if (component_id == hole_graph.outline_component)
            continue;
        const std::vector<ShapePos>& component = hole_graph.components[component_id];

        // Find the left-most point of the component.
        Point end = {mm.second.x + 1, 0};
        for (ShapePos hole_pos: component) {
            //std::cout << "hole_pos " << hole_pos << std::endl;
            const Shape& hole = shape.holes[hole_pos];
            auto furthest_points = hole.compute_furthest_points(90);
            if (strictly_greater(end.x, furthest_points.second.point.x))
                end = furthest_points.second.point;
        }

        ShapeElement ray;
        ray.type = ShapeElementType::LineSegment;
        ray.start.x = mm.first.x;
        ray.start.y = end.y;
        ray.end.x = end.x;
        ray.end.y = end.y;

        LengthDbl x_max = -std::numeric_limits<LengthDbl>::infinity();
        for (ShapePos shape_pos = 0;
                shape_pos <= (ShapePos)shape.holes.size();
                ++shape_pos) {
            if (shape_pos != (ShapePos)shape.holes.size() && hole_graph.holes_components[shape_pos] == component_id)
                continue;
            const Shape& current_shape = (shape_pos == (ShapePos)shape.holes.size())? shape.shape: shape.holes[shape_pos];
            for (ElementPos element_pos = 0;
                    element_pos < (ElementPos)current_shape.elements.size();
                    ++element_pos) {
                const ShapeElement& element = current_shape.elements[element_pos];
                ShapeElementIntersectionsOutput intersections = compute_intersections(ray, element);
                for (const ShapeElement& overlapping_part: intersections.overlapping_parts) {
                    auto x = overlapping_part.min_max().second.x;
                    if (strictly_greater(x, end.x))
                        continue;
                    if (strictly_lesser(x_max, x))
                        x_max = x;
                }
                for (const Point& intersection: intersections.improper_intersections) {
                    if (strictly_greater(intersection.x, end.x))
                        continue;
                    if (strictly_lesser(x_max, intersection.x))
                        x_max = intersection.x;
                }
                for (const Point& intersection: intersections.proper_intersections) {
                    if (strictly_greater(intersection.x, end.x))
                        continue;
                    if (strictly_lesser(x_max, intersection.x))
                        x_max = intersection.x;
                }
            }
        }
        bridges.push_back(build_line_segment({x_max, end.y}, end));
    }
    return bridges;
}

std::vector<ShapeWithHoles> shape::bridge_touching_holes(
        const ShapeWithHoles& shape)
{
    HoleGraph hole_graph = compute_hole_graph(shape);

    std::vector<ShapeWithHoles> v;
    v.push_back({shape.shape});
    for (ShapePos hole_pos: hole_graph.components[hole_graph.outline_component]) {
        if (hole_pos == shape.holes.size())
            continue;
        v.push_back({shape.holes[hole_pos]});
    }
    std::vector<ShapeWithHoles> faces = compute_boolean_operation(
            v,
            BooleanOperation::Difference);
    std::vector<ShapeWithHoles> output;
    for (const ShapeWithHoles& face: faces) {
        ShapeWithHoles shape_with_holes = face;
        for (ShapePos hole_pos = 0;
                hole_pos < (ShapePos)shape.holes.size();
                ++hole_pos) {
            if (hole_graph.holes_components[hole_pos] == hole_graph.outline_component)
                continue;
            const Shape& hole = shape.holes[hole_pos];
            if (face.contains(hole.find_point_strictly_inside(), true))
                shape_with_holes.holes.push_back(hole);
        }
        output.push_back(shape_with_holes);
    }
    return output;
}
