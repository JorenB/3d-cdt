#!/bin/bash

for kB in 0.4 -0.4; do
  for kD in 0.8 -0.8; do
    for T in 32; do
      for V in 8000; do
        # Define the output file name
        name="${kB}-${kD}-${T}-${V}-sphere3"
        
        ./3dcdt.x Cfg-$name.txt &
      done
    done
  done
done

