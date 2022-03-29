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
  "-pthread"
  learnta
  )

add_executable(learn_light EXCLUDE_FROM_ALL
  learn_light.cc
  )

target_link_libraries(learn_light
  ${Boost_LOG_LIBRARY}
  "-pthread"
  learnta
  )