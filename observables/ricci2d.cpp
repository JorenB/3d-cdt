// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "ricci2d.hpp"
#include <chrono>

void Ricci2d::process() {
	epsilons = {};
	for (int i = 1; i <= eps_max; i++) {
		epsilons.push_back(i);
	}

    std::vector<double> epsilonDistanceList;
	std::vector<Vertex::Label> origins;

	int vmax = 0;
	for (auto v : Universe::vertices) {
		if (v > vmax) vmax = v;
	}
	doneL.resize(vmax + 1, false);
	doneLr.resize(vmax + 1, false);
	vertexLr.resize(vmax + 1, false);

	for (std::vector<int>::iterator it = epsilons.begin(); it != epsilons.end(); it++) {
		Vertex::Label v;
		do {	
			v = Universe::verticesAll.pick();
		} while (Universe::sliceSizes[v->time] != Simulation::target2Volume);

		origins.push_back(v);
	}

	for (int i = 0; i < epsilons.size(); i++) {
		int epsilon = epsilons[i];
		// printf("%d - ", epsilon);

		auto origin = origins[i];

        double averageDistance = averageSphereDistance(origin, epsilon);
        epsilonDistanceList.push_back(averageDistance);

		// printf("%f\n", averageDistance);
    }

    std::string tmp = "";
    for (double dst : epsilonDistanceList) {
        tmp += std::to_string(dst);
        tmp += " ";
    }
	tmp.pop_back();
    output = tmp;
}

double Ricci2d::averageSphereDistance(Vertex::Label p1, int epsilon) {
    auto s1 = sphere2d(p1, epsilon);
	if (s1.size() == 0) return 0.0;
	int t1 = p1->time;
	std::uniform_int_distribution<> rv(0, s1.size()-1);
    auto p2 = s1.at(rv(rng));
    auto s2 = sphere2d(p2, epsilon);
	if (s2.size() == 0) return 0.0;
	int t2 = p2->time;
	if (s2.size() < s1.size()) {
		auto stmp = s1;
		s1 = s2;
		s2 = stmp;
	}
	
    std::vector<int> distanceList;

	///using std::chrono::high_resolution_clock;
    ///using std::chrono::duration_cast;
    ///using std::chrono::duration;
    ///using std::chrono::milliseconds;

	///auto t1 = high_resolution_clock::now();
	for (auto b : s1) {
		for (int i = 0; i < doneLr.size(); i++) {
			doneLr.at(i) = false;
			vertexLr.at(i) = false;
		}
		for (auto v : s2) {
			vertexLr.at(v) = true;
		}

		int countdown = s2.size();

		std::vector<Vertex::Label> thisDepth;
		std::vector<Vertex::Label> nextDepth;

		doneLr.at(b) = true;
		thisDepth.push_back(b);

		for (int currentDepth = 0; currentDepth < 3 * epsilon + 1; currentDepth++) {
			for (auto v : thisDepth) {
				if (vertexLr.at(v)) {
					distanceList.push_back(0);
					vertexLr.at(v) = false;
					countdown--;
				}

				for (auto neighbor : Universe::vertexNeighbors[v]) {
					if (neighbor->time != v->time) continue;
					//if (neighbor->time == tmax || neighbor->time == tmin) continue;
					if (!doneLr.at(neighbor)) {
						nextDepth.push_back(neighbor);
						doneLr.at(neighbor) = true;

						if (vertexLr.at(neighbor)) {
							distanceList.push_back(currentDepth + 1);
							vertexLr.at(neighbor) = false;
							countdown--;
						}
					}
					if (countdown == 0) break;
				}
				if (countdown == 0) break;
			}
			thisDepth = nextDepth;
			nextDepth.clear();
			if (countdown == 0) break;
		}
		assert(countdown == 0);
	}
    //auto t2 = high_resolution_clock::now();

    //auto ms_int = duration_cast<milliseconds>(t2 - t1);
	//printf("eps: %d, t: %d\n", epsilon, ms_int);

    int distanceSum = std::accumulate(distanceList.begin(), distanceList.end(), 0);
    double averageDistance = static_cast<double>(distanceSum)/static_cast<double>(epsilon*distanceList.size());

    return averageDistance;
}
