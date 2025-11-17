# TweakXL-macOS Core Library

# Core library sources
set(CORE_SOURCES
    core/TweakDBID.cpp
    core/Value.cpp
    core/Flat.cpp
    core/Record.cpp
)

set(CORE_HEADERS
    core/TweakDBID.hpp
    core/Value.hpp
    core/Flat.hpp
    core/Record.hpp
    core/Types.hpp
)

# TweakDB I/O sources
set(TWEAKDB_SOURCES
    tweakdb/BinaryReader.cpp
    tweakdb/Reader.cpp
    tweakdb/Writer.cpp
    tweakdb/Database.cpp
)

set(TWEAKDB_HEADERS
    tweakdb/BinaryReader.hpp
    tweakdb/Reader.hpp
    tweakdb/Writer.hpp
    tweakdb/Database.hpp
    tweakdb/Header.hpp
)

# Parser sources
set(PARSER_SOURCES
    parsers/YAMLParser.cpp
    parsers/TweakParser.cpp
    parsers/ModLoader.cpp
)

set(PARSER_HEADERS
    parsers/YAMLParser.hpp
    parsers/TweakParser.hpp
    parsers/TweakGrammar.hpp
    parsers/ModLoader.hpp
)

# Model sources
set(MODEL_SOURCES
    model/TweakSource.cpp
    model/Operation.cpp
)

set(MODEL_HEADERS
    model/TweakSource.hpp
    model/Operation.hpp
    model/TypeInfo.hpp
)

# Bridge sources
set(BRIDGE_SOURCES
    bridge/Applicator.cpp
    bridge/TypeCoercion.cpp
    bridge/Validator.cpp
)

set(BRIDGE_HEADERS
    bridge/Applicator.hpp
    bridge/TypeCoercion.hpp
    bridge/Validator.hpp
)

# Create core library
add_library(tweakxl-core
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${TWEAKDB_SOURCES}
    ${TWEAKDB_HEADERS}
    ${PARSER_SOURCES}
    ${PARSER_HEADERS}
    ${MODEL_SOURCES}
    ${MODEL_HEADERS}
    ${BRIDGE_SOURCES}
    ${BRIDGE_HEADERS}
)

target_include_directories(tweakxl-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tweakxl-core PUBLIC
    yaml-cpp
    taocpp::pegtl
    spdlog::spdlog
)

# CLI executable
add_executable(tweakxl-mac
    cli/main.cpp
    cli/Commands.cpp
    cli/Config.cpp
)

target_link_libraries(tweakxl-mac PRIVATE
    tweakxl-core
    CLI11::CLI11
)

# Install targets
install(TARGETS tweakxl-mac
    RUNTIME DESTINATION bin
)
