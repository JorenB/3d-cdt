// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "tau.hpp"

void Tau::process() {
	int n22 = 0;
	for (auto t : Universe::tetrasAll) {
		if (t->is22()) n22++;
	}
	assert(n22 == Universe::tetrasAll.size() - 2*Universe::tetras31.size());

	double tau = ((double) n22)/((double) Universe::tetrasAll.size());


	output = std::to_string(tau);
}
