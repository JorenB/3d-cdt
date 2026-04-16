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

/**
 * Returns if the vertex itself forms a triangle with vertices v0 and v1.
 * @param startingTetra tetrahedron that contains edge (v0, v1)
 * @throw Requires vertex itself, v0, and v1 to be all distinct.
 */
bool Vertex::formsTriangle(Tetra::Label startingTetra, Vertex::Label v0, Vertex::Label v1) {
	Vertex::Label vc = *this;

	// TODO: Assertions could be removed for optimization
	// Assuming a proper simplicial manifold, so triangle required distinct vertices
	assert(v0 != vc);
	assert(v1 != vc);
	assert(v0 != v1);

	// Requires startingTetra to contain edge (v0, v1)
	assert(startingTetra->hasEdge(v0, v1) && "startingTetra does not contain edge" );

	std::unordered_map<int, bool> explored;
	explored.reserve(v0->cnum);

	std::vector<Tetra::Label> currentBuffer = {startingTetra};
	std::vector<Tetra::Label> nextBuffer = {};

	// Perform breadth-first search around edge (v0,v1)
	do {
		for (auto currentTetra : currentBuffer) {
			for (auto neighbourTetra : currentTetra->tnbr) {
				// Stay around edge (v0, v1)
				if (!neighbourTetra->hasEdge(v0, v1)) continue;
				// Only explore new tetras
				if (explored.find(neighbourTetra) != explored.end()) continue;

				if (neighbourTetra->hasVertex(vc)) return true;
				explored[neighbourTetra] = true;
				nextBuffer.push_back(neighbourTetra);
			}
		}
		currentBuffer = nextBuffer;
		nextBuffer.clear();
	} while (currentBuffer.size() > 0);

	return false;
}