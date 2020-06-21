#!/bin/bash

DEFAULT_VERSION="1.0.0"
DEFAULT_VERSION_STR="dev-build"

NUM_VER=$1
STR_VER=$2

if [ -z "$NUM_VER" ]; then
    NUM_VER=$DEFAULT_VERSION
fi

if [ -z "$STR_VER" ]; then
    STR_VER=$DEFAULT_VERSION_STR
fi

if [ "$NUM_VER" = "-h" ]; then
    echo "Usage: $0 <num version: #.#.#> <str version>"
    exit 1
fi

echo "Setting version to: $NUM_VER $STR_VER"

projucer=../JUCE/Projucer.app/Contents/MacOS/Projucer

if [ -n "$PROJUCER" ]; then
    projucer=$PROJUCER
fi

if [ ! -x $projucer ]; then
    echo "Projucer not found, please set the PROJUCER env variable to the Projucer binary"
    exit 1
fi

$projucer --set-version $NUM_VER Server/AudioGridderServer.jucer
$projucer --set-version $NUM_VER Plugin/Fx/AudioGridder.jucer
$projucer --set-version $NUM_VER Plugin/Inst/AudioGridder.jucer

cat package/Version.hpp.in | sed "s/#STR_VER#/$STR_VER/" > Common/Source/Version.hpp
cat package/AudioGridder.iss.in | sed "s/#STR_VER#/$STR_VER/" > package/AudioGridder.iss

echo $STR_VER > package/VERSION
