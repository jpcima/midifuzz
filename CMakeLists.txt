cmake_minimum_required(VERSION "3.3")

project(midifuzz LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

###
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include(GNUWarnings)
enable_gnu_warning("all")

###
find_package(PkgConfig REQUIRED)
pkg_check_modules(LV2 "lv2" REQUIRED)

###
add_library(midifuzz STATIC
  "sources/libmidifuzz/midifuzz.cpp")
target_include_directories(midifuzz
  PRIVATE "sources/libmidifuzz"
  PUBLIC "sources/libmidifuzz/include")

###
add_library(midi-fuzzer-lv2 MODULE
  "sources/midifuzz.lv2/midifuzz.lv2.c"
  "sources/midifuzz.lv2/midifuzz_forge_write.c")
target_include_directories(midi-fuzzer-lv2
  PRIVATE ${LV2_INCLUDE_DIRS})
target_link_libraries(midi-fuzzer-lv2
  PRIVATE midifuzz
  PRIVATE ${LV2_LIBRARIES})
set_target_properties(midi-fuzzer-lv2 PROPERTIES
  PREFIX ""
  OUTPUT_NAME "midifuzz"
  LIBRARY_OUTPUT_DIRECTORY "lv2/midifuzz.lv2")

###
set(MIDIFUZZ_LV2_MANIFESTS "manifest.ttl;midifuzz.ttl")
set(MIDIFUZZ_LV2_MANIFESTS_BUILT)

foreach(_manifest ${MIDIFUZZ_LV2_MANIFESTS})
  add_custom_command(
    OUTPUT "lv2/midifuzz.lv2/${_manifest}"
    DEPENDS "sources/midifuzz.lv2/${_manifest}"
    COMMAND "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/sources/midifuzz.lv2/${_manifest}"
    "${CMAKE_CURRENT_BINARY_DIR}/lv2/midifuzz.lv2/${_manifest}")
  list(APPEND MIDIFUZZ_LV2_MANIFESTS_BUILT
    "lv2/midifuzz.lv2/${_manifest}")
endforeach()

add_custom_target(midi-fuzzer-lv2-manifests
  DEPENDS ${MIDIFUZZ_LV2_MANIFESTS_BUILT})

add_dependencies(midi-fuzzer-lv2
  midi-fuzzer-lv2-manifests)
