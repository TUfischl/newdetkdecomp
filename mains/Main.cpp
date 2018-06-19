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


// localbip-k-decomp V2.0
//
// Reference paper: W. Fischl, G. Gottlob and R. Pichler,
// Hypergraph Decomposition Methods: From Theory to Practice,
// Submitted for publication.
//
// Note: This program is a prototype implementation and does in no sense
// claim to be the most efficient way of implementing localbip-k-decomp. Moreover,
// several parts of the code have been developed within an implementation
// framework for evaluating several decomposition algorithms. These parts of 
// the code may therefore be unnecessary or are formulated in a more general 
// way than would be necessary for localbip-k-decomp.


#define _CRT_SECURE_NO_DEPRECATE

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <list>
#include <cstring>

using namespace std;

#include "../Parser.h"
#include "../Hypergraph.h"
#include "../Hypertree.h"
#include "../Component.h"
#include "../Vertex.h"
#include "../Hyperedge.h"
#include "../Globals.h"
#include "../DetKDecomp.h"
#include "../Hingetree.h"
#include "../HingeDecomp.h"
#include "../BalKDecomp.h"
#include "../Subedges.h"

void usage(int, char **, bool *);
Hypertree *decompKSamer(Hypergraph *, int);
Hypertree *decompKLocalBIP(Hypergraph *, int);
Hypertree *decompKGlobalBIP(Hypergraph *, int);
Hypertree *decompKBalSep(Hypergraph *, int);


bool verify(Hypergraph *, Hypertree *);


char *cInpFile, *cOutFile;



int main(int argc, char **argv)
{
	int iRandomInit, K = 3;
	bool bDef;
	bool bFound = false;
	time_t start, end;
	Hypergraph HG;
	Parser *P;
	Hypertree *HT;

	cout << "*** Hyperbench Analysis (version 1.0) ***" << endl << endl;

	// Check command line arguments and initialize random number generator
	usage(argc, argv, &bDef);
	//srand(200);
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


	cout << "Arity: " << HG.arity() << endl;
	cout << "Degree: " << HG.degree() << endl; 
	cout << "BIP: " << HG.bip(2) << endl;
	cout << "3-BIP: " << HG.bip(3) << endl;
	cout << "4-BIP: " << HG.bip(4) << endl;
	cout << "VC-dim: " << HG.vcDimension() << endl;

	do {

		HT = decompKSamer(&HG, K);

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
			K--;
		}

	} while (K >= 1 && HT==nullptr);


	//Start GHW analysis if K >= 2
	if (K >= 2) {

		HT = decompKGlobalBIP(&HG, K);

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

		HT = decompKLocalBIP(&HG, K);

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

		HT = decompKBalSep(&HG, K);

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

	}

	return EXIT_SUCCESS;
}


void usage(int argc, char **argv, bool *bDef)
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

		/*
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
		*/

		// Write usage error message
		if ((!*bDef && (argc < 2)) || (*bDef && (argc < 3)) || (i < argc - 1)) {
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


//Samer
Hypertree *decompKSamer(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *HT;
	DetKDecomp Decomp(HG, iWidth, false);

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




//LocalBIP
Hypertree *decompKLocalBIP(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *HT;
	DetKDecomp Decomp(HG, iWidth, true);

	// Apply the decomposition algorithm
	cout << "Building generalized hypertree (localbip-" << iWidth << "-decomp) ... " << endl;
	time(&start);
	HT = Decomp.buildHypertree();
	time(&end);
	if (HT == NULL)
		cout << "Generalized Hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
	else {
		cout << "Building generalized hypertree done in " << difftime(end, start) << " sec";
		cout << " (generalized-hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

		HT->shrink(false);
	}

	return HT;
}



//GlobalBIP
Hypertree *decompKGlobalBIP(Hypergraph *HG, int iWidth)
{
time_t start, end;
Hypertree *HT;
DetKDecomp Decomp(HG, iWidth, false);
Subedges subs(HG,iWidth);
HE_SET edges;

cout << "Adding subedges ... " << endl;
time(&start);
// Add Subedges
for (auto e : HG->allEdges())
    for (auto sub : *(subs.getSubedges(e)))
		edges.insert(sub);

for (auto e : edges)
HG->insertEdge(e);
time(&end);
cout << edges.size() << " subedges added in " << difftime(end, start) << " sec." << endl << endl;


// Apply the decomposition algorithm
cout << "Building generalized hypertree (globalbip-" << iWidth << "-decomp) ... " << endl;
time(&start);
HT = Decomp.buildHypertree();
time(&end);
if (HT == NULL)
cout << "Generalized hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
else {
cout << "Building generalized hypertree done in " << difftime(end, start) << " sec";
cout << " (generalized-hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

HT->shrink(false);
}

return HT;
}





//BalSeparator
Hypertree *decompKBalSep(Hypergraph *HG, int iWidth)
{
	time_t start, end;
	Hypertree *HT;
	BalKDecomp Decomp(HG, iWidth);
	BalKDecomp::init(HG, 0);

	// Apply the decomposition algorithm
	cout << "Building generalized hypertree (balsep-" << iWidth << "-decomp) ... " << endl;
	time(&start);
	HT = Decomp.buildHypertree();
	time(&end);
	if (HT == NULL)
		cout << "Generalized hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
	else {
		cout << "Building generalized hypertree done in " << difftime(end, start) << " sec";
		cout << " (generalized-hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

		//HT->shrink(false);
	}

	return HT;
}



/*
//With HingeDecomp
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



