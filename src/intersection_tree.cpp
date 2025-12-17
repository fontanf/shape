#include "shape/intersection_tree.hpp"

#include "shape/element_intersections.hpp"

//#include <iostream>

using namespace shape;

IntersectionTree::IntersectionTree(
        const std::vector<ShapeWithHoles>& shapes,
        const std::vector<ShapeElement>& elements,
        const std::vector<Point>& points):
    shapes_(&shapes),
    elements_(&elements),
    points_(&points),
    potentially_intersecting_shapes_(this->number_of_shapes()),
    potentially_intersecting_elements_(this->number_of_elements()),
    potentially_intersecting_points_(this->number_of_points())
{
    // Compute min/max of shapes and elements.
    std::vector<std::pair<Point, Point>> shapes_min_max(shapes.size());
    for (ShapePos shape_id = 0;
            shape_id < (ShapePos)shapes.size();
            ++shape_id) {
        const ShapeWithHoles& shape = shapes[shape_id];
        shapes_min_max[shape_id] = shape.compute_min_max();
    }
    std::vector<std::pair<Point, Point>> elements_min_max(elements.size());
    for (ElementPos element_id = 0;
            element_id < (ElementPos)elements.size();
            ++element_id) {
        const ShapeElement& element = elements[element_id];
        elements_min_max[element_id] = element.min_max();
    }

    Node root;
    // Compute root bounds.
    for (ShapePos shape_id = 0;
            shape_id < (ShapePos)shapes.size();
            ++shape_id) {
        root.l = (std::min)(root.l, shapes_min_max[shape_id].first.x);
        root.r = (std::max)(root.r, shapes_min_max[shape_id].first.x);
        root.b = (std::min)(root.b, shapes_min_max[shape_id].first.y);
        root.t = (std::max)(root.t, shapes_min_max[shape_id].first.y);
    }
    for (ElementPos element_id = 0;
            element_id < (ElementPos)elements.size();
            ++element_id) {
        const ShapeElement& element = elements[element_id];
        root.l = (std::min)(root.l, elements_min_max[element_id].first.x);
        root.r = (std::max)(root.r, elements_min_max[element_id].first.x);
        root.b = (std::min)(root.b, elements_min_max[element_id].first.y);
        root.t = (std::max)(root.t, elements_min_max[element_id].first.y);
    }
    for (ElementPos point_id = 0;
            point_id < (ElementPos)points.size();
            ++point_id) {
        const Point& point = points[point_id];
        root.l = (std::min)(root.l, point.x);
        root.r = (std::max)(root.r, point.x);
        root.b = (std::min)(root.b, point.y);
        root.t = (std::max)(root.t, point.y);
    }
    tree_.push_back(root);

    std::vector<StackElement> stack;
    StackElement stack_initial_element;
    stack_initial_element.node_id = 0;
    stack_initial_element.shape_ids = std::vector<ShapePos>(shapes.size());
    std::iota(stack_initial_element.shape_ids.begin(), stack_initial_element.shape_ids.end(), 0);
    stack_initial_element.element_ids = std::vector<ElementPos>(elements.size());
    std::iota(stack_initial_element.element_ids.begin(), stack_initial_element.element_ids.end(), 0);
    stack_initial_element.point_ids = std::vector<ElementPos>(points.size());
    std::iota(stack_initial_element.point_ids.begin(), stack_initial_element.point_ids.end(), 0);
    stack.push_back(stack_initial_element);

    while (!stack.empty()) {
        StackElement stack_element = stack.back();
        stack.pop_back();

        NodeId node_id = stack_element.node_id;
        Node& node = tree_[node_id];
        //std::cout << "node_id " << node_id
        //    << " size " << stack_element.shape_ids.size()
        //    << " l " << node.l
        //    << " r " << node.r
        //    << " b " << node.b
        //    << " t " << node.t
        //    << std::endl;

        // Compute x_middle / y_middle
        std::vector<LengthDbl> xs =  {
            2 * node.l / 3 + node.r / 3,
            node.l / 2 + node.r / 2,
            node.l / 3 + 2 * node.r / 3};
        std::vector<LengthDbl> ys =  {
            2 * node.b / 3 + node.t / 3,
            node.b / 2 + node.t / 2,
            node.b / 3 + 2 * node.t / 3};

        // Compute left/right/bottom/top shapes.
        std::vector<std::vector<ShapePos>> left_shape_ids(xs.size());
        std::vector<std::vector<ShapePos>> right_shape_ids(xs.size());
        std::vector<std::vector<ShapePos>> bottom_shape_ids(ys.size());
        std::vector<std::vector<ShapePos>> top_shape_ids(ys.size());
        for (ShapePos shape_id: stack_element.shape_ids) {
            auto mm = shapes_min_max[shape_id];
            for (int i = 0; i < (int)xs.size(); ++i) {
                if (!strictly_greater(mm.first.x, xs[i]))
                    left_shape_ids[i].push_back(shape_id);
                if (!strictly_lesser(mm.second.x, xs[i]))
                    right_shape_ids[i].push_back(shape_id);
            }
            for (int i = 0; i < (int)ys.size(); ++i) {
                if (!strictly_greater(mm.first.y, ys[i]))
                    bottom_shape_ids[i].push_back(shape_id);
                if (!strictly_lesser(mm.second.y, ys[i]))
                    top_shape_ids[i].push_back(shape_id);
            }
        }
        std::vector<std::vector<ShapePos>> left_element_ids(xs.size());
        std::vector<std::vector<ShapePos>> right_element_ids(xs.size());
        std::vector<std::vector<ShapePos>> bottom_element_ids(ys.size());
        std::vector<std::vector<ShapePos>> top_element_ids(ys.size());
        for (ElementPos element_id: stack_element.element_ids) {
            auto mm = elements_min_max[element_id];
            for (int i = 0; i < (int)xs.size(); ++i) {
                if (!strictly_greater(mm.first.x, xs[i]))
                    left_element_ids[i].push_back(element_id);
                if (!strictly_lesser(mm.second.x, xs[i]))
                    right_element_ids[i].push_back(element_id);
            }
            for (int i = 0; i < (int)ys.size(); ++i) {
                if (!strictly_greater(mm.first.y, ys[i]))
                    bottom_element_ids[i].push_back(element_id);
                if (!strictly_lesser(mm.second.y, ys[i]))
                    top_element_ids[i].push_back(element_id);
            }
        }
        std::vector<std::vector<ShapePos>> left_point_ids(xs.size());
        std::vector<std::vector<ShapePos>> right_point_ids(xs.size());
        std::vector<std::vector<ShapePos>> bottom_point_ids(ys.size());
        std::vector<std::vector<ShapePos>> top_point_ids(ys.size());
        for (ElementPos point_id: stack_element.point_ids) {
            const Point& point = points[point_id];
            for (int i = 0; i < (int)xs.size(); ++i) {
                if (!strictly_greater(point.x, xs[i]))
                    left_point_ids[i].push_back(point_id);
                if (!strictly_lesser(point.x, xs[i]))
                    right_point_ids[i].push_back(point_id);
            }
            for (int i = 0; i < (int)ys.size(); ++i) {
                if (!strictly_greater(point.y, ys[i]))
                    bottom_point_ids[i].push_back(point_id);
                if (!strictly_lesser(point.y, ys[i]))
                    top_point_ids[i].push_back(point_id);
            }
        }

        // Select best cut.
        char best = 'x';
        ShapePos n = stack_element.shape_ids.size() + stack_element.element_ids.size() + stack_element.point_ids.size();
        ShapePos n_best = n * (n - 1) / 2;
        //std::cout << "n " << n << std::endl;
        int i_best = -1;
        for (int i = 0; i < (int)xs.size(); ++i) {
            ShapePos nl = left_shape_ids[i].size() + left_element_ids[i].size() + left_point_ids[i].size();
            ShapePos nr = right_shape_ids[i].size() + right_element_ids[i].size() + right_point_ids[i].size();
            ShapePos nv = nl * (nl - 1) / 1 + nr * (nr - 1) / 2;
            //std::cout << "i " << i << " nv " << nv << std::endl;
            if (n_best > nv) {
                n_best = nv;
                best = 'v';
                i_best = i;
            }
        }
        for (int i = 0; i < (int)ys.size(); ++i) {
            ShapePos nb = bottom_shape_ids[i].size() + bottom_element_ids[i].size() + bottom_point_ids[i].size();
            ShapePos nt = top_shape_ids[i].size() + top_element_ids[i].size() + top_point_ids[i].size();
            ShapePos nh = nb * (nb - 1) / 2 + nt * (nt - 1) / 2;
            //std::cout << "i " << i << " nh " << nh << std::endl;
            if (n_best > nh) {
                n_best = nh;
                best = 'h';
                i_best = i;
            }
        }

        if (best == 'x') {
            node.direction = 'x';
            node.shape_ids = stack_element.shape_ids;
            node.element_ids = stack_element.element_ids;
            node.point_ids = stack_element.point_ids;
        } else if (best == 'v') {
            node.direction = 'v';
            node.position = xs[i_best];
            Node child_left;
            child_left.l = node.l;
            child_left.r = xs[i_best];
            child_left.b = node.b;
            child_left.t = node.t;
            StackElement stack_element_left;
            stack_element_left.node_id = tree_.size();
            node.lesser_child_id = tree_.size();
            stack_element_left.shape_ids = left_shape_ids[i_best];
            stack_element_left.element_ids = left_element_ids[i_best];
            stack_element_left.point_ids = left_point_ids[i_best];

            Node child_right;
            child_right.l = xs[i_best];
            child_right.r = node.r;
            child_right.b = node.b;
            child_right.t = node.t;
            StackElement stack_element_right;
            stack_element_right.node_id = tree_.size() + 1;
            node.greater_child_id = tree_.size() + 1;
            stack_element_right.shape_ids = right_shape_ids[i_best];
            stack_element_right.element_ids = right_element_ids[i_best];
            stack_element_right.point_ids = right_point_ids[i_best];

            tree_.push_back(child_left);
            tree_.push_back(child_right);
            stack.push_back(stack_element_right);
            stack.push_back(stack_element_left);
        } else {
            node.direction = 'h';
            node.position = ys[i_best];
            Node child_bottom;
            child_bottom.l = node.l;
            child_bottom.r = node.r;
            child_bottom.b = node.b;
            child_bottom.t = ys[i_best];
            StackElement stack_element_bottom;
            stack_element_bottom.node_id = tree_.size();
            node.lesser_child_id = tree_.size();
            stack_element_bottom.shape_ids = bottom_shape_ids[i_best];
            stack_element_bottom.element_ids = bottom_element_ids[i_best];
            stack_element_bottom.point_ids = bottom_point_ids[i_best];

            Node child_top;
            child_top.l = node.l;
            child_top.r = node.r;
            child_top.b = ys[i_best];
            child_top.t = node.t;
            StackElement stack_element_top;
            stack_element_top.node_id = tree_.size() + 1;
            node.greater_child_id = tree_.size() + 1;
            stack_element_top.shape_ids = top_shape_ids[i_best];
            stack_element_top.element_ids = top_element_ids[i_best];
            stack_element_top.point_ids = top_point_ids[i_best];

            tree_.push_back(child_bottom);
            tree_.push_back(child_top);
            stack.push_back(stack_element_top);
            stack.push_back(stack_element_bottom);
        }
    }
    //std::cout << "IntersectionTree::IntersectionTree end" << std::endl;
}

IntersectionTree::IntersectOutput IntersectionTree::intersect(
        const ShapeWithHoles& shape,
        bool strict) const
{
    //std::cout << "intersect..." << std::endl;
    IntersectOutput output;

    auto mm = shape.compute_min_max();

    potentially_intersecting_shapes_.clear();
    potentially_intersecting_elements_.clear();
    potentially_intersecting_points_.clear();
    std::vector<NodeId> stack = {0};

    while (!stack.empty()) {

        NodeId node_id = stack.back();
        stack.pop_back();
        const Node& node = tree_[node_id];

        if (node.direction == 'x') {
            for (ShapePos shape_id: node.shape_ids)
                potentially_intersecting_shapes_.add(shape_id);
            for (ElementPos element_id: node.element_ids)
                potentially_intersecting_elements_.add(element_id);
            for (ElementPos point_id: node.point_ids)
                potentially_intersecting_points_.add(point_id);
        } else if (node.direction == 'v') {
            if (!strictly_greater(mm.first.x, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.x, node.position))
                stack.push_back(node.greater_child_id);
        } else {  // node.direction == 'h'
            if (!strictly_greater(mm.first.y, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.y, node.position))
                stack.push_back(node.greater_child_id);
        }
    }

    for (ShapePos shape_id: potentially_intersecting_shapes_)
        if (shape::intersect(shape, this->shape(shape_id), strict))
            output.shape_ids.push_back(shape_id);
    for (ShapePos element_id: potentially_intersecting_elements_)
        if (shape::intersect(shape, this->element(element_id), strict))
            output.element_ids.push_back(element_id);
    for (ShapePos point_id: potentially_intersecting_points_)
        if (shape.contains(this->point(point_id), strict))
            output.point_ids.push_back(point_id);
    return output;
}

IntersectionTree::IntersectOutput IntersectionTree::intersect(
        const Shape& shape,
        bool strict) const
{
    //std::cout << "intersect..." << std::endl;
    IntersectOutput output;

    auto mm = shape.compute_min_max();

    potentially_intersecting_shapes_.clear();
    potentially_intersecting_elements_.clear();
    potentially_intersecting_points_.clear();
    std::vector<NodeId> stack = {0};

    while (!stack.empty()) {

        NodeId node_id = stack.back();
        stack.pop_back();
        const Node& node = tree_[node_id];

        if (node.direction == 'x') {
            for (ShapePos shape_id: node.shape_ids)
                potentially_intersecting_shapes_.add(shape_id);
            for (ElementPos element_id: node.element_ids)
                potentially_intersecting_elements_.add(element_id);
            for (ElementPos point_id: node.point_ids)
                potentially_intersecting_points_.add(point_id);
        } else if (node.direction == 'v') {
            if (!strictly_greater(mm.first.x, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.x, node.position))
                stack.push_back(node.greater_child_id);
        } else {  // node.direction == 'h'
            if (!strictly_greater(mm.first.y, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.y, node.position))
                stack.push_back(node.greater_child_id);
        }
    }

    for (ShapePos shape_id: potentially_intersecting_shapes_)
        if (shape::intersect(shape, this->shape(shape_id), strict))
            output.shape_ids.push_back(shape_id);
    for (ShapePos element_id: potentially_intersecting_elements_)
        if (shape::intersect(shape, this->element(element_id), strict))
            output.element_ids.push_back(element_id);
    for (ShapePos point_id: potentially_intersecting_points_)
        if (shape.contains(this->point(point_id), strict))
            output.point_ids.push_back(point_id);
    return output;
}

IntersectionTree::IntersectOutput IntersectionTree::intersect(
        const ShapeElement& element,
        bool strict) const
{
    IntersectOutput output;

    auto mm = element.min_max();

    potentially_intersecting_shapes_.clear();
    potentially_intersecting_elements_.clear();
    potentially_intersecting_points_.clear();
    std::vector<NodeId> stack = {0};

    while (!stack.empty()) {

        NodeId node_id = stack.back();
        stack.pop_back();
        const Node& node = tree_[node_id];

        if (node.direction == 'x') {
            for (ShapePos shape_id: node.shape_ids)
                potentially_intersecting_shapes_.add(shape_id);
            for (ElementPos element_id: node.element_ids)
                potentially_intersecting_elements_.add(element_id);
            for (ElementPos point_id: node.point_ids)
                potentially_intersecting_points_.add(point_id);
        } else if (node.direction == 'v') {
            if (!strictly_greater(mm.first.x, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.x, node.position))
                stack.push_back(node.greater_child_id);
        } else {  // node.direction == 'h'
            if (!strictly_greater(mm.first.y, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(mm.second.y, node.position))
                stack.push_back(node.greater_child_id);
        }
    }

    std::vector<ShapePos> intersecting_shapes;
    for (ShapePos shape_id: potentially_intersecting_shapes_)
        if (shape::intersect(this->shape(shape_id), element, strict))
            output.shape_ids.push_back(shape_id);
    for (ElementPos element_id: potentially_intersecting_elements_)
        if (!compute_intersections(this->element(element_id), element, strict).points.empty())
            output.element_ids.push_back(element_id);
    if (!strict) {
        for (ShapePos point_id: potentially_intersecting_points_)
            if (element.contains(this->point(point_id)))
                output.point_ids.push_back(point_id);
    }
    return output;
}

IntersectionTree::IntersectOutput IntersectionTree::intersect(
        const Point& point,
        bool strict) const
{
    IntersectOutput output;

    potentially_intersecting_shapes_.clear();
    potentially_intersecting_elements_.clear();
    potentially_intersecting_points_.clear();
    std::vector<NodeId> stack = {0};

    while (!stack.empty()) {

        NodeId node_id = stack.back();
        stack.pop_back();
        const Node& node = tree_[node_id];

        if (node.direction == 'x') {
            for (ShapePos shape_id: node.shape_ids)
                potentially_intersecting_shapes_.add(shape_id);
            for (ElementPos element_id: node.element_ids)
                potentially_intersecting_elements_.add(element_id);
            for (ElementPos point_id: node.point_ids)
                potentially_intersecting_points_.add(point_id);
        } else if (node.direction == 'v') {
            if (!strictly_greater(point.x, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(point.x, node.position))
                stack.push_back(node.greater_child_id);
        } else {  // node.direction == 'h'
            if (!strictly_greater(point.y, node.position))
                stack.push_back(node.lesser_child_id);
            if (!strictly_lesser(point.y, node.position))
                stack.push_back(node.greater_child_id);
        }
    }

    std::vector<ShapePos> intersecting_shapes;
    for (ShapePos shape_id: potentially_intersecting_shapes_)
        if (this->shape(shape_id).contains(point, strict))
            output.shape_ids.push_back(shape_id);
    if (!strict) {
        for (ElementPos element_id: potentially_intersecting_elements_)
            if (this->element(element_id).contains(point))
                output.element_ids.push_back(element_id);
        for (ShapePos point_id: potentially_intersecting_points_)
            if (equal(point, this->point(point_id)))
                output.point_ids.push_back(point_id);
    }
    return output;
}

std::vector<std::pair<ShapePos, ShapePos>> IntersectionTree::compute_intersecting_shapes(bool strict) const
{
    //std::cout << "compute_intersecting_shapes..." << std::endl;
    if (shapes_->empty())
        return {};

    std::vector<std::pair<ShapePos, ShapePos>> potentially_intersecting_shapes;
    for (const Node& node: tree_) {
        if (node.direction != 'x')
            continue;
        for (ShapePos pos_1 = 0;
                pos_1 < node.shape_ids.size();
                ++pos_1) {
            ShapePos shape_id_1 = node.shape_ids[pos_1];
            for (ShapePos pos_2 = pos_1 + 1;
                    pos_2 < node.shape_ids.size();
                    ++pos_2) {
                ShapePos shape_id_2 = node.shape_ids[pos_2];
                potentially_intersecting_shapes.push_back({shape_id_1, shape_id_2});
            }
        }
    }

    // Remove duplicates.
    std::sort(potentially_intersecting_shapes.begin(), potentially_intersecting_shapes.end());
    potentially_intersecting_shapes.erase(
            unique(potentially_intersecting_shapes.begin(), potentially_intersecting_shapes.end()),
            potentially_intersecting_shapes.end());

    // Compute intersections.
    std::vector<std::pair<ShapePos, ShapePos>> intersecting_shapes;
    for (auto p: potentially_intersecting_shapes)
        if (shape::intersect(this->shape(p.first), this->shape(p.second), strict))
            intersecting_shapes.push_back(p);
    return intersecting_shapes;
}

std::vector<IntersectionTree::ElementElementIntersection> IntersectionTree::compute_intersecting_elements(bool strict) const
{
    //std::cout << "compute_intersecting_elements..." << std::endl;
    if (elements_->empty())
        return {};

    std::vector<std::pair<ElementPos, ElementPos>> potentially_intersecting_elements;
    for (const Node& node: tree_) {
        if (node.direction != 'x')
            continue;
        for (ElementPos pos_1 = 0;
                pos_1 < node.element_ids.size();
                ++pos_1) {
            ElementPos element_id_1 = node.element_ids[pos_1];
            for (ElementPos pos_2 = pos_1 + 1;
                    pos_2 < node.element_ids.size();
                    ++pos_2) {
                ElementPos element_id_2 = node.element_ids[pos_2];
                potentially_intersecting_elements.push_back({element_id_1, element_id_2});
            }
        }
    }

    // Remove duplicates.
    std::sort(potentially_intersecting_elements.begin(), potentially_intersecting_elements.end());
    potentially_intersecting_elements.erase(
            unique(potentially_intersecting_elements.begin(), potentially_intersecting_elements.end()),
            potentially_intersecting_elements.end());

    // Compute intersections.
    std::vector<ElementElementIntersection> intersecting_elements;
    for (auto p: potentially_intersecting_elements) {
        auto intersections = shape::compute_intersections(
                this->element(p.first),
                this->element(p.second), strict);
        if (!intersections.points.empty()) {
            ElementElementIntersection intersection;
            intersection.element_id_1 = p.first;
            intersection.element_id_2 = p.second;
            intersection.intersections = intersections;
            intersecting_elements.push_back(intersection);
        }
    }
    return intersecting_elements;
}

std::vector<std::pair<ElementPos, ElementPos>> IntersectionTree::compute_equal_points() const
{
    //std::cout << "compute_equal_points..." << std::endl;
    if (points_->empty())
        return {};

    //std::cout << "points_->size() " << points_->size() << std::endl;
    std::vector<std::pair<ShapePos, ShapePos>> potentially_equal_points;
    for (const Node& node: tree_) {
        if (node.direction != 'x')
            continue;
        //std::cout << node.point_ids.size() << std::endl;
        for (ShapePos pos_1 = 0;
                pos_1 < node.point_ids.size();
                ++pos_1) {
            ShapePos point_id_1 = node.point_ids[pos_1];
            for (ShapePos pos_2 = pos_1 + 1;
                    pos_2 < node.point_ids.size();
                    ++pos_2) {
                ShapePos point_id_2 = node.point_ids[pos_2];
                potentially_equal_points.push_back({point_id_1, point_id_2});
            }
        }
    }
    //std::cout << "potentially_equal_points.size() " << potentially_equal_points.size() << std::endl;

    // Remove duplicates.
    //std::cout << "remove duplicates..." << std::endl;
    std::sort(potentially_equal_points.begin(), potentially_equal_points.end());
    potentially_equal_points.erase(
            unique(potentially_equal_points.begin(), potentially_equal_points.end()),
            potentially_equal_points.end());

    // Compute intersections.
    //std::cout << "compute intersections..." << std::endl;
    std::vector<std::pair<ShapePos, ShapePos>> equal_points;
    for (auto p: potentially_equal_points) {
        if (equal(this->point(p.first), this->point(p.second))) {
            equal_points.push_back(p);
            //std::cout << "equal " << this->point(p.first).to_string() << " " << this->point(p.second).to_string() << std::endl;
        }
    }
    //std::cout << "compute_equal_points end"
    //    " equal_points.size() " << equal_points.size() << std::endl;
    return equal_points;
}
