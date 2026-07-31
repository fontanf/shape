#include "shape/no_fit_polygon.hpp"

#include "shape/boolean_operations.hpp"
#include "shape/clean.hpp"
#include "shape/basic_shapes.hpp"

#include <algorithm>
#include <string>

using namespace shape;

namespace
{

/**
 * Same total order as shape::strictly_lesser_angle, but tolerant of the tiny
 * floating-point noise (on the order of 1e-16) that sqrt/trig-derived
 * CircularArc tangent directions carry relative to tangents computed
 * directly from stored points (e.g. a plain edge's direction, or another
 * arc's tangent obtained through a different center/radius). By
 * construction, an edge's direction is exactly equal to the tangent of the
 * arc it leads into (both are the external tangent to that arc's circle),
 * so the two *should* compare equal -- but shape::strictly_lesser_angle's
 * own cross-product test has no tolerance at all (a bare "> 0"), so any
 * nonzero noise gets treated as a strict order instead of a tie. For plain
 * polygons this never bites (both directions are typically bit-identical,
 * being derived from the same stored points via the same arithmetic), but
 * once arcs are involved -- especially for a self-NFP, or any pair of
 * shapes sharing rounded-corner structure -- such near-ties are frequent,
 * and silently corrupt the tie-handling logic below every time one occurs.
 */
bool direction_strictly_lesser(
        const Point& direction_1,
        const Point& direction_2)
{
    if (equal(direction_1.x, direction_2.x) && equal(direction_1.y, direction_2.y))
        return false;
    return strictly_lesser_angle(direction_1, direction_2);
}

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
    LengthDbl radius = distance(arc.center, arc.start);
    return point_at_tangent_direction(arc.center, radius, arc.orientation, target_direction);
}

/**
 * True if 'direction' lies strictly between arc's start and end tangent
 * directions (i.e. strictly inside the open span, not at either endpoint).
 *
 * no_fit_polygon requires every arc to span < 180° (see its precondition
 * check), so a direction is strictly inside iff turning CCW from the start
 * tangent to 'direction' is a positive turn, and turning CCW from
 * 'direction' to the end tangent is also a positive turn -- two plain
 * cross-product sign tests, no wraparound handling needed precisely because
 * the span cannot reach 180°.
 */
bool direction_strictly_inside_arc(
        const ShapeElement& arc,
        const Point& direction)
{
    Point tangent_start = arc.tangent(arc.start);
    Point tangent_end = arc.tangent(arc.end);
    LengthDbl cross_1 = tangent_start.x * direction.y - tangent_start.y * direction.x;
    LengthDbl cross_2 = direction.x * tangent_end.y - direction.y * tangent_end.x;
    return strictly_greater(cross_1, 0.0) && strictly_greater(cross_2, 0.0);
}

/**
 * Split 'arc' into consecutive sub-arc pieces at every direction in
 * 'target_directions' (given in the arc's own local frame) that falls
 * strictly inside its span, ordered along the arc from its own start to its
 * own end. Returns {arc} unchanged if none do.
 */
std::vector<ShapeElement> split_arc_into_pieces(
        const ShapeElement& arc,
        const std::vector<Point>& target_directions)
{
    std::vector<Point> split_points;
    for (const Point& direction: target_directions)
        if (direction_strictly_inside_arc(arc, direction))
            split_points.push_back(point_at_tangent_direction(arc, direction));

    if (split_points.empty())
        return {arc};

    std::sort(
            split_points.begin(),
            split_points.end(),
            [&arc](const Point& p1, const Point& p2)
            {
                return arc.length(p1) < arc.length(p2);
            });

    std::vector<ShapeElement> pieces;
    Point piece_start = arc.start;
    for (const Point& split_point: split_points) {
        if (equal(piece_start, split_point))
            continue;
        pieces.push_back(arc.extract(piece_start, split_point));
        piece_start = split_point;
    }
    if (!equal(piece_start, arc.end))
        pieces.push_back(arc.extract(piece_start, arc.end));
    // All split points coincided with piece_start (degenerate/duplicate
    // directions): fall back to the arc unsplit rather than an empty list.
    if (pieces.empty())
        return {arc};
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
 * 'negate_self'/'negate_other' select which of 'elements'/'other_elements'
 * is the orbiting side, whose directions get negated for output space (see
 * EdgeCursor below); both direction sets need to end up expressed in each
 * arc's own local frame to be tested/split against it.
 */
std::vector<ShapeElement> presplit_elements(
        const std::vector<ShapeElement>& elements,
        bool negate_self,
        const std::vector<ShapeElement>& other_elements,
        bool negate_other)
{
    bool has_arc = false;
    for (const ShapeElement& element: elements) {
        if (element.type == ShapeElementType::CircularArc) {
            has_arc = true;
            break;
        }
    }
    if (!has_arc)
        return elements;

    // Critical directions depend only on other_elements (and the negation
    // flags), never on which or how many arcs 'elements' itself has, so
    // they only need computing once and are then reused for every arc
    // found below.
    std::vector<Point> critical_directions;
    for (const ShapeElement& other: other_elements) {
        Point d_start = other.tangent(other.start);
        if (negate_other)
            d_start = {-d_start.x, -d_start.y};
        if (negate_self)
            d_start = {-d_start.x, -d_start.y};
        critical_directions.push_back(d_start);
        if (other.type == ShapeElementType::CircularArc) {
            Point d_end = other.tangent(other.end);
            if (negate_other)
                d_end = {-d_end.x, -d_end.y};
            if (negate_self)
                d_end = {-d_end.x, -d_end.y};
            critical_directions.push_back(d_end);
        }
    }

    // Also always split at the global tangent-order minimum direction (1,0)
    // [negated to (-1,0) in local frame if this is the orbiting side]: if
    // an arc's span straddles it, the piece starting exactly there needs
    // to exist so the start-position selection below can find it. Without
    // this, an arc whose span straddles that direction can undercut every
    // other candidate's own start direction (its tangent dips below all of
    // them partway through, past the comparator's wraparound point) without
    // any element actually starting there for the selection to pick.
    critical_directions.push_back(negate_self? Point{-1.0, 0.0}: Point{1.0, 0.0});

    std::vector<ShapeElement> result;
    result.reserve(elements.size());
    for (const ShapeElement& element: elements) {
        if (element.type != ShapeElementType::CircularArc) {
            result.push_back(element);
            continue;
        }
        for (const ShapeElement& piece: split_arc_into_pieces(element, critical_directions))
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
    const std::vector<ShapeElement>* elements = nullptr;
    ElementPos next_pos = 0;
    bool negate = false;

    // The element currently at the front of the cursor (already popped from
    // 'elements' conceptually; may be a suffix of an original element after
    // a partial consumption). Only meaningful if has_current() is true.
    ShapeElement current;
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

    /** Tangent direction at the current element's start, in output space (negated for orbiting). */
    Point start_direction() const
    {
        Point t = current.tangent(current.start);
        return negate? Point{-t.x, -t.y}: t;
    }

    /** Tangent direction at the current element's end, in output space (negated for orbiting). */
    Point end_direction() const
    {
        Point t = current.tangent(current.end);
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
        Point split_point = point_at_tangent_direction(current, local_target);
        ShapeElement prefix = current.extract(current.start, split_point);
        current = current.extract(split_point, current.end);
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
    ShapeElement placed = cursor.place(cursor.current, current_vertex);
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
    if (cursor.current.type == ShapeElementType::CircularArc
            && direction_strictly_inside_arc(
                    cursor.current,
                    cursor.negate? Point{-other_direction.x, -other_direction.y}: other_direction)) {
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
    if (strictly_greater(d_start, M_PI))
        d_start -= 2.0 * M_PI;

    // No overlap: o's span starts at/after f's end (d_start >= f_span), or
    // f's span starts at/after o's end (-d_start >= o_span).
    if (!strictly_lesser(d_start, f_span)) {
        consume_whole(fixed, result, current_vertex);
        return;
    }
    if (!strictly_lesser(-d_start, o_span)) {
        consume_whole(orbiting, result, current_vertex);
        return;
    }

    // Overlap exists. Peel off any lone prefix (at most one side has one:
    // whichever start has the strictly later, i.e. positive-offset, side).
    bool fixed_has_prefix = strictly_greater(d_start, 0.0);
    bool orbiting_has_prefix = strictly_lesser(d_start, 0.0);
    Point overlap_start = fixed_has_prefix? o_start: f_start;
    if (fixed_has_prefix)
        consume_up_to(fixed, overlap_start, result, current_vertex);
    if (orbiting_has_prefix)
        consume_up_to(orbiting, overlap_start, result, current_vertex);

    // Both cursors now start exactly at overlap_start. Determine how far the
    // overlap extends (same local/signed reasoning, now relative to the two
    // end directions), splitting off whichever side's remainder is longer.
    Angle d_end = angle_radian(f_end, o_end);
    if (strictly_greater(d_end, M_PI))
        d_end -= 2.0 * M_PI;
    bool fixed_has_suffix = strictly_lesser(d_end, 0.0);
    bool orbiting_has_suffix = strictly_greater(d_end, 0.0);
    Point overlap_end = fixed_has_suffix? o_end: f_end;

    ShapeElement fixed_local_piece = fixed.current;
    bool fixed_will_advance = !fixed_has_suffix;
    if (fixed_has_suffix)
        fixed_local_piece = fixed.split_current_at(overlap_end);
    ShapeElement orbiting_local_piece = orbiting.current;
    bool orbiting_will_advance = !orbiting_has_suffix;
    if (orbiting_has_suffix)
        orbiting_local_piece = orbiting.split_current_at(overlap_end);

    // Sum the two (local, unplaced) overlap pieces into a single arc.
    LengthDbl fixed_radius = distance(fixed_local_piece.center, fixed_local_piece.start);
    LengthDbl orbiting_radius = distance(orbiting_local_piece.center, orbiting_local_piece.start);
    Point orbiting_neg_center = {-orbiting_local_piece.center.x, -orbiting_local_piece.center.y};
    Point orbiting_neg_start = {-orbiting_local_piece.start.x, -orbiting_local_piece.start.y};
    Point combined_center = current_vertex
        + (fixed_local_piece.center - fixed_local_piece.start)
        + (orbiting_neg_center - orbiting_neg_start);
    LengthDbl combined_radius = fixed_radius + orbiting_radius;
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
    // Every arc must span < 180°: direction_strictly_inside_arc's
    // two-cross-product membership test and handle_arc_arc's overlap
    // detection both rely on an arc's own start and end tangent directions
    // determining a unique "short way" between them, which stops being true
    // at or past a half turn (see direction_strictly_inside_arc's and
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

    // Pre-split each side's arcs (if any), in their original (unrotated)
    // index order, against the other side's critical directions -- see
    // presplit_elements' comment for why this can't be done reactively
    // during the merge below, and for why it also always splits at the
    // global tangent-order minimum direction. Both calls read from the
    // shapes' own original element lists (not from each other's already-
    // split result) so the two splits don't depend on each other's outcome.
    std::vector<ShapeElement> fixed_elements = presplit_elements(
            fixed_shape.elements, false, orbiting_shape.elements, true);
    std::vector<ShapeElement> orbiting_elements = presplit_elements(
            orbiting_shape.elements, true, fixed_shape.elements, false);

    // Find the element whose start tangent direction is the global minimum
    // (per strictly_lesser_angle's total order) among fixed_elements, and
    // the element whose *negated* start tangent direction is the global
    // minimum among orbiting_elements. Starting the traced sequence at that
    // element guarantees every subsequent element's direction increases
    // monotonically all the way around before wrapping back, exactly once,
    // to close the loop -- which the merge below relies on.
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
    ElementPos fixed_start_pos = 0;
    for (ElementPos fixed_pos = 1;
            fixed_pos < (ElementPos)fixed_elements.size();
            ++fixed_pos) {
        const ShapeElement& element = fixed_elements[fixed_pos];
        const ShapeElement& best = fixed_elements[fixed_start_pos];
        if (direction_strictly_lesser(
                element.tangent(element.start),
                best.tangent(best.start))) {
            fixed_start_pos = fixed_pos;
        }
    }

    ElementPos orbiting_start_pos = 0;
    for (ElementPos orbiting_pos = 1;
            orbiting_pos < (ElementPos)orbiting_elements.size();
            ++orbiting_pos) {
        const ShapeElement& element = orbiting_elements[orbiting_pos];
        const ShapeElement& best = orbiting_elements[orbiting_start_pos];
        Point element_dir = element.tangent(element.start);
        Point best_dir = best.tangent(best.start);
        if (direction_strictly_lesser(
                Point{-element_dir.x, -element_dir.y},
                Point{-best_dir.x, -best_dir.y})) {
            orbiting_start_pos = orbiting_pos;
        }
    }

    // Starting vertex of the NFP: bottom of fixed minus top of orbiting.
    const Point nfp_start = fixed_elements[fixed_start_pos].start
        - orbiting_elements[orbiting_start_pos].start;

    const ElementPos fixed_num_elements = (ElementPos)fixed_elements.size();
    const ElementPos orbiting_num_elements = (ElementPos)orbiting_elements.size();

    {
        std::vector<ShapeElement> fixed_elements_rotated(fixed_num_elements);
        for (ElementPos i = 0; i < fixed_num_elements; ++i)
            fixed_elements_rotated[i] = fixed_elements[(fixed_start_pos + i) % fixed_num_elements];
        fixed_elements = std::move(fixed_elements_rotated);
    }
    {
        std::vector<ShapeElement> orbiting_elements_rotated(orbiting_num_elements);
        for (ElementPos i = 0; i < orbiting_num_elements; ++i)
            orbiting_elements_rotated[i] = orbiting_elements[(orbiting_start_pos + i) % orbiting_num_elements];
        orbiting_elements = std::move(orbiting_elements_rotated);
    }

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

        bool fixed_is_arc = fixed_cursor.current.type == ShapeElementType::CircularArc;
        bool orbiting_is_arc = orbiting_cursor.current.type == ShapeElementType::CircularArc;

        if (fixed_is_arc && orbiting_is_arc) {
            handle_arc_arc(fixed_cursor, orbiting_cursor, result, current_vertex);
            continue;
        }

        Point fixed_dir = fixed_cursor.start_direction();
        Point orbiting_dir = orbiting_cursor.start_direction();

        if (direction_strictly_lesser(fixed_dir, orbiting_dir)) {
            consume_up_to(fixed_cursor, orbiting_dir, result, current_vertex);
        } else if (direction_strictly_lesser(orbiting_dir, fixed_dir)) {
            consume_up_to(orbiting_cursor, fixed_dir, result, current_vertex);
        } else if (!fixed_is_arc && !orbiting_is_arc) {
            // Tied plain edges: fixed first, then orbiting (matches the
            // pre-arc-support behavior exactly, just spread over two loop
            // iterations instead of one: fixed's next element strictly
            // exceeds this tie, so orbiting is picked up correctly next).
            consume_whole(fixed_cursor, result, current_vertex);
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
            Shape convex_nfp = no_fit_polygon(fixed_part.shape, orbiting_part.shape);
            nfp_parts.push_back({convex_nfp, {}});
        }
        for (const ShapeWithHoles& swh: compute_union(nfp_parts).shapes_with_holes)
            group_unions.push_back(swh);
    }

    return compute_union(group_unions);
}
