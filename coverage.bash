#!/bin/bash
set -e

# Check for lcov
if ! [ $(which lcov) ]; then
  echo "Installing lcov..."
  echo -e "Executing \033[1msudo apt-get install lcov\033[0m"
  sudo apt-get install lcov
fi

# Create catkin workspace for coverage testing
WS_DIR="coverage"
if [ ! -d "$WS_DIR" ]; then
  mkdir $WS_DIR
fi

# Cleanup from previous coverage analysis 
LCOV_DIR="$WS_DIR/build"
LCOV_HTML="$WS_DIR/html"
LCOV_FILE="$WS_DIR/lcov.info"
if [ -d "$LCOV_DIR" ]; then
  GCDA=`find $LCOV_DIR -name *.gcda`
  if [ ! -z "$GCDA" ]; then
    rm $GCDA
  fi
fi
if [ -d "$LCOV_HTML" ]; then
  rm $LCOV_HTML -r
fi
if [ -f "$LCOV_FILE" ]; then
  rm $LCOV_FILE
fi
if [ -d "$LCOV_DIR/test_results" ]; then
  rm $LCOV_DIR/test_results -r
fi

# Build and run tests
CATKIN_ARGS="-DCMAKE_BUILD_TYPE=Debug --directory $WS_DIR --source src"
catkin_make $CATKIN_ARGS
catkin_make $CATKIN_ARGS tests
catkin_make $CATKIN_ARGS run_tests -j2

# Coverage analysis
echo
echo "Generating coverage analysis..."
lcov -d $LCOV_DIR -o $LCOV_FILE -q --capture
lcov -d $LCOV_DIR -o $LCOV_FILE -q --extract $LCOV_FILE "`pwd`/src/*"
lcov -d $LCOV_DIR -o $LCOV_FILE -q --remove  $LCOV_FILE "*/tests/*"
lcov -d $LCOV_DIR -l $LCOV_FILE
genhtml -o $LCOV_HTML  $LCOV_FILE -q

# Open HTML output in default web browser
x-www-browser $LCOV_HTML/index.html > /dev/null 2>&1 &

# Display test results
echo
catkin_test_results $LCOV_DIR/test_results --verbose

