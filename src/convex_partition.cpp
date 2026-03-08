#include "shape/convex_partition.hpp"

#include "shape/trapezoidation.hpp"

#include <queue>

using namespace shape;

namespace
{

/**
 * Find a shared element between two shapes.
 *
 * Returns {element_pos_1, element_pos_2} such that shape_1's element at
 * element_pos_1 is the reverse of shape_2's element at element_pos_2
 * (the two shapes are adjacent along that segment).
 * Returns {-1, -1} if no such pair exists.
 */
std::pair<ElementPos, ElementPos> find_shared_element(
        const Shape& shape_1,
        const Shape& shape_2)
{
    for (ElementPos element_pos_1 = 0;
            element_pos_1 < (ElementPos)shape_1.elements.size();
            ++element_pos_1) {
        ShapeElement reversed = shape_1.elements[element_pos_1].reverse();
        for (ElementPos element_pos_2 = 0;
                element_pos_2 < (ElementPos)shape_2.elements.size();
                ++element_pos_2) {
            if (equal(shape_2.elements[element_pos_2], reversed))
                return {element_pos_1, element_pos_2};
        }
    }
    return {-1, -1};
}

/**
 * Merge two shapes that share a common element.
 *
 * shape_1.elements[shape_1_element_pos] is the reverse of
 * shape_2.elements[shape_2_element_pos].
 *
 * The merged shape removes both shared edges and stitches the remaining
 * boundaries:
 *   - shape_1's edges after the shared edge (going around shape_1)
 *   - shape_2's edges after the shared edge (going around shape_2)
 */
Shape merge_shapes(
        const Shape& shape_1,
        const Shape& shape_2,
        ElementPos shape_1_element_pos,
        ElementPos shape_2_element_pos)
{
    Shape merged;

    for (ElementPos offset = 1;
            offset < (ElementPos)shape_1.elements.size();
            ++offset) {
        merged.elements.push_back(shape_1.elements[
            (shape_1_element_pos + offset) % shape_1.elements.size()]);
    }

    for (ElementPos offset = 1;
            offset < (ElementPos)shape_2.elements.size();
            ++offset) {
        merged.elements.push_back(shape_2.elements[
            (shape_2_element_pos + offset) % shape_2.elements.size()]);
    }

    return merged;
}

} // namespace

std::vector<Shape> shape::compute_convex_partition(
        const ShapeWithHoles& shape)
{
    std::vector<GeneralizedTrapezoid> trapezoids = trapezoidation(shape);

    std::vector<Shape> parts;
    for (const GeneralizedTrapezoid& trapezoid: trapezoids)
        parts.push_back(trapezoid.to_shape());

    std::vector<bool> active(parts.size(), true);

    // Each candidate pair is enqueued exactly once, with the element positions
    // already computed, so find_shared_element is never called twice for the
    // same pair.
    struct Candidate {
        ElementPos part_pos_1;
        ElementPos part_pos_2;
        ElementPos element_pos_1;
        ElementPos element_pos_2;
    };
    std::queue<Candidate> candidates;

    // Seed the queue with every pair of parts that share an element.
    for (ElementPos part_pos_1 = 0;
            part_pos_1 < (ElementPos)parts.size();
            ++part_pos_1) {
        for (ElementPos part_pos_2 = part_pos_1 + 1;
                part_pos_2 < (ElementPos)parts.size();
                ++part_pos_2) {
            std::pair<ElementPos, ElementPos> shared = find_shared_element(
                    parts[part_pos_1],
                    parts[part_pos_2]);
            if (shared.first != -1) {
                Candidate candidate;
                candidate.part_pos_1 = part_pos_1;
                candidate.part_pos_2 = part_pos_2;
                candidate.element_pos_1 = shared.first;
                candidate.element_pos_2 = shared.second;
                candidates.push(candidate);
            }
        }
    }

    while (!candidates.empty()) {
        Candidate candidate = candidates.front();
        candidates.pop();

        if (!active[candidate.part_pos_1] || !active[candidate.part_pos_2])
            continue;

        Shape merged = merge_shapes(
                parts[candidate.part_pos_1],
                parts[candidate.part_pos_2],
                candidate.element_pos_1,
                candidate.element_pos_2);
        if (!merged.is_convex())
            continue;

        active[candidate.part_pos_1] = false;
        active[candidate.part_pos_2] = false;

        ElementPos new_pos = (ElementPos)parts.size();
        parts.push_back(std::move(merged));
        active.push_back(true);

        // Enqueue pairs of the new shape with every active shape it touches.
        for (ElementPos part_pos = 0; part_pos < new_pos; ++part_pos) {
            if (!active[part_pos])
                continue;
            std::pair<ElementPos, ElementPos> shared = find_shared_element(
                    parts[new_pos],
                    parts[part_pos]);
            if (shared.first != -1) {
                Candidate new_candidate;
                new_candidate.part_pos_1 = new_pos;
                new_candidate.part_pos_2 = part_pos;
                new_candidate.element_pos_1 = shared.first;
                new_candidate.element_pos_2 = shared.second;
                candidates.push(new_candidate);
            }
        }
    }

    std::vector<Shape> output;
    for (ElementPos part_pos = 0;
            part_pos < (ElementPos)parts.size();
            ++part_pos) {
        if (active[part_pos])
            output.push_back(parts[part_pos]);
    }
    return output;
}
