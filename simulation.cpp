// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "simulation.hpp"
#include "observable.hpp"
#include <vector>

double Simulation::k0 = 0;
double Simulation::k3 = 0;

std::default_random_engine Simulation::rng(0);  // TODO(JorenB): seed properly
int Simulation::targetVolume = 0;
int Simulation::target2Volume = 0;
double Simulation::epsilon = 0.00004;
bool Simulation::measuring = false;

std::vector<Observable*> Simulation::observables3d;
std::vector<Observable*> Simulation::observables2d;

std::array<int, 3> Simulation::moveFreqs = {0, 0, 0};

void Simulation::start(double k0_, double k3_,int sweeps,  int thermalSweeps,int ksteps, int targetVolume_, int target2Volume_, int seed, std::string OutFile, int v1, int v2, int v3) {

	Simulation::moveFreqs = {v1, v2, v3};

	targetVolume = targetVolume_;
	target2Volume = target2Volume_;		
	k3 = k3_;
	k0 = k0_;
	
	for (auto o : observables3d) 
		o->clear();
	
	for (auto o : observables2d) 
		o->clear();
	

	rng.seed(seed);

	measuring = true;
//////////////////////////////////////////////////////////////////////
// ********************** START THERMAL SWEEPS ******************** //
//////////////////////////////////////////////////////////////////////

	printf("k0: %g, k3: %g, epsilon: %g \t thermal: %d \t sweeps: %d Target: %d\t Target2d: %d\t \n", k0, k3, epsilon, thermalSweeps, sweeps, targetVolume, target2Volume);

	for (int i = 1; i <= thermalSweeps; i++) {  // thermalization phase
		int total2v = 0;
		for (auto ss : Universe::sliceSizes) 
			total2v += ss;

		//int avg2v = total2v / Universe::nSlices;
		double n31 = Universe::tetras31.size();
		int n3 = Universe::tetrasAll.size();

		printf("Thermal: i: %d\t  Tetra::size: %d tetras31:  %g k3: %g \n",i, n3, n31, k3);

		performSweep(ksteps * 1000); //ksteps is for "how many thousand steps to perform" in a given sweep

		tune();  // tune k3 one step at each sweep

		if ( i % 100 == 0) 
			Universe::exportGeometry(OutFile);

		prepare();
		for (auto o : observables3d) 
			o->measure();
	}


////////////////////////////////////////////////////////////////////
// ********************** END THERMAL SWEEPS ******************** //
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// ********************** START MEASURE SWEEPS ****************** //
////////////////////////////////////////////////////////////////////
	printf("k0: %g, k3: %g, epsilon: %g", k0, k3, epsilon);

	for (int i = 1; i <= sweeps; i++) {  // number of measurement sweeps

		int total2v = 0;
		for (auto ss : Universe::sliceSizes) total2v += ss;
			int avg2v = total2v / Universe::nSlices;
		
		printf("SWEEPS: i: %d\t Target: %d\t Target2d: %d\t CURRENT: %d avgslice: %d\n",i, targetVolume, target2Volume, Tetra::size(), avg2v);


		performSweep(ksteps * 1000); //ksteps is for "how many thousand steps to perform" in a given sweep

		if (sweeps % (i/10 == 0)) 
			Universe::exportGeometry(OutFile);
		
		prepare();
		if (observables3d.size() > 0) 
			for (auto o : observables3d) 
				o->measure();
		

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

//////////////////////////////////////////////////////////////////
// ********************** END MEASURE SWEEPS ****************** //
//////////////////////////////////////////////////////////////////

}


int Simulation::attemptMove() {
	std::array<int, 3> cumFreqs = {moveFreqs[0], moveFreqs[1]+moveFreqs[0], moveFreqs[2]+moveFreqs[1]+moveFreqs[0]};
	int freqTotal = 0;
	
	freqTotal = moveFreqs[0] + moveFreqs[1] + moveFreqs[2];

	std::uniform_int_distribution<> moveGen(0, freqTotal-1);
	std::uniform_int_distribution<> binGen(0, 1);

	int move = moveGen(rng);

	if (move < cumFreqs[0]) {
		if (binGen(rng) == 0) {
			//printf("a\n");
			if (moveAdd()) return 1;
			else return -1;
		} else {
			//printf("d\n");
			if (moveDelete()) return 2;
			else return -2;
		}
	} else if (cumFreqs[0] <= move && move < cumFreqs[1]) {
		//printf("f\n");
		if (moveFlip())  return 3;
		else return -3;
		
	} else if (cumFreqs[1] <= move) {
		if (binGen(rng) == 0) {
			//printf("shiftf\n");
			if (binGen(rng) == 0) { 
				//printf("u\n"); 
				if (moveShift()) return 4;
				else return -4; 
			}
			else { 
			  //printf("d\n"); 
				if (moveShiftD()) return 4; 
				else return -4;
			}
		} else {
			//printf("shiftb\n");
			if (binGen(rng) == 0) { 
				//printf("u\n"); 
				if (moveShiftI()) return 5; 
				else return -5;
			}
			else { 
				//printf("d\n"); 
				if (moveShiftID()) return 5; 
				else return -5;
			}
		}
	}



	//Universe::check();

	return 0;
}

std::vector<int> Simulation::performSweep(int n) {
	std::vector<int> moves(6, 0);
	std::vector<int> failed_moves(6, 0);
	for (int i = 0; i < n; i++) {
		//printf("mov%d \n", i);
		int move_num = attemptMove(); 
		int move = abs(move_num);
		moves[move]++;
		if (move_num < 0)
			failed_moves[move]++;
		

		//Universe::check();
	}
		
	int m1 = moves[1] + moves[2];
	int m2 = moves[3];
	int m3 = moves[4] + moves[5];

	int f1 = failed_moves[1] + failed_moves[2];
	int f2 = failed_moves[3];
	int f3 = failed_moves[4] + failed_moves[5];


//	printf("%d\t%d\t%d\t\n", m1, m2, m3);
//	printf("%d\t%d\t%d\t\n", f1, f2, f3);
	
	if (m1 == f1)
		m1++,
		m1++,
		f1++;
		
	if (m2 == f2)
		m2++,
		m2++,
		f2++;
		
	if (m3 == f3)
		m3++,
		m3++,
		f3++;
		
	double r1 = double(m1)/double(f1);
	double r2 = double(m2)/double(f2);
	double r3 = double(m3)/double(f3);
	
	printf("%g\t%g\t%g\t\n", r1,r2,r3 );

	return moves;
}

bool Simulation::moveAdd() {
	double n31 = Universe::tetras31.size();
	int n3 = Universe::tetrasAll.size();
	
	int vol_switch = Universe::volfix_switch;
	
	double edS = exp(1 * k0 - 4 * k3);
	//double rg = n0/(n0+1.0)*n31/(n31+4.0);
	double rg = n31 / (n31 + 2.0);
	double ar = edS*rg;
	
	if (vol_switch == 0) {
		//if (targetVolume > 0) ar *= exp(epsilon * (n31 < targetVolume ? 4.0 : -4.0));
		if (targetVolume > 0) ar *= exp(4 * epsilon * (targetVolume - n31 - 1));
	}
	else {
		//if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 4.0 : -4.0));
		if (targetVolume > 0) ar *= exp(8 * epsilon * (targetVolume - n3 - 2));
	}

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
	int vol_switch = Universe::volfix_switch;
	
	
	double edS = exp(-1 * k0 + 4 * k3);
	//double rg = n0/(n0-1.0)*n31/(n31-4.0);
	double rg = n31/(n31-2.0);
	double ar = edS*rg;
	
	if (vol_switch == 0) {
		//if (targetVolume > 0) ar *= exp(epsilon * (n31 < targetVolume ? -4.0 : 4.0));
		if (targetVolume > 0) ar *= exp(-4 * epsilon * (targetVolume - n31 - 1));
	}
	else {
		//if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? -4.0 : 4.0));
		if (targetVolume > 0) ar *= exp(-8 * epsilon * (targetVolume - n3 - 2));
	}
	
	

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
	double n31 = Universe::tetras31.size();
	int vol_switch = Universe::volfix_switch;
	
	
	if (vol_switch == 1) {
		//if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 1.0 : -1.0));
		if (targetVolume > 0) ar *= exp(epsilon * (2 * targetVolume - 2 * n3 - 1));
	}
	

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
	double n31 = Universe::tetras31.size();
	int vol_switch = Universe::volfix_switch;
	
	
	if (vol_switch == 1) {
		//if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? 1.0 : -1.0));
		if (targetVolume > 0) ar *= exp(epsilon * (2 * targetVolume - 2 * n3 - 1));
	}
	

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
	double n31 = Universe::tetras31.size();
	
	int vol_switch = Universe::volfix_switch;
	
	
	if (vol_switch == 1) {
		//if (targetVolume > 0) ar *= exp(epsilon * (n3 < targetVolume ? -1.0 : 1.0));
		if (targetVolume > 0) ar *= exp(-epsilon * (2 * targetVolume - 2 * n3 - 1));
	}

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
	double n31 = Universe::tetras31.size();
	int vol_switch = Universe::volfix_switch;
	
	
	if (vol_switch == 1) {
		if (targetVolume > 0) ar *= exp(-epsilon * (2 * targetVolume - 2 * n3 - 1));
	}

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
	Universe::updateGeometry();
	//Universe::check();
}

void Simulation::tune() {
	double delta_k3 = 0.000001;
	double ratio = 100;

	int border_far = targetVolume*0.5; 
	int border_close = targetVolume*0.05;
	int border_vclose = targetVolume*0.002; 
	int border_vvclose = targetVolume*0.0001; 

	int vol_switch = Universe::volfix_switch;

	int fixvolume = 0;

	if (vol_switch == 0) {
		fixvolume = Universe::tetras31.size();
	}
	else {
		fixvolume = Universe::tetrasAll.size();
	}	

	if ((targetVolume - fixvolume) > border_far) 
		k3 -= delta_k3*ratio*1000;
	else if ((targetVolume - fixvolume) < -border_far) 
		k3 += delta_k3*ratio*1000;
	else if ((targetVolume - fixvolume) > border_close) 
		k3 -= delta_k3*1000;
	else if ((targetVolume - fixvolume) < -border_close)
		k3 += delta_k3*1000;
	else if ((targetVolume - fixvolume) > border_vclose)
		k3 -= delta_k3*100;
	else if ((targetVolume - fixvolume) < -border_vclose)
		k3 += delta_k3*100;
	else if ((targetVolume - fixvolume) > border_vvclose)
		k3 -= delta_k3*20;
	else if ((targetVolume - fixvolume) < -border_vvclose)
		k3 += delta_k3*20;
}
