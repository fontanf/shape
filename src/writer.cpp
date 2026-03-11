#include "shape/writer.hpp"

#include <fstream>

using namespace shape;

Writer& Writer::add_point(
        const Point& point,
        const std::string& label)
{
    this->points_.push_back({point, label});
    return *this;
}

Writer& Writer::add_points(
        const std::vector<Point>& points,
        const std::string& label)
{
    for (ElementPos point_pos = 0;
            point_pos < (ElementPos)points.size();
            ++point_pos) {
        const Point& point = points[point_pos];
        std::string l = "";
        if (!label.empty())
            l = label + " " + std::to_string(point_pos);
        this->points_.push_back({point, l});
    }
    return *this;
}

Writer& Writer::add_element(
        const ShapeElement& element,
        const std::string& label)
{
    this->elements_.push_back({element, label});
    return *this;
}

Writer& Writer::add_elements(
        const std::vector<ShapeElement>& elements,
        const std::string& label)
{
    for (ElementPos element_pos = 0;
            element_pos < (ElementPos)elements.size();
            ++element_pos) {
        const ShapeElement& element = elements[element_pos];
        std::string l = "";
        if (!label.empty())
            l = label + " " + std::to_string(element_pos);
        this->elements_.push_back({element, l});
    }
    return *this;
}

Writer& Writer::add_shape(
        const Shape& shape,
        const std::string& label)
{
    this->shapes_.push_back({shape, label});
    return *this;
}

Writer& Writer::add_shapes(
        const std::vector<Shape>& shapes,
        const std::string& label)
{
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const Shape& shape = shapes[shape_pos];
        std::string l = "";
        if (!label.empty())
            l = label + " " + std::to_string(shape_pos);
        this->shapes_.push_back({shape, l});
    }
    return *this;
}

Writer& Writer::add_shape_with_holes(
        const ShapeWithHoles& shape_with_holes,
        const std::string& label)
{
    this->shapes_with_holes_.push_back({shape_with_holes, label});
    return *this;
}

Writer& Writer::add_shapes_with_holes(
        const std::vector<ShapeWithHoles>& shapes_with_holes,
        const std::string& label)
{
    for (ShapePos shape_with_holes_pos = 0;
            shape_with_holes_pos < (ShapePos)shapes_with_holes.size();
            ++shape_with_holes_pos) {
        const ShapeWithHoles& shape_with_holes = shapes_with_holes[shape_with_holes_pos];
        std::string l = "";
        if (!label.empty())
            l = label + " " + std::to_string(shape_with_holes_pos);
        this->shapes_with_holes_.push_back({shape_with_holes, l});
    }
    return *this;
}

AxisAlignedBoundingBox Writer::compute_min_max() const
{
    AxisAlignedBoundingBox output;
    for (const WriterPoint& point: this->points_) {
        output.x_min = (std::min)(output.x_min, point.point.x);
        output.x_max = (std::max)(output.x_max, point.point.x);
        output.y_min = (std::min)(output.y_min, point.point.y);
        output.y_max = (std::max)(output.y_max, point.point.y);
    }
    for (const WriterShapeElement& element: this->elements_)
        output = merge(output, element.element.min_max());
    for (const WriterShape& shape: this->shapes_)
        output = merge(output, shape.shape.compute_min_max());
    for (const WriterShapeWithHoles& shape: this->shapes_with_holes_)
        output = merge(output, shape.shape_with_holes.compute_min_max());
    return output;
}

void Writer::write_svg(const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
    }

    AxisAlignedBoundingBox aabb = compute_min_max();
    LengthDbl width = (aabb.x_max - aabb.x_min);
    LengthDbl height = (aabb.y_max - aabb.y_min);

    std::string s = "<svg viewBox=\""
        + std::to_string(aabb.x_min)
        + " " + std::to_string(-aabb.y_min - height)
        + " " + std::to_string(width)
        + " " + std::to_string(height)
        + "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    file << s;

    for (ElementPos point_pos = 0;
             point_pos < (ElementPos)this->points_.size();
             ++point_pos) {
        const WriterPoint& point = points_[point_pos];
        file << "<g>" << std::endl;
        file << point.point.to_svg();
        file << "</g>" << std::endl;
    }
    for (ElementPos element_pos = 0;
             element_pos < (ElementPos)this->elements_.size();
             ++element_pos) {
        const WriterShapeElement& element = elements_[element_pos];
        file << "<g>" << std::endl;
        file << element.element.to_svg();
        file << "</g>" << std::endl;
    }
    for (ShapePos shape_pos = 0;
             shape_pos < (ShapePos)this->shapes_.size();
             ++shape_pos) {
        const WriterShape& shape = shapes_[shape_pos];
        file << "<g>" << std::endl;
        file << shape.shape.to_svg();
        file << "</g>" << std::endl;
    }
    for (ShapePos shape_pos = 0;
             shape_pos < (ShapePos)this->shapes_with_holes_.size();
             ++shape_pos) {
        const WriterShapeWithHoles& shape = shapes_with_holes_[shape_pos];
        file << "<g>" << std::endl;
        file << shape.shape_with_holes.to_svg("blue");
        file << "</g>" << std::endl;
    }

    file << "</svg>" << std::endl;
}

void Writer::write_json(const std::string& file_path) const
{
    if (file_path.empty())
        return;
    std::ofstream file{file_path};
    if (!file.good()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + ": "
                "unable to open file \"" + file_path + "\".");
    }

    nlohmann::json json;
    for (ElementPos element_pos = 0;
            element_pos < (ShapePos)this->elements_.size();
            ++element_pos) {
        json["elements"][element_pos] = this->elements_[element_pos].element.to_json();
        json["elements"][element_pos]["label"] = this->elements_[element_pos].label;
    }
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)this->shapes_.size();
            ++shape_pos) {
        json["shapes"][shape_pos] = this->shapes_[shape_pos].shape.to_json();
        json["shapes"][shape_pos]["label"] = this->shapes_[shape_pos].label;
    }
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)this->shapes_with_holes_.size();
            ++shape_pos) {
        json["shapes_with_holes"][shape_pos] = this->shapes_with_holes_[shape_pos].shape_with_holes.to_json();
        json["shapes_with_holes"][shape_pos]["label"] = this->shapes_with_holes_[shape_pos].label;
    }

    file << std::setw(4) << json << std::endl;
}
