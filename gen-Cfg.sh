#!/bin/bash

# Loop over the variables and create the files
for V in 8000; do
#  for kB in $(seq -1.0 0.2 1.0); do
#    for kD in $(seq -1.0 0.2 1.0) ; do
for kB in 0.45 ;do
for kD in 0.9 ; do      
for k0 in -2.1 ; do
# Define the file names
      
      name=${kB}-${kD}-32-$V-sphere3
#      name=${k0}-32-$V-sphere3
    
	if [[ ! -f conf-$name.cdt ]]; then
        echo "File conf-$name.cdt does not exist. Skipping..."
        continue
     fi

rand=$RANDOM

  # Create the file content
cat << EOF > "Cfg-${name}.txt"
	k0               $k0
	k3               1.77
	kB               $kB
	kD               $kD
	genus            0
	targetvolume     16000
	targetvertex     $V
	volfixswitch     0
	epsilon          0.0001
	seed             $rand
	outputdir        -1
	fileid           $name
	thermalsweeps    1000
	measuresweeps    0
	ksteps           16000
	strictness       3
	abcd             1
	v1               2
	v2               4
	v3               2
	infile           conf-${name}.cdt
	outfile          conf-${name}_2.cdt
EOF

done
done
done
done

echo "Files created successfully in the $outputdir directory."

