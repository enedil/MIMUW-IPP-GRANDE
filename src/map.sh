#!/usr/bin/env bash

function usage() {
    >&2 echo "Usage:"
    >&2 echo "    $0 getDescription_outputs.txt [routeId1 [routeId2 [...]]]"
    >&2 echo "every routeId should be between 1 and 999 (inclusive)"
    exit 1
}

function getlength() {
    ruby -pe '$_=($_.split(";").select.each_with_index {|_,i| i%3 == 2}).inject(0) {|sum, x| sum + x.to_i}' <<< "$1"
}

if (( $# < 2 ))
then 
    usage
fi
# check if all arguments are valid
for arg in "${@:2}"; do
    if ! [[ "$arg" =~ ^[0-9]+$ ]]; then
        usage
    fi
    if (( "$arg" > 1000 )) || (( "$arg" == 0 )); then
        usage
    fi
done

output_file="$1"

for arg in "${@:2}"; do
    out=$(grep "^$arg;" "$output_file")
    if [[ ! -z "$out"  ]]
    then
        echo -n "$arg;"
        getlength "$out"
        echo
    fi
done
