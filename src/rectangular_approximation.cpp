#include "shape/rectangular_approximation.hpp"

#include "shape/approximation.hpp"
#include "shape/trapezoidation.hpp"

#include <algorithm>
#include <fstream>
#include <queue>

using namespace shape;

namespace
{

AreaDbl rectangle_area(const AxisAlignedBoundingBox& rectangle)
{
    return (rectangle.x_max - rectangle.x_min)
        * (rectangle.y_max - rectangle.y_min);
}

AxisAlignedBoundingBox trapezoid_rectangle(const GeneralizedTrapezoid& trapezoid)
{
    AxisAlignedBoundingBox rectangle;
    rectangle.x_min = trapezoid.x_min();
    rectangle.x_max = trapezoid.x_max();
    rectangle.y_min = trapezoid.y_bottom();
    rectangle.y_max = trapezoid.y_top();
    return rectangle;
}

AxisAlignedBoundingBox merge_rectangles(
        const AxisAlignedBoundingBox& rectangle_1,
        const AxisAlignedBoundingBox& rectangle_2)
{
    AxisAlignedBoundingBox rectangle;
    rectangle.x_min = (std::min)(rectangle_1.x_min, rectangle_2.x_min);
    rectangle.x_max = (std::max)(rectangle_1.x_max, rectangle_2.x_max);
    rectangle.y_min = (std::min)(rectangle_1.y_min, rectangle_2.y_min);
    rectangle.y_max = (std::max)(rectangle_1.y_max, rectangle_2.y_max);
    return rectangle;
}

/**
 * Rectangle of the outer approximation of a shape.
 */
struct ApproximationRectangle
{
    AxisAlignedBoundingBox rectangle;

    /**
     * The trapezoid covered by the rectangle, if it covers a single one (only
     * these rectangles are split); empty otherwise.
     */
    std::vector<GeneralizedTrapezoid> trapezoid;

    /** Incremented each time the rectangle changes. */
    Counter version = 0;

    bool removed = false;
};

/**
 * Split of a trapezoid into the two trapezoids of half its height.
 */
struct TrapezoidSplit
{
    /**
     * Trapezoid split, or empty if it is too thin to be split.
     */
    std::vector<GeneralizedTrapezoid> trapezoids;

    /** Waste removed by the split. */
    AreaDbl waste_reduction = 0.0;
};

TrapezoidSplit split_trapezoid(const GeneralizedTrapezoid& trapezoid)
{
    TrapezoidSplit split;
    LengthDbl y_middle = (trapezoid.y_bottom() + trapezoid.y_top()) / 2;
    if (!strictly_lesser(trapezoid.y_bottom(), y_middle)
            || !strictly_lesser(y_middle, trapezoid.y_top())) {
        return split;
    }
    LengthDbl x_middle_left = trapezoid.x_left(y_middle);
    LengthDbl x_middle_right = trapezoid.x_right(y_middle);
    split.trapezoids.push_back(GeneralizedTrapezoid(
                trapezoid.y_bottom(),
                y_middle,
                trapezoid.x_bottom_left(),
                trapezoid.x_bottom_right(),
                x_middle_left,
                x_middle_right));
    split.trapezoids.push_back(GeneralizedTrapezoid(
                y_middle,
                trapezoid.y_top(),
                x_middle_left,
                x_middle_right,
                trapezoid.x_top_left(),
                trapezoid.x_top_right()));
    split.waste_reduction = rectangle_area(trapezoid_rectangle(trapezoid))
        - rectangle_area(trapezoid_rectangle(split.trapezoids[0]))
        - rectangle_area(trapezoid_rectangle(split.trapezoids[1]));
    return split;
}

/**
 * Candidate split of a rectangle, in the priority queue of the splits.
 */
struct SplitCandidate
{
    /** Waste removed by the split, counted once for each copy. */
    AreaDbl waste_reduction;

    ShapePos shape_pos;

    ElementPos rectangle_pos;

    Counter version;

    bool operator<(const SplitCandidate& candidate) const
    {
        return waste_reduction < candidate.waste_reduction;
    }
};

/**
 * Candidate merge of two rectangles, in the priority queue of the merges.
 */
struct MergeCandidate
{
    /** Waste added by the merge, counted once for each copy. */
    AreaDbl waste_increase;

    ShapePos shape_pos;

    ElementPos rectangle_pos_1;

    Counter version_1;

    ElementPos rectangle_pos_2;

    Counter version_2;

    bool operator<(const MergeCandidate& candidate) const
    {
        // Smallest waste increase first.
        return waste_increase > candidate.waste_increase;
    }
};

}

std::vector<std::vector<AxisAlignedBoundingBox>> shape::outer_approximation_by_rectangles(
        const std::vector<OuterApproximationByRectanglesInputShape>& shapes,
        AreaDbl maximum_waste,
        ElementPos maximum_number_of_rectangles)
{
    // Initial rectangles: the bounding boxes of the trapezoids of each shape.
    std::vector<std::vector<ApproximationRectangle>> rectangles(shapes.size());
    AreaDbl waste = 0.0;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        const ShapeWithHoles& shape = shapes[shape_pos].shape;
        if (shape.shape.elements.empty())
            continue;
        ShapeWithHoles polygon = shape;
        if (!shape.is_polygon()) {
            AxisAlignedBoundingBox aabb = shape.compute_min_max();
            LengthDbl segment_length = (std::max)(
                    aabb.x_max - aabb.x_min,
                    aabb.y_max - aabb.y_min) / 64;
            polygon = approximate_by_line_segments(shape, segment_length);
        }
        AreaDbl rectangles_area = 0.0;
        for (const GeneralizedTrapezoid& trapezoid: trapezoidation(polygon)) {
            ApproximationRectangle rectangle;
            rectangle.rectangle = trapezoid_rectangle(trapezoid);
            rectangle.trapezoid.push_back(trapezoid);
            rectangles_area += rectangle_area(rectangle.rectangle);
            rectangles[shape_pos].push_back(rectangle);
        }
        waste += shapes[shape_pos].copies * (rectangles_area - shape.compute_area());
    }

    // Split the rectangles while the waste is larger than the maximum waste.
    std::priority_queue<SplitCandidate> split_candidates;
    auto add_split_candidate = [&shapes, &rectangles, &split_candidates](
            ShapePos shape_pos,
            ElementPos rectangle_pos)
    {
        const ApproximationRectangle& rectangle = rectangles[shape_pos][rectangle_pos];
        if (rectangle.trapezoid.empty())
            return;
        TrapezoidSplit split = split_trapezoid(rectangle.trapezoid.front());
        if (split.trapezoids.empty() || !strictly_greater(split.waste_reduction, 0.0))
            return;
        SplitCandidate candidate;
        candidate.waste_reduction = shapes[shape_pos].copies * split.waste_reduction;
        candidate.shape_pos = shape_pos;
        candidate.rectangle_pos = rectangle_pos;
        candidate.version = rectangle.version;
        split_candidates.push(candidate);
    };
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        for (ElementPos rectangle_pos = 0;
                rectangle_pos < (ElementPos)rectangles[shape_pos].size();
                ++rectangle_pos) {
            add_split_candidate(shape_pos, rectangle_pos);
        }
    }
    while (strictly_greater(waste, maximum_waste) && !split_candidates.empty()) {
        SplitCandidate candidate = split_candidates.top();
        split_candidates.pop();
        std::vector<ApproximationRectangle>& shape_rectangles = rectangles[candidate.shape_pos];
        if (shape_rectangles[candidate.rectangle_pos].version != candidate.version)
            continue;
        if ((ElementPos)shape_rectangles.size() >= maximum_number_of_rectangles)
            continue;
        TrapezoidSplit split = split_trapezoid(
                shape_rectangles[candidate.rectangle_pos].trapezoid.front());
        ApproximationRectangle& rectangle_bottom = shape_rectangles[candidate.rectangle_pos];
        rectangle_bottom.rectangle = trapezoid_rectangle(split.trapezoids[0]);
        rectangle_bottom.trapezoid = {split.trapezoids[0]};
        rectangle_bottom.version++;
        ApproximationRectangle rectangle_top;
        rectangle_top.rectangle = trapezoid_rectangle(split.trapezoids[1]);
        rectangle_top.trapezoid = {split.trapezoids[1]};
        shape_rectangles.push_back(rectangle_top);
        waste -= candidate.waste_reduction;
        add_split_candidate(candidate.shape_pos, candidate.rectangle_pos);
        add_split_candidate(candidate.shape_pos, (ElementPos)shape_rectangles.size() - 1);
    }

    // Merge rectangles while the waste stays at most the maximum waste.
    std::priority_queue<MergeCandidate> merge_candidates;
    auto add_merge_candidate = [&shapes, &rectangles, &merge_candidates](
            ShapePos shape_pos,
            ElementPos rectangle_pos_1,
            ElementPos rectangle_pos_2)
    {
        const ApproximationRectangle& rectangle_1 = rectangles[shape_pos][rectangle_pos_1];
        const ApproximationRectangle& rectangle_2 = rectangles[shape_pos][rectangle_pos_2];
        MergeCandidate candidate;
        candidate.waste_increase = shapes[shape_pos].copies * (
                rectangle_area(merge_rectangles(rectangle_1.rectangle, rectangle_2.rectangle))
                - rectangle_area(rectangle_1.rectangle)
                - rectangle_area(rectangle_2.rectangle));
        candidate.shape_pos = shape_pos;
        candidate.rectangle_pos_1 = rectangle_pos_1;
        candidate.version_1 = rectangle_1.version;
        candidate.rectangle_pos_2 = rectangle_pos_2;
        candidate.version_2 = rectangle_2.version;
        merge_candidates.push(candidate);
    };
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        for (ElementPos rectangle_pos_1 = 0;
                rectangle_pos_1 < (ElementPos)rectangles[shape_pos].size();
                ++rectangle_pos_1) {
            for (ElementPos rectangle_pos_2 = rectangle_pos_1 + 1;
                    rectangle_pos_2 < (ElementPos)rectangles[shape_pos].size();
                    ++rectangle_pos_2) {
                add_merge_candidate(shape_pos, rectangle_pos_1, rectangle_pos_2);
            }
        }
    }
    while (!merge_candidates.empty()) {
        MergeCandidate candidate = merge_candidates.top();
        merge_candidates.pop();
        std::vector<ApproximationRectangle>& shape_rectangles = rectangles[candidate.shape_pos];
        ApproximationRectangle& rectangle_1 = shape_rectangles[candidate.rectangle_pos_1];
        ApproximationRectangle& rectangle_2 = shape_rectangles[candidate.rectangle_pos_2];
        if (rectangle_1.removed
                || rectangle_2.removed
                || rectangle_1.version != candidate.version_1
                || rectangle_2.version != candidate.version_2) {
            continue;
        }
        // The other candidates increase the waste at least as much.
        if (strictly_greater(waste + candidate.waste_increase, maximum_waste))
            break;
        rectangle_1.rectangle = merge_rectangles(rectangle_1.rectangle, rectangle_2.rectangle);
        rectangle_1.trapezoid.clear();
        rectangle_1.version++;
        rectangle_2.removed = true;
        waste += candidate.waste_increase;
        for (ElementPos rectangle_pos = 0;
                rectangle_pos < (ElementPos)shape_rectangles.size();
                ++rectangle_pos) {
            if (rectangle_pos == candidate.rectangle_pos_1
                    || shape_rectangles[rectangle_pos].removed) {
                continue;
            }
            add_merge_candidate(
                    candidate.shape_pos,
                    (std::min)(rectangle_pos, candidate.rectangle_pos_1),
                    (std::max)(rectangle_pos, candidate.rectangle_pos_1));
        }
    }

    std::vector<std::vector<AxisAlignedBoundingBox>> output(shapes.size());
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        for (const ApproximationRectangle& rectangle: rectangles[shape_pos])
            if (!rectangle.removed)
                output[shape_pos].push_back(rectangle.rectangle);
    }
    return output;
}

void shape::outer_approximation_by_rectangles_export_inputs(
        const std::string& file_path,
        const std::vector<OuterApproximationByRectanglesInputShape>& shapes,
        AreaDbl maximum_waste,
        ElementPos maximum_number_of_rectangles)
{
    std::ofstream file{file_path};
    nlohmann::json json;
    for (ShapePos shape_pos = 0;
            shape_pos < (ShapePos)shapes.size();
            ++shape_pos) {
        json["shapes"][shape_pos]["shape"] = shapes[shape_pos].shape.to_json();
        json["shapes"][shape_pos]["copies"] = shapes[shape_pos].copies;
    }
    json["maximum_waste"] = maximum_waste;
    json["maximum_number_of_rectangles"] = maximum_number_of_rectangles;
    file << std::setw(4) << json << std::endl;
}
