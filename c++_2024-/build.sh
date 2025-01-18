#!/bin/bash
set -e                  # -e exits on error, -x prints the values
cd "$(dirname "$0")"    # change to script's directory 

# usage
# build.sh [debug|release] [assets|]
# defaults to debug no assets
unset debug
unset release
unset assets

# unpack args
for arg in "$@"; do declare $arg='1'; done
if [ -z ${release} ]; then debug='1'; fi # -z to check if var is unset or empty
if [ ${debug} ]; then unset release; fi
if [ ${release} ]; then unset debug; fi

# figure out compile args (-fdiagnostics-absolute-flags is necessary so that tools such as vim can find the error file, regardless of their working t directory)
common="-x objective-c++ -std=gnu++14 -fobjc-arc -D__MACOS=1 -D__GL33=1 -framework Cocoa -framework IOKit -fdiagnostics-absolute-paths"
cl_debug="clang++ -g -O0 ${common}"
cl_release="clang++ -g -O2 -DNDEBUG ${common}"
if [ ${debug} ]; then
    outdir="./bin/Debug"
    objectdir=".bin/Debug/"
    compile="$cl_debug"
fi
if [ ${release} ]; then
    outdir="./bin/Release"
    objectdir=".bin/Release/"
    compile="$cl_release"
fi

# create dirs
mkdir -p ${outdir}

# build
$compile ./src/main.cpp -o "${outdir}/app-macos"

# post-build step (-r for recursive)
if [ ${assets} ]; then cp -r ./assets/ "${outdir}/assets/"; fi
