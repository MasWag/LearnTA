cmake_minimum_required(VERSION 3.1)
project(LearnTA CXX)

set(CMAKE_CXX_FLAGS "-Wall -DBOOST_LOG_DYN_LINK=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-flto -O3 -DRELEASE -DNDEBUG")
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  add_definitions(
    "-DLearnTA_VERSION=\"${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}\"")
else()
  add_compile_definitions(
    LearnTA_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS
  unit_test_framework
  system
  log)
find_package(Eigen3 REQUIRED)
# find_package(TBB REQUIRED)

include_directories(
  include/
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS})

## Config for LearnTA
add_library(learnta
  src/intersection.cc
  src/ta2za.cc
  src/timed_automaton.cc
  src/observation_table.cc
  )

## Config for Test
enable_testing()

add_executable(unit_test EXCLUDE_FROM_ALL
  src/intersection.cc
  src/ta2za.cc
  src/timed_automaton.cc
  src/observation_table.cc
  tests/unit_test.cc
  tests/zone_test.cc
  tests/elementary_language_test.cc
  tests/forward_regional_elementary_language_test.cc
  tests/backward_regional_elementary_language_test.cc
  tests/timed_condition_test.cc
  tests/timed_word_test.cc
  tests/fractional_order_test.cc
  tests/timed_automaton_runner_test.cc
  tests/symbolic_membership_oracle_test.cc
  tests/equivalence_test.cc
  tests/juxtaposed_zone_test.cc
  tests/timed_automaton_equivalence_oracle_test.cc
  tests/observation_table_test.cc
  #tests/learner_test.cc
  tests/constraint_test.cc
  tests/timed_automaton_test.cc
  tests/internal_transition_maker_test.cc
  tests/single_morphism_test.cc
  tests/recognizable_languages_test.cc
  tests/counterexample_analyzer_test.cc
  tests/renamig_relation_test.cc
  tests/external_transition_maker_test.cc
  tests/neighbor_conditions_test.cc
  tests/equivalence_oracle_by_test_test.cc
  tests/imprecise_clock_handler_test.cc
  )

target_link_libraries(unit_test
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  )

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION
  "Create and install the HTML based API documentation (requires Doxygen)"
  ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

add_subdirectory(examples)
