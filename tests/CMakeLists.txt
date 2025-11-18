# TweakXL-macOS Tests

include(CTest)
include(Catch)

# Test sources
set(TEST_SOURCES
    core/TweakDBID_test.cpp
    core/Value_test.cpp
    core/Flat_test.cpp
    core/Record_test.cpp
    tweakdb/BinaryReader_test.cpp
    tweakdb/TweakDBReader_test.cpp
    parsers/YAMLParser_test.cpp
    integration/ModPipeline_test.cpp
)

# Create test executable
add_executable(tweakxl-tests
    ${TEST_SOURCES}
    test_main.cpp
)

target_link_libraries(tweakxl-tests PRIVATE
    tweakxl-core
    Catch2::Catch2WithMain
)

# Discover tests
catch_discover_tests(tweakxl-tests)

# Add custom target to run tests
add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tweakxl-tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
