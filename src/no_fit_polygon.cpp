#include "shape/no_fit_polygon.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
#include "shape/equalize.hpp"
#include "shape/basic_shapes.hpp"
#include "shape/offset.hpp"

#include <algorithm>
#include <string>

using namespace shape;

namespace
{

/**
 * Element of one side of the merge, with the tangent directions at its start
 * and at its end, in output space (negated for the orbiting side).
 *
 * The directions of all the elements of both sides are equalized together
 * (see no_fit_polygon), and then only compared exactly: two directions equal
 * within the tolerance are exactly equal. They are not recomputed from the
 * points of the elements: when an arc is split at a direction, both pieces
 * keep this exact direction at the split point.
 */
struct DirectedElement
{
    ShapeElement element;
    Point start_direction;
    Point end_direction;
};

/**
 * Point on a circle (given center, radius, orientation) whose tangent
 * direction (for a CCW/Anticlockwise traversal) is parallel to
 * 'target_direction'. Used both to split an existing arc element at a given
 * direction, and to place the endpoint of a newly-constructed (summed) arc.
 *
 * Derived by inverting ShapeElement::tangent(): for an Anticlockwise arc,
 * tangent(p) = normalize((p - center).rotate(90)), so the radial direction
 * (p - center) is target_direction.rotate(-90); for Clockwise it is
 * target_direction.rotate(90).
 */
Point point_at_tangent_direction(
        const Point& center,
        LengthDbl radius,
        ShapeElementOrientation orientation,
        const Point& target_direction)
{
    Point radial_direction = (orientation != ShapeElementOrientation::Clockwise)?
        target_direction.rotate(-90.0):
        target_direction.rotate(90.0);
    return center + radius * normalize(radial_direction);
}

Point point_at_tangent_direction(
        const ShapeElement& arc,
        const Point& target_direction)
{
    return point_at_tangent_direction(arc.center, arc.radius(), arc.orientation, target_direction);
}

/**
 * True if 'direction' (in output space) lies strictly between the start and
 * end directions of a circular arc element (i.e. strictly inside the open
 * span, not at either endpoint).
 *
 * no_fit_polygon requires every arc to span < 180° (see its precondition
 * check), so a direction is strictly inside iff turning CCW from the start
 * direction to 'direction' is a positive turn, and turning CCW from
 * 'direction' to the end direction is also a positive turn -- two plain
 * cross-product sign tests, no wraparound handling needed precisely because
 * the span cannot reach 180°. The directions are equalized, so a direction
 * equal to the start or end direction is not strictly inside.
 */
bool direction_strictly_inside(
        const DirectedElement& element,
        const Point& direction)
{
    if (element.element.type != ShapeElementType::CircularArc)
        return false;
    // The products are compared rather than subtracted, so that a fused
    // multiply-add can't make the cross product of two equal directions
    // nonzero.
    const Point& start = element.start_direction;
    const Point& end = element.end_direction;
    return start.x * direction.y > start.y * direction.x
        && direction.x * end.y > direction.y * end.x;
}

/**
 * Split 'arc' into consecutive sub-arc pieces at every direction in
 * 'target_directions' (in output space) that falls strictly inside its span,
 * ordered along the arc from its own start to its own end. Returns {arc}
 * unchanged if none do.
 */
std::vector<DirectedElement> split_arc_into_pieces(
        const DirectedElement& arc,
        bool negate,
        const std::vector<Point>& target_directions)
{
    std::vector<Point> split_directions;
    for (const Point& direction: target_directions)
        if (direction_strictly_inside(arc, direction))
            split_directions.push_back(direction);
    if (split_directions.empty())
        return {arc};

    // Order the directions along the arc, and remove the duplicates.
    std::sort(
            split_directions.begin(),
            split_directions.end(),
            [&arc](const Point& direction_1, const Point& direction_2)
            {
                return angle_radian(arc.start_direction, direction_1)
                    < angle_radian(arc.start_direction, direction_2);
            });
    split_directions.erase(
            std::unique(split_directions.begin(), split_directions.end()),
            split_directions.end());

    std::vector<DirectedElement> pieces;
    DirectedElement remainder = arc;
    for (const Point& direction: split_directions) {
        Point local_direction = (negate)? Point{-direction.x, -direction.y}: direction;
        Point split_point = point_at_tangent_direction(remainder.element, local_direction);
        DirectedElement piece;
        piece.element = remainder.element.extract(remainder.element.start, split_point);
        piece.start_direction = remainder.start_direction;
        piece.end_direction = direction;
        pieces.push_back(piece);
        remainder.element = remainder.element.extract(split_point, remainder.element.end);
        remainder.start_direction = direction;
    }
    pieces.push_back(remainder);
    return pieces;
}

/**
 * Build the final ordered element list for one side of the merge (fixed or
 * orbiting): identical to 'elements' except that every CircularArc element
 * is replaced in place by pieces pre-split at every critical direction
 * contributed by the *other* side -- each of its plain edges' own tangent
 * direction, plus (for each arc it has) that arc's start and end tangents.
 * A shape may contain any number of arcs (each still required to span
 * < 180°); each one is split independently against the same set of
 * other-side critical directions.
 *
 * This has to happen up front, before the main merge loop runs, rather than
 * relying purely on reactively splitting an arc against whichever direction
 * the loop happens to be comparing it to at the time: the loop only ever
 * looks at each side's *current* element, so if an arc is not the first
 * element of its shape (e.g. a circular segment cut off by
 * decompose_into_basic_shapes starts with its chord instead, depending on
 * which point ends up bottom/top-most), some of the other side's edges can
 * be fully consumed against something else entirely before that arc ever
 * becomes current -- permanently missing a split their direction should
 * have triggered.
 *
 * The directions of both sides are in output space (see DirectedElement);
 * 'negate_self' tells whether 'elements' is the orbiting side, whose split
 * points are computed in its own (non-negated) frame.
 */
std::vector<DirectedElement> presplit_elements(
        const std::vector<DirectedElement>& elements,
        bool negate_self,
        const std::vector<DirectedElement>& other_elements)
{
    // Critical directions depend only on other_elements, so they only need
    // computing once and are then reused for every arc found below. They are
    // already in output space, like the directions of 'elements'.
    std::vector<Point> critical_directions;
    for (const DirectedElement& other: other_elements) {
        critical_directions.push_back(other.start_direction);
        critical_directions.push_back(other.end_direction);
    }

    // Also always split at the global tangent-order minimum direction (1,0):
    // if an arc's span straddles it, the piece starting exactly there needs
    // to exist so the start-position selection below can find it. Without
    // this, an arc whose span straddles that direction can undercut every
    // other candidate's own start direction (its tangent dips below all of
    // them partway through, past the comparator's wraparound point) without
    // any element actually starting there for the selection to pick.
    critical_directions.push_back(Point{1.0, 0.0});

    std::vector<DirectedElement> result;
    result.reserve(elements.size());
    for (const DirectedElement& element: elements) {
        if (element.element.type != ShapeElementType::CircularArc) {
            result.push_back(element);
            continue;
        }
        for (const DirectedElement& piece: split_arc_into_pieces(element, negate_self, critical_directions))
            result.push_back(piece);
    }
    return result;
}

/**
 * Incremental cursor over one shape's (already rotated to start at the
 * chosen start position) element sequence, used to drive the merge loop
 * below. 'negate' is true for the orbiting side: every direction/point
 * query already accounts for tracing -orbiting_shape instead of
 * orbiting_shape, so callers never need to negate anything themselves.
 */
struct EdgeCursor
{
    const std::vector<DirectedElement>* elements = nullptr;
    ElementPos next_pos = 0;
    bool negate = false;

    // The element currently at the front of the cursor (already popped from
    // 'elements' conceptually; may be a suffix of an original element after
    // a partial consumption). Only meaningful if has_current() is true.
    DirectedElement current;
    bool has_current_ = false;

    bool has_current() const { return has_current_; }

    void init()
    {
        if (elements != nullptr && !elements->empty()) {
            current = (*elements)[0];
            next_pos = 1;
            has_current_ = true;
        }
    }

    void advance()
    {
        if (elements != nullptr && next_pos < (ElementPos)elements->size()) {
            current = (*elements)[next_pos++];
            has_current_ = true;
        } else {
            has_current_ = false;
        }
    }

    /** Equalized direction at the current element's start, in output space. */
    Point start_direction() const { return current.start_direction; }

    /** Equalized direction at the current element's end, in output space. */
    Point end_direction() const { return current.end_direction; }

    /**
     * Exact (not equalized) direction of the current element, in output
     * space, for a line segment.
     */
    Point line_segment_direction() const
    {
        Point t = current.element.end - current.element.start;
        return negate? Point{-t.x, -t.y}: t;
    }

    /** Build the (possibly negated) placed copy of a local sub-element, shifted so its start is at 'placed_start'. */
    ShapeElement place(
            const ShapeElement& local_element,
            const Point& placed_start) const
    {
        ShapeElement placed;
        if (!negate) {
            placed = local_element;
        } else if (local_element.type == ShapeElementType::LineSegment) {
            placed = build_line_segment(
                    {-local_element.start.x, -local_element.start.y},
                    {-local_element.end.x, -local_element.end.y});
        } else {
            placed = build_circular_arc(
                    {-local_element.start.x, -local_element.start.y},
                    {-local_element.end.x, -local_element.end.y},
                    {-local_element.center.x, -local_element.center.y},
                    local_element.orientation);
        }
        Point shift = placed_start - placed.start;
        placed.shift(shift.x, shift.y);
        return placed;
    }

    /**
     * Split the current element (which must be a CircularArc) at the point
     * whose tangent direction (in output/negated space) is 'target_direction',
     * which must be strictly inside its current span. Returns the local
     * (unplaced) prefix sub-element; the cursor's current element becomes the
     * local (unplaced) remainder (still not advanced).
     */
    ShapeElement split_current_at(const Point& target_direction)
    {
        Point local_target = negate?
            Point{-target_direction.x, -target_direction.y}:
            target_direction;
        Point split_point = point_at_tangent_direction(current.element, local_target);
        ShapeElement prefix = current.element.extract(current.element.start, split_point);
        current.element = current.element.extract(split_point, current.element.end);
        // The remainder starts exactly at the target direction, which is not
        // recomputed from the split point.
        current.start_direction = target_direction;
        return prefix;
    }
};

/** Append 'element' to 'result' and update 'current_vertex', unless it is degenerate (zero length). */
void append_if_nondegenerate(
        std::vector<ShapeElement>& result,
        Point& current_vertex,
        const ShapeElement& element)
{
    if (!equal(element.start, element.end)) {
        result.push_back(element);
        current_vertex = element.end;
    }
}

/** Consume the cursor's current element in full (no split), appending it to the result. */
void consume_whole(
        EdgeCursor& cursor,
        std::vector<ShapeElement>& result,
        Point& current_vertex)
{
    ShapeElement placed = cursor.place(cursor.current.element, current_vertex);
    append_if_nondegenerate(result, current_vertex, placed);
    cursor.advance();
}

/**
 * Consume the cursor's current element up to 'other_direction' (a direction
 * in output/negated space): if the current element is a CircularArc whose
 * span strictly contains that direction, split it there, append only the
 * prefix, and leave the (unplaced) remainder as the new current element
 * (cursor not advanced). Otherwise the whole element is consumed as usual.
 */
void consume_up_to(
        EdgeCursor& cursor,
        const Point& other_direction,
        std::vector<ShapeElement>& result,
        Point& current_vertex)
{
    if (direction_strictly_inside(cursor.current, other_direction)) {
        ShapeElement local_prefix = cursor.split_current_at(other_direction);
        ShapeElement placed = cursor.place(local_prefix, current_vertex);
        append_if_nondegenerate(result, current_vertex, placed);
    } else {
        consume_whole(cursor, result, current_vertex);
    }
}

/**
 * Handle the case where both cursors currently point at a CircularArc
 * element (possible whenever both shapes have at least one arc; with
 * several arcs per shape this can trigger more than once over the course of
 * the merge, once per pair of arcs that end up concurrently current). If
 * their tangent-direction spans don't overlap, the earlier one is consumed
 * alone as usual. If they do overlap, the two circles are summed over the
 * overlapping direction range -- for two circular arcs sharing a range of
 * tangent (equivalently: normal) directions, their Minkowski sum over that
 * range is itself a circular arc: center = sum of centers, radius = sum of
 * radii, same direction range. Any non-overlapping prefix/suffix on either
 * side is emitted separately as a lone arc contribution.
 */
void handle_arc_arc(
        EdgeCursor& fixed,
        EdgeCursor& orbiting,
        std::vector<ShapeElement>& result,
        Point& current_vertex)
{
    Point f_start = fixed.start_direction();
    Point f_end = fixed.end_direction();
    Point o_start = orbiting.start_direction();
    Point o_end = orbiting.end_direction();

    // f_span/o_span: each side's own tangent span, in (0, pi) -- guaranteed
    // small by no_fit_polygon's precondition that no arc spans >= 180
    // degrees, and that guarantee survives negation (a uniform shift
    // changes where the pair sits, not their relative separation).
    //
    // strictly_lesser_angle (used elsewhere in this file) is NOT safe here:
    // it is a *global* total order with a fixed branch cut at 0/360 degrees.
    // Negating orbiting's tangent directions shifts each by +180 degrees
    // independently; if the pre-negation pair straddled that fixed cut, the
    // shift can flip their order under that comparator even though the
    // underlying arc itself is perfectly well-formed. angle_radian, being
    // relative to whichever vector is passed as its first argument rather
    // than a fixed global reference, has no such cut and is what all the
    // comparisons below use instead.
    Angle f_span = angle_radian(f_start, f_end);
    Angle o_span = angle_radian(o_start, o_end);

    // Signed (shortest-way) CCW offset of o_start relative to f_start, in
    // (-pi, pi].
    Angle d_start = angle_radian(f_start, o_start);
    if (d_start > M_PI)
        d_start -= 2.0 * M_PI;

    // No overlap: o's span starts at/after f's end (d_start >= f_span), or
    // f's span starts at/after o's end (-d_start >= o_span).
    if (!(d_start < f_span)) {
        consume_whole(fixed, result, current_vertex);
        return;
    }
    if (!(-d_start < o_span)) {
        consume_whole(orbiting, result, current_vertex);
        return;
    }

    // Overlap exists. Peel off any lone prefix (at most one side has one:
    // whichever start has the strictly later, i.e. positive-offset, side).
    bool fixed_has_prefix = (d_start > 0.0);
    bool orbiting_has_prefix = (d_start < 0.0);
    Point overlap_start = fixed_has_prefix? o_start: f_start;
    if (fixed_has_prefix)
        consume_up_to(fixed, overlap_start, result, current_vertex);
    if (orbiting_has_prefix)
        consume_up_to(orbiting, overlap_start, result, current_vertex);

    // Both cursors now start exactly at overlap_start. Determine how far the
    // overlap extends (same local/signed reasoning, now relative to the two
    // end directions), splitting off whichever side's remainder is longer.
    Angle d_end = angle_radian(f_end, o_end);
    if (d_end > M_PI)
        d_end -= 2.0 * M_PI;
    bool fixed_has_suffix = (d_end < 0.0);
    bool orbiting_has_suffix = (d_end > 0.0);
    Point overlap_end = fixed_has_suffix? o_end: f_end;

    ShapeElement fixed_local_piece = fixed.current.element;
    bool fixed_will_advance = !fixed_has_suffix;
    if (fixed_has_suffix)
        fixed_local_piece = fixed.split_current_at(overlap_end);
    ShapeElement orbiting_local_piece = orbiting.current.element;
    bool orbiting_will_advance = !orbiting_has_suffix;
    if (orbiting_has_suffix)
        orbiting_local_piece = orbiting.split_current_at(overlap_end);

    // Sum the two (local, unplaced) overlap pieces into a single arc.
    Point orbiting_neg_center = {-orbiting_local_piece.center.x, -orbiting_local_piece.center.y};
    Point orbiting_neg_start = {-orbiting_local_piece.start.x, -orbiting_local_piece.start.y};
    Point combined_center = current_vertex
        + (fixed_local_piece.center - fixed_local_piece.start)
        + (orbiting_neg_center - orbiting_neg_start);
    LengthDbl combined_radius = fixed_local_piece.radius() + orbiting_local_piece.radius();
    Point combined_end = point_at_tangent_direction(
            combined_center, combined_radius, ShapeElementOrientation::Anticlockwise, overlap_end);
    ShapeElement combined = build_circular_arc(
            current_vertex, combined_end, combined_center, ShapeElementOrientation::Anticlockwise);
    append_if_nondegenerate(result, current_vertex, combined);

    // If not advancing, the cursor's .current was already updated to the
    // (unplaced) post-overlap remainder by split_current_at above.
    if (fixed_will_advance)
        fixed.advance();
    if (orbiting_will_advance)
        orbiting.advance();
}

}  // namespace

Shape shape::no_fit_polygon(
        const Shape& fixed_shape,
        const Shape& orbiting_shape)
{
    if (!fixed_shape.is_convex()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "fixed_shape is not convex.");
    }
    if (!orbiting_shape.is_convex()) {
        throw std::runtime_error(
                FUNC_SIGNATURE + "; "
                "orbiting_shape is not convex.");
    }
    // Every arc must span < 180°: direction_strictly_inside's
    // two-cross-product membership test and handle_arc_arc's overlap
    // detection both rely on an arc's own start and end tangent directions
    // determining a unique "short way" between them, which stops being true
    // at or past a half turn (see direction_strictly_inside's and
    // handle_arc_arc's comments). There is no limit on how many such arcs a
    // shape may contain -- e.g. every corner of an inflated convex polygon
    // rounds into its own arc, and all of those are individually well under
    // 180° since a single polygon vertex's exterior turning angle can never
    // reach a half turn.
    auto check_arc_spans = [](const Shape& s, const char* shape_name) {
        for (const ShapeElement& element: s.elements) {
            if (element.type != ShapeElementType::CircularArc)
                continue;
            Angle span = angle_radian(
                    element.tangent(element.start),
                    element.tangent(element.end));
            if (!strictly_lesser(span, M_PI)) {
                throw std::runtime_error(
                        FUNC_SIGNATURE + "; "
                        + std::string(shape_name) + " has a circular arc "
                        "spanning >= 180 degrees.");
            }
        }
    };
    check_arc_spans(fixed_shape, "fixed_shape");
    check_arc_spans(orbiting_shape, "orbiting_shape");

    // Tangent directions of the elements of both sides, in output space
    // (negated for the orbiting side), equalized together with the global
    // tangent-order minimum direction (1, 0): directions equal within the
    // tolerance become exactly equal, so that they can then be compared
    // exactly, like the points in the boolean operations. Otherwise, the
    // floating-point noise of the tangents of circular arcs (e.g. the tangent
    // (1, -1e-16) at the bottom of a circle) would put directions that
    // should be equal on either side of each other, or of the branch cut of
    // strictly_lesser_angle at (1, 0).
    std::vector<Point> directions = {{1.0, 0.0}};
    for (const Shape* shape: {&fixed_shape, &orbiting_shape}) {
        bool negate = (shape == &orbiting_shape);
        for (const ShapeElement& element: shape->elements) {
            Point start_direction = element.tangent(element.start);
            Point end_direction = element.tangent(element.end);
            if (negate) {
                start_direction = {-start_direction.x, -start_direction.y};
                end_direction = {-end_direction.x, -end_direction.y};
            }
            directions.push_back(start_direction);
            directions.push_back(end_direction);
        }
    }
    std::vector<Point> equalized_directions = equalize_points(directions);
    // The directions equal to (1, 0) must remain exactly (1, 0), which is
    // not necessarily the representative chosen by equalize_points.
    Point first_direction = equalized_directions[0];
    for (Point& direction: equalized_directions)
        if (direction == first_direction)
            direction = {1.0, 0.0};
    std::vector<DirectedElement> fixed_directed_elements;
    std::vector<DirectedElement> orbiting_directed_elements;
    ElementPos direction_pos = 1;
    for (const Shape* shape: {&fixed_shape, &orbiting_shape}) {
        std::vector<DirectedElement>& directed_elements = (shape == &fixed_shape)?
            fixed_directed_elements:
            orbiting_directed_elements;
        for (const ShapeElement& element: shape->elements) {
            DirectedElement directed_element;
            directed_element.element = element;
            directed_element.start_direction = equalized_directions[direction_pos];
            directed_element.end_direction = equalized_directions[direction_pos + 1];
            direction_pos += 2;
            directed_elements.push_back(directed_element);
        }
    }

    // Pre-split each side's arcs (if any), in their original (unrotated)
    // index order, against the other side's critical directions -- see
    // presplit_elements' comment for why this can't be done reactively
    // during the merge below, and for why it also always splits at the
    // global tangent-order minimum direction. Both calls read from the
    // shapes' own original element lists (not from each other's already-
    // split result) so the two splits don't depend on each other's outcome.
    std::vector<DirectedElement> fixed_elements = presplit_elements(
            fixed_directed_elements, false, orbiting_directed_elements);
    std::vector<DirectedElement> orbiting_elements = presplit_elements(
            orbiting_directed_elements, true, fixed_directed_elements);

    // Find the element whose start direction is the global minimum (per
    // strictly_lesser_angle's total order) among fixed_elements, and among
    // orbiting_elements (whose directions are already negated). Starting the
    // traced sequence at that element guarantees every subsequent element's
    // direction increases monotonically all the way around before wrapping
    // back, exactly once, to close the loop -- which the merge below relies
    // on.
    //
    // For a pure polygon this is equivalent to (and was previously
    // implemented as) picking the bottom-most vertex for fixed_shape / the
    // top-most vertex for orbiting_shape: a convex polygon's tangent
    // direction is minimal exactly at the edge leaving its bottom-most
    // point. That geometric shortcut is not reliable once a shape can
    // contain a CircularArc element, since an arc's own tangent sweeps
    // continuously across its span and could dip below every other
    // element's own start direction partway through without any element
    // actually starting there -- which is exactly why presplit_elements
    // above always splits every arc at the global minimum direction (1, 0)
    // too, guaranteeing there is always an element that starts exactly
    // there for this search to find.
    auto find_start_pos = [](const std::vector<DirectedElement>& elements)
    {
        ElementPos start_pos = 0;
        for (ElementPos pos = 1; pos < (ElementPos)elements.size(); ++pos) {
            if (strictly_lesser_angle(
                        elements[pos].start_direction,
                        elements[start_pos].start_direction)) {
                start_pos = pos;
            }
        }
        return start_pos;
    };
    ElementPos fixed_start_pos = find_start_pos(fixed_elements);
    ElementPos orbiting_start_pos = find_start_pos(orbiting_elements);

    // Starting vertex of the NFP: bottom of fixed minus top of orbiting.
    const Point nfp_start = fixed_elements[fixed_start_pos].element.start
        - orbiting_elements[orbiting_start_pos].element.start;

    const ElementPos fixed_num_elements = (ElementPos)fixed_elements.size();
    const ElementPos orbiting_num_elements = (ElementPos)orbiting_elements.size();

    std::rotate(
            fixed_elements.begin(),
            fixed_elements.begin() + fixed_start_pos,
            fixed_elements.end());
    std::rotate(
            orbiting_elements.begin(),
            orbiting_elements.begin() + orbiting_start_pos,
            orbiting_elements.end());

    EdgeCursor fixed_cursor;
    fixed_cursor.elements = &fixed_elements;
    fixed_cursor.negate = false;
    fixed_cursor.init();

    EdgeCursor orbiting_cursor;
    orbiting_cursor.elements = &orbiting_elements;
    orbiting_cursor.negate = true;
    orbiting_cursor.init();

    // Merge both element sequences in non-decreasing angular (tangent
    // direction) order (rotating calipers) and trace the Minkowski-sum
    // shape. A CircularArc element contributes a continuous range of
    // directions (from its start to its end tangent) instead of a single
    // one; whenever the other side's current direction falls strictly
    // inside that range, the arc is split there so only the relevant prefix
    // is merged in, leaving the remainder for later iterations. Two arcs
    // active on both sides at once (possible whenever both shapes have at
    // least one arc) are summed over their overlapping range -- see
    // handle_arc_arc.
    //
    // When two edges have the same direction (parallel) they are both added
    // in sequence, producing collinear intermediate vertices that are removed
    // by remove_aligned_vertices at the end.
    std::vector<ShapeElement> result;
    result.reserve(fixed_num_elements + orbiting_num_elements);
    Point current_vertex = nfp_start;

    while (fixed_cursor.has_current() || orbiting_cursor.has_current()) {
        if (!orbiting_cursor.has_current()) {
            consume_whole(fixed_cursor, result, current_vertex);
            continue;
        }
        if (!fixed_cursor.has_current()) {
            consume_whole(orbiting_cursor, result, current_vertex);
            continue;
        }

        bool fixed_is_arc = fixed_cursor.current.element.type == ShapeElementType::CircularArc;
        bool orbiting_is_arc = orbiting_cursor.current.element.type == ShapeElementType::CircularArc;

        if (fixed_is_arc && orbiting_is_arc) {
            handle_arc_arc(fixed_cursor, orbiting_cursor, result, current_vertex);
            continue;
        }

        Point fixed_dir = fixed_cursor.start_direction();
        Point orbiting_dir = orbiting_cursor.start_direction();

        if (strictly_lesser_angle(fixed_dir, orbiting_dir)) {
            consume_up_to(fixed_cursor, orbiting_dir, result, current_vertex);
        } else if (strictly_lesser_angle(orbiting_dir, fixed_dir)) {
            consume_up_to(orbiting_cursor, fixed_dir, result, current_vertex);
        } else if (!fixed_is_arc && !orbiting_is_arc) {
            // Tied plain edges. The equalization of the directions is meant
            // for the floating-point noise of arc tangents, but, on unit
            // vectors, it also ties plain edges up to ~1e-6 rad apart that
            // are not parallel. Consuming the fixed one first regardless
            // would then add them in the wrong order whenever the orbiting
            // one has the smaller direction, putting the vertex between them
            // inside the actual Minkowski sum, by up to that angle times the
            // shorter edge's length (e.g. an edge-length wrong vertex, and
            // ~7e-5 off the sum, for two edges ~9.5e-7 rad apart of a
            // self-NFP, see fontanf/packingsolver#598): the result is not
            // even centrally symmetric anymore. So the exact directions of
            // the segments decide, and the fixed one only goes first if they
            // are exactly parallel. (Once consumed, the next element of that
            // side strictly exceeds this tie, so the other one is picked up
            // correctly next.)
            if (strictly_lesser_angle(
                        orbiting_cursor.line_segment_direction(),
                        fixed_cursor.line_segment_direction())) {
                consume_whole(orbiting_cursor, result, current_vertex);
            } else {
                consume_whole(fixed_cursor, result, current_vertex);
            }
        } else if (fixed_is_arc) {
            // Tie at the arc's own start: the plain edge (a single point,
            // contributing nothing beyond it) goes first; the arc, which is
            // only just starting to sweep past this direction, goes next.
            consume_whole(orbiting_cursor, result, current_vertex);
        } else {
            consume_whole(fixed_cursor, result, current_vertex);
        }
    }
    // After all edges, current_vertex == nfp_start (the shape is closed).

    Shape result_shape;
    result_shape.elements = std::move(result);

    // Remove collinear vertices introduced by parallel-edge pairs (arcs are
    // left untouched by this cleanup).
    return remove_aligned_vertices(result_shape).second;
}

MultiShapeWithHoles shape::no_fit_polygon(
        const ShapeWithHoles& fixed_shape,
        const ShapeWithHoles& orbiting_shape)
{
    std::vector<BasicShape> fixed_parts = decompose_into_basic_shapes(fixed_shape).basic_shapes;
    std::vector<BasicShape> orbiting_parts = decompose_into_basic_shapes(orbiting_shape).basic_shapes;

    // Union per fixed_part group first, instead of unioning all
    // fixed_parts.size() * orbiting_parts.size() convex-convex NFP pieces in
    // one shot: the pieces from a single fixed_part are the ones most likely
    // to overlap/be adjacent (they all come from NFPs against the same
    // fixed_part), so this collapses most of the overlap within small
    // groups before the final union only has to merge the (typically far
    // fewer) per-group results.
    std::vector<ShapeWithHoles> group_unions;
    for (const BasicShape& fixed_part: fixed_parts) {
        std::vector<ShapeWithHoles> nfp_parts;
        nfp_parts.reserve(orbiting_parts.size());
        for (const BasicShape& orbiting_part: orbiting_parts) {
            // The NFP is fixed_part + (-orbiting_part). If one of them is a
            // disk, it is the other one inflated by the radius of the disk,
            // shifted by the center of the disk.
            Shape convex_nfp;
            if (fixed_part.type == BasicShapeType::Circle
                    && orbiting_part.type == BasicShapeType::Circle) {
                convex_nfp = build_circle(fixed_part.circle_radius + orbiting_part.circle_radius);
                convex_nfp.shift(
                        fixed_part.circle_center.x - orbiting_part.circle_center.x,
                        fixed_part.circle_center.y - orbiting_part.circle_center.y);
            } else if (fixed_part.type == BasicShapeType::Circle) {
                convex_nfp = inflate(
                        orbiting_part.shape.rotate(180),
                        fixed_part.circle_radius).shape;
                convex_nfp.shift(
                        fixed_part.circle_center.x,
                        fixed_part.circle_center.y);
            } else if (orbiting_part.type == BasicShapeType::Circle) {
                convex_nfp = inflate(
                        fixed_part.shape,
                        orbiting_part.circle_radius).shape;
                convex_nfp.shift(
                        -orbiting_part.circle_center.x,
                        -orbiting_part.circle_center.y);
            } else {
                convex_nfp = no_fit_polygon(fixed_part.shape, orbiting_part.shape);
            }
            nfp_parts.push_back({convex_nfp, {}});
        }
        for (const ShapeWithHoles& swh: compute_union(nfp_parts).shapes_with_holes)
            group_unions.push_back(swh);
    }

    return compute_union(group_unions);
}
