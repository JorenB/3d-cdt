// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#pragma once

#include "pool.hpp"
#include "vertex.hpp"
#include "halfedge.hpp"
#include <iostream>
#include <typeinfo>

class Tetra : public Pool<Tetra> {
public:
	static const unsigned pool_size = 5000000;
	enum Type { THREEONE, ONETHREE, TWOTWO };
	int time;  // Slab number

	Type type;

	inline const char* ToString(Tetra::Type t) {
		switch (t)
		{
			case Tetra::Type::THREEONE: return "31";
			case Tetra::Type::ONETHREE: return "13";
			case Tetra::Type::TWOTWO: return "22";
		}
	}

	void setVertices(Pool<Vertex>::Label v0, Pool<Vertex>::Label v1, Pool<Vertex>::Label v2, Pool<Vertex>::Label v3) {
		if (v0->time == v1->time && v0->time == v2->time) type = THREEONE;
		if (v1->time == v2->time && v1->time == v3->time) type = ONETHREE;
		if (v0->time == v1->time && v2->time == v3->time) type = TWOTWO;
		assert(v0->time != v3->time);

		vs = {v0, v1, v2, v3};

		time = v0->time;
	}

	void setHalfEdges(Pool<HalfEdge>::Label h0, Pool<HalfEdge>::Label h1, Pool<HalfEdge>::Label h2) {
		hes = {h0, h1, h2};
	}

	HalfEdge::Label getHalfEdgeFrom(Vertex::Label v) {
		for (int i = 0; i < 3; i++) {
			if (hes[i]->vs[0] == v) return hes[i];
		}

		return false;
	}
	
	HalfEdge::Label getHalfEdgeTo(Vertex::Label v) {
		for (int i = 0; i < 3; i++) {
			if (hes[i]->vs[1] == v) return hes[i];
		}

		return false;
	}

	void setTetras(Pool<Tetra>::Label t0, Pool<Tetra>::Label t1, Pool<Tetra>::Label t2, Pool<Tetra>::Label t3) {
		tnbr = {t0, t1, t2, t3};
	}

	bool is31() { return type == THREEONE; }
	bool is13() { return type == ONETHREE; }
	bool is22() { return type == TWOTWO; }

	bool hasVertex(Pool<Vertex>::Label v) {
		for (int i = 0; i < 4; i++) {
			if (vs[i] == v) return true;
		}
		return false;
	}

	bool neighborsTetra(Pool<Tetra>::Label t) {
		for (int i = 0; i < 4; i++) {
			if (tnbr[i] == t) return true;
		}

		return false;
	}

	Tetra::Label getTetraOpposite(Vertex::Label v) {
		assert(hasVertex(v));

		for (int i = 0; i < 4; i++) {
			if (vs[i] == v) return tnbr[i];
		}
		assert(false);
	}

	Vertex::Label getVertexOpposite(Vertex::Label v) {
		auto tn = getTetraOpposite(v);

		std::array<Vertex::Label, 3> face;
		int i = 0;
		for (auto tv : vs) {
			if (tv != v) { 
				face[i] = tv;
				i++;
			}
		}

		for (auto tnv : tn->vs) {
			if ((tnv != face[0]) && (tnv != face[1]) && (tnv != face[2])) return tnv;
		}

		assert(false);
	}

	Vertex::Label getVertexOppositeTetra(Tetra::Label tn) {
		for (int i = 0; i < 4; i++) {
			if (tnbr[i] == tn) return vs[i];
		}

		assert(false);
	}

	void exchangeTetraOpposite(Vertex::Label v, Tetra::Label tn) {
		for (int i = 0; i < 4; i++) {
			if (vs[i] == v) tnbr[i] = tn;
		}
	}



	void log() {
		Pool<Tetra>::Label t = *this;
		printf("t: %d - %s\n", (int) t, ToString(type));
		printf("\t");
		for (int i = 0; i < 4; i++) printf("v%d: %d ", i, vs[i]);
		printf("\n\t");
		for (int i = 0; i < 4; i++) printf("t%d: %d ", i, tnbr[i]);
		printf("\n");
	}

//private:
	std::array<Pool<Tetra>::Label, 4> tnbr;
	std::array<Pool<Vertex>::Label, 4> vs;
	std::array<Pool<HalfEdge>::Label, 3> hes = {-1, -1, -1};

};
