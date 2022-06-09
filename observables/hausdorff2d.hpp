// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#pragma once

#include <string>
#include <vector>
#include "../observable.hpp"
#include "../universe.hpp"

class Hausdorff2d : public Observable {
    public:
		using Observable::Observable;
        Hausdorff2d(std::string id) : Observable(id) { name = "hausdorff2d"; average = false; }
        Hausdorff2d(std::string id, bool average_) : Observable(id) { name = "hausdorff2d"; average = average_; }

        void process();

	private:
		int max_epsilon;

		bool average;

		void initialize() { }

		std::vector<int> distanceList2d(Vertex::Label origin);
};
