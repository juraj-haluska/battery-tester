#!/bin/bash
FIRST=5
while read line
do
  if [[ $line == ID:"$2"* ]]; then
    echo ${line: FIRST :${#line}-FIRST-2}
  fi
done < $1
