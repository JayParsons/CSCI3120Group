#!/bin/bash

for (( i = 1; $i <= 128; i = $i + 1))
do
  dd if=/dev/zero of=size_$i.txt bs=1k count=$i
done
