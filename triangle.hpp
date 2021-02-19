// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "pool.hpp"
#include "vertex.hpp"
#include "halfedge.hpp"

class Triangle : public Pool<Triangle> {
public:
	static const unsigned pool_size = 1000000;
	int time;  // Slice number

	void setVertices(Vertex::Label v0, Vertex::Label v1, Vertex::Label v2) {
		vs = {v0, v1, v2};

		assert(v0->time == v1->time);
		assert(v0->time == v2->time);
		time = v0->time;
	}
		
	void setHalfEdges(HalfEdge::Label h0, HalfEdge::Label h1, HalfEdge::Label h2) {
		hes = {h0, h1, h2};
	}

	void setTriangleNeighbors(Triangle::Label tr0, Triangle::Label tr1, Triangle::Label tr2) {
		trnbr = {tr0, tr1, tr2};
	}

	std::array<Vertex::Label, 3> vs;
	std::array<HalfEdge::Label, 3> hes;
	std::array<Triangle::Label, 3> trnbr;

private:

};
