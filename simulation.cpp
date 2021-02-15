// Copyright 2020 Joren Brunekreef and Andrzej Görlich
#include "simulation.hpp"
#include "observable.hpp"
#include <vector>

double Simulation::k0 = 0;
double Simulation::k3 = 0;

std::default_random_engine Simulation::rng(0);  // TODO(JorenB): seed properly
int Simulation::targetVolume = 0;
int Simulation::target2Volume = 0;
double Simulation::epsilon = 0.02;
bool Simulation::measuring = false;

std::vector<Observable*> Simulation::observables3d;
std::vector<Observable*> Simulation::observables2d;

std::array<int, 3> Simulation::moveFreqs = {4, 1, 1};

void Simulation::start(int measurements, double k0_, double k3_, int targetVolume_, int target2Volume_, int seed, int thermalSteps, int kSteps, int sweeps) {
	k0 = k0_;
	targetVolume = targetVolume_;
	target2Volume = target2Volume_;

	for (auto o : observables3d) {
		o->clear();
	}
	for (auto o : observables2d) {
		o->clear();
	}

	rng.seed(seed);
	
	k3 = k3_;

	measuring = true;
	
	bool fromThermal = false;  // If the input is already thermalized, skip this part 
	int maN3 = 0;  // Moving average of N3 just for checkup purposes.

	if(fromThermal == false) {  
		// Pump up volume to the target + some moves just for spice
		printf(" * * move to target 3vol * * \n");
		while (Tetra::size() < targetVolume) {
			moveAdd();		
			moveShift();
			moveShiftD();
			moveShiftI();
			moveShiftID();
			moveFlip();
		}  
		printf(" * * done * * \n\n");

		for (int i = 0; i < thermalSteps; i++) {  // thermalization phase
			printf("THERMAL: i: %d\t Target: %d\t Target2d: %d\t CURRENT: %d MovingAverageN3: %d \n",i, targetVolume, target2Volume, Tetra::size(), maN3);
			printf("k0: %g, k3: %g, epsilon: %g \t thermal: %d \t ksteps: %d\n", k0, k3, epsilon, thermalSteps, kSteps);

			for (int j = 0; j < kSteps * 1000; j++) {
				attemptMove();
			}

			tune();  // tune k3 one step

			maN3 *= i;  // multiply the average with the previous i		
			maN3 += Tetra::size();  // add the current size to the sum
			maN3 /= (i + 1);  // calculate average

			if (target2Volume > 0) {
				bool hit = false;
				//printf("move to target 2vol...\n");
				do {
					attemptMove();
					for (auto s : Universe::sliceSizes) {
						if (s == target2Volume) {
							hit = true;
							break;
						}
					}
				} while (!hit);
				//printf("done\n");

				prepare();
				for (auto o : observables2d) {
					//printf("m2d\n");
					o->measure();
				}
			}  // it should be possible to have both two-dimensional and three-dimensional measurements in principle, I think
			else {
				prepare();
				for (auto o : observables3d) {
					printf("m\n");
					o->measure();
				}
			}
		}
	}  // end thermalization

	maN3 = 0; // reset moving average to 0

	for (int i = 0; i < sweeps; i++) {  // measurement phase
		maN3 *= i;
		maN3 += Tetra::size();
		maN3 /= (i+1);

		printf("SWEEPS: i: %d\t Target: %d\t Target2d: %d\t CURRENT: %d MovingAverageN3: %d \n",i, targetVolume, target2Volume, Tetra::size(), maN3);


		sweep(targetVolume * 1000);

		if (observables3d.size() > 0) {
			prepare();
			for (auto o : observables3d) {
				o->measure();
			}
		}

		if (target2Volume > 0) { 
			bool hit = false;
			do {
				attemptMove();
				for (auto s : Universe::sliceSizes) {
					if (s == target2Volume) {
						hit = true;
						break;
					}
				}
			} while (!hit);
			
			prepare();
			for (auto o : observables2d) {
				o->measure();
			}
		}
	}
}

int Simulation::attemptMove() {
	std::array<int, 3> cumFreqs = {0, 0, 0};
	int freqTotal = 0;
	int prevCumFreq = 0;
	for (int i = 0; i < moveFreqs.size(); i++) {
		freqTotal += moveFreqs[i];
		cumFreqs[i] = prevCumFreq + moveFreqs[i];
		prevCumFreq = cumFreqs[i];
	}

	std::uniform_int_distribution<> moveGen(0, freqTotal-1);
	std::uniform_int_distribution<> binGen(0, 1);

	int move = moveGen(rng);

	if (move < cumFreqs[0]) {
		if (binGen(rng) == 0) {
			//printf("a\n");
			if (moveAdd()) return 1;
		} else {
			//printf("d\n");
			if (moveDelete()) return 2;
		}
	} else if (cumFreqs[0] <= move && move < cumFreqs[1]) {
		//printf("f\n");
		if (moveFlip()) { 
			//printf("fls\n"); 
			return 3; 
		}
	} else if (cumFreqs[1] <= move) {
		if (binGen(rng) == 0) {
			//printf("shiftf\n");
			if (binGen(rng) == 0) { 
				//printf("u\n"); 
				if (moveShift()) return 4; 
			}
			else { 
			  //printf("d\n"); 
				if (moveShiftD()) return 4; 
			}
		} else {
			//printf("shiftb\n");
			if (binGen(rng) == 0) { 
				//printf("u\n"); 
				if (moveShiftI()) return 5; 
			}
			else { 
			//printf("d\n"); 
				if (moveShiftID()) return 5; 
			}
		}
	}



	//Universe::check();

	return 0;
}

std::vector<int> Simulation::sweep(int n) {
	std::vector<int> moves(6, 0);
	for (int i = 0; i < n; i++) {
		//printf("mov%d \n", i);

		int move = attemptMove();
		moves[move]++;
		//Universe::check();
	}

	return moves;
}

bool Simulation::moveAdd() {
	double n31 = Universe::tetras31.size();
	int n3 = Universe::tetrasAll.size();

	double edS = exp(1 * k0 - 4 * k3);
	//double rg = n0/(n0+1.0)*n31/(n31+4.0);
	double rg = n31 / (n31 + 2.0);
	double ar = edS*rg;

	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 4.0 : -4.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}

	Tetra::Label t = Universe::tetras31.pick();

	Universe::move26(t);
	return true;
}

bool Simulation::moveDelete() {
	double n31 = Universe::tetras31.size();
	int n3 = Universe::tetrasAll.size();

	double edS = exp(-1 * k0 + 4 * k3);
	//double rg = n0/(n0-1.0)*n31/(n31-4.0);
	double rg = n31/(n31-2.0);
	double ar = edS*rg;

	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? -4.0 : 4.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}


	Vertex::Label v = Universe::verticesAll.pick();
	if (v->cnum != 6) return false;
	if (v->scnum != 3) return false;
	
	Universe::move62(v);

	return true;
}

bool Simulation::moveFlip() {
	Tetra::Label t012 = Universe::tetras31.pick();
	std::uniform_int_distribution<> neighborGen(0, 2);

	Tetra::Label t230 = t012->tnbr[neighborGen(rng)];

	if (!t230->is31()) return false;

	if (!t012->tnbr[3]->neighborsTetra(t230->tnbr[3])) return false;

	return Universe::move44(t012, t230);
}

bool Simulation::moveShift() {
	double edS = exp(- 1.0 * k3);
	double rg = 1.0;
	double ar = edS*rg;
	int n3 = Universe::tetrasAll.size();
	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 1.0 : -1.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}

	Tetra::Label t = Universe::tetras31.pick();
	std::uniform_int_distribution<> neighborGen(0, 2);
	Tetra::Label tn = t->tnbr[neighborGen(rng)];

	if (!tn->is22()) return false;

	return Universe::move23u(t, tn);
}

bool Simulation::moveShiftD() {
	double edS = exp(- 1.0 * k3);
	double rg = 1.0;
	double ar = edS*rg;
	int n3 = Universe::tetrasAll.size();
	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 1.0 : -1.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}

	Tetra::Label tv = Universe::tetras31.pick();
	auto t = tv->tnbr[3];
	std::uniform_int_distribution<> neighborGen(1, 3);
	Tetra::Label tn = t->tnbr[neighborGen(rng)];

	if (!tn->is22()) return false;

	return Universe::move23d(t, tn);
}

bool Simulation::moveShiftI() {
	double edS = exp(1 * k3);
	double rg = 1.0;
	double ar = edS*rg;
	int n3 = Universe::tetrasAll.size();

	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? -1.0 : 1.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}

	Tetra::Label t = Universe::tetras31.pick();

	std::uniform_int_distribution<> neighborGen(0, 2);
	int neighbor = neighborGen(rng);
	Tetra::Label t22l = t->tnbr[neighbor], t22r = t->tnbr[(neighbor + 2) % 3];

	if (!t22l->is22()) return false;
	if (!t22r->is22()) return false;
	if (!t22l->neighborsTetra(t22r)) return false;

	int sv = 0;
	for (int i = 0; i < 4; i++) {
		if (t22r->hasVertex(t22l->vs[i])) sv++;
	}
	if (sv != 3) return false;

	return Universe::move32u(t, t22l, t22r);
}

bool Simulation::moveShiftID() {
	double edS = exp(1 * k3);
	double rg = 1.0;
	double ar = edS*rg;
	int n3 = Universe::tetrasAll.size();

	if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? -1.0 : 1.0));

	if (ar < 1.0) { 
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		double r = uniform(rng);
		if (r > ar) return false;
	}

	Tetra::Label t = Universe::tetras31.pick()->tnbr[3];

	std::uniform_int_distribution<> neighborGen(0, 2);
	int neighbor = neighborGen(rng);
	Tetra::Label t22l = t->tnbr[1 + neighbor], t22r = t->tnbr[1 + (neighbor + 2) % 3];

	if (!t22l->is22()) return false;
	if (!t22r->is22()) return false;
	if (!t22l->neighborsTetra(t22r)) return false;

	int sv = 0;
	for (int i = 0; i < 4; i++) {
		if (t22r->hasVertex(t22l->vs[i])) sv++;
	}
	if (sv != 3) return false;

	return Universe::move32d(t, t22l, t22r);
}


void Simulation::prepare() {
	Universe::updateVertexData();
	Universe::updateHalfEdgeData();
	Universe::updateTriangleData();
	//Universe::check();
}

void Simulation::tune() { 

	double loc_epsilon = 0.00001;

	int border_far = targetVolume*0.5; 
	int border_close = targetVolume*0.05;
	int border_vclose = targetVolume*0.001; 

	double ratio = 100;

			
		if ((targetVolume - Tetra::size()) > border_far) {
			k3 -= loc_epsilon*ratio*100;
		} else if ((targetVolume - Tetra::size()) < -border_far) {
			k3 += loc_epsilon*ratio*100;
		} else if ((targetVolume - Tetra::size()) > border_close) {
			k3 -= loc_epsilon*100;
		} else if ((targetVolume - Tetra::size()) < -border_close) {
			k3 += loc_epsilon*100;
		} else if ((targetVolume - Tetra::size()) > border_vclose) {
			k3 -= loc_epsilon*10;
		} else if ((targetVolume - Tetra::size()) < -border_vclose) {
			k3 += loc_epsilon*10;
		}

/*
	bool done = false;

	for (int k = 0; k < 1 && !done; k++) {
		for (int i = 0; i < targetVolume; i++) {
			std::vector<int> tmps = sweep(100);
			for (int j = 0; j < 6; j++) success[j] += tmps[j];
			volumes.push_back(Tetra::size());
		}

		double avg = 0.0;
		for (auto v : volumes) avg += (double) v;
		avg /= volumes.size();

		double sd = 0.0;
		for (auto v : volumes) sd += ((double) v - avg)*((double) v - avg);
		sd /= volumes.size();

		if ((targetVolume - avg)*(targetVolume - avg) < 2*sd) {
			epsilon *= 0.7;
			if (epsilon < 0.02) {
				epsilon = 0.02;
				k3 -= 0.003 * (targetVolume - avg)/sqrt(sd);
			}
		} else if ((targetVolume - avg)*(targetVolume - avg) > 8*sd) {
			epsilon *= 1.2;
			if (epsilon > 5.0) epsilon = 5.0;
		} else if ((targetVolume - avg)*(targetVolume - avg) < 0.04*targetVolume*targetVolume) {
			k3 += 0.6*(avg - targetVolume)/abs((avg-targetVolume)) * epsilon;
		}
		volumes.clear();

		//if (k >= tuneSteps && abs(avg-targetVolume) < 0.1*targetVolume && epsilon < 0.021) done = true;

		//if ((targetVolume - avg)*(targetVolume - avg) < 0.01*targetVolume*targetVolume) {
		//  int totalFreq = moveFreqs[0] + moveFreqs[1] + moveFreqs[2];
		//  double rate[3];
		//  rate[0] = totalFreq/(double) moveFreqs[0] * (success[1] + success[2])/1000000;
		//  rate[1] = totalFreq/(double) moveFreqs[0] * (success[3])/1000000;
		//  rate[2] = totalFreq/(double) moveFreqs[0] * (success[4] + success[5])/1000000;

		//  double addRate = totalFreq/(double) moveFreqs[0] * (success[1])/1000000;
		//  double delRate = totalFreq/(double) moveFreqs[0] * (success[2])/1000000;

		//  printf("rates: %f - %f - %f\n", rate[0], rate[1], rate[2]);
		//  printf("add: %f, del: %f\n", addRate, delRate);
		//  }

		//printf("step %d - epsilon: %f, k3: %f, avg: %d, sd: %d\n", k, epsilon, k3, (int) avg, (int) sd);
		//printf("fail: %d, add: %d, del: %d, flip: %d, shift: %d, ishift: %d\n", success[0], success[1], success[2], success[3], success[4], success[5]);

		//Universe::check();

		//success = std::vector<int>(6,0);
	//}//
*/


}
