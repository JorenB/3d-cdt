// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include "../observable.hpp"
#include "../universe.hpp"

class CNum : public Observable {
public:
	CNum(std::string id) : Observable(id) { name = "cnum"; }

	void process();

private:
	void initialize() { }
};
