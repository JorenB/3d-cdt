// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include <unordered_map>
#include <algorithm>
#include "minbu.hpp"

void Minbu::process() {
	std::string tmp = "";

	int slice;
	for (int i = 0; i < Universe::nSlices; i++) {
		if (Universe::sliceSizes[i] == Simulation::target2Volume) {
			slice = i;
			break;
		}
	}

	std::unordered_map<int, HalfEdge::Label> sliceEdges;
	for (auto he : Universe::halfEdges) {
		if (he->vs[0]->time == slice) sliceEdges[he] = he;
	}

	std::unordered_map<int, bool> done;
	std::vector<std::array<Vertex::Label, 3>> minNecks;

	for (auto& it : sliceEdges) {
		auto he = it.second;
		if (done.find(he) != done.end()) continue;

        auto start = he->next->adj->next;

		std::vector<HalfEdge::Label> fronts;
		HalfEdge::Label cur = start;
		do {
			fronts.push_back(cur);
			cur = cur->adj->next;
		} while (cur->adj->next->adj != he);

		start = he->prev->adj->prev;

		std::vector<HalfEdge::Label> backs;
		cur = start;
		do {
			backs.push_back(cur);
			cur = cur->adj->prev;
		} while (cur->adj->prev->adj != he);

		for (auto f : fronts) {
			for (auto b : backs) {
				if (f->vs[1] == b->vs[0] && done.find(b) == done.end() && done.find(f) == done.end()) {
					std::array<Vertex::Label, 3> nek = {f->vs[0], b->vs[1], f->vs[1]};
					std::sort(nek.begin(), nek.end());
					minNecks.push_back(nek);
					for (auto t : Universe::tetras31) {
						if (t->hasVertex(f->vs[0]) && t->hasVertex(b->vs[1]) && t->hasVertex(f->vs[1])) {
							t->log();
							std::cout << std::flush;
							assert(false);
						}
					}
				}
			}
		}
		done[he] = true;
		done[he->adj] = true;
	}

	std::vector<std::array<Vertex::Label, 2>> neckLinks;
	std::sort(minNecks.begin(), minNecks.end());
	for (auto n : minNecks) {
		neckLinks.push_back({n[0], n[1]});
		neckLinks.push_back({n[1], n[2]});
		neckLinks.push_back({n[0], n[2]});
	}

	printf("------\n");

	std::vector<int> histogram;
	histogram.resize(Simulation::target2Volume / 2 + 1, 0);
	for (auto neck : minNecks) {
		Triangle::Label tr;
		for (auto tri : Universe::triangles) {
			if (tri->hasVertex(neck[0]) || tri->hasVertex(neck[1]) || tri->hasVertex(neck[2])) {
				tr = tri;
				break;
			}
		}

        auto origin = tr;

		std::vector<Triangle::Label> tdone;
		std::vector<Triangle::Label> thisDepth;
		std::vector<Triangle::Label> nextDepth;

		tdone.push_back(origin);
		thisDepth.push_back(origin);

		std::vector<Triangle::Label> triangleList;

		int totalTr = 0;
		int currentDepth = 0;
		do {
			for (auto t : thisDepth) {
				for (auto he : t->hes) {
					auto v1 = he->vs[0];
					auto v2 = he->vs[1];
					if ((v1 == neck[0] || v1 == neck[1] || v1 == neck[2]) && (v2 == neck[0] || v2 == neck[1] || v2 == neck[2])) {
						continue;
					}
					auto neighbor = he->adj->triangle;
					if (std::find(tdone.begin(), tdone.end(), neighbor) == tdone.end()) {
						nextDepth.push_back(neighbor);
						tdone.push_back(neighbor);
						totalTr++;
					}
				}
			}
			thisDepth = nextDepth;
			nextDepth.clear();
			currentDepth++;
		} while (thisDepth.size() > 0);

		if (totalTr + 1 < Simulation::target2Volume / 2) histogram.at(totalTr + 1) += 1;
		else
            histogram.at(Simulation::target2Volume - totalTr - 1) += 1;
	}

	// REMEMBER: remove first three. remove every even-numbered bin.
	for (auto h : histogram) {
		tmp += std::to_string(h) + " ";
	}

	output = tmp;
}
