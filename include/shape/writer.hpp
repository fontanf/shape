#pragma once

#include "shape/shape.hpp"

namespace shape
{

/**
 * Collect points, elements and shapes, and write them to an SVG or a JSON
 * file, to visualize them while debugging.
 *
 * Each object can be given a label. The functions adding several objects at
 * once give each of them the label followed by its position in the list, or
 * no label if 'label' is empty. The functions adding objects return the
 * writer, so that the calls can be chained.
 */
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

    /**
     * Write the objects to an SVG file, with the y-axis going up.
     *
     * The points are drawn as disks whose radius is proportional to the size
     * of the drawing. The labels aren't written. Nothing is written if
     * 'file_path' is empty.
     *
     * Throw std::runtime_error if the file can't be opened.
     */
    void write_svg(const std::string& file_path) const;

    /**
     * Write the objects to a JSON file, which scripts/visualize.py can
     * display.
     *
     * The objects are written in the lists "points", "elements", "shapes"
     * and "shapes_with_holes", each object with its label. Nothing is written
     * if 'file_path' is empty.
     *
     * Throw std::runtime_error if the file can't be opened.
     */
    void write_json(const std::string& file_path) const;

private:

    AxisAlignedBoundingBox compute_min_max() const;

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
