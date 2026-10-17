# Conventions

These conventions are shared by the whole library. The comments of the headers in `include/shape/` refer to them instead of repeating them.

## Data model

- A `ShapeElement` is a line segment or a circular arc. A circular arc has a `start`, an `end`, a `center` and an `orientation`: `Anticlockwise`, `Clockwise`, or `Full` for a whole circle, which starts and ends at the same point and goes anticlockwise.
- A `Shape` is a list of elements where the end of each element is the start of the next one. It is closed (the end of the last element is the start of the first one), unless `is_path` is true: a path is an open chain of elements, without interior.
- A `ShapeWithHoles` is an outline (`shape`) and a list of `holes`.
- A `MultiShapeWithHoles` is a list of shapes with holes, standing for their union. It is the output of the boolean operations, since, for example, a union may consist of several disjoint parts.

## Orientation

Every closed shape is anticlockwise: its signed area (`Shape::compute_area`) is positive. **This also holds for the holes**: a hole is stored as an anticlockwise shape, not as a clockwise one.

So, along the outline, the material is on the left of the boundary; along a hole, it is on the right. For example:

- `ShapeElement::normal` points to the right of the element, which is the exterior for the outline;
- a circular arc is convex seen from the material if it is anticlockwise on the outline, or clockwise on a hole;
- a hole which is a full circle is concave seen from the material.

Functions which mirror shapes (`axial_symmetry_*`) reverse the order of the elements so that the results stay anticlockwise.

## Tolerance and scaling

All floating-point comparisons use a fixed absolute tolerance of `1e-6` (see `equal`, `strictly_lesser` and `strictly_greater` in `shape.hpp`), rather than a relative one:

- two values are equal if they differ by at most `1e-6`;
- two points are equal if each of their coordinates are equal: the tolerance region around a point is a square, not a disk.

The library doesn't aim for exact computations. The tolerance absorbs the floating-point errors of the computations made on the shapes before they are given to the library (scaling, rotating, combining shapes...). A point within the tolerance of a boundary is considered on it, and this is the expected behavior.

For this tolerance to be meaningful, **input coordinates must be scaled so that the largest values are on the order of 1000**. Much larger or much smaller objects must be rescaled before use, otherwise the tolerance is too loose or too tight relative to the geometry. Scaling by a power of two (see `largest_power_of_two_lesser_or_equal` and `smallest_power_of_two_greater_or_equal`) is exact in floating-point arithmetic, so the results can be scaled back without rounding errors.

Different functions combine the tolerance differently (for example, a check summing two distances before comparing them to the tolerance), so being "equal within tolerance" isn't always symmetric across functions. The comment of each function says what it compares when it matters.

## Circular arcs

The center of a circular arc isn't required to be exactly at the same distance from its start and its end, since computed centers carry rounding errors. `ShapeElement::recompute_center` replaces the center by its projection on the perpendicular bisector of the start and the end; the boolean operations do this on their inputs, and `recompute_centers` (`clean.hpp`) does it for a whole shape.

## Valid shapes

`Shape::check` and `ShapeWithHoles::check` check that:

- a shape has at least one element, the end of each element is the start of the next one, and it is closed unless it is a path (within tolerance);
- its boundary doesn't intersect itself, apart from consecutive elements sharing their common vertex;
- its area isn't negative, i.e. it is anticlockwise;
- the outline of a shape with holes isn't a path, and each hole is valid and inside the outline.

A valid shape with holes also follows a rule which `check` doesn't enforce yet:

- a hole may touch the outline or another hole **at a single point at most**. Several contact points split the material, so the input should be several shapes with holes; a shared edge means that the outline and the hole, or the two holes, should be merged;
- the outline or a hole never touches itself.

Most functions require valid inputs. Unless their comment says otherwise, invalid inputs may make them throw or return wrong results.
