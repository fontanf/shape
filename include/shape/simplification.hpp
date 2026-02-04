#pragma once

#include "shape/shape.hpp"

namespace shape
{

struct SimplifyInputShape
{
    ShapeWithHoles shape;

    ShapePos copies;
};

std::vector<ShapeWithHoles> simplify(
        const std::vector<SimplifyInputShape>& shapes,
        AreaDbl maximum_approximation_area);

void simplify_export_inputs(
        const std::string& file_path,
        const std::vector<SimplifyInputShape>& shapes,
        AreaDbl maximum_approximation_area);

}
