#  Copyright (c) Microsoft
#  Copyright (c) 2024 Eclipse Foundation
#
#  This program and the accompanying materials are made available
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
#
#  SPDX-License-Identifier: MIT
#
#  Contributors:
#     Microsoft         - Initial version
#     Frederic Desbiens - 2024 version.

#!/bin/bash

# Use paths relative to this script's location
SCRIPT=$(readlink -f "$0")
SCRIPTDIR=$(dirname "$SCRIPT")
BASEDIR=$(dirname "$SCRIPTDIR")
SOMEIP_DEVICE_ROLE="${1:-1}"

if [ "$SOMEIP_DEVICE_ROLE" != "1" ] && [ "$SOMEIP_DEVICE_ROLE" != "2" ]; then
  echo "ERROR: SOMEIP device role must be 1 or 2."
  echo "Usage: $(basename "$0") [1|2]"
  exit 1
fi

# echo $BASEDIR

# If you want to build into a different directory, change this variable
BUILDDIR="$BASEDIR/build"

# Create our build folder if required and clear it
mkdir -p $BUILDDIR
rm -rf $BUILDDIR/*

# Generate the build system using Ninja
echo "Building SOME/IP profile role=$SOMEIP_DEVICE_ROLE"
cmake -B"$BUILDDIR" -GNinja \
  -DCMAKE_TOOLCHAIN_FILE=$BASEDIR/../../cmake/arm-gcc-cortex-m4.cmake \
  -DENABLE_OPENSOMEIP=ON \
  -DSOMEIP_DEVICE_ROLE=$SOMEIP_DEVICE_ROLE \
  -DOPENSOMEIP_SOURCE_DIR=$BASEDIR/../../third_party/opensomeip \
  $BASEDIR

# And then do the build
cmake --build $BUILDDIR
