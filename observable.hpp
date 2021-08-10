// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <chrono>
#include <string>
#include <algorithm>
#include "universe.hpp"
#include "simulation.hpp"

class Observable {
public:
	std::string name;

	Observable(std::string identifier_) {
		identifier = identifier_;
		//identifier = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	void measure() {
		process();
		write();
	}

	void clear();

	static std::string data_dir;
private:
	std::string identifier;

protected:
	static std::default_random_engine rng;
	virtual void process() = 0;
	virtual void initialize() = 0;

	void write();

	static std::vector<bool> doneL;

	// toolbox

	static std::vector<Vertex::Label> sphere(Vertex::Label origin, int radius);
	static std::vector<Vertex::Label> sphere2d(Vertex::Label origin, int radius);
	static std::vector<Tetra::Label> sphereDual(Tetra::Label origin, int radius);
	static std::vector<Triangle::Label> sphere2dDual(Triangle::Label origin, int radius);

	static int distance(Vertex::Label v1, Vertex::Label v2);
	static int distanceDual(Tetra::Label t1, Tetra::Label t2);
	

	std::string extension = ".dat";
	std::string output;
};
