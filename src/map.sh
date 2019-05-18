#!/usr/bin/env bash

usage() {
    >&2 echo "Usage:"
    >&2 echo "    $0 getDescription_outputs.txt [routeId1 [routeId2 [...]]]"
    >&2 echo "every routeId should be between 1 and 999 (inclusive)"
    exit 1
}

getlength() {
    ruby -pe 'l=$_.split(";");$_="#{l[0]};#{(l.select.each_with_index{|_,i|i%3==2}.map(&:to_i).inject:+)}\n"' 
}

if (( $# < 2 )); then 
    usage
fi
# check if all arguments are valid
for arg in "${@:2}"; do
    if ! [[ "$arg" =~ ^[0-9]+$ ]] || (( "$arg" > 1000 )) || (( "$arg" == 0 )); then
        usage
    fi
done
regex=$(sed -E "s/\s+/;\|\^/g" <<< "${@:2}")
egrep '('"$regex"')' "$1" | getlength
