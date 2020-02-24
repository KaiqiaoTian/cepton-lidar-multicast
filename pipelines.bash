#!/bin/bash
set -e

source `find /opt/ros -name setup.bash | sort | head -1` # Source ROS environment
catkin_make_isolated --install -DCMAKE_BUILD_TYPE=Release
catkin_make_isolated --install -DCMAKE_BUILD_TYPE=Release --make-args tests
catkin_make_isolated --install -DCMAKE_BUILD_TYPE=Release --make-args run_tests -j1
catkin_test_results build_isolated # Display test results and error on any test failure

