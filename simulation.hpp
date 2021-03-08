// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <random>
#include <vector>
#include "universe.hpp"
//#include "observable.hpp"

class Observable;
class Simulation {
public:
	static double lambda;

	static void start(double k0, double k3, int sweeps, int thermalSweeps,int ksteps,  int targetVolume_, int target2Volume, int seed,  std::string OutFile);

	static void addObservable3d(Observable& o) {
		observables3d.push_back(&o);
	}
	
	static void addObservable2d(Observable& o) {
		observables2d.push_back(&o);
	}

	static std::array<int, 3> moveFreqs;
	static int attemptMove();

	static int targetVolume;
	static int target2Volume;

	static int thermal;

	static double k3_s;
	

private:
	static std::default_random_engine rng;

	static double k0;
	static double k3;

	static double epsilon;
	static bool measuring;

	static std::vector<Observable*> observables3d;
	static std::vector<Observable*> observables2d;

	static std::vector<int> PerformSweep(int n);

	static bool moveAdd();
	static bool moveDelete();
	static bool moveFlip();
	static bool moveShift();
	static bool moveShiftD();
	static bool moveShiftI();
	static bool moveShiftID();

	static void prepare();
	static void tune();

	//static void grow();
	//static void thermalize();
};
