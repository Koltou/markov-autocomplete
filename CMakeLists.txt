cmake_minimum_required(VERSION 3.16)
project(LinearAlgebraProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ICU prebuilt binaries are linked against the release dynamic CRT (/MD).
# Force every target (ours + FLTK) to use the same runtime in all configs
# to avoid heap corruption from mixing debug/release CRTs.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

include(FetchContent)

# ── Eigen (header-only — skip its CMakeLists.txt to avoid target conflicts) ──
FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(eigen)
add_library(Eigen3::Eigen INTERFACE IMPORTED)
set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${eigen_SOURCE_DIR}"
)

# ── FLTK (built from source, no external install required) ───────────────────
FetchContent_Declare(
    fltk
    GIT_REPOSITORY https://github.com/fltk/fltk.git
    GIT_TAG        release-1.3.9
)
set(FLTK_BUILD_TEST     OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLUID    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fltk)

# ── ICU (prebuilt for Windows) ───────────────────────────────────────────────
FetchContent_Declare(
    icu
    URL https://github.com/unicode-org/icu/releases/download/release-78.3/icu4c-78.3-Win64-MSVC2022.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(icu)

# ── markov_autocomplete ───────────────────────────────────────────────────────
add_executable(markov_autocomplete
    main.cpp
    src/autocomplete_gui.cpp
    src/nexttoken_markov_chain.cpp
    src/trie.cpp
)

target_include_directories(markov_autocomplete
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${fltk_SOURCE_DIR}
        ${fltk_BINARY_DIR}
        ${icu_SOURCE_DIR}/include
)

target_link_libraries(markov_autocomplete
    PRIVATE
        Eigen3::Eigen
        fltk
        "${icu_SOURCE_DIR}/lib64/icuuc.lib"
        "${icu_SOURCE_DIR}/lib64/icuin.lib"
        "${icu_SOURCE_DIR}/lib64/icudt.lib"
)

add_custom_command(TARGET markov_autocomplete POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:markov_autocomplete>
        ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:markov_autocomplete>
    # Copy ICU DLLs to output directory
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icuuc78.dll" "${CMAKE_BINARY_DIR}/$<CONFIG>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icuin78.dll" "${CMAKE_BINARY_DIR}/$<CONFIG>/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icudt78.dll" "${CMAKE_BINARY_DIR}/$<CONFIG>/"
    # Also copy to source dir alongside exe
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icuuc78.dll" "${CMAKE_SOURCE_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icuin78.dll" "${CMAKE_SOURCE_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${icu_SOURCE_DIR}/bin64/icudt78.dll" "${CMAKE_SOURCE_DIR}/"
)
