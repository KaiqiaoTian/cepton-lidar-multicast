#!/bin/bash

# Generate Eclipse project and Makefiles
cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j8

# Disable line that breaks the IDE (necessary for Ubuntu 18.04)
sed -i 's@<pathentry kind="src" path="\[Subprojects\]"\/>@<!--pathentry kind="src" path="\[Subprojects\]"\/-->@g' .cproject

