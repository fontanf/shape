#include "shape/supports.hpp"

//#include <iostream>

using namespace shape;

ShapeSupports shape::compute_shape_supports(
        const Shape& shape,
        bool is_hole)
{
    ShapeSupports supports;

    // Find a starting element.
    ElementPos element_start_pos = -1;
    LengthDbl x_min = std::numeric_limits<LengthDbl>::infinity();
    ElementPos element_prev_pos = shape.elements.size() - 1;
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)shape.elements.size();
            ++element_pos) {
        const ShapeElement& element = shape.elements[element_pos];

        LengthDbl x_min_cur = element.start.x;
        if (element.type == ShapeElementType::CircularArc) {
            LengthDbl radius = element.radius();
            Angle starting_angle = angle_radian(element.start - element.center);
            Angle ending_angle = angle_radian(element.end - element.center);
            if (element.orientation == ShapeElementOrientation::Clockwise)
                std::swap(starting_angle, ending_angle);
            if (starting_angle <= ending_angle) {
                if (starting_angle <= M_PI
                        && M_PI <= ending_angle) {
                    x_min_cur = std::min(x_min_cur, element.center.x - radius);
                }
            } else {  // starting_angle > ending_angle
                if (starting_angle <= M_PI
                        || ending_angle <= M_PI) {
                    x_min_cur = std::min(x_min_cur, element.center.x - radius);
                }
            }
        }

        if (x_min > x_min_cur) {
            x_min = x_min_cur;
            element_start_pos = element_pos;
        }
    }

    int current_status = 0;
    Shape current_support;
    LengthDbl slope_prev = 0.0;
    for (ElementPos p = 0;
            p < (ElementPos)shape.elements.size();
            ++p) {
        ElementPos element_pos = (element_start_pos + p) % shape.elements.size();
        const ShapeElement& element = shape.elements[element_pos];
        //std::cout << "p " << p
        //    << " element_pos " << element_pos
        //    << " " << element.to_string()
        //    << " current_status " << current_status
        //    << " current_support.size() " << current_support.elements.size()
        //    << std::endl;
        if (current_status == 0) {
            switch (element.type) {
            case ShapeElementType::LineSegment: {
                if (strictly_lesser(element.end.x, element.start.x)) {
                    current_status = 1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements = {element};
                    slope_prev = (element.end.y - element.start.y) / (element.end.x - element.start.x);
                } else if (strictly_greater(element.end.x, element.start.x)) {
                    current_status = -1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements = {element};
                    slope_prev = (element.end.y - element.start.y) / (element.end.x - element.start.x);
                }
                break;
            } case ShapeElementType::CircularArc: {
                // TODO
                throw std::invalid_argument(
                        FUNC_SIGNATURE);
                break;
            }
            }
        } else if (current_status == 1) {
            switch (element.type) {
            case ShapeElementType::LineSegment: {
                LengthDbl slope = (element.end.y - element.start.y) / (element.end.x - element.start.x);
                //std::cout << "slope " << slope << " slope_prev " << slope_prev << std::endl;

                if (equal(element.end.x, element.start.x)) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supporting_parts.push_back(current_support);
                        } else {
                            supports.supported_parts.push_back(current_support);
                        }
                    }
                    current_status = 0;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                if (strictly_greater(element.end.x, element.start.x)) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supporting_parts.push_back(current_support);
                        } else {
                            supports.supported_parts.push_back(current_support);
                        }
                    }
                    current_status = -1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                if (slope < slope_prev) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supporting_parts.push_back(current_support);
                        } else {
                            supports.supported_parts.push_back(current_support);
                        }
                    }
                    current_status = 1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                current_support.elements.push_back(element);
                slope_prev = slope;
                break;
            } case ShapeElementType::CircularArc: {
                // TODO
                throw std::invalid_argument(
                        FUNC_SIGNATURE);
                break;
            }
            }
        } else if (current_status == -1) {
            switch (element.type) {
            case ShapeElementType::LineSegment: {
                LengthDbl slope = (element.end.y - element.start.y) / (element.end.x - element.start.x);
                //std::cout << "slope " << slope << " slope_prev " << slope_prev << std::endl;

                if (equal(element.end.x, element.start.x)) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supported_parts.push_back(current_support);
                        } else {
                            supports.supporting_parts.push_back(current_support);
                        }
                    }
                    current_status = 0;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                if (strictly_lesser(element.end.x, element.start.x)) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supported_parts.push_back(current_support);
                        } else {
                            supports.supporting_parts.push_back(current_support);
                        }
                    }
                    current_status = 1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                if (slope < slope_prev) {
                    if (!current_support.elements.empty()) {
                        if (!is_hole) {
                            supports.supported_parts.push_back(current_support);
                        } else {
                            supports.supporting_parts.push_back(current_support);
                        }
                    }
                    current_status = -1;
                    //std::cout << "current_status " << current_status << std::endl;
                    current_support.elements.clear();
                    current_support.elements.push_back(element);
                    slope_prev = slope;
                    continue;
                }

                current_support.elements.push_back(element);
                slope_prev = slope;
                break;
            } case ShapeElementType::CircularArc: {
                // TODO
                throw std::invalid_argument(
                        FUNC_SIGNATURE);
                break;
            }
            }
        }
    }

    if (!current_support.elements.empty()) {
        if ((current_status == 1 && !is_hole)
                || (current_status == -1 && is_hole)) {
            supports.supporting_parts.push_back(current_support);
        } else if ((current_status == -1 && !is_hole)
                || (current_status == 1 && is_hole)) {
            supports.supported_parts.push_back(current_support);
        }
    }

    if (!is_hole) {
        for (Shape& support: supports.supporting_parts)
            support = support.reverse();
        for (Shape& support: supports.supported_parts)
            support = support.reverse();
    }

    return supports;
}

ShapeSupports shape::compute_shape_supports(
        const ShapeWithHoles& shape)
{
    ShapeSupports supports = compute_shape_supports(shape.shape, false);
    for (const Shape& hole: shape.holes) {
        ShapeSupports hole_supports = compute_shape_supports(hole, true);
        supports.supported_parts.insert(
                supports.supported_parts.end(),
                hole_supports.supported_parts.begin(),
                hole_supports.supported_parts.end());
        supports.supporting_parts.insert(
                supports.supporting_parts.end(),
                hole_supports.supporting_parts.begin(),
                hole_supports.supporting_parts.end());
    }
    return supports;
}
