#pragma once

#include "shape/shape.hpp"

namespace shape
{

class Writer
{

public:

    Writer& add_point(
            const Point& point,
            const std::string& label = "");

    Writer& add_points(
            const std::vector<Point>& points,
            const std::string& label = "");

    Writer& add_element(
            const ShapeElement& element,
            const std::string& label = "");

    Writer& add_elements(
            const std::vector<ShapeElement>& elements,
            const std::string& label = "");

    Writer& add_shape(
            const Shape& shape,
            const std::string& label = "");

    Writer& add_shapes(
            const std::vector<Shape>& shapes,
            const std::string& label = "");

    Writer& add_shape_with_holes(
            const ShapeWithHoles& shape_with_holes,
            const std::string& label = "");

    Writer& add_shapes_with_holes(
            const std::vector<ShapeWithHoles>& shapes_with_holes,
            const std::string& label = "");

    void write_svg(const std::string& file_path) const;

    void write_json(const std::string& file_path) const;

private:

    std::pair<Point, Point> compute_min_max() const;

    struct WriterPoint
    {
        Point point;
        std::string label;
    };

    std::vector<WriterPoint> points_;

    struct WriterShapeElement
    {
        ShapeElement element;
        std::string label;
    };

    std::vector<WriterShapeElement> elements_;

    struct WriterShape
    {
        Shape shape;
        std::string label;
    };

    std::vector<WriterShape> shapes_;

    struct WriterShapeWithHoles
    {
        ShapeWithHoles shape_with_holes;
        std::string label;
    };

    std::vector<WriterShapeWithHoles> shapes_with_holes_;
};

}
