cmake_minimum_required(VERSION 3.20)
project(log_viewer_base)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()
find_package(yaml-cpp REQUIRED)

# log_viewer ============================
set(TARGET log_viewer_base)
ament_auto_add_library(${TARGET} SHARED
  src/${TARGET}.cpp
  src/graph_inspector.cpp
)
target_link_libraries(${TARGET} ${CURSES_LIBRARIES} yaml-cpp)

set(EXAMPLE_TARGET log_viewer_stdout)
ament_auto_add_executable(${EXAMPLE_TARGET}
  example/log_viewer_stdout.cpp
)
target_link_libraries(${EXAMPLE_TARGET} ${TARGET})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
