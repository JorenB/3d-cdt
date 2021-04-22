// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "../observable.hpp"
#include "../universe.hpp"

class MaxSTCNum : public Observable {
public:
	MaxSTCNum(std::string id) : Observable(id) { name = "maxstcnum"; }

	void process();

private:
	void initialize() { }
};
