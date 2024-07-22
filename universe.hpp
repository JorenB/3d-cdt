// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <vector>
#include <random>
#include <string>
#include "globals.hpp"
#include "vertex.hpp"
#include "halfedge.hpp"
#include "triangle.hpp"
#include "tetra.hpp"
#include "pool.hpp"
#include "bag.hpp"

class Universe {
public:
	static int nSlices;
	static std::vector<int> slabSizes;
	static std::vector<int> sliceSizes;
	static std::string fID;
	static std::string OutFile;

	
	//static int A1,A2,B1,B2,C,D;

	static Bag<Tetra, Tetra::pool_size> tetrasAll;  // All tetrahedra.
	static Bag<Tetra, Tetra::pool_size> tetras31;  // All 31-tetrahedra.
	static Bag<Vertex, Vertex::pool_size> verticesAll;  // All vertices.
	static Bag<Vertex, Vertex::pool_size> verticesSix;  // Vertices with six tetrahedra, suitable for (6,2)-move

	static bool initialize();

	static bool exportGeometry(std::string geometryFilename);

	static bool move26(Tetra::Label t);
	static bool move62(Vertex::Label v);

	static bool move44(Tetra::Label t123, Tetra::Label t134);

	static bool move23u(Tetra::Label t31, Tetra::Label t22);
	static bool move32u(Tetra::Label t31, Tetra::Label t22l, Tetra::Label t22r);

	static bool move23d(Tetra::Label t13, Tetra::Label t22);
	static bool move32d(Tetra::Label t13, Tetra::Label t22l, Tetra::Label t22r);

	static void updateVertexData();
	static void updateHalfEdgeData();
	static void updateTriangleData();

	static void updateGeometry();

	static std::vector<Vertex::Label> vertices;
	static std::vector<Tetra::Label> tetras;
	static std::vector<HalfEdge::Label> halfEdges;
	static std::vector<Triangle::Label> triangles;

	static std::vector<std::vector<Vertex::Label>> vertexNeighbors;
	static std::vector<std::array<Triangle::Label, 3>> triangleNeighbors;

	static int getMaxord();
	static int getMaxordSlice();
	
	static void check();
	
	static void countABC();
	static void InitABC();

private:
	Universe() {}
	static std::default_random_engine rng;
};
