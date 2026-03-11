#include "shape/extract_borders.hpp"

using namespace shape;

std::vector<Shape> shape::extract_borders(
        const Shape& shape)
{
    std::vector<Shape> res;

    AxisAlignedBoundingBox aabb = shape.compute_min_max();
    //std::cout << "aabb.x_min " << aabb.x_min
    //    << " aabb.y_min " << aabb.y_min
    //    << " aabb.x_max " << aabb.x_max
    //    << " aabb.y_max " << aabb.y_max
    //    << std::endl;

    Shape shape_border;
    shape_border.is_path = false;
    ElementPos element_0_pos = 0;
    for (ElementPos element_pos = 0;
            element_pos < shape.elements.size();
            ++element_pos) {
        const ShapeElement& shape_element = shape.elements[element_pos];
        if (shape_element.start.x == aabb.x_min) {
            element_0_pos = element_pos;
            break;
        }
    }
    //std::cout << "element_0_pos " << element_0_pos << std::endl;
    // 0: left; 1: bottom; 2: right; 3: top.
    const ShapeElement& element_0 = shape.elements[element_0_pos];
    int start_border = (element_0.start.y == aabb.y_min)? 1: 0;
    LengthDbl start_coordinate = element_0.start.y;
    for (ElementPos element_pos = 0;
            element_pos < shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[(element_0_pos + element_pos) % shape.elements.size()];
        //std::cout << "element_pos " << ((element_0_pos + element_pos) % shape.elements.size())
        //    << " / " << shape.elements.size()
        //    << ": " << element.to_string()
        //    << "; start_border: " << start_border
        //    << std::endl;
        shape_border.elements.push_back(element);
        bool close = false;
        if (start_border == 0) {
            if (equal(element.end.x, aabb.x_min)) {
                ShapeElement new_element;
                new_element.type = ShapeElementType::LineSegment;
                new_element.start = element.end;
                new_element.end = shape_border.elements[0].start;
                shape_border.elements.push_back(new_element);
                close = true;
                if (!equal(element.end.y, aabb.y_min)) {
                    start_border = 0;
                } else {
                    start_border = 1;
                }
            } else if (equal(element.end.y, aabb.y_min)) {
                if (element.end.x != shape_border.elements[0].start.x) {
                    ShapeElement new_element_1;
                    new_element_1.type = ShapeElementType::LineSegment;
                    new_element_1.start = element.end;
                    new_element_1.end = {aabb.x_min, aabb.y_min};
                    shape_border.elements.push_back(new_element_1);
                    ShapeElement new_element_2;
                    new_element_2.type = ShapeElementType::LineSegment;
                    new_element_2.start = new_element_1.end;
                    new_element_2.end = shape_border.elements[0].start;
                    shape_border.elements.push_back(new_element_2);
                }
                close = true;
                if (!equal(element.end.x, aabb.x_max)) {
                    start_border = 1;
                } else {
                    start_border = 2;
                }
            }
        } else if (start_border == 1) {
            if (equal(element.end.y, aabb.y_min)) {
                ShapeElement new_element;
                new_element.type = ShapeElementType::LineSegment;
                new_element.start = element.end;
                new_element.end = shape_border.elements[0].start;
                shape_border.elements.push_back(new_element);
                close = true;
                if (!equal(element.end.x, aabb.x_max)) {
                    start_border = 1;
                } else {
                    start_border = 2;
                }
            } else if (equal(element.end.x, aabb.x_max)) {
                if (element.end.y != shape_border.elements[0].start.y) {
                    ShapeElement new_element_1;
                    new_element_1.type = ShapeElementType::LineSegment;
                    new_element_1.start = element.end;
                    new_element_1.end = {aabb.x_max, aabb.y_min};
                    shape_border.elements.push_back(new_element_1);
                    ShapeElement new_element_2;
                    new_element_2.type = ShapeElementType::LineSegment;
                    new_element_2.start = new_element_1.end;
                    new_element_2.end = shape_border.elements[0].start;
                    shape_border.elements.push_back(new_element_2);
                }
                close = true;
                if (!equal(element.end.y, aabb.y_max)) {
                    start_border = 2;
                } else {
                    start_border = 3;
                }
            }
        } else if (start_border == 2) {
            if (equal(element.end.x, aabb.x_max)) {
                ShapeElement new_element;
                new_element.type = ShapeElementType::LineSegment;
                new_element.start = element.end;
                new_element.end = shape_border.elements[0].start;
                shape_border.elements.push_back(new_element);
                close = true;
                if (!equal(element.end.y, aabb.y_max)) {
                    start_border = 2;
                } else {
                    start_border = 3;
                }
            } else if (equal(element.end.y, aabb.y_max)) {
                if (element.end.y != shape_border.elements[0].start.y) {
                    ShapeElement new_element_1;
                    new_element_1.type = ShapeElementType::LineSegment;
                    new_element_1.start = element.end;
                    new_element_1.end = {aabb.x_max, aabb.y_max};
                    shape_border.elements.push_back(new_element_1);
                    ShapeElement new_element_2;
                    new_element_2.type = ShapeElementType::LineSegment;
                    new_element_2.start = new_element_1.end;
                    new_element_2.end = shape_border.elements[0].start;
                    shape_border.elements.push_back(new_element_2);
                }
                close = true;
                if (!equal(element.end.x, aabb.x_max)) {
                    start_border = 3;
                } else {
                    start_border = 0;
                }
            }
        } else if (start_border == 3) {
            if (equal(element.end.y, aabb.y_max)) {
                ShapeElement new_element;
                new_element.type = ShapeElementType::LineSegment;
                new_element.start = element.end;
                new_element.end = shape_border.elements[0].start;
                shape_border.elements.push_back(new_element);
                close = true;
                if (!equal(element.end.x, aabb.x_min)) {
                    start_border = 3;
                } else {
                    start_border = 0;
                }
            } else if (equal(element.end.x, aabb.x_min)) {
                if (element.end.x != shape_border.elements[0].start.x) {
                    ShapeElement new_element_1;
                    new_element_1.type = ShapeElementType::LineSegment;
                    new_element_1.start = element.end;
                    new_element_1.end = {aabb.x_min, aabb.y_max};
                    shape_border.elements.push_back(new_element_1);
                    ShapeElement new_element_2;
                    new_element_2.type = ShapeElementType::LineSegment;
                    new_element_2.start = new_element_1.end;
                    new_element_2.end = shape_border.elements[0].start;
                    shape_border.elements.push_back(new_element_2);
                }
                close = true;
                if (!equal(element.end.y, aabb.y_max)) {
                    start_border = 0;
                } else {
                    start_border = 1;
                }
            }
        }
        //std::cout << "shape_border " << shape_border.to_string(0) << std::endl;
        // New shape.
        if (close) {
            //std::cout << "close " << shape_border.to_string(0) << std::endl;
            if (shape_border.elements.size() >= 3)
                res.push_back(shape_border.reverse());
            shape_border.elements.clear();
        }
    }

    return res;
}
