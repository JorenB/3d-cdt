// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include <iostream>
#include <fstream>
#include "observable.hpp"

std::default_random_engine Observable::rng(0);  // TODO: seed properly

void Observable::write() {
    std::string filename = /*data_dir + */name + "-" + identifier + extension;

	std::ifstream infile(filename);
	//if (!infile.good()) { printf("output file deleted\n"); exit(1); }
	//infile.close();

    std::ofstream file;
    file.open(filename, std::ios::app); // | std::ios::out);

	assert(file.is_open());

	file << output << "\n";
	file.close();

//    std::cout << filename << "\n";
}

void Observable::clear() {
    std::string filename = data_dir + name + "-" + identifier + extension;

    std::ofstream file;
    file.open(filename, std::ios::app);// | std::ios::trunc);
	
	assert(file.is_open());

	file.close();

	initialize();
}

std::vector<Vertex::Label> Observable::sphere(Vertex::Label origin, int radius) {
	std::vector<Vertex::Label> done;
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    std::vector<Vertex::Label> vertexList;

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
               if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
                   nextDepth.push_back(neighbor);
                   done.push_back(neighbor);
                   if(currentDepth == radius-1) vertexList.push_back(neighbor);
               }
            }
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

    return vertexList;
}

std::vector<Vertex::Label> Observable::sphere2d(Vertex::Label origin, int radius) {
	std::vector<Vertex::Label> done;
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    std::vector<Vertex::Label> vertexList;

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
				if (neighbor->time != origin->time) continue;
				if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
					nextDepth.push_back(neighbor);
					done.push_back(neighbor);
					if(currentDepth == radius-1) vertexList.push_back(neighbor);
				}
			}
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

    return vertexList;
}

std::vector<Tetra::Label> Observable::sphereDual(Tetra::Label origin, int radius) {
	std::vector<Tetra::Label> done;
    std::vector<Tetra::Label> thisDepth;
    std::vector<Tetra::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    std::vector<Tetra::Label> tetraList;

    for (int currentDepth = 0; currentDepth < radius; currentDepth++) {
        for (auto t : thisDepth) {
			//std::vector<Tetra::Label> tnbr = t->tnbr;
            for (auto neighbor : t->tnbr) {
               if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
                   nextDepth.push_back(neighbor);
                   done.push_back(neighbor);
                   if(currentDepth == radius-1) tetraList.push_back(neighbor);
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
                   if(currentDepth == radius-1) triangleList.push_back(neighbor);
               }
            }
        }
        thisDepth = nextDepth;
        nextDepth.clear();
    }

    return triangleList;
}
