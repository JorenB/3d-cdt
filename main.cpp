// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "config.hpp"
#include "universe.hpp"
#include "simulation.hpp"

#include "observables/volume_profile.hpp"
#include "observables/tau.hpp"
#include "observables/hausdorff2d.hpp"
#include "observables/hausdorff2d_dual.hpp"

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
	std::string fID = cfr.getString("fileID");
	int measurements = cfr.getInt("measurements");

	std::string InFile = cfr.getString("infile");

	int thermalSteps = cfr.getInt("thermalSteps"); // Number of thermalization sweeps
	int kSteps = cfr.getInt("kSteps");  // Number of thousand steps in a sweep
	int sweeps = cfr.getInt("sweeps");  // Number of measurement sweeps 

	double k3_s = cfr.getDouble("k3"); // Cosmological constant from input
	
	printf("fID: %s\n", fID.c_str());
	printf("seed: %d\n", seed);

	Universe::initialize(InFile, fID);

	printf("\n\n#######################\n");
	printf("* * * Initialized * * *\n");
	printf("#######################\n\n");


	//VolumeProfile vp3(fID);
	//Simulation::addObservable3d(vp3);

	//VolumeProfile vp(fID);
	//Simulation::addObservable2d(vp);
	
	//Tau tau(fID);
	//Simulation::addObservable3d(tau);

	Hausdorff2d haus(fID, true);
	//Hausdorff2d haus(fID, false);
	Simulation::addObservable2d(haus);

	//Hausdorff2dDual hausd(fID);
	//Simulation::addObservable2d(hausd);
	
	Simulation::start(measurements, k0, k3_s, targetVolume, target2Volume, seed, thermalSteps, kSteps);
	
	printf("\n\n####################\n");
	printf("* * * Finished * * *\n");
	printf("####################\n\n");

	printf("t31: %d\n", Universe::tetras31.size());
		
	return 0;
}
