// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#pragma once

#include <string>
#include <vector>
#include "../observable.hpp"
#include "../universe.hpp"

class Ricci2d : public Observable {
public:
	Ricci2d(std::string id) : Observable(id) { name = "ricci2d"; eps_max = 10; };
	Ricci2d(std::string id, int eps_max_) : Observable(id) { name = "ricci2d"; eps_max = eps_max_; }

	void process();

private:
	int eps_max;
	std::vector<int> epsilons;
	std::vector<bool> doneLr;
	std::vector<bool> vertexLr;

	double averageSphereDistance(Vertex::Label p1, int epsilon);

	void initialize() { };
};
