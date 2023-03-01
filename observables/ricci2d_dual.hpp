// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#pragma once

#include <string>
#include <vector>
#include "../observable.hpp"
#include "../universe.hpp"

class Ricci2dDual : public Observable {
public:
	Ricci2dDual(std::string id) : Observable(id) { eps_max = 10; };
	Ricci2dDual(std::string id, int eps_max_) : Observable(id) { name = "ricci2d_dual"; eps_max = eps_max_; }

	void process();

private:
	int eps_max;
	std::vector<int> epsilons;
	std::vector<bool> doneLr;
	std::vector<bool> triangleLr;

	double averageSphereDistanceDual(Triangle::Label p1, int epsilon);

	void initialize() { };
};
