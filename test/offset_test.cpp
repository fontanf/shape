#include "shape/offset.hpp"

#include <gtest/gtest.h>

#include "test_params.hpp"

using namespace shape;


struct InflateTestParams : TestParams<InflateTestParams>
{
    Shape shape;
    LengthDbl offset = 0;
    ShapeWithHoles expected_result;


    static InflateTestParams from_json(
            nlohmann::basic_json<>& json_item)
    {
        InflateTestParams test_params = TestParams::from_json(json_item);
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.offset = json_item["offset"];
        test_params.expected_result = ShapeWithHoles::from_json(json_item["expected_result"]);
        return test_params;
    }
};

class InflateTest: public testing::TestWithParam<InflateTestParams> { };

TEST_P(InflateTest, Inflate)
{
    InflateTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << "..." << std::endl;
    std::cout << "shape " << test_params.shape.to_string(0) << std::endl;
    std::cout << "offset " << test_params.offset << std::endl;
    std::cout << "expected_result " << test_params.expected_result.to_string(0) << std::endl;

    auto result = inflate(
        test_params.shape,
        test_params.offset);
    std::cout << "result " << result.to_string(0) << std::endl;

    EXPECT_TRUE(equal(result, test_params.expected_result));
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        InflateTest,
        testing::ValuesIn(InflateTestParams::read_dir((fs::path("data") / "tests" / "offset" / "inflate").string())));


struct DeflateTestParams : TestParams<DeflateTestParams>
{
    Shape shape;
    LengthDbl offset = 0;
    std::vector<Shape> expected_result;


    static DeflateTestParams from_json(
            nlohmann::basic_json<>& json_item)
    {
        DeflateTestParams test_params = TestParams::from_json(json_item);
        test_params.shape = Shape::from_json(json_item["shape"]);
        test_params.offset = json_item["offset"];
        for (auto& json_shape: json_item["expected_result"].items())
            test_params.expected_result.emplace_back(Shape::from_json(json_shape.value()));
        return test_params;
    }
};

class DeflateTest: public testing::TestWithParam<DeflateTestParams> { };

TEST_P(DeflateTest, Deflate)
{
    DeflateTestParams test_params = GetParam();
    std::cout << "Testing " << test_params.name << "..." << std::endl;
    std::cout << "hole " << test_params.shape.to_string(0) << std::endl;
    std::cout << "offset " << test_params.offset << std::endl;
    std::cout << "expected_result" << std::endl;
    for (const Shape& hole: test_params.expected_result)
        std::cout << "- " << hole.to_string(2) << std::endl;

    auto result = deflate(
        test_params.shape,
        test_params.offset);
    std::cout << "result" << std::endl;
    for (const Shape& hole: result)
        std::cout << "- " << hole.to_string(2) << std::endl;

    ASSERT_EQ(result.size(), test_params.expected_result.size());
    for (const Shape& expected_hole: test_params.expected_result) {
        EXPECT_NE(std::find(
                      result.begin(),
                      result.end(),
                      expected_hole),
                  result.end());
    }
}

INSTANTIATE_TEST_SUITE_P(
        Shape,
        DeflateTest,
        testing::ValuesIn(DeflateTestParams::read_dir((fs::path("data") / "tests" / "offset" / "deflate").string())));
