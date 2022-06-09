// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <string>
#include <vector>
#include "../observable.hpp"
#include "../universe.hpp"

class Hausdorff2dDual : public Observable {
    public:
		using Observable::Observable;
        Hausdorff2dDual(std::string id) : Observable(id) { name = "hausdorff2d_dual"; }

        void process();

	private:
		int max_epsilon;

		void initialize() { }

		std::vector<int> distanceList2dDual(Triangle::Label origin);
};
