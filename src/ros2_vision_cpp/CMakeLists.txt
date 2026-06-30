cmake_minimum_required(VERSION 3.8)
project(ros2_vision_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Find standard packages and MoveIt 2 components
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

# -------------------------------------------------------------
# EXECUTABLE 1: Camera Vision Node (Your original file)
# -------------------------------------------------------------
add_executable(cam_control_cpp src/marker_follower.cpp)
target_compile_features(cam_control_cpp PRIVATE cxx_std_17)
target_include_directories(cam_control_cpp PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(cam_control_cpp ${OpenCV_LIBRARIES})
ament_target_dependencies(cam_control_cpp
  rclcpp geometry_msgs visualization_msgs sensor_msgs cv_bridge tf2 tf2_ros tf2_geometry_msgs
)

# -------------------------------------------------------------
# EXECUTABLE 2: MoveIt Tracking Control Node (The new file)
# -------------------------------------------------------------
add_executable(goal_pose_publisher src/goal_pose_publisher.cpp)
target_compile_features(goal_pose_publisher PRIVATE cxx_std_17)
ament_target_dependencies(goal_pose_publisher
  rclcpp
  geometry_msgs
  moveit_ros_planning_interface
)

# -------------------------------------------------------------
# Install instructions so ROS 2 can discover both binaries
# -------------------------------------------------------------
install(TARGETS 
    cam_control_cpp
    goal_pose_publisher
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()