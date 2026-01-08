#include "shape/writer.hpp"

#include <fstream>

using namespace shape;

Writer& Writer::add_point(const Point& point)
{
    this->points_.push_back(point);
    return *this;
}

Writer& Writer::add_points(const std::vector<Point>& points)
{
    for (const Point& point: points)
        this->points_.push_back(point);
    return *this;
}

Writer& Writer::add_element(const ShapeElement& element)
{
    this->elements_.push_back(element);
    return *this;
}

Writer& Writer::add_elements(const std::vector<ShapeElement>& elements)
{
    for (const ShapeElement& element: elements)
        this->elements_.push_back(element);
    return *this;
}

Writer& Writer::add_shape(const Shape& shape)
{
    this->shapes_.push_back(shape);
    return *this;
}

Writer& Writer::add_shapes(const std::vector<Shape>& shapes)
{
    for (const Shape& shape: shapes)
        this->shapes_.push_back(shape);
    return *this;
}

Writer& Writer::add_shape_with_holes(const ShapeWithHoles& shape_with_holes)
{
    this->shapes_with_holes_.push_back(shape_with_holes);
    return *this;
}

Writer& Writer::add_shapes_with_holes(const std::vector<ShapeWithHoles>& shapes_with_holes)
{
    for (const ShapeWithHoles& shape: shapes_with_holes)
        this->shapes_with_holes_.push_back(shape);
    return *this;
}

std::pair<Point, Point> Writer::compute_min_max() const
{
    LengthDbl x_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl x_max = -std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_min = std::numeric_limits<LengthDbl>::infinity();
    LengthDbl y_max = -std::numeric_limits<LengthDbl>::infinity();
    for (const Point& point: this->points_) {
        x_min = (std::min)(x_min, point.x);
        x_max = (std::max)(x_max, point.x);
        y_min = (std::min)(y_min, point.y);
        y_max = (std::max)(y_max, point.y);
    }
    for (const ShapeElement& element: this->elements_) {
        auto points = element.min_max();
        x_min = (std::min)(x_min, points.first.x);
        x_max = (std::max)(x_max, points.second.x);
        y_min = (std::min)(y_min, points.first.y);
        y_max = (std::max)(y_max, points.second.y);
    }
    for (const Shape& shape: this->shapes_) {
        auto points = shape.compute_min_max();
        x_min = (std::min)(x_min, points.first.x);
        x_max = (std::max)(x_max, points.second.x);
        y_min = (std::min)(y_min, points.first.y);
        y_max = (std::max)(y_max, points.second.y);
    }
    for (const ShapeWithHoles& shape: this->shapes_with_holes_) {
        auto points = shape.compute_min_max();
        x_min = (std::min)(x_min, points.first.x);
        x_max = (std::max)(x_max, points.second.x);
        y_min = (std::min)(y_min, points.first.y);
        y_max = (std::max)(y_max, points.second.y);
    }
    return {{x_min, y_min}, {x_max, y_max}};
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

    auto mm = compute_min_max();
    LengthDbl width = (mm.second.x - mm.first.x);
    LengthDbl height = (mm.second.y - mm.first.y);

    std::string s = "<svg viewBox=\""
        + std::to_string(mm.first.x)
        + " " + std::to_string(-mm.first.y - height)
        + " " + std::to_string(width)
        + " " + std::to_string(height)
        + "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    file << s;

    for (ElementPos point_pos = 0;
             point_pos < (ElementPos)this->points_.size();
             ++point_pos) {
        const Point& point = points_[point_pos];
        file << "<g>" << std::endl;
        file << point.to_svg();
        file << "</g>" << std::endl;
    }
    for (ElementPos element_pos = 0;
             element_pos < (ElementPos)this->elements_.size();
             ++element_pos) {
        const ShapeElement& element = elements_[element_pos];
        file << "<g>" << std::endl;
        file << element.to_svg();
        file << "</g>" << std::endl;
    }
    for (ShapePos shape_pos = 0;
             shape_pos < (ShapePos)this->shapes_.size();
             ++shape_pos) {
        const Shape& shape = shapes_[shape_pos];
        file << "<g>" << std::endl;
        file << shape.to_svg();
        file << "</g>" << std::endl;
    }
    for (ShapePos shape_pos = 0;
             shape_pos < (ShapePos)this->shapes_with_holes_.size();
             ++shape_pos) {
        const ShapeWithHoles& shape = shapes_with_holes_[shape_pos];
        file << "<g>" << std::endl;
        file << shape.to_svg("blue");
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
        json["elements"][element_pos] = this->elements_[element_pos].to_json();
    }
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)this->shapes_.size();
            ++shape_pos) {
        json["shapes"][shape_pos] = this->shapes_[shape_pos].to_json();
    }
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)this->shapes_with_holes_.size();
            ++shape_pos) {
        json["shapes_with_holes"][shape_pos] = this->shapes_with_holes_[shape_pos].to_json();
    }

    file << std::setw(4) << json << std::endl;
}
