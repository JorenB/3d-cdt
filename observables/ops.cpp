// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "ops.hpp"

void OPs::process() {
	int n31 = Universe::tetras31.size();
	int n3 = Universe::tetrasAll.size();
	int n22 = n3 - 2 * n31;

	int n0 = Universe::verticesAll.size();

	double op2 = ((double) n22)/((double) n3);
	double S = - Simulation::k0 * n0 + Simulation::k3 * n3;


	output = std::to_string(n0) + " " 
		+ std::to_string(n22) + " "
	   	+ std::to_string(n31) + " "
		+ std::to_string(n3) + " "
		+ std::to_string(op2) + " "
		+ std::to_string(S) + " "
		+ std::to_string(Simulation::k3);
}
