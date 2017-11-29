/*
#include "Hypergraph.h"
#include "Vertex.h"
#include "Subedges.h"
#include "SubedgeSeparatorFactory.h"

using namespace std;

int main(int argc, char **argv) {
	Vertex* v1 = new Vertex("v1");
	Vertex* v2 = new Vertex("v2");
	Vertex* v3 = new Vertex("v3");
	Hyperedge* e1 = new Hyperedge("e1");
	Hyperedge* e2 = new Hyperedge("e2");
	Hyperedge* e3 = new Hyperedge("e3");

	e1->add(v1);
	e1->add(v2);
	e2->add(v3);
	e2->add(v1);
	e3->add(v2);
	e3->add(v3);

	Hypergraph H;

	H.insertEdge(e1);
	H.insertEdge(e2);
	H.insertEdge(e3);

	if (H.isConnected())
		cout << "Hypergraph is connected!" << endl;
	else
		cout << "Hypergraph is NOT connected!" << endl;

	Hypergraph HD;
	H.makeDual(HD);

	vector<int> ve{ 1,2,3,4,5,6,7,8,9,10 };
	vector<int> sort{ 10,9,8,7,6,5,4,3,2,1 };

	sortVectors<int>(ve, sort, 0, (int)ve.size() - 1);

	for (auto i : ve)
		cout << i << " ";
	cout << endl;

	Subedges S(&H,2);
	HE_SET sub_edges;

	cout << "Subedges for " << *e1 << endl;
	for (auto he : *(S.getSubedges(e1))) {
		cout << *he << endl;
		sub_edges.insert(he);
	}
	cout << endl;

	cout << "Subedges for " << *e2 << endl;
	for (auto he : *(S.getSubedges(e2))){
		cout << *he << endl;
	sub_edges.insert(he);
}
	cout << endl;

	cout << "Subedges for " << *e3 << endl;
	for (auto he : *(S.getSubedges(e3))){
		cout << *he << endl;
	sub_edges.insert(he);
	}
	cout << endl;

	cout << "All subedges:" << endl;
	for (auto he : sub_edges)
		cout << *he << endl;

	HE_VEC sep;
	sep.push_back(e1);
	sep.push_back(e2);

	
	cout << "Test SubedgeSeparatorFactory for " << sep << endl;
	SubedgeSeparatorFactory sf(&H, &sep, &sep, &S);
	HE_VEC *sub_sep;
	while ((sub_sep = sf.next()) != nullptr)
		cout << *sub_sep << endl;

	return 0;
}

*/

// det-k-decomp V1.0
//
// Reference paper: G. Gottlob and M. Samer,
// A Backtracking-Based Algorithm for Computing Hypertree-Decompositions,
// Submitted for publication.
//
// Note: This program is a prototype implementation and does in no sense
// claim to be the most efficient way of implementing det-k-decomp. Moreover,
// several parts of the code have been developed within an implementation
// framework for evaluating several decomposition algorithms. These parts of 
// the code may therefore be unnecessary or are formulated in a more general 
// way than would be necessary for det-k-decomp.


// det-k-decomp V2.0
//
// Reference paper: G. Gottlob and M. Samer,
// A Backtracking-Based Algorithm for Computing Hypertree-Decompositions,
// Submitted for publication.
//
// Note: This program is a prototype implementation and does in no sense
// claim to be the most efficient way of implementing det-k-decomp. Moreover,
// several parts of the code have been developed within an implementation
// framework for evaluating several decomposition algorithms. These parts of 
// the code may therefore be unnecessary or are formulated in a more general 
// way than would be necessary for det-k-decomp.


#define _CRT_SECURE_NO_DEPRECATE

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <list>
#include <cstring>

using namespace std;

#include "Parser.h"
#include "Hypergraph.h"
#include "Hypertree.h"
#include "Component.h"
#include "Vertex.h"
#include "Hyperedge.h"
#include "Globals.h"
#include "DetKDecomp.h"
#include "Hingetree.h"
#include "HingeDecomp.h"
#include "BalKDecomp.h"

void usage(int, char **, int *, bool *);
Hypertree *decompK(Hypergraph *, int);
bool verify(Hypergraph *, Hypertree *);


char *cInpFile, *cOutFile;



int main(int argc, char **argv)
{
	int iRandomInit, K = 0;
	bool bDef;
	time_t start, end;
	Hypergraph HG;
	Parser *P;
	Hypertree *HT;

	cout << "*** det-k-decomp (version 2.0) ***" << endl << endl;

	// Check command line arguments and initialize random number generator
	usage(argc, argv, &K, &bDef);
	//srand(100);
	srand((unsigned int)time(NULL));
	iRandomInit = random_range(999, 9999);
	for (int i = 0; i < iRandomInit; i++) rand();

	// Create parser object
	if ((P = new Parser(bDef)) == NULL)
		writeErrorMsg("Error assigning memory.", "main");

	// Parse file
	cout << "Parsing input file \"" << cInpFile << "\" ... " << endl;
	time(&start);
	P->parseFile(cInpFile);
	time(&end);
	cout << "Parsing input file done in " << difftime(end, start) << " sec";
	cout << " (" << P->getNbrOfAtoms() << " atoms, " << P->getNbrOfVars() << " variables)." << endl << endl;

	// Build hypergraph
	cout << "Building hypergraph ... " << endl;
	time(&start);
	HG.buildHypergraph(P);
	if (!HG.isConnected())
		cerr << "Warning: Hypergraph is not connected." << endl;
	time(&end);
	cout << "Building hypergraph done in " << difftime(end, start) << " sec." << endl << endl;
	delete P;

	HT = decompK(&HG, K);

	// Check hypertree conditions
	if (HT != NULL)
	{
		cout << "Checking hypertree conditions ... " << endl;
		time(&start);
		verify(&HG, HT);
		time(&end);
		cout << "Checking hypertree conditions done in " << difftime(end, start) << " sec." << endl << endl;
		HT->outputToGML(&HG, cOutFile);
		cout << "GML output written to: " << cOutFile << endl << endl;
		delete HT;
	}

	return EXIT_SUCCESS;
}


void usage(int argc, char **argv, int *K, bool *bDef)
{
	int i, j, k;
	*bDef = false;

	// Check arguments
	for (i = 1; (i < argc) && (argv[i][0] == '-'); i++)
		if (strcmp(argv[i], "-def") == 0)
			*bDef = true;
		else {
			cerr << "Unknown argument \"" << argv[i] << "\"." << endl;
			exit(EXIT_FAILURE);
		}

		if (i < argc - 1) {
			for (j = 0; argv[i][j] == '0'; j++);
			for (k = 0; (k < 6) && (argv[i][j + k] != '\0'); k++)
				if (!((argv[i][j + k] >= '0') && (argv[i][j + k] <= '9')))
					break;
			if (argv[i][j + k] == '\0') {
				*K = atoi(argv[i++]);
				if (*K < 1) {
					cerr << "Illegal argument k = 0." << endl;
					exit(EXIT_FAILURE);
				}
			}
			else {
				cerr << "Illegal argument k = " << argv[i] << "." << endl;
				exit(EXIT_FAILURE);
			}
		}

		// Write usage error message
		if ((!*bDef && (argc < 3)) || (*bDef && (argc < 4)) || (i < argc - 1)) {
			cerr << "Usage: " << argv[0] << " [-def] <k> <filename>" << endl;
			exit(EXIT_FAILURE);
		}

		cInpFile = argv[i];

		// Construct output-file name
		for (i = (int)strlen(cInpFile) - 1; i > 0; i--)
			if (cInpFile[i] == '.')
				break;
		if (i > 0)
			cInpFile[i] = '\0';
		cOutFile = new char[strlen(cInpFile) + 5];
		if (cOutFile == NULL)
			writeErrorMsg("Error assigning memory.", "usage");
		strcpy(cOutFile, cInpFile);
		strcat(cOutFile, ".gml");
		if (i > 0)
			cInpFile[i] = '.';
}

/*
Hypertree *decompK(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *HT;
	DetKDecomp Decomp(HG, iWidth, true);

	// Apply the decomposition algorithm
	cout << "Building hypertree (det-" << iWidth << "-decomp) ... " << endl;
	time(&start);
	HT = Decomp.buildHypertree();
	time(&end);
	if (HT == NULL)
		cout << "Hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
	else {
		cout << "Building hypertree done in " << difftime(end, start) << " sec";
		cout << " (hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

		HT->shrink(false);
	}

	return HT;
}*/


Hypertree *decompK(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *HT;
	BalKDecomp Decomp(HG, iWidth);
	BalKDecomp::init(HG, 1);

	// Apply the decomposition algorithm
	cout << "Building hypertree (det-" << iWidth << "-decomp) ... " << endl;
	time(&start);
	HT = Decomp.buildHypertree();
	time(&end);
	if (HT == NULL)
		cout << "Hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
	else {
		cout << "Building hypertree done in " << difftime(end, start) << " sec";
		cout << " (hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

		HT->shrink(false);
	}

	return HT;
}



/*

Hypertree *decompK(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *htree{ nullptr };
	HingeDecomp HingeDecomp(HG,iWidth);
	Hingetree *hinge{ nullptr };

	cout << "Building hingetree" << endl;
	time(&start);
	hinge = HingeDecomp.buildHingetree();
	time(&end);
	cout << *hinge << endl;
	cout << "Nbr of Hinges: " << hinge->nbrOfNodes() << endl;
	cout << "Largest Hinge: " << hinge->sizeOfLargestHinge() << endl;
	cout << "Building hingetree done in " << difftime(end, start) << " sec" << endl;

	//if (bGhw) {

		// Apply the decomposition algorithm
		cout << "Building hypertree (det-" << iWidth << "-decomp) ... " << endl;
		time(&start);
		htree = HingeDecomp.buildHypertree();
		time(&end);
		if (htree == nullptr) {
			cout << "Hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
		}
		else {
			cout << "Building hypertree done in " << difftime(end, start) << " sec";
			cout << " (hypertree-width: " << htree->getHTreeWidth() << ")." << endl << endl;

			htree->shrink(false);
		}

	//}

	return htree;
}

*/

bool verify(Hypergraph *HG, Hypertree *HT)
{
	bool bAllCondSat = true;
	Vertex *WitnessNode;
	Hyperedge *WitnessEdge;
	Hypertree *WitnessTree;

	// Check acyclicity of the hypertree
	HT->resetLabels();
	if (HT->isCyclic())
		writeErrorMsg("Hypertree contains cycles.", "verify");
	HT->setIDLabels();

	// Check condition 1
	cout << "Condition 1: ";
	cout.flush();
	if ((WitnessEdge = HT->checkCond1(HG)) == NULL)
		cout << "satisfied." << endl;
	else {
		cout << "violated! (see atom \"" << WitnessEdge->getName() << "\")" << endl;
		bAllCondSat = false;
	}

	// Check condition 2
	cout << "Condition 2: ";
	cout.flush();
	if ((WitnessNode = HT->checkCond2(HG)) == NULL)
		cout << "satisfied." << endl;
	else {
		cout << "violated! (see variable \"" << WitnessNode->getName() << "\")" << endl;
		bAllCondSat = false;
	}

	// Check condition 3
	cout << "Condition 3: ";
	cout.flush();
	if ((WitnessTree = HT->checkCond3(HG)) == NULL)
		cout << "satisfied." << endl;
	else {
		cout << "violated! (see hypertree node \"" << WitnessTree->getLabel() << "\")" << endl;
		bAllCondSat = false;
	}

	// Check condition 4
	cout << "Condition 4: ";
	cout.flush();
	if ((WitnessTree = HT->checkCond4(HG)) == NULL)
		cout << "satisfied." << endl;
	else
		cout << "violated!" << endl;
	// (see hypertree node \"" << WitnessTree->getLabel() << "\")" << endl;

	return bAllCondSat;
}



