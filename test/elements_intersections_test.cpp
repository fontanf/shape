#include "shape/elements_intersections.hpp"
//#include "shape/writer.hpp"

#include <gtest/gtest.h>

#include <fstream>

using namespace shape;


struct ComputeIntersectionsTestParams
{
    ShapeElement element_1;
    ShapeElement element_2;
    ShapeElementIntersectionsOutput expected_output;

    template <class basic_json>
    static ComputeIntersectionsTestParams from_json(
            basic_json& json_item)
    {
        ComputeIntersectionsTestParams test_params;
        test_params.element_1 = ShapeElement::from_json(json_item["element_1"]);
        test_params.element_2 = ShapeElement::from_json(json_item["element_2"]);
        for (auto& json_element: json_item["expected_output"]["overlapping_parts"].items())
            test_params.expected_output.overlapping_parts.emplace_back(ShapeElement::from_json(json_element.value()));
        for (auto& json_point: json_item["expected_output"]["improper_intersections"].items())
            test_params.expected_output.improper_intersections.emplace_back(Point::from_json(json_point.value()));
        for (auto& json_point: json_item["expected_output"]["proper_intersections"].items())
            test_params.expected_output.proper_intersections.emplace_back(Point::from_json(json_point.value()));
        return test_params;
    }

    static ComputeIntersectionsTestParams read_json(
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
        return from_json(json);
    }
};

class ComputeIntersectionsTest: public testing::TestWithParam<ComputeIntersectionsTestParams> { };

TEST_P(ComputeIntersectionsTest, ComputeIntersections)
{
    ComputeIntersectionsTestParams test_params = GetParam();
    std::cout << "element_1 " << test_params.element_1.to_string() << std::endl;
    std::cout << "element_2 " << test_params.element_2.to_string() << std::endl;
    std::cout << "expected_output" << std::endl;
    std::cout << "  " << test_params.expected_output.to_string(2) << std::endl;
    //Writer().add_element(test_params.element_1).add_element(test_params.element_2).write_json("elements_intersections_input.json");

    ShapeElementIntersectionsOutput intersections = compute_intersections(
            test_params.element_1,
            test_params.element_2);
    std::cout << "output" << std::endl;
    std::cout << "  " << intersections.to_string(2) << std::endl;

    ASSERT_EQ(intersections.overlapping_parts.size(), test_params.expected_output.overlapping_parts.size());
    for (const auto& expected_intersection: test_params.expected_output.overlapping_parts) {
        EXPECT_NE(std::find_if(
                    intersections.overlapping_parts.begin(),
                    intersections.overlapping_parts.end(),
                    [&expected_intersection](const ShapeElement& overlapping_part) { return equal(overlapping_part, expected_intersection) || equal(overlapping_part.reverse(), expected_intersection); }),
                intersections.overlapping_parts.end());
    }
    ASSERT_EQ(intersections.improper_intersections.size(), test_params.expected_output.improper_intersections.size());
    for (const Point& expected_intersection: test_params.expected_output.improper_intersections) {
        EXPECT_NE(std::find_if(
                    intersections.improper_intersections.begin(),
                    intersections.improper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.improper_intersections.end());
    }
    ASSERT_EQ(intersections.proper_intersections.size(), test_params.expected_output.proper_intersections.size());
    for (const Point& expected_intersection: test_params.expected_output.proper_intersections) {
        EXPECT_NE(std::find_if(
                    intersections.proper_intersections.begin(),
                    intersections.proper_intersections.end(),
                    [&expected_intersection](const Point& point) { return equal(point, expected_intersection); }),
                intersections.proper_intersections.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        ComputeIntersectionsTest,
        testing::ValuesIn(std::vector<ComputeIntersectionsTestParams>{
            {  // Non-intersecting line segments
                build_line_segment({0, 0}, {0, 1}),
                build_line_segment({1, 0}, {1, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segments
                build_line_segment({11, 1}, {10, 2}),
                build_line_segment({9, 3}, {11, 3}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segments
                build_line_segment({31.49606296, 0}, {31.49606296, 9.448818519999994}),
                build_line_segment({31.49606295999999, 134.8031497200001}, {31.49606296, 144.25196848}),
                {{}, {}, {}},
            }, {  // Simple line segment intersection.
                build_line_segment({1, 0}, {1, 2}),
                build_line_segment({0, 1}, {2, 1}),
                {{}, {}, {{1, 1}}},
            }, {  // One line segment touching another.
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 1}, {2, 1}),
                {{}, {{0, 1}}, {}},
            }, {  // Two identical line segments.
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 0}, {0, 2}),
                {{build_line_segment({0, 0}, {0, 2})}, {}, {}},
            }, {  // Two identical line segments (reversed).
                build_line_segment({0, 0}, {0, 2}),
                build_line_segment({0, 2}, {0, 0}),
                {{build_line_segment({0, 0}, {0, 2})}, {}, {}},
            }, {  // Two overlapping line segments.
                build_line_segment({0, 0}, {0, 3}),
                build_line_segment({0, 1}, {0, 4}),
                {{build_line_segment({0, 1}, {0, 3})}, {}, {}},
            }, {  // Two overlapping line segments.
                build_line_segment({144.25196848, 31.49606296}, {134.8031497200001, 31.49606295999999}),
                build_line_segment({135.4330709199999, 31.49606296}, {8.818897719999987, 31.49606296}),
                {{build_line_segment({134.8031497200001, 31.49606296}, {135.4330709199999, 31.49606296})}, {}, {}},
            }, {  // Two touching colinear line segments.
                build_line_segment({0, 0}, {0, 1}),
                build_line_segment({0, 1}, {0, 2}),
                {{}, {{0, 1}}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({2, 0}, {2, 2}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {0, 1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({1, 1}, {2, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_circular_arc({2, 0}, {0, 2}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({2, 1}, {3, 1}),
                {{}, {}, {}},
            }, {  // Non-intersecting line segment and circular arc.
                build_line_segment({15, 0}, {35, 0}),
                build_circular_arc({2.5, -9.682458365518542}, {2.5, 9.682458365518542}, {5, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {}},
            }, {  // Intersecting line segment and circular arc.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({0, 0}, {0, 2}),
                {{}, {}, {{0, 1}}},
            }, {  // Intersecting line segment and circular arc.
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                build_line_segment({0, 0}, {0, 2}),
                {{}, {}, {{0, 1}}},
            }, {  // Intersecting line segment and circular arc at two points.
                build_line_segment({-1, -2}, {-1, 2}),
                build_circular_arc({1, 1}, {1, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{-1, -1}, {-1, 1}}},
            }, {
                build_line_segment({39.2075327238964, 921.938482687602}, {39.2004011663701, 921.949097066976}),
                build_circular_arc({39.2075268512914, 921.938491537799}, {39.2075327238964, 921.938482687602}, {39.2066965548415, 921.937934215805}, ShapeElementOrientation::Clockwise),
                {{}, {{39.2075327238964, 921.938482687602}}, {}},
            }, {
                build_line_segment({398683.828041/(1<<14), 9213001.274628/(1<<14)}, {398845.790901/(1<<14), 9213041.800811/(1<<14)}),
                build_circular_arc({398645.799546/(1<<14), 9213137.752427/(1<<14)}, {398972.799200/(1<<14), 9213016.000000/(1<<14)}, {398972.799200/(1<<14), 9213516.000000/(1<<14)}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{398828.138764737/(1<<14), 9213037.38391307/(1<<14)}}},
            }, {  // Touching line segment and circular arc.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({-1, 1}, {1, 1}),
                {{}, {{0, 1}}, {}},
            }, {  // Touching line segment and circular arc at two points.
                build_line_segment({2, 0}, {-2, 0}),
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1, 0}, {-1, 0}}, {}},
            }, {  // Touching line segment and circular arc.
                build_circular_arc({5, -1}, {7, -3}, {7, -1}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({6, 5}, {7, -3}),
                {{}, {{7, -3}}, {}},
            }, {  // Non-intersecting circular arcs.
                build_circular_arc({2, 0}, {0, 2}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({3, 0}, {1, 2}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({3, 0}, {-1, 0}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({1, 0}, {-3, 0}, { -1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({3, 0}, {-1, 0}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-3, 0}, {1, 0}, {-1, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs.
                build_circular_arc({-1, 0}, {3, 0}, {1, 0}, ShapeElementOrientation::Clockwise),
                build_circular_arc({-3, 0}, {1, 0}, {-1, 0}, ShapeElementOrientation::Clockwise),
                {{}, {}, {{0, 1.73205080756888}}},
            }, {  // Intersecting circular arcs at two points.
                build_circular_arc({-2, 1}, {-2, -1}, {-1, 0}, ShapeElementOrientation::Clockwise),
                build_circular_arc({2, 1}, {2, -1}, {1, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {}, {{0, -1}, {0, 1}}},
            }, {  // Touching circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1, 0}, {-1, 0}}, {}},
            }, {  // Identical circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {  // Identical circular arcs (reversed).
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({-1, 0}, {1, 0}, {0, 0}, ShapeElementOrientation::Clockwise),
                {{build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {  // Overlapping circular arcs.
                build_circular_arc({1, 0}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({0, 1}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {{build_circular_arc({0, 1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise)}, {}, {}},
            }, {  // Overlapping circular arcs with two overlapping parts.
                build_circular_arc({0, 1}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({0, -1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                {
                    {
                        build_circular_arc({0, 1}, {-1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                        build_circular_arc({0, -1}, {1, 0}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                    }, {}, {}
                },
            }, {
                build_line_segment({1.96721311, -0.78740157}, {60.98360656, -0.78740157}),
                build_circular_arc({1.941450017182601, -0.7869799841717368}, {1.96721311, -0.78740157}, {1.96721311, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{1.96721311, -0.78740157}}, {}},
            }, {
                build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({-2, 0}, {1, 0}),
                {{}, {{1, 0}}, {{-1, 0}}},
            }, {
                build_circular_arc({1, 0}, {0, -1}, {0, 0}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({1, 0}, {1, 1}),
                {{}, {{1, 0}}, {}},
            }, {
                build_line_segment({23.5210023, 10.26937568}, {23.62204724, 11.81102362}),
                build_line_segment({23.62204724, 14.18786926646811}, {23.62204724, 11.81102362}),
                {{}, {{23.62204724, 11.81102362}}, {}},
            }, {
                build_line_segment({23.62204724, 11.81102362}, {23.62204724, 23.62204724}),
                build_line_segment({27.43352932, 18.85373163}, {23.62204724, 14.18786926646811}),
                {{}, {{23.62204724, 14.18786926646811}}, {}},
            }, {
                build_line_segment({6.146061335761145, 4.1594407268466}, {6.354632555761145, 4.0610510468466}),
                build_line_segment({6.180307632126364, 3.737782318431193}, {6.180307632126364, 4.143285659332687}),
                {{}, {{6.180307632126364, 4.143285659332687}}, {}},
            }, {
                build_line_segment({100, 150}, {0, 0}),
                build_line_segment({33.33333333333334, 50}, {50, 75}),
                {{build_line_segment({33.33333333333334, 50}, {50, 75})}, {}, {}},
            }, {  // Overlapping line segments with numerical issues.
                build_line_segment({56.45661773889309, 154.3210357234225}, {74.21458856416608, 160.7844084041661}),
                build_line_segment({67.70333256416608, 158.4145050441661}, {85.46130335612965, 164.8778778164265}),
                {{build_line_segment({67.70333256416608, 158.4145050441661}, {74.21458856416608, 160.7844084041661})}, {}, {}},
            }, {
                build_line_segment({-7.366927306090344e-05, 10.55946821808402}, {5.13599999957334e-05, 15.2838776652485}),
                build_line_segment({3.07199999957334e-05, 13.7089063052485}, {9.249437985040833e-05, 18.43331575366353}),
                {{}, {{5.13599999957334e-05, 15.2838776652485}}, {}},
            }, {
                build_line_segment({5.13599999957334e-05, 15.2838776652485}, {-7.366927306090344e-05, 10.55946821808402}),
                build_line_segment({3.07199999957334e-05, 13.7089063052485}, {9.249437985040833e-05, 18.43331575366353}),
                {{}, {{5.13599999957334e-05, 15.2838776652485}}, {}},
            }, {
                build_circular_arc({76.06499362185757, 125.0425051718358}, {74.96691740817168, 120.9444289625928}, {77.56499362102196, 122.44442896}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({74.96691740817168, 120.9444289625928}, {70.86884114060604, 122.0425051711132}, {72.36884114102195, 119.44442896}, ShapeElementOrientation::Anticlockwise),
                {{}, {{74.96691740817168, 120.9444289625928}}, {}},
            }, {
                build_circular_arc({238.83881584236954, 217.49258032}, {235.84334613525346, 214.65738697566024}, {238.83881584236954, 214.49258032}, ShapeElementOrientation::Anticlockwise),
                build_line_segment({238.26347654820481, 217.49258032}, {237.72895727304757, 217.27973190470124}),
                {{}, {{237.72895727304757, 217.27973190470124}}, {}},
            }, {
                build_line_segment({183.39919911092201, 729.40916251034298}, {183.41937227791138, 729.31121842765481}),
                build_line_segment({183.39920460245543, 729.40916364125258}, {183.43953995336733, 729.21327321405704}),
                {{}, {{183.41937227791138, 729.31121842765481}}, {}},
            }, {
                build_line_segment({183.39920460245543, 729.40916364125258}, {183.43953995336733, 729.21327321405704}),
                build_line_segment({183.39919911092201, 729.40916251034298}, {183.41937227791138, 729.31121842765481}),
                {{}, {{183.41937227791138, 729.31121842765481}}, {}},
            }, {
                build_line_segment({178.02228433503524, 725.01878331495016}, {179.86580017703071, 728.02975619768279}),
                build_line_segment({179.69546574465804, 728.13457023003161}, {179.86556593933292, 728.0293755354337}),
                {{}, {{179.86556593933292, 728.0293755354337}}, {}},
            }, {
                build_line_segment({178.02228433503524, 725.01878331495016}, {179.86580017703071, 728.02975619768279}),
                build_line_segment({179.86556593933292, 728.0293755354337}, {179.69546574465804, 728.13457023003161}),
                {{}, {{179.86556593933292, 728.0293755354337}}, {}},
            }, {
                build_line_segment({183.6076093704865, 729.46924222308246}, {183.64265921689528, 729.37558586218279}),
                build_line_segment({183.60760024465787, 729.46923880733618}, {183.6076093704865, 729.46924222308246}),
                {{}, {{183.6076093704865, 729.46924222308246}}, {}},
            }, {
                build_line_segment({183.64265921689528, 729.37558586218279}, {183.6076093704865, 729.46924222308246}),
                build_line_segment({183.60760024465787, 729.46923880733618}, {183.6076093704865, 729.46924222308246}),
                {{}, {{183.6076093704865, 729.46924222308246}}, {}},
            }, {
                build_line_segment({180.7738429642061, 728.6642298308816}, {183.43954544490074, 729.21327434496663}),
                build_line_segment({183.43954544490074, 729.21327434496663}, {183.43953995336733, 729.21327321405704}),
                {{build_line_segment({183.43954544490074, 729.21327434496663}, {183.43953995336733, 729.21327321405704})}, {}, {}},
            }, {
                build_line_segment({183.43954544490074, 729.21327434496663}, {183.43953995336733, 729.21327321405704}),
                build_line_segment({180.7738429642061, 728.6642298308816}, {183.43954544490074, 729.21327434496663}),
                {{build_line_segment({183.43954544490074, 729.21327434496663}, {183.43953995336733, 729.21327321405704})}, {}, {}},
            }, {
                build_line_segment({161.27721259797397, 170.77573182212603}, {175.94670963797395, 145.36689766212604}),
                build_line_segment({175.94672295355016, 145.36687463796503}, {161.27700079355017, 170.77557879796504}),
                {{}, {{175.94670963797395, 145.36689766212604}}, {}},
            }, {
                build_line_segment({175.94672295355016, 145.36687463796503}, {161.27700079355017, 170.77557879796504}),
                build_line_segment({161.27721259797397, 170.77573182212603}, {175.94670963797395, 145.36689766212604}),
                {{}, {{175.94670963797395, 145.36689766212604}}, {}},
            }, {
                build_line_segment({13.855000461167737, -4.0098356572654881}, {14.165710131167737, -4.0711615672654879}),
                build_line_segment({14.163825378659944, -4.0707893786320799}, {14.477859588659943, -4.1328341486320799}),
                {{}, {{14.165710131167737, -4.0711615672654879}}, {}},
            }, {
                build_line_segment({14.165710131167737, -4.0711615672654879}, {13.855000461167737, -4.0098356572654881}),
                build_line_segment({14.163825378659944, -4.0707893786320799}, {14.477859588659943, -4.1328341486320799}),
                {{}, {{14.165710131167737, -4.0711615672654879}}, {}},
            }, {
                build_circular_arc({2.5, -9.6824583655185421}, {2.5, 9.6824583655185421}, {4.4408920985006262e-16, 0}, ShapeElementOrientation::Anticlockwise),
                build_circular_arc({2.5, 9.6824583655185421}, {2.5, -9.6824583655185421}, {5, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{2.5, -9.6824583655185421}, {2.5, 9.6824583655185421}}, {}},
            }, {
                build_circular_arc({2.5, 9.6824583655185421}, {2.5, -9.6824583655185421}, {4.4408920985006262e-16, 0}, ShapeElementOrientation::Clockwise),
                build_circular_arc({2.5, 9.6824583655185421}, {2.5, -9.6824583655185421}, {5, 0}, ShapeElementOrientation::Anticlockwise),
                {{}, {{2.5, -9.6824583655185421}, {2.5, 9.6824583655185421}}, {}},
            }}));
