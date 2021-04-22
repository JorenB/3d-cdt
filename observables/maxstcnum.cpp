// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "maxstcnum.hpp"

void MaxSTCNum::process() {
	std::string tmp = "";
	
	std::vector<int> maxcs(Universe::nSlices, 0);

	for (auto v : Universe::vertices) {
		if (maxcs[v->time] < v->cnum) maxcs[v->time] = v->cnum;
	}

	for (auto m : maxcs) {
		tmp += std::to_string(m);
		tmp += " ";
	}
	tmp.pop_back();
	
	output = tmp;
}
