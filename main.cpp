// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "config.hpp"
#include "universe.hpp"
#include "simulation.hpp"

#include "observables/volume_profile.hpp"

std::default_random_engine rng(1);

int main(int argc, const char * argv[]) {
	std::string fname;
	if (argc > 1) {
		fname = std::string(argv[1]);
		printf("%s\n", fname.c_str());
	}
	ConfigReader cfr;
	cfr.read(fname);

	double k0 = cfr.getDouble("k0");
	double k3_s = cfr.getDouble("k3");
	int genus = cfr.getInt("genus");
	int targetVolume = cfr.getInt("targetvolume");
	int target2Volume = cfr.getInt("target2volume");
	int volfixSwitch = cfr.getInt("volfixswitch");
	int seed = cfr.getInt("seed");
	std::string outputDir = cfr.getString("outputdir");
	std::string fID = cfr.getString("fileid");
	int thermalSweeps = cfr.getInt("thermalsweeps"); // Number of thermalization sweeps
	int sweeps = cfr.getInt("measuresweeps");  // Number of measurement sweeps 
	int kSteps = cfr.getInt("ksteps");
	int strictness = cfr.getInt("strictness");
	int v1 = cfr.getInt("v1");
	int v2 = cfr.getInt("v2");
	int v3 = cfr.getInt("v3");
	std::string inFile = cfr.getString("infile");
	std::string outFile = cfr.getString("outfile");
	
	printf("fID: %s\n", fID.c_str());
	printf("seed: %d\n", seed);
	printf("strictness: %d\n", strictness);

	Observable::data_dir = outputDir;

	Universe::initialize(inFile, fID, strictness, volfixSwitch);

	printf("\n\n#######################\n");
	printf("* * * Initialized * * *\n");
	printf("#######################\n\n");

	VolumeProfile vp3(fID);
	Simulation::addObservable3d(vp3);

	Simulation::start(k0, k3_s, sweeps, thermalSweeps, kSteps, targetVolume, target2Volume, seed, outFile,v1,v2,v3);
	
	printf("\n\n####################\n");
	printf("* * * Finished * * *\n");
	printf("####################\n\n");

	printf("t31: %d\n", Universe::tetras31.size());
		
	return 0;
}
