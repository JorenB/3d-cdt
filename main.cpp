// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "config.hpp"
#include "universe.hpp"
#include "simulation.hpp"

#include "observables/volume_profile.hpp"
#include "observables/cnum.hpp"
#include "observables/maxcnum.hpp"
#include "observables/tau.hpp"
#include "observables/action.hpp"
#include "observables/hausdorff2d.hpp"
#include "observables/hausdorff2d_dual.hpp"
#include "observables/minbu.hpp"

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
	int targetVolume = cfr.getInt("targetVolume");
	int target2Volume = cfr.getInt("target2Volume");
	int genus = cfr.getInt("genus");
	int seed = cfr.getInt("seed");
	int strictness = cfr.getInt("strictness");

	std::string fID = cfr.getString("fileID");
	std::string outputDir = cfr.getString("outputDir");
	std::string outFile = cfr.getString("outfile");
	std::string inFile = cfr.getString("infile");
	
	int kSteps = cfr.getInt("ksteps");
	int thermalSweeps = cfr.getInt("thermalSweeps"); // Number of thermalization sweeps
	int sweeps = cfr.getInt("sweeps");  // Number of measurement sweeps 

	double k3_s = cfr.getDouble("k3"); // Cosmological constant from input
	
	int volfixSwitch = cfr.getInt("volfix_switch");
	
	int v1 = cfr.getInt("v1");
	int v2 = cfr.getInt("v2");
	int v3 = cfr.getInt("v3");

	
	printf("fID: %s\n", fID.c_str());
	printf("seed: %d\n", seed);
	printf("strictness: %d\n", strictness);

	Universe::initialize(inFile, fID, strictness, volfixSwitch);

	printf("\n\n#######################\n");
	printf("* * * Initialized * * *\n");
	printf("#######################\n\n");

	Observable::data_dir = outputDir;
	VolumeProfile vp3(fID);
	Simulation::addObservable3d(vp3);

	Tau tau(fID);
	Simulation::addObservable3d(tau);

	Action action(fID);
	Simulation::addObservable3d(action);

	Simulation::start(k0, k3_s, sweeps, thermalSweeps, kSteps, targetVolume, target2Volume, seed, outFile,v1,v2,v3);
	
	printf("\n\n####################\n");
	printf("* * * Finished * * *\n");
	printf("####################\n\n");

	printf("t31: %d\n", Universe::tetras31.size());
		
	return 0;
}
