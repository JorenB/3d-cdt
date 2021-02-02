// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#include <vector>
#include <algorithm>
#include "vertex.hpp"
#include "tetra.hpp"

bool Vertex::neighborsVertex(Vertex::Label v) {
	Vertex::Label vc = *this;
	if (v == vc) return false;

	auto t = tetra;
	std::vector<Tetra::Label> current = {t};
	std::vector<Tetra::Label> next = {};
	std::vector<Tetra::Label> done = {};

	do {
		for (auto tc : current) {
			for (auto tcn : tc->tnbr) {
				if (!tcn->hasVertex(vc)) continue;
				if (std::find(done.begin(), done.end(), tcn) == done.end()) {
					if (tcn->hasVertex(v)) return true;
					done.push_back(tcn);
					next.push_back(tcn);
				}
			}
		}
		current = next;
		next.clear();
	} while (current.size() > 0);

	return false;
}

