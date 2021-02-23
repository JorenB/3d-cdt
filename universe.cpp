// Copyright 2021 Joren Brunekreef, Daniel Nemeth and Andrzej Görlich
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include "universe.hpp"

int Universe::nSlices = 0;
std::vector<int> Universe::slabSizes;
std::vector<int> Universe::sliceSizes;
std::string Universe::fID;
std::default_random_engine Universe::rng(0);
Bag<Tetra, Tetra::pool_size> Universe::tetrasAll(rng);
Bag<Tetra, Tetra::pool_size> Universe::tetras31(rng);
Bag<Vertex, Vertex::pool_size> Universe::verticesAll(rng);
Bag<Vertex, Vertex::pool_size> Universe::verticesSix(rng);

std::vector<Vertex::Label> Universe::vertices;
std::vector<Tetra::Label> Universe::tetras;

std::vector<HalfEdge::Label> Universe::halfEdges;
std::vector<Triangle::Label> Universe::triangles;

std::vector<std::vector<Vertex::Label>> Universe::vertexNeighbors;
std::vector<std::array<Triangle::Label, 3>> Universe::triangleNeighbors;


bool Universe::initialize(std::string geometryFilename, std::string fID_) {
	fID = fID_;
	std::ifstream infile(geometryFilename.c_str());

	assert(!infile.fail());

	bool ordered;  // first line is a switch indicating whether tetrahedron data is ordered by convention
	infile >> ordered;

	int n0;
	infile >> n0;
	printf("n0: %d\n", n0);
	int line;

	int maxTime = 0;
	std::vector<Vertex::Label> vs(n0);
	for (int i = 0; i < n0; i++) {
		infile >> line;
		auto v = Vertex::create();
		verticesAll.add(v);
		v->time = line;
		vs.at(i) = v;
		if (v->time > maxTime) maxTime = v->time;
	}
	infile >> line;
	if (line != n0) return false;


	nSlices = maxTime+1;
	slabSizes.resize(maxTime+1);
	sliceSizes.resize(maxTime+1);
	std::fill(slabSizes.begin(), slabSizes.end(), 0);
	std::fill(sliceSizes.begin(), sliceSizes.end(), 0);

	int n3;
	infile >> n3;
	printf("n3: %d\n", n3);
	//std::vector<HalfEdge::Label> hes;
	for (int i = 0; i < n3; i++) {
		auto t = Tetra::create();
		int tvs[4];
		for (int j = 0; j < 4; j++) {
			infile >> tvs[j];
		}

		int tts[4];
		for (int j = 0; j < 4; j++) {
			infile >> tts[j];
		}

		t->setVertices(tvs[0], tvs[1], tvs[2], tvs[3]);
		if (t->is31()) {

			for (int j = 0; j < 3; j++) {
				Pool<Vertex>::Label v = tvs[j];
				v->tetra = t;
			}
		}
		t->setTetras(tts[0], tts[1], tts[2], tts[3]);
		
		tetrasAll.add(t);
		if (t->is31()) tetras31.add(t);
		slabSizes.at(t->vs[1]->time) += 1;
		if (t->is31()) sliceSizes.at(t->vs[0]->time) += 1;
		//printf("t: %d, %d %d %d %d\n", t, t->v1, t->v2, t->v3, t->v4);
	}
	infile >> line;
	if (line != n3) return false;
	printf("read %s\n", geometryFilename.c_str());

	if (!ordered) {
		for (auto t : tetrasAll) {  // reorder to convention
			auto tnbr = t->tnbr;
			Tetra::Label t012 = -1, t013 = -1, t023 = -1, t123 = -1;
			for (auto tn : tnbr) {
				if (!tn->hasVertex(t->vs[3])) { t012 = tn; continue; }
				if (!tn->hasVertex(t->vs[2])) { t013 = tn; continue; }
				if (!tn->hasVertex(t->vs[1])) { t023 = tn; continue; }
				if (!tn->hasVertex(t->vs[0])) { t123 = tn; continue; }
			}

			assert(t012 >= 0);
			assert(t013 >= 0);
			assert(t023 >= 0);
			assert(t123 >= 0);

			t->setTetras(t123, t023, t013, t012);
		}
	}

	for (auto v : vs) {
		int cnum = 0, scnum = 0;
		for (auto t : tetrasAll) {
			if (t->hasVertex(v)) cnum++;
			if (!t->is31()) continue;
			if (t->vs[0] == v || t->vs[1] == v || t->vs[2] == v) scnum++;
		}
		v->scnum = scnum;
		v->cnum = cnum;
	}

	return true;
}

bool Universe::exportGeometry() {
	return exportGeometry("geometries/conf-" + fID + ".dat");
}

bool Universe::exportGeometry(std::string geometryFilename) {
	updateGeometry();

	std::unordered_map<int, int> vertexMap;
	std::vector<Vertex::Label> intVMap;
	intVMap.resize(vertices.size());

	int i = 0;
	for (auto v : vertices) {
		vertexMap.insert({v, i});
		intVMap.at(i) = v;
		i++;
	}

	// TODO: rewrite to incorporate 'tetras' and add 'updateTetraData()'
	std::unordered_map<int, int> tetraMap;
	std::vector<Tetra::Label> intTMap;
	intTMap.resize(tetrasAll.size());

	i = 0;
	for (auto t : tetrasAll) {
		tetraMap.insert({t, i});
		intTMap.at(i) = t;
		i++;
	}

	std::string out = "1\n";  // indicating well-orderedness

	out += std::to_string(vertices.size());
	out += "\n";

	for (int j = 0; j < intVMap.size(); j++) {
		out += std::to_string(intVMap.at(j)->time);
		out += "\n";
	}

	out += std::to_string(vertices.size());
	out += "\n";

	out += std::to_string(tetrasAll.size());
	out += "\n";

	for (int j = 0; j < intTMap.size(); j++) {
		for (auto v : intTMap.at(j)->vs) {
			out += std::to_string(vertexMap.at(v));
			out += "\n";
		}
		for (auto t : intTMap.at(j)->tnbr) {
			out += std::to_string(tetraMap.at(t));
			out += "\n";
		}
	}
	
	out += std::to_string(tetrasAll.size());

    std::ofstream file;
    file.open(geometryFilename, std::ios::out | std::ios::trunc);
	assert(file.is_open());

	file << out << "\n";
	file.close();

    std::cout << geometryFilename << "\n";

	return true;
}

bool Universe::move26(Tetra::Label t) {  // takes 31 simplex
	assert(t->is31());
	int time = t->vs[0]->time;
	auto tv = t->tnbr[3];  // vertical 13 neighbor
	assert(tv->is13());

	auto vn = Vertex::create();
	verticesAll.add(vn);
	vn->time = time;
	vn->scnum = 3;
	vn->cnum = 6;

	auto v0 = t->vs[0];
	auto v1 = t->vs[1];
	auto v2 = t->vs[2];
	auto vt = t->vs[3];
	auto vb = tv->vs[0];
	
	auto tn01 = Tetra::create();
	auto tn12 = Tetra::create();
	auto tn20 = Tetra::create();

	auto tvn01 = Tetra::create();
	auto tvn12 = Tetra::create();
	auto tvn20 = Tetra::create();

	tetrasAll.add(tn01);
	tetrasAll.add(tn12);
	tetrasAll.add(tn20);

	tetras31.add(tn01);
	tetras31.add(tn12);
	tetras31.add(tn20);

	tetrasAll.add(tvn01);
	tetrasAll.add(tvn12);
	tetrasAll.add(tvn20);

	auto to0 = t->getTetraOpposite(v0);
	auto to1 = t->getTetraOpposite(v1);
	auto to2 = t->getTetraOpposite(v2);

	auto tvo0 = tv->getTetraOpposite(v0);
	auto tvo1 = tv->getTetraOpposite(v1);
	auto tvo2 = tv->getTetraOpposite(v2);

	tn01->setVertices(v0, v1, vn, vt);
	tn12->setVertices(v1, v2, vn, vt);
	tn20->setVertices(v2, v0, vn, vt);

	tvn01->setVertices(vb, v0, v1, vn);
	tvn12->setVertices(vb, v1, v2, vn);
	tvn20->setVertices(vb, v2, v0, vn);

	tn01->setTetras(tn12, tn20, to2, tvn01);
	tn12->setTetras(tn20, tn01, to0, tvn12);
	tn20->setTetras(tn01, tn12, to1, tvn20);

	tvn01->setTetras(tn01, tvn12, tvn20, tvo2);
	tvn12->setTetras(tn12, tvn20, tvn01, tvo0);
	tvn20->setTetras(tn20, tvn01, tvn12, tvo1);

	to0->exchangeTetraOpposite(t->getVertexOpposite(v0), tn12);
	to1->exchangeTetraOpposite(t->getVertexOpposite(v1), tn20);
	to2->exchangeTetraOpposite(t->getVertexOpposite(v2), tn01);

	tvo0->exchangeTetraOpposite(tv->getVertexOpposite(v0), tvn12);
	tvo1->exchangeTetraOpposite(tv->getVertexOpposite(v1), tvn20);
	tvo2->exchangeTetraOpposite(tv->getVertexOpposite(v2), tvn01);

	slabSizes.at(time) += 2;
	slabSizes.at((time - 1 + nSlices) % nSlices) += 2;
	sliceSizes.at(time) += 2;
	
	tetrasAll.remove(t);
	tetras31.remove(t);
	tetrasAll.remove(tv);

	Tetra::destroy(t);
	Tetra::destroy(tv);

	vn->tetra = tn01;
	//verticesSix.add(vn);

	v0->tetra = tn01;
	v1->tetra = tn12;
	v2->tetra = tn20;

	v0->scnum++;
	v1->scnum++;
	v2->scnum++;

	v0->cnum += 2;
	v1->cnum += 2;
	v2->cnum += 2;
	vt->cnum += 2;
	vb->cnum += 2;

	//if (verticesSix.contains(v0)) { verticesSix.remove(v0); }
	//if (verticesSix.contains(v1)) { verticesSix.remove(v1); }
	//if (verticesSix.contains(v2)) { verticesSix.remove(v2); }

	return true;
}

bool Universe::move62(Vertex::Label v) {
	assert(v->cnum == 6);
	int time = v->time;
	auto t01 = v->tetra;
	auto tv01 = t01->tnbr[3];

	//printf("v: %d\n", v);
	//t01->log();

	int vpos = -1;
	for (int i = 0; i < 3; i++) {
		if (t01->vs[i] == v) { vpos = i; break; }
	}
	assert(vpos >= 0);

	auto v0 = t01->vs[(vpos + 1) % 3];
	auto v1 = t01->vs[(vpos + 2) % 3];
	auto v2 = t01->getVertexOpposite(v0);

	auto t12 = t01->getTetraOpposite(v0);
	auto t20 = t01->getTetraOpposite(v1);

	auto tv12 = tv01->getTetraOpposite(v0);
	auto tv20 = tv01->getTetraOpposite(v1);

	assert(t01->is31());
	assert(t12->is31());
	assert(t20->is31());


	assert(tv01->is13());
	assert(tv12->is13());
	assert(tv20->is13());

	auto to01 = t01->getTetraOpposite(v);
	auto to12 = t12->getTetraOpposite(v);
	auto to20 = t20->getTetraOpposite(v);

	auto tvo01 = tv01->getTetraOpposite(v);
	auto tvo12 = tv12->getTetraOpposite(v);
	auto tvo20 = tv20->getTetraOpposite(v);

	if ((to01 == t12) || (to01 == t20)) return false;
	if ((to12 == t20) || (to12 == t01)) return false;
	if ((to20 == t01) || (to20 == t12)) return false;

	if ((tvo01 == tv12) || (tvo01 == tv20)) return false;
	if ((tvo12 == tv20) || (tvo12 == tv01)) return false;
	if ((tvo20 == tv01) || (tvo20 == tv12)) return false;

	auto tn = Tetra::create();
	auto tvn = Tetra::create();

	auto vt = t01->vs[3];
	auto vb = tv01->vs[0];

	tetrasAll.add(tn);
	tetras31.add(tn);
	tetrasAll.add(tvn);

	tn->setVertices(v0, v1, v2, vt);
	tvn->setVertices(vb, v0, v1, v2);

	tn->setTetras(to12, to20, to01, tvn);
	tvn->setTetras(tn, tvo12, tvo20, tvo01);

	v0->tetra = tn;
	v1->tetra = tn;
	v2->tetra = tn;

	v0->scnum--;
	v1->scnum--;
	v2->scnum--;

	v0->cnum -= 2;
	v1->cnum -= 2;
	v2->cnum -= 2;
	vt->cnum -= 2;
	vb->cnum -= 2;

	to01->exchangeTetraOpposite(t01->getVertexOpposite(v), tn);
	to12->exchangeTetraOpposite(t12->getVertexOpposite(v), tn);
	to20->exchangeTetraOpposite(t20->getVertexOpposite(v), tn);

	tvo01->exchangeTetraOpposite(tv01->getVertexOpposite(v), tvn);
	tvo12->exchangeTetraOpposite(tv12->getVertexOpposite(v), tvn);
	tvo20->exchangeTetraOpposite(tv20->getVertexOpposite(v), tvn);

	tetrasAll.remove(t01);
	tetrasAll.remove(t12);
	tetrasAll.remove(t20);

	tetras31.remove(t01);
	tetras31.remove(t12);
	tetras31.remove(t20);

	tetrasAll.remove(tv01);
	tetrasAll.remove(tv12);
	tetrasAll.remove(tv20);

	Tetra::destroy(t01);
	Tetra::destroy(t12);
	Tetra::destroy(t20);

	Tetra::destroy(tv01);
	Tetra::destroy(tv12);
	Tetra::destroy(tv20);
	
	if (v == 234) {
		
		assert(verticesAll.contains(v));
		//assert(verticesSix.contains(v));
	}


	verticesAll.remove(v);
	//verticesSix.remove(v);
	Vertex::destroy(v);

	slabSizes.at(time) -= 2;
	slabSizes.at((time - 1 + nSlices) % nSlices) -= 2;
	sliceSizes.at(time) -= 2;

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);

	return true;
}

bool Universe::move44(Tetra::Label t012, Tetra::Label t230) {
	Vertex::Label v0, v1, v2, v3;

	v1 = t012->getVertexOppositeTetra(t230);
	v3 = t230->getVertexOppositeTetra(t012);

	for (int i = 0; i < 3; i++) {
		if (t012->vs[i] == v1) {
			v2 = t012->vs[(i + 1) % 3];
			v0 = t012->vs[(i + 2) % 3];
			break;	
		}
	}

	auto tv012 = t012->tnbr[3];
	auto tv230 = t230->tnbr[3];

	if (v0->scnum == 3) { return false; }
	if (v2->scnum == 3) { return false; }

	if (v1 == v3) return false;

	auto vt = t012->vs[3];
	auto vb = tv012->vs[0];

	auto ta01 = t012->getTetraOpposite(v2);
	auto ta12 = t012->getTetraOpposite(v0);
	auto ta23 = t230->getTetraOpposite(v0);
	auto ta30 = t230->getTetraOpposite(v2);

	auto tva01 = tv012->getTetraOpposite(v2);
	auto tva12 = tv012->getTetraOpposite(v0);
	auto tva23 = tv230->getTetraOpposite(v0);
	auto tva30 = tv230->getTetraOpposite(v2);

	if (ta01 == t230) return false;
	if (ta23 == t012) return false;

	if (tva01 == tv230) return false;
	if (tva23 == tv012) return false;

	//for (auto tt : tetrasAll) {
	//	if (tt->hasVertex(v1) && tt->hasVertex(v3)) return false;
	//}

	auto t012vo2 = t012->getVertexOpposite(v2);
	auto t230vo0 = t230->getVertexOpposite(v0);

	auto tv012vo2 = tv012->getVertexOpposite(v2);
	auto tv230vo0 = tv230->getVertexOpposite(v0);

	auto tn013 = t230;
	auto tn123 = t012;
	auto tvn013 = tv230;
	auto tvn123 = tv012;

	tn013->setVertices(v0, v1, v3, vt);
	tn123->setVertices(v1, v2, v3, vt);
	tvn013->setVertices(vb, v0, v1, v3);
	tvn123->setVertices(vb, v1, v2, v3);

	tn013->setTetras(tn123, ta30, ta01, tvn013);
	tn123->setTetras(ta23, tn013, ta12, tvn123);
	tvn013->setTetras(tn013, tvn123, tva30, tva01);
	tvn123->setTetras(tn123, tva23, tvn013, tva12);

	ta01->exchangeTetraOpposite(t012vo2, tn013);
	ta23->exchangeTetraOpposite(t230vo0, tn123);

	tva01->exchangeTetraOpposite(tv012vo2, tvn013);
	tva23->exchangeTetraOpposite(tv230vo0, tvn123);

	v0->scnum--;
	v1->scnum++;
	v2->scnum--;
	v3->scnum++;

	v0->cnum -= 2;
	v1->cnum += 2;
	v2->cnum -= 2;
	v3->cnum += 2;

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);
	updateVertexBags(v3);

	v0->tetra = tn013;
	v2->tetra = tn123;

	/*tn013->log();
	tn123->log();
	tvn013->log();
	tvn123->log();

	printf("ta01\n");
	ta01->log();
	printf("ta12\n");
	ta12->log();
	printf("ta23\n");
	ta23->log();
	printf("ta30\n");
	ta30->log();
	*/

	return true;
}

bool Universe::move23u(Tetra::Label t31, Tetra::Label t22) {
	Vertex::Label v0, v1, v2, v3, v4;

	v0 = t31->getVertexOppositeTetra(t22);
	v1 = t22->getVertexOppositeTetra(t31);

	int v0pos = -1;
	for (int i = 0; i < 3; i++) {
		if (t31->vs[i] == v0) { v0pos = i; break; }
	}
	assert(v0pos >= 0);

	v2 = t31->vs[(v0pos + 1) % 3];
	v4 = t31->vs[(v0pos + 2) % 3];

	v3 = t31->vs[3];

	Tetra::Label ta023, ta034, ta123, ta124, ta134;

	ta023 = t31->getTetraOpposite(v4);
	ta034 = t31->getTetraOpposite(v2);
	ta123 = t22->getTetraOpposite(v4);
	ta124 = t22->getTetraOpposite(v3);
	ta134 = t22->getTetraOpposite(v2);

	if (ta023->hasVertex(v1)) return false;
	if (ta123->hasVertex(v0)) return false;
	if (ta034->hasVertex(v1)) return false;
	if (ta134->hasVertex(v0)) return false;

	if (v0->neighborsVertex(v1)) return false;

	auto tn31 = Tetra::create();
	auto tn22l = Tetra::create();
	auto tn22r = Tetra::create();

	tetrasAll.add(tn31);
	tetras31.add(tn31);
	tetrasAll.add(tn22l);
	tetrasAll.add(tn22r);

	tn31->setVertices(v0, v2, v4, v1);
	tn22l->setVertices(v0, v2, v1, v3);
	tn22r->setVertices(v0, v4, v1, v3);

	tn31->setTetras(ta124, tn22r, tn22l, t31->tnbr[3]);
	tn22l->setTetras(ta123, tn22r, ta023, tn31);
	tn22r->setTetras(ta134, tn22l, ta034, tn31);

	int time = tn31->vs[0]->time;
	slabSizes.at(time) += 1;

	t31->tnbr[3]->exchangeTetraOpposite(t31->tnbr[3]->vs[0], tn31);

	ta023->exchangeTetraOpposite(t31->getVertexOpposite(v4), tn22l);
	ta034->exchangeTetraOpposite(t31->getVertexOpposite(v2), tn22r);
	ta123->exchangeTetraOpposite(t22->getVertexOpposite(v4), tn22l);
	ta124->exchangeTetraOpposite(t22->getVertexOpposite(v3), tn31);
	ta134->exchangeTetraOpposite(t22->getVertexOpposite(v2), tn22r);

	v0->cnum += 2;
	v1->cnum += 2;
	
	tetrasAll.remove(t31);
	tetras31.remove(t31);
	tetrasAll.remove(t22);

	Tetra::destroy(t31);
	Tetra::destroy(t22);

	tn31->vs[0]->tetra = tn31;
	tn31->vs[1]->tetra = tn31;
	tn31->vs[2]->tetra = tn31;

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);
	updateVertexBags(v3);
	updateVertexBags(v4);
	
	return true;
}

bool Universe::move32u(Tetra::Label t31, Tetra::Label t22l, Tetra::Label t22r) {
	Vertex::Label v0, v1, v2, v3, v4;
	
	v1 = t31->vs[3];
	v3 = t22l->getVertexOppositeTetra(t31);
	v4 = t31->getVertexOppositeTetra(t22l);

	int v4pos = -1;
	for (int i = 0; i < 3; i++) {
		if (t31->vs[i] == v4) { v4pos = i; break; }
	}
	assert(v4pos >= 0);

	v0 = t31->vs[(v4pos + 1) % 3];
	v2 = t31->vs[(v4pos + 2) % 3];

	Tetra::Label ta023, ta034, ta123, ta124, ta134;

	ta023 = t22l->getTetraOpposite(v1);
	ta034 = t22r->getTetraOpposite(v1);
	ta123 = t22l->getTetraOpposite(v0);
	ta124 = t31->getTetraOpposite(v0);
	ta134 = t22r->getTetraOpposite(v0);

	if (ta023->hasVertex(v4)) return false;
	if (ta123->hasVertex(v4)) return false;
	if (ta034->hasVertex(v2)) return false;
	if (ta124->hasVertex(v3)) return false;
	if (ta134->hasVertex(v2)) return false;

	auto tn31 = Tetra::create();
	auto tn22 = Tetra::create();

	tetrasAll.add(tn31);
	tetras31.add(tn31);
	tetrasAll.add(tn22);

	tn31->setVertices(v0, v2, v4, v3);
	tn22->setVertices(v2, v4, v1, v3);

	tn31->setTetras(tn22, ta034, ta023, t31->tnbr[3]);
	tn22->setTetras(ta134, ta123, tn31, ta124);

	t31->tnbr[3]->exchangeTetraOpposite(t31->tnbr[3]->vs[0], tn31);
	ta023->exchangeTetraOpposite(t22l->getVertexOpposite(v1), tn31);
	ta034->exchangeTetraOpposite(t22r->getVertexOpposite(v1), tn31);
	ta123->exchangeTetraOpposite(t22l->getVertexOpposite(v0), tn22);
	ta124->exchangeTetraOpposite(t31->getVertexOpposite(v0), tn22);
	ta134->exchangeTetraOpposite(t22r->getVertexOpposite(v0), tn22);

	v0->cnum -= 2;
	v1->cnum -= 2;

	tetrasAll.remove(t31);
	tetras31.remove(t31);
	tetrasAll.remove(t22l);
	tetrasAll.remove(t22r);

	Tetra::destroy(t31);
	Tetra::destroy(t22l);
	Tetra::destroy(t22r);

	int time = tn31->vs[0]->time;
	slabSizes.at(time) -= 1;

	tn31->vs[0]->tetra = tn31;
	tn31->vs[1]->tetra = tn31;
	tn31->vs[2]->tetra = tn31;

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);
	updateVertexBags(v3);
	updateVertexBags(v4);
	
	return true;
}

bool Universe::move23d(Tetra::Label t13, Tetra::Label t22) {
	Vertex::Label v0, v1, v2, v3, v4;

	v0 = t13->getVertexOppositeTetra(t22);
	v1 = t22->getVertexOppositeTetra(t13);

	int v0pos = -1;
	auto t31 = t13->tnbr[0];  // only because spatial ordering of 13-simplices is currently not guaranteed
	for (int i = 0; i < 3; i++) {
		if (t31->vs[i] == v0) { v0pos = i; break; }
	}
	assert(v0pos >= 0);

	v2 = t31->vs[(v0pos + 1) % 3];
	v4 = t31->vs[(v0pos + 2) % 3];

	v3 = t13->vs[0];

	Tetra::Label ta023, ta034, ta123, ta124, ta134;

	ta023 = t13->getTetraOpposite(v4);
	ta034 = t13->getTetraOpposite(v2);
	ta123 = t22->getTetraOpposite(v4);
	ta124 = t22->getTetraOpposite(v3);
	ta134 = t22->getTetraOpposite(v2);

	if (ta023->hasVertex(v1)) return false;
	if (ta123->hasVertex(v0)) return false;
	if (ta034->hasVertex(v1)) return false;
	if (ta134->hasVertex(v0)) return false;

	if (v0->neighborsVertex(v1)) return false;
	//for (auto tt : tetrasAll) {  // CHANGE! INEFFICIENT TMP SOLUTION
	//	if (tt->hasVertex(v0) && tt->hasVertex(v1)) return false;
	//}


	auto tn13 = Tetra::create();
	auto tn22l = Tetra::create();
	auto tn22r = Tetra::create();

	tetrasAll.add(tn13);
	tetrasAll.add(tn22l);
	tetrasAll.add(tn22r);

	tn13->setVertices(v1, v0, v2, v4);
	tn22l->setVertices(v1, v3, v0, v2);
	tn22r->setVertices(v1, v3, v0, v4);

	tn13->setTetras(t13->tnbr[0], ta124, tn22r, tn22l);
	tn22l->setTetras(ta023, tn13, ta123, tn22r);
	tn22r->setTetras(ta034, tn13, ta134, tn22l);

	int time = t31->vs[0]->time;
	slabSizes.at(time) += 1;

	t13->tnbr[0]->exchangeTetraOpposite(t13->tnbr[0]->vs[3], tn13);

	ta023->exchangeTetraOpposite(t13->getVertexOpposite(v4), tn22l);
	ta034->exchangeTetraOpposite(t13->getVertexOpposite(v2), tn22r);
	ta123->exchangeTetraOpposite(t22->getVertexOpposite(v4), tn22l);
	ta124->exchangeTetraOpposite(t22->getVertexOpposite(v3), tn13);
	ta134->exchangeTetraOpposite(t22->getVertexOpposite(v2), tn22r);

	v0->cnum += 2;
	v1->cnum += 2;
	
	tetrasAll.remove(t13);
	tetrasAll.remove(t22);

	Tetra::destroy(t13);
	Tetra::destroy(t22);

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);
	updateVertexBags(v3);
	updateVertexBags(v4);
	
	return true;
}

bool Universe::move32d(Tetra::Label t13, Tetra::Label t22l, Tetra::Label t22r) {
	Vertex::Label v0, v1, v2, v3, v4;
	
	v1 = t13->vs[0];
	v3 = t22l->getVertexOppositeTetra(t13);
	v4 = t13->getVertexOppositeTetra(t22l);

	auto t31 = t13->tnbr[0];
	int v4pos = -1;
	for (int i = 0; i < 3; i++) {
		if (t31->vs[i] == v4) { v4pos = i; break; }
	}
	assert(v4pos >= 0);

	v0 = t31->vs[(v4pos + 1) % 3];
	v2 = t31->vs[(v4pos + 2) % 3];

	Tetra::Label ta023, ta034, ta123, ta124, ta134;

	ta023 = t22l->getTetraOpposite(v1);
	ta034 = t22r->getTetraOpposite(v1);
	ta123 = t22l->getTetraOpposite(v0);
	ta124 = t13->getTetraOpposite(v0);
	ta134 = t22r->getTetraOpposite(v0);

	if (ta023->hasVertex(v4)) return false;
	if (ta123->hasVertex(v4)) return false;
	if (ta034->hasVertex(v2)) return false;
	if (ta124->hasVertex(v3)) return false;
	if (ta134->hasVertex(v2)) return false;

	auto tn13 = Tetra::create();
	auto tn22 = Tetra::create();

	tetrasAll.add(tn13);
	tetrasAll.add(tn22);

	tn13->setVertices(v3, v0, v2, v4);
	tn22->setVertices(v1, v3, v2, v4);

	tn13->setTetras(t13->tnbr[0], tn22, ta034, ta023);
	tn22->setTetras(tn13, ta124, ta134, ta123);

	t13->tnbr[0]->exchangeTetraOpposite(t13->tnbr[0]->vs[3], tn13);
	ta023->exchangeTetraOpposite(t22l->getVertexOpposite(v1), tn13);
	ta034->exchangeTetraOpposite(t22r->getVertexOpposite(v1), tn13);
	ta123->exchangeTetraOpposite(t22l->getVertexOpposite(v0), tn22);
	ta124->exchangeTetraOpposite(t13->getVertexOpposite(v0), tn22);
	ta134->exchangeTetraOpposite(t22r->getVertexOpposite(v0), tn22);

	v0->cnum -= 2;
	v1->cnum -= 2;

	tetrasAll.remove(t13);
	tetrasAll.remove(t22l);
	tetrasAll.remove(t22r);

	Tetra::destroy(t13);
	Tetra::destroy(t22l);
	Tetra::destroy(t22r);

	int time = tn13->vs[3]->time;
	slabSizes.at(time) -= 1;

	updateVertexBags(v0);
	updateVertexBags(v1);
	updateVertexBags(v2);
	updateVertexBags(v3);
	updateVertexBags(v4);

	return true;
}

void Universe::updateVertexBags(Vertex::Label v) {
	//if (verticesSix.contains(v) && (v->cnum != 6 || v->scnum != 3)) verticesSix.remove(v);
	//if (!verticesSix.contains(v) && v->cnum == 6 && v->scnum == 3) verticesSix.add(v);
}

void Universe::updateVertexData() {
	vertices.clear();
	int max = 0;
	for (auto v : verticesAll) {
		vertices.push_back(v);
		if (v > max) max = v;
	}

	vertexNeighbors.clear();
	vertexNeighbors.resize(max+1);

	for (auto v : verticesAll) {
		std::vector<Vertex::Label> nbr = {};
		auto t = v->tetra;
		//std::vector<Tetra::Label> ts = {t};
		std::vector<Tetra::Label> current = {t};
		std::vector<Tetra::Label> next = {};
		std::vector<Tetra::Label> done = {};

		do {
			for (auto tc : current) {
				for (auto tcn : tc->tnbr) {
					if (!tcn->hasVertex(v)) continue;
					if (std::find(done.begin(), done.end(), tcn) == done.end()) {
						done.push_back(tcn);
						next.push_back(tcn);
					}
				}
			}
			current = next;
			next.clear();
		} while (current.size() > 0);
		
		for (auto td : done) {
			for (auto vd : td->vs) {
				if (std::find(nbr.begin(), nbr.end(), vd) == nbr.end() && vd != v) nbr.push_back(vd);
			}
		}

		vertexNeighbors.at(v) = nbr;
	}
}

void Universe::updateHalfEdgeData() {
	for (int i = halfEdges.size() - 1; i >= 0; i--) {
		HalfEdge::destroy(halfEdges[i]);
	}

	halfEdges.clear();

	for (auto t : tetras31) {
		//t->log();
		std::array<HalfEdge::Label, 3> these;
		for (int i = 0; i < 3; i++) {
			auto h = HalfEdge::create();
			h->setVertices(t->vs[i], t->vs[(i + 1) % 3]);

			h->tetra = t;
			these[i] = h;
			halfEdges.push_back(h);
		}

		t->setHalfEdges(these[0], these[1], these[2]);

		for (int i = 0; i < 3; i++) {
			these[i]->next = these[(i + 1) % 3];
			these[i]->prev = these[(i - 1 + 3) % 3];
		}
	}

	for (auto t : tetras31) {
		for (int i = 0; i < 3; i++) {
			auto v = t->vs[i];
			auto vt = t->vs[3];

			auto tc = t->getTetraOpposite(v);
			Tetra::Label tn;
			v = vt;
			//t->log();
			//tc->log();
			//printf("======\n");
			while (tc->is22()) {
				tn = tc->getTetraOpposite(v);

				auto vo = v;
				v = tc->vs[2] == v ? tc->vs[3] : tc->vs[2];
				if (tn->is22()) {
					if (vo == tc->vs[2]) assert(v == tc->vs[3]);
					if (vo == tc->vs[3]) assert(v == tc->vs[2]);
				}
				//tc->log();
				//tn->log();
				//printf("vo: %d, v: %d\n", vo, v);
				tc = tn;
			}
			assert(tc->is31());

			auto hthis = t->hes[(i + 1) % 3];
			auto hthat = tc->getHalfEdgeTo(t->vs[(i + 1) % 3]);
			hthis->adj = hthat;
			hthat->adj = hthis;

			//printf("==========================\n");
		}
	}

	//for (auto h : halfEdges) {
	//	printf("h: %d, vi: %d, vf: %d, n: %d, p: %d, a: %d, t: %d\n", h, h->vs[0], h->vs[1], h->next, h->prev, h->adj, h->tetra);
	//}

	//for (auto h : halfEdges) {
	//	if (h->vs[0]->time != 1) continue;
	//	printf("%d %d %d\n", h->next, h->prev, h->adj);
	//}
}

void Universe::updateTriangleData() {
	for (int i = triangles.size() - 1; i >= 0; i--) {
		Triangle::destroy(triangles[i]);
	}

	triangles.clear();

	for (auto t : tetras31) {
		auto tr = Triangle::create();

		tr->setVertices(t->vs[0], t->vs[1], t->vs[2]);
		tr->setHalfEdges(t->hes[0], t->hes[1], t->hes[2]);

		t->hes[0]->triangle = tr;
		t->hes[1]->triangle = tr;
		t->hes[2]->triangle = tr;

		triangles.push_back(tr);
		
		//printf("tr: %d, v0: %d, v1: %d, v2: %d, h0: %d, h1: %d, h2: %d\n", tr, tr->vs[0], tr->vs[1], tr->vs[2], tr->hes[0], tr->hes[1], tr->hes[2]);
	}
	triangleNeighbors.resize(triangles.size());

	for (auto tr : triangles) {
		tr->setTriangleNeighbors(tr->hes[0]->getAdjacent()->triangle, tr->hes[1]->getAdjacent()->triangle, tr->hes[2]->getAdjacent()->triangle);

		triangleNeighbors.at(tr) = tr->trnbr;

	}
}

void Universe::updateGeometry() {
	updateVertexData();
	updateHalfEdgeData();
	updateTriangleData();
}

void Universe::check() {
	printf("====================================================\n");
	printf("c tetras: %d\n", Universe::tetrasAll.size());
	int count = 0;

	assert(Universe::tetrasAll.size() == Tetra::size());
	for (auto t : Universe::tetrasAll) {
		//printf("count: %d, t: %d\n", count, (int) t);
		count++;
		//t->log();

		for (int i = 0; i < 4; i++) {
			assert(verticesAll.contains(t->vs[i]));
			for (int j = i + 1; j < 4; j++) {
				assert(t->vs[i] != t->vs[j]);
			}
		}

		// do all neighbors still exist?
		for (int i = 0; i < 4; i++) {
			if (!tetrasAll.contains(t->tnbr[i])) {
				t->log();
				t->tnbr[i]->log();
			}
			assert(tetrasAll.contains(t->tnbr[i]));
			assert(t->tnbr[i]->neighborsTetra(t));
			assert(t->tnbr[i] != t);
			assert(t->tnbr[i] >= 0);
			int sv = 0;
			for (int j = 0; j < 4; j++) {
				auto vv = t->tnbr[i]->vs[j];
				if (t->hasVertex(vv)) sv++;
			}

			assert(sv >= 3);

			if (t->is31()) {
				if (i < 3) assert(t->tnbr[i]->is31() || t->tnbr[i]->is22());
				else assert(t->tnbr[i]->is13());
			} else if (t->is13()) {
				if (i == 0) assert(t->tnbr[i]->is31());
				else assert(t->tnbr[i]->is13() || t->tnbr[i]->is22());
			}
		}

		for (int i = 0; i < 4; i++) {
			assert(t->getTetraOpposite(t->vs[i]) == t->tnbr[i]);
			assert(t->tnbr[i]->getTetraOpposite(t->getVertexOpposite(t->vs[i])) == t);
		}

	}

	for (auto v : verticesAll) {
		int cnum = 0;
		assert(Universe::tetrasAll.contains(v->tetra));
		
		for (auto tt : Universe::tetrasAll) {
			if (tt->hasVertex(v)) cnum++;

			if (tt->hasVertex(v)) {
				for (auto vn : tt->vs) {
					if (vn == v) continue;
					
					assert(v->neighborsVertex(vn));
					assert(vn->neighborsVertex(v));
				}
			}
		}
		assert(cnum == v->cnum);

	}

	for (auto tr : triangles) {
		for (auto trn : tr->trnbr) {
			bool found = false;
			for (auto trnn : trn->trnbr) {
				if (trnn == tr) found = true;
			}
			assert(found);
		}
	}

	

	printf("====================================================\n");

}

// export geometry as Mathematica graph (very much redundant information)
void Universe::exportGraph() {
	

	printf("\n");
}

void Universe::exportSliceGraph(int time) {
}
