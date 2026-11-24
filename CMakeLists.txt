cmake_minimum_required(VERSION 3.28)

# NOTE: update EXECUTABLE_NAME in .github/workflows/cmake.yml when changing executable name in this file
set(MAIN_PROJECT_NAME "oop")
set(MAIN_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}_main")
set(MAIN_LIBRARY_NAME "${MAIN_PROJECT_NAME}")
set(TESTS_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}_test")

# needs to be before `project()` to take effect
option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compilation database" ON)

project(${MAIN_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(cmake/Options.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/CopyHelper.cmake)

include(CTest)
include(cmake/RegisterTests.cmake)

###############################################################################

# External dependencies with FetchContent

include(FetchContent)

set(FETCHCONTENT_QUIET OFF CACHE INTERNAL "Override verbose output" FORCE)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE INTERNAL "Override update strategy" FORCE)

# 1. SFML 3.0.0 (pentru interfața grafică)
FetchContent_Declare(
        SFML
        SYSTEM
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        3.0.0
        GIT_SHALLOW    1
        GIT_PROGRESS   ON
)

# 2. nlohmann/json (pentru tracks.json și config.json)
FetchContent_Declare(
        nlohmann_json
        SYSTEM
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
        GIT_SHALLOW    1
)

# FreeType (dependinta SFML) declara o versiune veche de CMake, respinsa de CMake 4.x.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
# Jocul foloseste doar grafica: nu compilam modulele Audio si Network.
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML nlohmann_json)

###############################################################################

# External dependencies with find_package

# find_package(Threads REQUIRED)

###############################################################################

# Configure sources

# 1. BIBLIOTECA (toată logica jocului — pune aici toate .cpp-urile din src/)
add_library(${MAIN_LIBRARY_NAME} STATIC)

target_sources(${MAIN_LIBRARY_NAME}
        PRIVATE
        src/Riddle.cpp
        src/internal/Detail.cpp
)

# 2. APLICAȚIA în consolă (scenariul cu tastatura.txt)
add_executable(${MAIN_EXECUTABLE_NAME}
        app/main.cpp
)

# 3. APLICAȚIA grafică SFML (aceeași bibliotecă de joc, altă interfață)
set(GUI_EXECUTABLE_NAME "${MAIN_PROJECT_NAME}_gui")
add_executable(${GUI_EXECUTABLE_NAME}
        app/gui_main.cpp
        app/RiddleGUI.h
        app/RiddleGUI.cpp
)

set_compiler_flags(RUN_SANITIZERS ${USE_ASAN} TARGET_NAMES ${MAIN_EXECUTABLE_NAME} ${GUI_EXECUTABLE_NAME} ${MAIN_LIBRARY_NAME})

###############################################################################

# Configure includes (Interfața publică în include/)

target_include_directories(${MAIN_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_include_directories(${MAIN_LIBRARY_NAME} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_include_directories(${MAIN_LIBRARY_NAME} SYSTEM PUBLIC
        ${EXT_PUBLIC_DIR}
)
target_include_directories(${MAIN_LIBRARY_NAME} SYSTEM PRIVATE
        ${EXT_PRIVATE_DIR}
)

###############################################################################

# Link libraries

# Legăm nlohmann_json la bibliotecă (ca să poată parsa tracks.json / config.json)
target_link_libraries(${MAIN_LIBRARY_NAME} PUBLIC nlohmann_json::nlohmann_json)

# Aplicația în consolă folosește doar biblioteca de joc
target_link_libraries(${MAIN_EXECUTABLE_NAME} PRIVATE ${MAIN_LIBRARY_NAME})

# Aplicația grafică: biblioteca + SFML 3
target_include_directories(${GUI_EXECUTABLE_NAME} PRIVATE app)
target_link_libraries(${GUI_EXECUTABLE_NAME} PRIVATE
        ${MAIN_LIBRARY_NAME}
        SFML::Graphics
        SFML::Window
        SFML::System
)

###############################################################################

# copy binaries to "bin" folder; these are uploaded as artifacts on each release
install(TARGETS ${MAIN_EXECUTABLE_NAME} ${GUI_EXECUTABLE_NAME} DESTINATION ${DESTINATION_DIR})
install(TARGETS ${MAIN_LIBRARY_NAME} ARCHIVE DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)
if(APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/launcher.command DESTINATION ${DESTINATION_DIR})
endif()

# Copiază automat fișierele de date și folderul assets (font.ttf) lângă executabil
copy_files(FILES tastatura.txt DIRECTORY assets COPY_TO_DESTINATION TARGET_NAME ${MAIN_EXECUTABLE_NAME})
copy_files(DIRECTORY assets TARGET_NAME ${GUI_EXECUTABLE_NAME})

###############################################################################

# Tests

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()