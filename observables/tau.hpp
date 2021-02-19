// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "../observable.hpp"
#include "../universe.hpp"

class Tau : public Observable {
public:
	Tau(std::string id) : Observable(id) { name = "tau"; }

	void process();
		
private:
	void initialize() { }
};
