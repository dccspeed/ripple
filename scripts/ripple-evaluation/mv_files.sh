#! /usr/bin/env sh

IN_DIR=$1
OUT_DIR=$2

echo "find $IN_DIR/. -name . -o -type d -prune -o -exec mv -t $OUT_DIR/ {} +"
find $IN_DIR/. -name . -o -type d -prune -o -exec mv -t $OUT_DIR/ {} +
