cmake_minimum_required(VERSION 3.8)
project(ros2_console_tui_node)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()
find_package(Curses REQUIRED)
find_package(yaml-cpp REQUIRED)

# log_viewer ============================
set(TARGET log_viewer)
ament_auto_add_executable(${TARGET}_exec_ src/${TARGET}.cpp)
target_link_libraries(${TARGET}_exec_ ${CURSES_LIBRARIES} yaml-cpp)
unset(TARGET)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package()
