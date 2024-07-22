#include "globals.hpp"
#include <string>

int A1 = 0,A2 = 0,B1 = 0,B2 = 0,C = 0,D = 0;
int strictness;
int genus;
int abcd;
double epsilon;

double k0, k3, kB, kD;
int sweeps, thermalSweeps, ksteps, targetVolume, target2Volume, seed, thermal,targetvertex;
int volfix_switch;

std::string outFile;
std::string fID ;
std::string inFile;

int v1, v2, v3;

int n0, n3, n31;


char filen22[256];


#include <random>

//std::uniform_int_distribution<> neighborGen2(0, 2);
