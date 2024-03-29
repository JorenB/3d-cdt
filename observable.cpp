// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include <iostream>
#include <fstream>
#include <vector>
#include "observable.hpp"

std::default_random_engine Observable::rng(0);  // TODO(JorenB): seed properly
std::string Observable::data_dir = "";
std::vector<bool> Observable::doneL;

void Observable::write() {
    std::string filename = data_dir + "/" + name + "-" + identifier + extension;

	std::ifstream infile(filename);

    std::ofstream file;
    file.open(filename, std::ios::app);

	assert(file.is_open());

	file << output << "\n";
	file.close();
}

void Observable::clear() {
    std::string filename = data_dir + "/" + name + "-" + identifier + extension;

    std::ofstream file;
    file.open(filename, std::ios::app);

	assert(file.is_open());

	file.close();

	initialize();
}

std::vector<Vertex::Label> Observable::sphere(Vertex::Label origin, int radius) {
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;


    std::vector<Vertex::Label> vertexList;
	std::vector<Vertex::Label> flippedVertices;
    doneL.at(origin) = true;
    thisDepth.push_back(origin);
	flippedVertices.push_back(origin);

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
				if (!doneL.at(neighbor)) {
					flippedVertices.push_back(neighbor);
					nextDepth.push_back(neighbor);
					doneL.at(neighbor) = true;
					if(currentDepth == radius-1) vertexList.push_back(neighbor);
				}
			}
		}
        thisDepth = nextDepth;
        nextDepth.clear();
    }

	for (auto v : flippedVertices) {
		doneL.at(v) = false;
	}

    return vertexList;
}

std::vector<Vertex::Label> Observable::sphere2d(Vertex::Label origin, int radius) {
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;

    std::vector<Vertex::Label> vertexList;
    std::vector<Vertex::Label> flippedVertices;
    doneL.at(origin) = true;
    thisDepth.push_back(origin);
	flippedVertices.push_back(origin);

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
				if (neighbor->time != origin->time) continue;
				if (!doneL.at(neighbor)) {
					flippedVertices.push_back(neighbor);
					nextDepth.push_back(neighbor);
					doneL.at(neighbor) = true;
					if(currentDepth == radius-1) vertexList.push_back(neighbor);
				}
			}
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

	for (auto v : flippedVertices) {
		doneL.at(v) = false;
	}

    return vertexList;
}

std::vector<Tetra::Label> Observable::sphereDual(Tetra::Label origin, int radius) {
    // TODO(JorenB): optimize BFS (see sphere())
	std::vector<Tetra::Label> done;
    std::vector<Tetra::Label> thisDepth;
    std::vector<Tetra::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    std::vector<Tetra::Label> tetraList;

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto t : thisDepth) {
            for (auto neighbor : t->tnbr) {
               if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
                   nextDepth.push_back(neighbor);
                   done.push_back(neighbor);
                   if (currentDepth == radius - 1) tetraList.push_back(neighbor);
               }
            }
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

    return tetraList;
}

std::vector<Triangle::Label> Observable::sphere2dDual(Triangle::Label origin, int radius) {
	std::vector<Triangle::Label> done;
    std::vector<Triangle::Label> thisDepth;
    std::vector<Triangle::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    std::vector<Triangle::Label> triangleList;

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto t : thisDepth) {
            for (auto neighbor : t->trnbr) {
               if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
                   nextDepth.push_back(neighbor);
                   done.push_back(neighbor);
                   if (currentDepth == radius - 1) triangleList.push_back(neighbor);
               }
            }
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

    return triangleList;
}
