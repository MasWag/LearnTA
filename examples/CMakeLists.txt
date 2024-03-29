include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

include_directories(
  ../include/
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS})

add_executable(learn_simple_dta EXCLUDE_FROM_ALL
  learn_simple_dta.cc
  )

target_link_libraries(learn_simple_dta
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_unbalanced EXCLUDE_FROM_ALL
  learn_unbalanced.cc
  )

target_link_libraries(learn_unbalanced
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_light EXCLUDE_FROM_ALL
  learn_light.cc
  )

target_link_libraries(learn_light
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_CAS EXCLUDE_FROM_ALL
  learn_CAS.cc
  )

target_link_libraries(learn_CAS
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_PC EXCLUDE_FROM_ALL
  learn_PC.cc
  )

target_link_libraries(learn_PC
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_ota_json EXCLUDE_FROM_ALL
  learn_ota_json.cc
  )

target_link_libraries(learn_ota_json
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_unbalanced_loop EXCLUDE_FROM_ALL
  learn_unbalanced_loop.cc
  )

target_link_libraries(learn_unbalanced_loop
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_fddi EXCLUDE_FROM_ALL
  learn_fddi.cc
  )

target_link_libraries(learn_fddi
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )

add_executable(learn_fischer EXCLUDE_FROM_ALL
  learn_fischer.cc
  )

target_link_libraries(learn_fischer
  ${Boost_LOG_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  "-pthread"
  learnta
  )
