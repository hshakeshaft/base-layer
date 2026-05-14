#!/usr/bin/env sh
set -ex

BUILD_DIR="build"
CFLAGS="-std=c89 -Wall -Wextra -Werror -pedantic -O0 -g"


if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

# examples
cc $CFLAGS -o $BUILD_DIR/hs_strings examples/hs_strings.c
./$BUILD_DIR/hs_strings

cc $CFLAGS -DHS_LOGGING_EXAMPLE__INCLUDE_DEFAULT_LOGGERS -o $BUILD_DIR/hs_logging examples/hs_logging.c
./$BUILD_DIR/hs_logging foo bar baz
