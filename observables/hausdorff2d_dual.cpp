#include "hausdorff2d_dual.hpp"

void Hausdorff2dDual::process() {
    std::string tmp = "";
	
	std::uniform_int_distribution<> triangleGen(0, Universe::triangles.size() - 1);
	Triangle::Label tr;
	do {
		tr = Universe::triangles.at(triangleGen(rng));
	} while (Universe::sliceSizes[tr->time] != Simulation::target2Volume);
//	} while (tr->time != 1);

	auto dsts = distanceList2dDual(tr);
	
	for (auto d : dsts) {
		tmp += std::to_string(d);
		tmp += " ";
	}
	tmp.pop_back();

    output = tmp; 
}

std::vector<int> Hausdorff2dDual::distanceList2dDual(Triangle::Label origin) {
	std::vector<int> dsts;
	std::vector<Triangle::Label> done;
    std::vector<Triangle::Label> thisDepth;
    std::vector<Triangle::Label> nextDepth;

    done.push_back(origin);
    thisDepth.push_back(origin);


	int currentDepth = 0;
	do {

        for (auto t : thisDepth) {
            for (auto neighbor : t->trnbr) {
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
