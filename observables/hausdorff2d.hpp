#pragma once

#include "../observable.hpp"
#include "../universe.hpp"

class Hausdorff2d : public Observable {
    public:
		using Observable::Observable;
        Hausdorff2d(std::string id) : Observable(id) { name = "hausdorff2d"; };

        void process();

	private:
		int max_epsilon;

		void initialize() { }

		std::vector<int> distanceList2d(Vertex::Label origin);
};
