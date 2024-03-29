// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "hausdorff2d.hpp"

void Hausdorff2d::process() {
    std::string tmp = "";
	std::vector<int> profile = {};
	int max_epsilon = 30;
	profile.resize(max_epsilon, 0);

	int vmax = 0;
	for (auto v : Universe::vertices) {
		if (v > vmax) vmax = v;
	}
	doneL.resize(vmax + 1, false);

	if (!average) {
		for (int i = 1; i <= max_epsilon; i++) {
			Vertex::Label v;
			do {
				v = Universe::verticesAll.pick();
			} while (Universe::sliceSizes[v->time] != Simulation::target2Volume);

			auto s1 = sphere2d(v, i);

			profile.at(i-1) = s1.size();
		}
	} else if (average) {
		printf("avg\n");

		int counter = 0;
		for (auto v : Universe::verticesAll) {
			if (Universe::sliceSizes[v->time] != Simulation::target2Volume) continue;
			counter++;

			auto singleProfile = distanceList2d(v);
			if (singleProfile.size() > profile.size()) profile.resize(singleProfile.size(), 0);

			std::string tmp = "";
			for (int i = 0; i < singleProfile.size(); i++) {
				profile.at(i) += singleProfile.at(i);
				tmp += std::to_string(profile.at(i) / counter);
				tmp += " ";
			}
		}

		for (int i = 0; i < profile.size(); i++) {
			profile.at(i) /= counter;
		}
	}

	for (auto d : profile) {
		tmp += std::to_string(d);
		tmp += " ";
	}
	tmp.pop_back();

    output = tmp;
}

std::vector<int> Hausdorff2d::distanceList2d(Vertex::Label origin) {
    // TODO(JorenB): optimize BFS
	std::vector<int> dsts;
	std::vector<Vertex::Label> done;
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

	int currentDepth = 0;
	do {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
				if (neighbor->time != origin->time) continue;
				if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
					nextDepth.push_back(neighbor);
					done.push_back(neighbor);
				}
			}
        }
		dsts.push_back(thisDepth.size());

        thisDepth = nextDepth;
        nextDepth.clear();
		currentDepth++;
    } while (thisDepth.size() > 0);

    return dsts;
}
