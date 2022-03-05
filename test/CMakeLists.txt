add_executable(Test
  test_parser.cpp
)

target_link_libraries(
  Test
  gtest
  gtest_main
  ALL
)

include(GoogleTest)
gtest_discover_tests(Test)