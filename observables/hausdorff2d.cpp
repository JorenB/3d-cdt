#include "hausdorff2d.hpp"

void Hausdorff2d::process() {
    std::string tmp = "";
	Vertex::Label v;
	do {
		v = Universe::verticesAll.pick();
	} while (Universe::sliceSizes[v->time] != Simulation::target2Volume);

	auto dsts = distanceList2d(v);

	for (auto d : dsts) {
		tmp += std::to_string(d);
		tmp += " ";
	}
	tmp.pop_back();
	
    output = tmp; 
}

std::vector<int> Hausdorff2d::distanceList2d(Vertex::Label origin) {
	std::vector<int> dsts;
	std::vector<Vertex::Label> done;
    std::vector<Vertex::Label> thisDepth;
    std::vector<Vertex::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);

    //std::vector<Vertex::Label> vertexList;

	int currentDepth = 0;
	do {
        for (auto v : thisDepth) {
            for (auto neighbor : Universe::vertexNeighbors[v]) {
				if (neighbor->time != origin->time) continue;
				if (std::find(done.begin(), done.end(), neighbor) == done.end()) {
					nextDepth.push_back(neighbor);
					done.push_back(neighbor);
					//if(currentDepth == radius-1) vertexList.push_back(neighbor);
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
