#!/bin/bash

PATH="/home/felix/Dokumente/programming/c/crispy"

echo "Debug:"
$PATH/res/runner/test_runner.py $PATH/cmake-build-debug/crispy $PATH/res/test/ $PATH/res/expected/ $PATH/res/input

echo "Release:"
$PATH/res/runner/test_runner.py $PATH/cmake-build-release/crispy $PATH/res/test/ $PATH/res/expected/ $PATH/res/input
