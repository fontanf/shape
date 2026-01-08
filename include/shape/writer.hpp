#pragma once

#include "shape/shape.hpp"

namespace shape
{

class Writer
{

public:

    Writer& add_point(const Point& point);

    Writer& add_points(const std::vector<Point>& points);

    Writer& add_element(const ShapeElement& element);

    Writer& add_elements(const std::vector<ShapeElement>& elements);

    Writer& add_shape(const Shape& shape);

    Writer& add_shapes(const std::vector<Shape>& shapes);

    Writer& add_shape_with_holes(const ShapeWithHoles& shape_with_holes);

    Writer& add_shapes_with_holes(const std::vector<ShapeWithHoles>& shapes_with_holes);

    void write_svg(const std::string& file_path) const;

    void write_json(const std::string& file_path) const;

private:

    std::pair<Point, Point> compute_min_max() const;

    std::vector<Point> points_;

    std::vector<ShapeElement> elements_;

    std::vector<Shape> shapes_;

    std::vector<ShapeWithHoles> shapes_with_holes_;
};

}
