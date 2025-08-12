cmake_minimum_required(VERSION 3.0.2)
project(livox_bag_aggregate)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rosbag
  sensor_msgs
  livox_ros_driver
)

catkin_package()
include_directories(${catkin_INCLUDE_DIRS})

add_executable(livox_bag_aggregate src/livox_bag_aggregate.cpp)
target_link_libraries(livox_bag_aggregate ${catkin_LIBRARIES})