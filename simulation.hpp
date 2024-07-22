// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <random>
#include <vector>
#include <string>
#include "universe.hpp"

class Observable;
class Simulation {
public:
	static double lambda;

	static void start();

	static void addObservable3d(Observable& o) { observables3d.push_back(&o); }

	static void addObservable2d(Observable& o) { observables2d.push_back(&o); }

	static std::array<int, 3> moveFreqs;
	static int attemptMove();




private:
	static std::default_random_engine rng;

	static bool measuring;

	static std::vector<Observable*> observables3d;
	static std::vector<Observable*> observables2d;

	static std::vector<int> performSweep(int n);

	static bool moveAdd();
	static bool moveDelete();
	static bool moveFlip();
	static bool moveShift();
	static bool moveShiftD();
	static bool moveShiftI();
	static bool moveShiftID();
	
	static std::pair<int,std::pair<int, int>> getDeltaABCD(int a[3]);

	static void prepare();
	static void tune();
	static void tunek0();
};
