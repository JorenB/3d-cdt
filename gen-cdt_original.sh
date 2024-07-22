#!/bin/bash

# Define the ranges for the loop variables


# Loop over the variables and generate the configuration files
#for kB in $(seq -1.0 0.2 1.0); do
#  for kD in $(seq -1.0 0.2 1.0); do
    for k0 in 5 ; do
    for T in 32; do
      for V in 16000; do
        # Define the output file name
        output_config="conf-${k0}-${T}-${V}-sphere3.cdt"
        
        # Copy the input configuration file to the new file
        cp conf.dat $output_config
        
        # Print a message indicating the file was created
        echo "Created $output_config"
      done
  done
done

echo "All configuration files generated successfully."

