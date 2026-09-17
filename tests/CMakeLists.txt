include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# One test executable per *_test.cpp, named after its path (e.g. memory_lru_replacer_test).
# New algorithms are picked up automatically — just drop in a matching *_test.cpp.
file(GLOB_RECURSE TURTLEDB_TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

foreach(test_src ${TURTLEDB_TESTS})
  file(RELATIVE_PATH rel "${CMAKE_CURRENT_SOURCE_DIR}" "${test_src}")
  string(REPLACE "/" "_" test_name "${rel}")
  string(REPLACE ".cpp" "" test_name "${test_name}")
  add_executable(${test_name} "${test_src}")
  target_link_libraries(${test_name} PRIVATE turtledb GTest::gtest_main)
  gtest_discover_tests(${test_name})
endforeach()
