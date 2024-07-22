// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include "simulation.hpp"
#include "observable.hpp"
#include "globals.hpp"
#include <numeric>


std::default_random_engine Simulation::rng(0);  // TODO(JorenB): seed properly
bool Simulation::measuring = false;

std::vector<Observable*> Simulation::observables3d;
std::vector<Observable*> Simulation::observables2d;

std::array<int, 3> Simulation::moveFreqs = {0, 0, 0};

void Simulation::start() {
	Simulation::moveFreqs = {v1, v2, v3};

	for (auto o : observables3d) o->clear();
	for (auto o : observables2d) o->clear();

	rng.seed(seed);

	measuring = true;
//////////////////////////////////////////////////////////////////////
// ********************** START THERMAL SWEEPS ******************** //
//////////////////////////////////////////////////////////////////////

//	printf("k0: %g, k3: %g, epsilon: %g \t thermal: %d \t sweeps: %d Target: %d\t Target2d: %d\t \n", k0, k3, epsilon, thermalSweeps, sweeps, targetVolume, targetVolume);
	
	//printf("Initialized ABCD: %d\t%d\t%d\t%d\t%d\t%d\t\n", A1,A2,B1,B2,C,D);
//	printf("Initialized n0 n31 n3: %d\t%d\t%d\n", n0, n31, n3);


	for (int i = 0; i < thermalSweeps; i++) {  // thermalization phase
		int total2v = 0;
		
		printf("Thermal i: %d, n0: %d n3: %d n31: %d\t", i, n0, n3, n31);
		
	
		if(abcd) printf("k0: %g\n",k0);
		else printf("k3: %g\n",k3);

		printf("Rolld ABCD: %d\t%d\t%d\t%d\t%d\t%d\n",A1, A2, B1, B2, C, D);
		
		for (auto ss : Universe::sliceSizes) total2v += ss;

		performSweep(ksteps * 1000);
	
		
		if(abcd) tunek0();
		else tune();

		if (i % (thermalSweeps / 10) == 0) Universe::exportGeometry(outFile);

		
		if (observables3d.size() > 0) {
			prepare();
			for (auto o : observables3d) o->measure();
			
			FILE* file = fopen(filen22, "a");
	
			fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %g %g\n",
			n0, n3,n31, n3-2*n31,Universe::getMaxord(),Universe::getMaxordSlice(), A1, A2,B1, B2, C, D, k0, k3);
			
			fclose(file);
		}
	
		
		
	}


////////////////////////////////////////////////////////////////////
// ********************** END THERMAL SWEEPS ******************** //
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// ********************** START MEASURE SWEEPS ****************** //
////////////////////////////////////////////////////////////////////
	//printf("k0: %g, k3: %g, epsilon: %g\n", k0, k3, epsilon);

	for (int i = 0; i < sweeps; i++) {  // number of measurement sweeps
		int total2v = 0;
		
		printf("Sweeps i: %d, n0: %d n3: %d n31: %d\t", i, n0, n3, n31);
		
	
		if(abcd) printf("k0: %g\n",k0);
		else printf("k3: %g\n",k3);

		printf("Rolld ABCD: %d\t%d\t%d\t%d\t%d\t%d\n",A1, A2, B1, B2, C, D);
		
		for (auto ss : Universe::sliceSizes) total2v += ss;

		performSweep(ksteps * 1000);


		if (i % (sweeps / 10) == 0) Universe::exportGeometry(outFile);

		
		if (observables3d.size() > 0) {
			prepare();
			for (auto o : observables3d) o->measure();
			
			FILE* file = fopen(filen22, "a");
	
			fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %g %g\n",
			n0, n3,n31, n3-2*n31,Universe::getMaxord(),Universe::getMaxordSlice(), A1, A2,B1, B2, C, D, k0, k3);
			
			fclose(file);
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
		if (binGen(rng)) {
			if (moveAdd()) return 1;
			else return -1;
		} 
		else {
			if (moveDelete()) return 2;
			else return -2;
		}
	}
	else if (move < cumFreqs[1]) {
		if (moveFlip())  return 3;
		else return -3;
	}
	else {
		if (binGen(rng)) {
			if (moveShift()) return 4;
			else return -4;
		}
		else {
			if (moveShiftI()) return 5;
			else return -5;
		}
	}
	

	return 0;
}

std::vector<int> Simulation::performSweep(int n) {
	std::vector<int> moves(6, 0);
	std::vector<int> failed_moves(6, 0);
	for (int i = 0; i < n; i++) {
	
		int move_num = attemptMove();
		int move = abs(move_num);
		moves[move]++;
		if (move_num < 0) failed_moves[move]++;
	}

	int m1 = moves[1] + moves[2];
	int m2 = moves[3];
	int m3 = moves[4] + moves[5];

	int f1 = failed_moves[1] + failed_moves[2];
	int f2 = failed_moves[3];
	int f3 = failed_moves[4] + failed_moves[5];

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

    double r1 = static_cast<double>(m1)/static_cast<double>(f1);
    double r2 = static_cast<double>(m2)/static_cast<double>(f2);
    double r3 = static_cast<double>(m3)/static_cast<double>(f3);

	return moves;
}

bool Simulation::moveAdd() {
	double ar = exp( k0 )*n31/(n31+2);
	
	if(abcd) ar *= exp(epsilon * ( 2*(targetvertex - n0) - 1));	
	else {
		ar *= exp(- 4 * k3);

		if (volfix_switch == 0 && targetVolume > 0) ar *= exp(4 * epsilon * (targetVolume - n31 - 1));	 
		else if (targetVolume > 0) ar *= exp(8 * epsilon * (targetVolume - n3 - 2));
	}
	

	
	Tetra::Label t = Universe::tetras31.pick(); //pick a place to perform the move
	
	if(ar < 1) {
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		if (ar < uniform(rng)) return false;
	}
	

	if( Universe::move26(t)) {
		A1 += 3;
		A2 += 3;
		C += 2 ;
		
		n0++;
		n31+=2;
		n3+=4;		
		
		return true;
	}
	
	return false;
}

bool Simulation::moveDelete() {
	double ar = exp(-k0) * n0 / (n31-2);

	Vertex::Label v = Universe::verticesAll.pick();
	
	if (v->cnum != 6 || v->scnum != 3) return false;
	
	if(abcd) ar *= exp(epsilon * (2*(n0 - targetvertex) - 1));
	else {
		ar *= exp(4 * k3);

		if (volfix_switch == 0 && targetVolume > 0) ar *= exp(-4 * epsilon * (targetVolume - n31 - 1));
		else if (targetVolume > 0) ar *= exp(-8 * epsilon * (targetVolume - n3 - 2));
	}
	
	
	

	if(ar < 1) {
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		if (ar < uniform(rng)) return false;
	}
	
	

	if(Universe::move62(v)) {
		A1 -= 3;
		A2 -= 3;
		C -= 2 ;
		
		n0--;
		n31 -= 2;
		n3 -= 4;
		
		return true;
	}
	return false;
}

bool Simulation::moveFlip() {
	Tetra::Label t012 = Universe::tetras31.pick();
	std::uniform_int_distribution<> neighborGen(0, 2);
	
	Tetra::Label t230 = t012->tnbr[neighborGen(rng)];

	if (!t230->is31()) return false;

	if (!t012->tnbr[3]->neighborsTetra(t230->tnbr[3])) return false;
	
	return Universe::move44(t012, t230);
}

std::pair<int,std::pair<int, int>> Simulation::getDeltaABCD(int a[3]) {
			
		if( (a[0] == 0 && a[1] == 0 && a[2] == 0) || (a[0] == 2 && a[1] == 2 && a[2] == 2) ) { return std::make_pair(-1,std::make_pair(2,1));  } //dA = -1; dB = 2; dD = 1;
		else if( (a[0] == 0 && a[1] == 1 && a[2] == 0) || (a[0] == 1 && a[1] == 0 && a[2] == 0) ||
				 (a[0] == 2 && a[1] == 1 && a[2] == 2) || (a[0] == 1 && a[1] == 2 && a[2] == 2) ) { return std::make_pair(0,std::make_pair(0,2)); } // dD = 2;
		else if( (a[0] == 1 && a[1] == 1 && a[2] == 0) || (a[0] == 1 && a[1] == 1 && a[2] == 2) ) {  return std::make_pair(1,std::make_pair(-2,3)); } //dA = 1; dB = -2; dD = 3;
		else if( (a[0] == 0 && a[1] == 0 && a[2] == 1) || (a[0] == 2 && a[1] == 2 && a[2] == 1) ) {  return std::make_pair(-2,std::make_pair(4,0)); } //dA = -2; dB = 4;			 
		else if (a[0] == 1 && a[1] == 1 && a[2] == 1) { return std::make_pair(0,std::make_pair(0,2)); }// dD = 2;
		else if ( (a[0] == 0 && a[1] == 1 && a[2] == 1) || (a[0] == 1 && a[1] == 0 && a[2] == 1) ||
				 (a[0] == 2 && a[1] == 1 && a[2] == 1) || (a[0] == 1 && a[1] == 2 && a[2] == 1) ) { return std::make_pair(-1,std::make_pair(2,1)); } // dA = -1; dB = 2 ; dD = 1; 
		
		
		assert(0 == 1);

		
		return std::make_pair(-1,std::make_pair(-1,-1));
	}

bool Simulation::moveShift() {
	double rg; //= 1.0;
	double ar = 1.0; // = edS*rg;

	Tetra::Label t, tn, tv, tno;
	
	std::uniform_int_distribution<> binGen(0, 1);
	int bingen_ud = binGen(rng);
	
	std::uniform_int_distribution<> neighborGen(0, 2);
	int neighbor = neighborGen(rng);
	
	int dA = 0, dB = 0, dD = 0;
	int a[3] = {0,0,0};
	
	
	t = Universe::tetras31.pick();
	 
	if(bingen_ud) t = t->tnbr[3];
	tn = t->tnbr[bingen_ud + neighbor]; 
	
	if (!tn->is22()) return false;
	
	
	
	if(neighbor == 0) { 
			a[0] = t->getNeighborType(1+bingen_ud);
			a[1] = t->getNeighborType(2+bingen_ud);
		}
		else if(neighbor == 1) {
			a[0] = t->getNeighborType(0+bingen_ud);
			a[1] = t->getNeighborType(2+bingen_ud);
		}
		else  {
			a[0] = t->getNeighborType(0+bingen_ud);
			a[1] = t->getNeighborType(1+bingen_ud);
		}
			
	a[2] = tn->getTetraOpposite(t->vs[3*(1-bingen_ud)])->type; // 0: (3,1) ,1:(2,2),   2:(1,3)

	
	std::pair<int,std::pair<int, int>> dBD = getDeltaABCD(a); //fill dBD
	
	if(abcd) ar = exp( -kB*dBD.second.first - kD*dBD.second.second);
	else {
		ar =  exp(-k3);
		if (volfix_switch == 1 && targetVolume > 0) ar *= exp(epsilon * (2 * (targetVolume - n3) - 1)); 
	}
	
	
	if(ar < 1) {
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		if (ar < uniform(rng)) return false;
	}

	bool moveDone;
	
	if(bingen_ud) {
		moveDone = Universe::move23d(t, tn);
		
		if(moveDone) {
			A2 += dBD.first;
			B2 += dBD.second.first;

			D += dBD.second.second;
			n3++;
		} 
	}
	else {		
		moveDone = Universe::move23u(t, tn);
		
		if(moveDone) {
			A1 += dBD.first;
			B1 += dBD.second.first;

			D += dBD.second.second;
			n3++;
		}
	}
	

	return moveDone ;
}

bool Simulation::moveShiftI() { 
	double ar = 1.0;
	
	std::uniform_int_distribution<> binGen(0, 1);
	int bingen_ud = binGen(rng);
	
	std::uniform_int_distribution<> neighborGen(0, 2);
	int neighbor = neighborGen(rng);
	
	Tetra::Label t,t22l,t22r, tnol, tnor, tnot;
	
	t = Universe::tetras31.pick();

	if(bingen_ud) t=t->tnbr[3];
	
	t22l = t->tnbr[bingen_ud + neighbor], t22r = t->tnbr[(bingen_ud + neighbor + 2) % 3];
	
	if (!t22l->is22()) return false;
	if (!t22r->is22()) return false;
	if (!t22l->neighborsTetra(t22r)) return false;
	
	int sv = 0;
	for (int i = 0; i < 4; i++) if (t22r->hasVertex(t22l->vs[i])) sv++;
	if (sv != 3) return false;
	
	if(bingen_ud) {
		tnol = t22l->getTetraOpposite(t->vs[0]);
		tnor = t22r->getTetraOpposite(t->vs[0]);
	}
	else { 
		tnol = t22l->getTetraOpposite(t->vs[3]);
		tnor = t22r->getTetraOpposite(t->vs[3]);
	}
	tnot = t->tnbr[bingen_ud + (neighbor + 1) % 3];
	

	int a[3] = { tnol->type, tnor->type, tnot->type};
	
	std::pair<int,std::pair<int, int>> dBD = getDeltaABCD(a); //fill dBD;

	if(abcd) ar = exp(kB*dBD.second.first + kD*dBD.second.second);
	else {
		ar = exp(k3);
		if (volfix_switch == 1 && targetVolume > 0) ar *= exp(- epsilon * (2 * targetVolume - 2 * n3 - 1));
	}
	
		
	if(ar < 1) {
		std::uniform_real_distribution<> uniform(0.0, 1.0);
		if (ar < uniform(rng)) return false;
	}
	
	
	bool moveDone;
	
	if(bingen_ud) {
		moveDone = Universe::move32d(t, t22l, t22r);
		
		if(moveDone) {
			A2 -= dBD.first;
			B2 -= dBD.second.first;

			D -= dBD.second.second;
			n3--;
		}
		
	}
	else {
		moveDone = Universe::move32u(t, t22l, t22r);

		if(moveDone) {
			A1 -= dBD.first;
			B1 -= dBD.second.first;
			
			D -= dBD.second.second;
			n3--;
		}
	}
	
	return moveDone;
}

void Simulation::prepare() { Universe::updateGeometry(); }

void Simulation::tune() {
	double delta_k3 = 0.000001;
	double ratio = 100;

	int border_far = targetVolume*0.5;
	int border_close = targetVolume*0.05;
	int border_vclose = targetVolume*0.002;
	int border_vvclose = targetVolume*0.0001;
	
	int fixvolume = 0;

	if (volfix_switch == 0) fixvolume = n31;
	else fixvolume = n3;
	

	if ((targetVolume - fixvolume) > border_far) k3 -= delta_k3*ratio*1000;
	else if ((targetVolume - fixvolume) < -border_far) k3 += delta_k3*ratio*1000;
	else if ((targetVolume - fixvolume) > border_close) k3 -= delta_k3*1000;
	else if ((targetVolume - fixvolume) < -border_close) k3 += delta_k3*1000;
	else if ((targetVolume - fixvolume) > border_vclose) k3 -= delta_k3*100;
	else if ((targetVolume - fixvolume) < -border_vclose) k3 += delta_k3*100;
	else if ((targetVolume - fixvolume) > border_vvclose) k3 -= delta_k3*20;
	else if ((targetVolume - fixvolume) < -border_vvclose) k3 += delta_k3*20;
}


void Simulation::tunek0() {
	double delta_k0 = -0.00001;
	double ratio = 1000;

	int border_far = targetvertex*0.5;
	int border_close = targetvertex*0.05;
	int border_vclose = targetvertex*0.002;
	int border_vvclose = targetvertex*0.0001;
	

	if ((targetvertex - n0) > border_far) k0 -= delta_k0*ratio*1000;
	else if ((targetvertex - n0) < -border_far) k0 += delta_k0*ratio*1000;
	else if ((targetvertex - n0) > border_close) k0 -= delta_k0*1000;
	else if ((targetvertex - n0) < -border_close) k0 += delta_k0*1000;
	else if ((targetvertex - n0) > border_vclose) k0 -= delta_k0*100;
	else if ((targetvertex - n0) < -border_vclose) k0 += delta_k0*100;
	else if ((targetvertex - n0) > border_vvclose) k0 -= delta_k0*20;
	else if ((targetvertex - n0) < -border_vvclose) k0 += delta_k0*20;
}
