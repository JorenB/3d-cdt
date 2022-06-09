// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "pool.hpp"

class Tetra;
class Vertex : public Pool<Vertex> {
public:
	static const unsigned pool_size = 3000000;
	int time;  // Slice number

	int scnum;  // Spatial coordination number
	int cnum;

	Pool<Tetra>::Label tetra = -1;  // Some 31-simplex containing this vertex in its base

	bool neighborsVertex(Vertex::Label v);

private:
};
