#include "shape/convex_hull.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include <fstream>

using namespace shape;
namespace fs = boost::filesystem;

struct ConvexHullTestParams
{
    Shape shape;
    bool expect_throw = false;
    // Only meaningful if !expect_throw.
    Shape expected_output;


    static ConvexHullTestParams read_json(
            const std::string& file_path)
    {
        std::ifstream file(file_path);
        if (!file.good()) {
            throw std::runtime_error(
                    FUNC_SIGNATURE + ": "
                    "unable to open file \"" + file_path + "\".");
        }

        nlohmann::json json;
        file >> json;
        ConvexHullTestParams test_params;
        test_params.shape = Shape::from_json(json["shape"]);
        test_params.expected_output = Shape::from_json(json["expected_output"]);
        return test_params;
    }
};

void PrintTo(const ConvexHullTestParams& params, std::ostream* os)
{
    *os << "shape " << params.shape.to_string(0) << "\n";
    *os << "expect_throw " << params.expect_throw << "\n";
    if (!params.expect_throw)
        *os << "expected_output " << params.expected_output.to_string(0) << "\n";
}

class ConvexHullTest: public testing::TestWithParam<ConvexHullTestParams> { };

TEST_P(ConvexHullTest, ConvexHull)
{
    ConvexHullTestParams test_params = GetParam();
    PrintTo(test_params, &std::cout);

    if (test_params.expect_throw) {
        EXPECT_THROW(shape::convex_hull(test_params.shape), std::invalid_argument);
        return;
    }

    Shape output = shape::convex_hull(test_params.shape);
    std::cout << "output " << output.to_string(0) << std::endl;
    EXPECT_EQ(output, test_params.expected_output);
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ConvexHullTest,
        testing::ValuesIn(std::vector<ConvexHullTestParams>{
            {
                // Already-convex triangle: hull is the shape itself.
                build_shape({{0, 0}, {3, 0}, {1, 3}}),
                false,
                build_shape({{0, 0}, {3, 0}, {1, 3}}),
            }, {
                // Concave pentagon: reflex angle at (2, 1) is dropped from
                // the hull.
                build_shape({{0, 0}, {4, 0}, {2, 1}, {4, 4}, {0, 4}}),
                false,
                build_shape({{0, 0}, {4, 0}, {4, 4}, {0, 4}}),
            }, {
                // convex_hull only looks at each element's start point, so a
                // shape with a circular arc bulging past the chord between
                // its endpoints used to silently produce a hull that
                // doesn't actually contain the input shape (found via
                // fuzzing). Convex hulls of arc-containing shapes aren't
                // supported yet; reject them clearly instead.
                build_shape({{0, 0}, {1, 0}, {0, 0, 1}, {0, 1}}),
                true,
                {},
            }, {
                // (-1, 1e-9) is collinear, within the tolerance, with the
                // lowest point (0, 0) and (10, 0), but on the left of
                // (0, 0). With (0, 0) as the pivot, it was sorted as if it
                // were on its right, and, depending on the implementation of
                // std::sort, dropped from the hull. The points are now sorted
                // by their exact angle around (0, 0), and (-1, 1e-9) comes
                // last.
                build_shape({{0, 0}, {10, 0}, {10, 10}, {-1, 10}, {-1, 1e-9}}),
                false,
                build_shape({{-1, 1e-9}, {0, 0}, {10, 0}, {10, 10}, {-1, 10}}),
            }, {
                // The two lowest points are close to each other. The points
                // were sorted around the lowest one with 'counter_clockwise',
                // whose collinearity check is not symmetric: a near point can
                // be on the line through the lowest point and a far point
                // within the tolerance while the far point isn't on the line
                // through the lowest point and the near point. The sort was
                // then inconsistent and the output wasn't convex.
                build_shape({
                        {-162.14686580729096, -159.75894183525972},
                        {-281.52374304048334, -251.03896141876899},
                        {-281.52374501221811, -251.03896332651928},
                        {24.499879567359496, -148.94909807684283},
                        {17.84085330125453, -71.889757366578394},
                        {160.84969528197314, 162.07757174225461}}),
                false,
                build_shape({
                        {-281.52374501221811, -251.03896332651928},
                        {24.499879567359496, -148.94909807684283},
                        {160.84969528197314, 162.07757174225461},
                        {-281.52374304048334, -251.03896141876899}}),
            }, {
                // Several vertices aligned horizontally with the lowest
                // vertex: the furthest one, (2, 0), is a vertex of the hull.
                build_shape({{0, 0}, {1, 0}, {2, 0}, {2, 2}, {1, 1}, {0, 2}}),
                false,
                build_shape({{0, 0}, {2, 0}, {2, 2}, {0, 2}}),
            },
            ConvexHullTestParams::read_json(
                    (fs::path("data") / "tests" / "convex_hull" / "0.json").string()),
        }),
        [](const testing::TestParamInfo<ConvexHullTest::ParamType>& info) {
            return std::to_string(info.index);
        });
