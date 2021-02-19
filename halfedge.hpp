// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once
#include "pool.hpp"

class Vertex;
class Triangle;
class Tetra;
class HalfEdge : public Pool<HalfEdge> {
public:
	static const unsigned pool_size = 1000000;

	std::array<Pool<Vertex>::Label, 2> vs = {-1, -1};

	Pool<HalfEdge>::Label adj = -1;  // The adjacent half-edge
	Pool<HalfEdge>::Label next = -1;  // The next half-edge
	Pool<HalfEdge>::Label prev = -1;  // The previous half-edge

	Pool<Tetra>::Label tetra = -1;  // The 31-simplex containing this half-edge
	Pool<Triangle>::Label triangle = -1;

	void setVertices(Pool<Vertex>::Label ve, Pool<Vertex>::Label vf) {
		vs[0] = ve;
		vs[1] = vf;
	}

	HalfEdge::Label getAdjacent() {
		return adj;
	}

	void setAdjacent(HalfEdge::Label he) {
		he->adj = *this;
		adj = he;
	}

private:

};
