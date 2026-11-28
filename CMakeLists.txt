# CMakeLists.txt — cross-platform build (team Windows/MSVC workflow + macOS/Linux).
# The core is pure C++20 with NO Windows/MFC includes; the MFC GUI (gui/, P4) is
# added separately on Windows only. Verified locally with CMake 4.5.1 + clang.
cmake_minimum_required(VERSION 3.20)
project(TheGodExperiment LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- core library (P1 + P2 + P3 logic; header-only DSA/common + a few .cpp) ---
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS core/*.cpp)
list(FILTER CORE_SOURCES EXCLUDE REGEX "core/sim/HeadlessMain.cpp$")
add_library(tge_core STATIC ${CORE_SOURCES})
target_include_directories(tge_core PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/third_party)

# --- unit tests (doctest single header) ---
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/unit/*.cpp)
add_executable(tge_tests ${TEST_SOURCES} tests/reference/prng_ref.c)
target_link_libraries(tge_tests PRIVATE tge_core)
target_include_directories(tge_tests PRIVATE ${CMAKE_SOURCE_DIR}/third_party/doctest)
enable_testing()
add_test(NAME unit COMMAND tge_tests)

# --- headless runner ---
add_executable(runner core/sim/HeadlessMain.cpp)
target_link_libraries(runner PRIVATE tge_core)

# --- seed hunting tool ---
add_executable(seedhunt tools/SeedHunt.cpp)
target_link_libraries(seedhunt PRIVATE tge_core)

# --- MFC GUI (P4) is Windows-only; guarded so non-Windows builds still work ---
if(WIN32 AND EXISTS ${CMAKE_SOURCE_DIR}/gui/CMakeLists.txt)
  add_subdirectory(gui)
endif()
