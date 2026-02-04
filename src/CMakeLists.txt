find_package(Threads)
add_library(Shape_shape)
target_sources(Shape_shape PRIVATE
    shape.cpp
    elements_intersections.cpp
    shapes_intersections.cpp
    boolean_operations.cpp
    convex_hull.cpp
    clean.cpp
    extract_borders.cpp
    trapezoidation.cpp
    intersection_tree.cpp
    equalize.cpp
    offset.cpp
    simplification.cpp
    approximation.cpp
    supports.cpp
    writer.cpp)
target_include_directories(Shape_shape PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Shape_shape PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(Shape_shape PUBLIC
    OptimizationTools::containers
    nlohmann_json::nlohmann_json)
set_property(TARGET Shape_shape PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(Shape::shape ALIAS Shape_shape)

add_library(Shape_shape_shared SHARED $<TARGET_OBJECTS:Shape_shape>)
