#pragma once

#include "shape.hpp"

namespace shape
{

std::pair<bool, Shape> remove_redundant_vertices(
        const Shape& shape);

std::pair<bool, ShapeWithHoles> remove_redundant_vertices(
        const ShapeWithHoles& shape);

Shape remove_backtracks(
        const Shape& shape);


std::pair<bool, Shape> remove_aligned_vertices(
        const Shape& shape);

std::pair<bool, ShapeWithHoles> remove_aligned_vertices(
        const ShapeWithHoles& shape);


ShapeWithHoles remove_small_holes(
        const ShapeWithHoles& shape_with_holes,
        AreaDbl area_limit);


ShapeWithHoles clean_extreme_slopes_outer(
        const Shape& shape);

std::vector<Shape> clean_extreme_slopes_inner(
        const Shape& shape);

std::vector<ShapeWithHoles> fix_self_intersections(
        const ShapeWithHoles& shape);

}
