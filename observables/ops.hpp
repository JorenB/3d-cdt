// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "../observable.hpp"
#include "../universe.hpp"

class OPs : public Observable {
public:
	OPs(std::string id) : Observable(id) { name = "ops"; }

	void process();
		
private:
	void initialize() { }
};
