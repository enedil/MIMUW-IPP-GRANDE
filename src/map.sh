#!/usr/bin/env bash

function usage() {
    >&2 echo "Usage:"
    >&2 echo "    $0 getDescription_outputs.txt [routeId1 [routeId2 [...]]]"
    >&2 echo "every routeId should be between 1 and 999 (inclusive)"
    exit 1
}

if (( $# < 2 ))
then 
    usage
else
    echo xx
fi

output="$1"

