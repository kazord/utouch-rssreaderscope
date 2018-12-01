
# Google Mock unfortunately has to be compiled from source
include(FindGMock)


# Include our test library headers
include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GMOCK_INCLUDE_DIRS}
  ${TEST_INCLUDE_DIRS}
)

# Where to find the scope ini file and .so
set(TEST_SCOPE_DIRECTORY "${CMAKE_BINARY_DIR}/src")

file(GLOB_RECURSE
  TEST_FIXTURES
  "unit/*.py"
)

# Make this file show up in QtCreator
add_custom_target(hidden_test_fixtures ALL
  SOURCES ${TEST_FIXTURES}
)


# Add the unit tests
add_subdirectory(unit)

