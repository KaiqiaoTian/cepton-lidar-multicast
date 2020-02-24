#!/bin/bash
set -e

#http://stackoverflow.com/questions/2497215/extract-domain-name-from-url
ROS_MASTER_HOSTNAME=$(echo $ROS_MASTER_URI | sed -e "s/[^/]*\/\/\([^@]*@\)\?\([^:/]*\).*/\2/")
echo "Hostname '$ROS_MASTER_HOSTNAME' extracted from ROS_MASTER_URI=$ROS_MASTER_URI"

echo "Pinging ROS master..."
ping -c 4 $ROS_MASTER_HOSTNAME

