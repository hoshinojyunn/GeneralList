include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)


set(SOURCES main.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g)
target_link_libraries(${CMAKE_PROJECT_NAME} gtest_main)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)