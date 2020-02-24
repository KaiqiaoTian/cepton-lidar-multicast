#!/bin/bash
set -e

my_ip_addr=$(ip addr | grep 'state UP' -A2 | tail -n1 | awk '{print $2}' | cut -f1  -d'/')

# Make sure to source this script, otherwise export does nothing
export ROS_IP=$my_ip_addr

