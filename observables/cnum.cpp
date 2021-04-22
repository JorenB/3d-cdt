// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "cnum.hpp"

void CNum::process() {
	std::string tmp = "";
	
	std::array<int, 750> histogram;
	std::fill(histogram.begin(), histogram.end(), 0);
	
	for (auto v : Universe::vertices) {
		if (Universe::sliceSizes[v->time] != Simulation::target2Volume) continue;
		if (v->scnum > histogram.size() - 1) {
			printf("overflow. cnum: %d\n", v->scnum);
			continue;
		}
		histogram.at(v->scnum) += 1;
	}

	for (auto h : histogram) {
		tmp += std::to_string(h);
		tmp += " ";
	}
	tmp.pop_back();
	
	output = tmp;
}
