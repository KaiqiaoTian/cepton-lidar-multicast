# ROS Workspace

Collection of scripts and documentation to automate using a ros/catkin workspace. Most scripts must be run from the root of a catkin workspace.

## IDE setup

1. [Visual Studion Code](IDE/vscode.md)
1. [Eclipse](IDE/Eclipse.md)
1. [KDevelop](IDE/KDevelop.md)

## Setup/Update Scripts

1. [update](update.bash) (Update version controlled software with [rosinstall](http://docs.ros.org/independent/api/rosinstall/html/) and [wstool](http://wiki.ros.org/wstool))
1. [deps](deps.bash) (Install dependencies with [rosdep](http://docs.ros.org/independent/api/rosdep/html/))

## Build Scripts

1. [clean](clean.bash) (Clean build files from workspace)
1. [debug](debug.bash) (Build workspace in debug mode)
1. [release](release.bash) (Build workspace in release mode)
1. [install](install.bash) (Build workspace in release mode and install)
1. [tests](tests.bash) (Build workspace in release mode and run tests)
1. [coverage](coverage.bash) (Build workspace in debug mode, run tests, and print coverage report)

## ROS Master and Time Synchronization Scripts

1. [ros\_ip](ros_ip.bash) (Set the [ROS\_IP](http://wiki.ros.org/ROS/EnvironmentVariables#ROS_IP.2BAC8-ROS_HOSTNAME) environment variable to the first active network address)
1. [ping](ping.bash) (Ping the ROS master)
1. [time_check](time_check.bash) (Check time syncronization error with ROS master using [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol))
1. [time_sync](time_sync.bash) (Syncronization time with ROS master using [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol))
1. Install [chrony](https://chrony.tuxfamily.org/) for more precise syncronization: http://wiki.ros.org/ROS/NetworkSetup

