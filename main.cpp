// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich

// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich

//#include "globals.hpp"

#include "globals.hpp"

#include "config.hpp"
#include "universe.hpp"
#include "simulation.hpp"

#include "observables/volume_profile.hpp"
#include "observables/ricci2d.hpp"

std::default_random_engine rng(1);


int main(int argc, const char * argv[]) {
	std::string fname;
	if (argc > 1) {
		fname = std::string(argv[1]);
		printf("%s\n", fname.c_str());
	}
	ConfigReader cfr;
	cfr.read(fname);
	


	k0 = cfr.getDouble("k0");
	k3 = cfr.getDouble("k3");
	kB = cfr.getDouble("kB");
	kD = cfr.getDouble("kD");
	
	genus = cfr.getInt("genus");
	targetVolume = cfr.getInt("targetvolume");
	targetvertex = cfr.getInt("targetvertex");
	target2Volume = targetVolume;
	volfix_switch = cfr.getInt("volfixswitch");
	epsilon = cfr.getDouble("epsilon");
	seed = cfr.getInt("seed");
	std::string outputDir = cfr.getString("outputdir");
	fID = cfr.getString("fileid");
	thermalSweeps = cfr.getInt("thermalsweeps");  // Number of thermalization sweeps
	sweeps = cfr.getInt("measuresweeps");  // Number of measurement sweeps
	ksteps = cfr.getInt("ksteps");
	strictness = cfr.getInt("strictness");
	abcd = cfr.getInt("abcd");
	v1 = cfr.getInt("v1");
	v2 = cfr.getInt("v2");
	v3 = cfr.getInt("v3");
	inFile = cfr.getString("infile");
	outFile = cfr.getString("outfile");
	
	
	printf("genus: %d\n",genus);
	printf("targetVolume :%d\n",targetVolume);
	printf("targetvertex: %d\n",targetvertex);
	printf("volfix_switch: %d\n",volfix_switch);
	printf("epsilon: %g\n",epsilon);
	printf("seed: %d\n",seed);
	printf("sweeps: %d\n",sweeps);
	printf("thermalSweeps: %d\n",thermalSweeps);
	printf("strictness: %d\n",strictness);
	printf("abcd: %d\n",abcd);
	printf("v1: %d\n",v1);
	printf("v2: %d\n",v2);
	printf("v3: %d\n",v3);

	printf("fID: %s\n", fID.c_str());

	Observable::data_dir = outputDir;
	
	
	sprintf(filen22, "outfile-%s.out", fID.c_str());

	Universe::initialize();

	printf("\n\n#######################\n");
	printf("* * * Initialized * * *\n");
	printf("#######################\n\n");


	VolumeProfile vp3(fID);	
	Simulation::addObservable3d(vp3);

    //Ricci2d ricci2d(fID);
    //Simulation::addObservable2d(ricci2d);

	Simulation::start();
	


	printf("\n\n####################\n");
	printf("* * * Finished * * *\n");
	printf("####################\n\n");

	printf("t31: %d\n", Universe::tetras31.size());

	return 0;
}
	


