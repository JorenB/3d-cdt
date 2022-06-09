// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "vertex.hpp"
#include "tetra.hpp"

bool Vertex::neighborsVertex(Vertex::Label v) {
	Vertex::Label vc = *this;
	if (v == vc) return false;

	auto t = tetra;

	std::unordered_map<int, bool> done;
	done.reserve(v->cnum);

	std::vector<Tetra::Label> current = {t};
	std::vector<Tetra::Label> next = {};

	do {
		for (auto tc : current) {
			for (auto tcn : tc->tnbr) {
				if (!tcn->hasVertex(vc)) continue;
				if (done.find(tcn) == done.end()) {
					if (tcn->hasVertex(v)) return true;
					done[tcn] = true;
					next.push_back(tcn);
				}
			}
		}
		current = next;
		next.clear();
	} while (current.size() > 0);

	return false;
}
