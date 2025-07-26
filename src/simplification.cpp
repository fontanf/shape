#include "shape/simplification.hpp"

#include "shape/boolean_operations.hpp"

#include "optimizationtools/containers/indexed_binary_heap.hpp"

//#include <iostream>

using namespace shape;

namespace
{

struct ApproximatedElementKey
{
    ShapePos shape_pos = -1;

    ElementPos element_pos = -1;
};

struct ApproximatedShapeElement
{
    ShapeElement element;

    ElementPos element_key_id;

    bool removed = false;

    ElementPos element_prev_pos = -1;

    ElementPos element_next_pos = -1;
};

struct ApproximatedShape
{
    ElementPos number_of_elements = 0;

    Point min = {-std::numeric_limits<LengthDbl>::infinity(), -std::numeric_limits<LengthDbl>::infinity()};
    Point max = {+std::numeric_limits<LengthDbl>::infinity(), +std::numeric_limits<LengthDbl>::infinity()};

    ShapePos copies = 1;

    bool outer = true;

    std::vector<ApproximatedShapeElement> elements;

    std::vector<ShapeWithHoles> union_input;

    Shape shape() const
    {
        Shape new_shape;

        //for (ElementPos element_pos = 0;
        //        element_pos < (ElementPos)elements.size();
        //        ++element_pos) {
        //    const ApproximatedShapeElement& element = elements[element_pos];
        //    std::cout << "element_pos " << element_pos
        //        << " removed " << element.removed
        //        << " element_prev_pos " << element.element_prev_pos
        //        << " element_next_pos " << element.element_next_pos
        //        << std::endl;
        //}

        ElementPos initial_element_pos = 0;
        while (this->elements[initial_element_pos].removed)
            initial_element_pos++;
        //std::cout << "initial_element_pos " << initial_element_pos
        //    << " / " << this->elements.size()
        //    << std::endl;

        ElementPos element_pos = initial_element_pos;
        for (;;) {
            //std::cout << "element_pos " << element_pos
            //    << " element_next_pos " << this->elements[element_pos].element_next_pos
            //    << " removed " << this->elements[element_pos].removed
            //    << std::endl;
            new_shape.elements.push_back(this->elements[element_pos].element);
            element_pos = this->elements[element_pos].element_next_pos;
            if (element_pos == initial_element_pos)
                break;
        }

        return new_shape;
    }
};

AreaDbl compute_approximation_cost(
        std::vector<ApproximatedShape>& approximated_shapes,
        const ApproximatedElementKey& element_key)
{
    ApproximatedShape& shape = approximated_shapes[element_key.shape_pos];
    if (element_key.element_pos >= (ElementPos)shape.elements.size()) {
        throw std::runtime_error(
                "irregular::compute_approximation_cost; "
                "element_pos: " + std::to_string(element_key.element_pos) + "; "
                "shape.elements.size(): " + std::to_string(shape.elements.size()) + ".\n");
    }
    const ApproximatedShapeElement& element = shape.elements[element_key.element_pos];
    //std::cout << "element_pos " << element_pos << " / " << shape.elements.size() << std::endl;
    //std::cout << "element_prev_pos " << element.element_prev_pos << std::endl;
    //std::cout << "element_next_pos " << element.element_next_pos << std::endl;
    const ApproximatedShapeElement& element_prev = shape.elements[element.element_prev_pos];
    const ApproximatedShapeElement& element_next = shape.elements[element.element_next_pos];

    // Compute previous and next angles.
    Angle angle_prev = angle_radian(
            element_prev.element.start - element_prev.element.end,
            element.element.end - element.element.start);
    Angle angle_next = angle_radian(
            element.element.start - element.element.end,
            element_next.element.end - element_next.element.start);

    //std::cout
    //    << "shape_pos " << element_key.shape_pos
    //    << " element_pos " << element_key.element_pos
    //    << " angle " << angle_next << std::endl;

    if (angle_prev == 0) {
        throw std::logic_error(
                "shape::simplify: angle_prev == 0.0; "
                "element: " + element.element.to_string() + "; "
                "element_nex: " + element_next.element.to_string() + ".");
    }
    if (angle_next == 0) {
        //shape.shape().write_svg("shape.svg");
        throw std::logic_error(
                "shape::simplify: angle_next == 0.0; "
                "element: " + element.element.to_string() + "; "
                "element_nex: " + element_next.element.to_string() + ".");
    }

    if (shape.outer) {
        // Outer approximation.
        if (angle_next == M_PI) {
            return 0.0;
        } else if (angle_next > M_PI) {
            if (angle_prev > M_PI) {
                // Check if the triangle is bounded.
                // Compute intersection.
                LengthDbl x1 = element_prev.element.start.x;
                LengthDbl y1 = element_prev.element.start.y;
                LengthDbl x2 = element_prev.element.end.x;
                LengthDbl y2 = element_prev.element.end.y;
                LengthDbl x3 = element_next.element.start.x;
                LengthDbl y3 = element_next.element.start.y;
                LengthDbl x4 = element_next.element.end.x;
                LengthDbl y4 = element_next.element.end.y;
                LengthDbl denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
                //std::cout << "element_prev " << element_prev.element.to_string() << std::endl;
                //std::cout << "element_next " << element_next.element.to_string() << std::endl;
                //std::cout << "denom " << denom << std::endl;
                // If no intersection, no approximation possible.
                if (denom == 0.0)
                    return std::numeric_limits<Angle>::infinity();
                LengthDbl xp = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
                LengthDbl yp = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;
                //std::cout << "xp " << xp << " yp " << yp << std::endl;

                // We don't want to increase the bounding box of the shape.
                if (strictly_lesser(xp, shape.min.x)
                        || strictly_lesser(yp, shape.min.y)
                        || strictly_greater(xp, shape.max.x)
                        || strictly_greater(yp, shape.max.y)) {
                    return std::numeric_limits<Angle>::infinity();
                }

                AreaDbl cost = shape.copies * compute_area(
                        element.element.start,
                        {xp, yp},
                        element.element.end);
                if (cost < 0)
                    return std::numeric_limits<Angle>::infinity();
                return cost;
            } else {
                // No approximation possible.
                return std::numeric_limits<Angle>::infinity();
            }
        } else {
            // angle_next < M_PI
            Angle cost = shape.copies * compute_area(
                    element.element.start,
                    element_next.element.end,
                    element.element.end);
            return cost;
        }
    } else {
        // Inner approximation.
        if (angle_next == M_PI) {
            return 0.0;
        } else if (angle_next < M_PI) {
            if (angle_prev < M_PI) {
                // Check if the triangle is bounded.
                // Compute intersection.
                LengthDbl x1 = element_prev.element.start.x;
                LengthDbl y1 = element_prev.element.start.y;
                LengthDbl x2 = element_prev.element.end.x;
                LengthDbl y2 = element_prev.element.end.y;
                LengthDbl x3 = element_next.element.start.x;
                LengthDbl y3 = element_next.element.start.y;
                LengthDbl x4 = element_next.element.end.x;
                LengthDbl y4 = element_next.element.end.y;
                LengthDbl denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
                // If no intersection, no approximation possible.
                if (denom == 0)
                    return std::numeric_limits<Angle>::infinity();
                LengthDbl xp = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
                LengthDbl yp = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;
                AreaDbl cost = shape.copies * compute_area(
                        element.element.start,
                        element.element.end,
                        {xp, yp});
                if (cost < 0)
                    return std::numeric_limits<Angle>::infinity();
                return cost;
            } else {
                // No approximation possible.
                return std::numeric_limits<Angle>::infinity();
            }
        } else {
            // angle_next < M_PI
            Angle cost = shape.copies * compute_area(
                    element.element.start,
                    element.element.end,
                    element_next.element.end);
            return cost;
        }
    }
    return 0.0;
}

void apply_approximation(
        std::vector<ApproximatedShape>& approximated_shapes,
        const ApproximatedElementKey& element_key)
{
    ApproximatedShape& shape = approximated_shapes[element_key.shape_pos];
    ApproximatedShapeElement& element = shape.elements[element_key.element_pos];
    //std::cout << "element_pos " << element_pos << " / " << shape.elements.size() << std::endl;
    //std::cout << "element_prev_pos " << element.element_prev_pos << std::endl;
    //std::cout << "element_next_pos " << element.element_next_pos << std::endl;
    ApproximatedShapeElement& element_prev = shape.elements[element.element_prev_pos];
    ApproximatedShapeElement& element_next = shape.elements[element.element_next_pos];
    //std::cout << "element_prev " << element_prev.element.to_string() << std::endl;
    //std::cout << "element_cur  " << element.element.to_string() << std::endl;
    //std::cout << "element_next " << element_next.element.to_string() << std::endl;

    // Compute previous and next angles.
    Angle angle_prev = angle_radian(
            element_prev.element.start - element_prev.element.end,
            element.element.end - element.element.start);
    Angle angle_next = angle_radian(
            element.element.start - element.element.end,
            element_next.element.end - element_next.element.start);

    if (shape.outer) {
        // Outer approximation.
        if (angle_next == M_PI) {
            element_next.element.start = element.element.start;
        } else if (angle_next > M_PI) {
            if (angle_prev > M_PI) {
                // Check if the triangle is bounded.
                // Compute intersection.
                LengthDbl x1 = element_prev.element.start.x;
                LengthDbl y1 = element_prev.element.start.y;
                LengthDbl x2 = element_prev.element.end.x;
                LengthDbl y2 = element_prev.element.end.y;
                LengthDbl x3 = element_next.element.start.x;
                LengthDbl y3 = element_next.element.start.y;
                LengthDbl x4 = element_next.element.end.x;
                LengthDbl y4 = element_next.element.end.y;
                LengthDbl denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
                if (denom == 0.0) {
                    throw std::runtime_error(
                            "irregular::apply_approximation: outer; "
                            "shape_pos: " + std::to_string(element_key.shape_pos) + "; "
                            "element_pos: " + std::to_string(element_key.element_pos) + "; "
                            "element_prev.element: " + element_prev.element.to_string() + "; "
                            "element.element: " + element.element.to_string() + "; "
                            "element_next.element: " + element_next.element.to_string() + "; "
                            "angle_prev: " + std::to_string(angle_prev) + "; "
                            "angle_next: " + std::to_string(angle_next) + "; "
                            "denom: " + std::to_string(denom) + ".");
                }
                LengthDbl xp = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
                LengthDbl yp = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;
                element_prev.element.end = {xp, yp};
                element_next.element.start = {xp, yp};
                shape.union_input.push_back({build_triangle(
                            element.element.start,
                            {xp, yp},
                            element.element.end)});
            } else {
                // No approximation possible.
                throw std::runtime_error(
                        "irregular::apply_approximation: outer; "
                        "element_prev.element: " + element_prev.element.to_string() + "; "
                        "element.element: " + element.element.to_string() + "; "
                        "element_next.element: " + element_next.element.to_string() + "; "
                        "angle_prev: " + std::to_string(angle_prev) + "; "
                        "angle_next: " + std::to_string(angle_next) + ".");
            }
        } else {
            // angle_next < M_PI
            element_next.element.start = element.element.start;
            shape.union_input.push_back({build_triangle(
                        element.element.start,
                        element_next.element.end,
                        element.element.end)});
        }
    } else {
        // Inner approximation.
        if (angle_next == M_PI) {
            element_next.element.start = element.element.start;
        } else if (angle_next < M_PI) {
            if (angle_prev < M_PI) {
                // Check if the triangle is bounded.
                // Compute intersection.
                LengthDbl x1 = element_prev.element.start.x;
                LengthDbl y1 = element_prev.element.start.y;
                LengthDbl x2 = element_prev.element.end.x;
                LengthDbl y2 = element_prev.element.end.y;
                LengthDbl x3 = element_next.element.start.x;
                LengthDbl y3 = element_next.element.start.y;
                LengthDbl x4 = element_next.element.end.x;
                LengthDbl y4 = element_next.element.end.y;
                LengthDbl denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
                if (denom == 0.0) {
                    throw std::runtime_error(
                            "irregular::apply_approximation: inner; "
                            "denom: " + std::to_string(denom) + ".");
                }
                LengthDbl xp = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
                LengthDbl yp = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;
                AreaDbl cost = compute_area(
                        element.element.start,
                        element.element.end,
                        {xp, yp});
                element_prev.element.end = {xp, yp};
                element_next.element.start = {xp, yp};
                shape.union_input.push_back({build_triangle(
                            element.element.start,
                            element.element.end,
                            {xp, yp})});
            } else {
                // No approximation possible.
                throw std::runtime_error(
                        "irregular::apply_approximation: inner; "
                        "angle_prev: " + std::to_string(angle_prev) + "; "
                        "angle_next: " + std::to_string(angle_next) + ".");
            }
        } else {
            // angle_next < M_PI
            element_next.element.start = element.element.start;
            shape.union_input.push_back({build_triangle(
                        element.element.start,
                        element.element.end,
                        element_next.element.end)});
        }
    }
    element_prev.element_next_pos = element.element_next_pos;
    element_next.element_prev_pos = element.element_prev_pos;
    element.removed = true;
    shape.number_of_elements--;
    //std::cout << "element_prev " << element_prev.element.to_string() << std::endl;
    //std::cout << "element_next " << element_next.element.to_string() << std::endl;
}

}

std::vector<ShapeWithHoles> shape::simplify(
        const std::vector<SimplifyInputShape>& shapes,
        AreaDbl maximum_approximation_area)
{
    //std::cout << "shape_simplification" << std::endl;
    //for (const SimplifyInputShape& shape: shapes)
    //    std::cout << shape.shape.to_string(2) << std::endl;

    // Check that the input doesn't contain any circular arc.
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const SimplifyInputShape& shape = shapes[shape_pos];
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)shape.shape.elements.size();
                ++element_pos) {
            const ShapeElement& element = shape.shape.elements[element_pos];
            if (element.type == ShapeElementType::CircularArc) {
                throw std::invalid_argument(
                        "shape::simplify: "
                        "all shape elements must be line segments; "
                        "shape_pos: " + std::to_string(shape_pos) + "; "
                        "element_pos: " + std::to_string(element_pos) + "; "
                        "element: " + element.to_string() + ".");
            }
        }
    }

    // Build element_keys, approximated_bin_types, approximated_item_types.
    std::vector<ApproximatedElementKey> element_keys;
    std::vector<ApproximatedShape> approximated_shapes;
    AreaDbl total_bin_area = 0.0;
    AreaDbl total_item_area = 0.0;
    ElementPos total_number_of_elements = 0;

    // Add elements from bin types.
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const SimplifyInputShape& input_shape = shapes[shape_pos];
        Shape shape = shape::remove_redundant_vertices(input_shape.shape).second;
        ApproximatedShape approximated_shape;

        auto mm = shape.compute_min_max();
        approximated_shape.min = mm.first;
        approximated_shape.max = mm.second;
        approximated_shape.copies = input_shape.copies;
        approximated_shape.outer = input_shape.outer;
        if (approximated_shape.outer)
            approximated_shape.union_input.push_back({input_shape.shape});

        total_number_of_elements += input_shape.copies * shape.elements.size();
        for (ElementPos element_pos = 0;
                element_pos < (ElementPos)shape.elements.size();
                ++element_pos) {
            const ShapeElement& element = shape.elements[element_pos];
            ApproximatedShapeElement approximated_element;
            approximated_element.element = element;
            approximated_element.element_prev_pos = (element_pos != 0)?
                element_pos - 1:
                shape.elements.size() - 1;
            approximated_element.element_next_pos = (element_pos != (ElementPos)shape.elements.size() - 1)?
                element_pos + 1:
                0;
            approximated_element.element_key_id = element_keys.size();
            approximated_shape.elements.push_back(approximated_element);
            approximated_shape.number_of_elements++;
            ApproximatedElementKey element_key;
            element_key.shape_pos = shape_pos;
            element_key.element_pos = element_pos;
            element_keys.push_back(element_key);
        }
        approximated_shapes.push_back(approximated_shape);
    }

    // Initialize priority_queue.
    optimizationtools::IndexedBinaryHeap<AreaDbl> priority_queue(
            element_keys.size());
    for (ElementPos element_key_id = 0;
            element_key_id < (ElementPos)element_keys.size();
            ++element_key_id) {
        const ApproximatedElementKey& element_key = element_keys[element_key_id];
        ApproximatedShape& shape = approximated_shapes[element_key.shape_pos];
        //std::cout << "element_key_id " << element_key_id
        //    << " shape_pos " << element_key.shape_pos
        //    << " element_pos " << element_key.element_pos
        //    << " shape.elements.size() " << shape.elements.size()
        //    << std::endl;
        AreaDbl cost = compute_approximation_cost(
                approximated_shapes,
                element_key);
        //std::cout << "element_key_id " << element_key_id
        //    << " shape_pos " << element_key.shape_pos
        //    << " element_pos " << element_key.element_pos
        //    << " cost " << cost
        //    << std::endl;
        if (cost < std::numeric_limits<AreaDbl>::infinity())
            priority_queue.update_key(element_key_id, cost);
    }

    AreaDbl current_cost = 0.0;
    while (!priority_queue.empty()) {

        // Check stop criterion based on average number of element per item.
        if (total_number_of_elements <= 8 * shapes.size())
            break;

        // Draw highest priority element.
        ElementPos element_key_id = priority_queue.top().first;
        //std::cout << "element_key_id " << element_key_id
        //    << " " << priority_queue.size() << std::endl;
        AreaDbl new_cost = priority_queue.top().second;
        priority_queue.pop();

        if (new_cost == std::numeric_limits<AreaDbl>::infinity())
            break;

        // Check stop criterion based on maximum approximation ratio.
        if (current_cost + new_cost
                > maximum_approximation_area) {
            break;
        }

        ApproximatedShape& shape = approximated_shapes[element_keys[element_key_id].shape_pos];

        if (shape.number_of_elements <= 4)
            continue;

        const ApproximatedElementKey& element_key = element_keys[element_key_id];
        //std::cout << "apply element_key_id " << element_key_id
        //    << " shape_pos " << element_key.shape_pos
        //    << " element_pos " << element_key.element_pos
        //    << " cost " << new_cost
        //    << " current_cost " << current_cost
        //    << " max_cost " << maximum_approximation_area
        //    << std::endl;

        // Apply simplification.
        current_cost += new_cost;
        total_number_of_elements -= shape.copies;
        const ApproximatedShapeElement& element = shape.elements[element_key.element_pos];
        ElementPos element_prev_pos = element.element_prev_pos;
        ElementPos element_prev_prev_pos = shape.elements[element_prev_pos].element_prev_pos;
        ElementPos element_next_pos = element.element_next_pos;
        ElementPos element_next_next_pos = shape.elements[element_next_pos].element_next_pos;
        ElementPos element_next_next_next_pos = shape.elements[element_next_next_pos].element_next_pos;
        apply_approximation(
                approximated_shapes,
                element_key);

        // Update priority queue values.
        AreaDbl cost_prev = compute_approximation_cost(
                approximated_shapes,
                {element_key.shape_pos, element_prev_pos});
        AreaDbl cost_prev_prev = compute_approximation_cost(
                approximated_shapes,
                {element_key.shape_pos, element_prev_prev_pos});
        AreaDbl cost_next = compute_approximation_cost(
                approximated_shapes,
                {element_key.shape_pos, element_next_pos});
        AreaDbl cost_next_next = compute_approximation_cost(
                approximated_shapes,
                {element_key.shape_pos, element_next_next_pos});
        AreaDbl cost_next_next_next = compute_approximation_cost(
                approximated_shapes,
                {element_key.shape_pos, element_next_next_next_pos});
        //std::cout << "element_prev_pos " << element_prev_pos
        //    << " cost_prev " << cost_prev << std::endl;
        //std::cout << "element_prev_prev_pos " << element_prev_prev_pos
        //    << " cost_prev_prev " << cost_prev_prev << std::endl;
        //std::cout << "element_next_pos " << element_next_pos
        //    << " cost_next " << cost_next << std::endl;
        //std::cout << "element_next_next_pos " << element_next_next_pos
        //    << " cost_next_next " << cost_next_next << std::endl;
        //std::cout << "element_next_next_next_pos " << element_next_next_next_pos
        //    << " cost_next_next_next " << cost_next_next_next << std::endl;
        if (priority_queue.contains(shape.elements[element_prev_pos].element_key_id)
                || cost_prev < std::numeric_limits<AreaDbl>::infinity()) {
            priority_queue.update_key(
                    shape.elements[element_prev_pos].element_key_id,
                    cost_prev);
        }
        if (priority_queue.contains(shape.elements[element_prev_prev_pos].element_key_id)
                || cost_prev_prev < std::numeric_limits<AreaDbl>::infinity()) {
            priority_queue.update_key(
                    shape.elements[element_prev_prev_pos].element_key_id,
                    cost_prev_prev);
        }
        if (priority_queue.contains(shape.elements[element_next_pos].element_key_id)
                || cost_next < std::numeric_limits<AreaDbl>::infinity()) {
            priority_queue.update_key(
                    shape.elements[element_next_pos].element_key_id,
                    cost_next);
        }
        if (priority_queue.contains(shape.elements[element_next_next_pos].element_key_id)
                || cost_next_next < std::numeric_limits<AreaDbl>::infinity()) {
            priority_queue.update_key(
                    shape.elements[element_next_next_pos].element_key_id,
                    cost_next_next);
        }
        if (priority_queue.contains(shape.elements[element_next_next_next_pos].element_key_id)
                || cost_next_next_next < std::numeric_limits<AreaDbl>::infinity()) {
            priority_queue.update_key(
                    shape.elements[element_next_next_next_pos].element_key_id,
                    cost_next_next_next);
        }
    }
    //std::cout << "end" << std::endl;

    // Build output.
    std::vector<ShapeWithHoles> shapes_new;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const SimplifyInputShape& shape = shapes[shape_pos];
        const ApproximatedShape& approximated_shape = approximated_shapes[shape_pos];
        ShapeWithHoles shape_new;
        if (shape.outer) {
            shape_new = compute_union(approximated_shape.union_input).front();
            if (strictly_lesser(shape_new.shape.compute_area(), shapes[shape_pos].shape.compute_area())) {
                throw std::logic_error(
                        "shape::simplify: inconsistent area.");
            }
        } else {
            auto difference_output = compute_difference({shape.shape}, approximated_shape.union_input);
            for (const ShapeWithHoles& s: difference_output)
                shape_new.holes.push_back(s.shape);
        }
        shapes_new.push_back(shape_new);
    }

    //std::cout << "shape_simplification end" << std::endl;
    return shapes_new;
}
