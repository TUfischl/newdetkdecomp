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
#include "../Vertex.h"
#include "../Hyperedge.h"
#include "../Globals.h"
#include "../DetKDecomp.h"
#include "../BalKDecomp.h"
#include "../Subedges.h"

void usage(int, char **, int *, bool *);
HypertreeSharedPtr decompK(HypergraphSharedPtr, int);


char *cInpFile, *cOutFile;



int main(int argc, char **argv)
{
	int iRandomInit, K = 0;
	bool bDef;
	time_t start, end;
	HypergraphSharedPtr HG = make_shared<Hypergraph>();
	Parser *P;
	HypertreeSharedPtr HT;

	cout << "*** localbip-k-decomp (version 2.0) ***" << endl << endl;

	// Check command line arguments and initialize random number generator
	usage(argc, argv, &K, &bDef);
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
	HG->buildHypergraph(*P);
	if (!HG->isConnected())
		cerr << "Warning: Hypergraph is not connected." << endl;
	time(&end);
	cout << "Building hypergraph done in " << difftime(end, start) << " sec." << endl << endl;
	delete P;

	HT = decompK(HG, K);

	// Check hypertree conditions
	if (HT != NULL)
	{
		cout << "Checking hypertree conditions ... " << endl;
		time(&start);
		HT->verify();
		time(&end);
		cout << "Checking hypertree conditions done in " << difftime(end, start) << " sec." << endl << endl;
		HT->outputToGML(cOutFile);
		cout << "GML output written to: " << cOutFile << endl << endl;
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

//LocalBIP
HypertreeSharedPtr decompK(HypergraphSharedPtr HG, int iWidth)
{
	time_t start, end;
	HypertreeSharedPtr HT;
	DetKDecomp Decomp(HG, iWidth, true);

	// Apply the decomposition algorithm
	cout << "Building generalized hypertree (localbip-" << iWidth << "-decomp) ... " << endl;
	time(&start);
	HT = Decomp.buildHypertree();
	time(&end);
	if (HT == NULL)
		cout << "Generalized hypertree of width " << iWidth << " not found in " << difftime(end, start) << " sec." << endl << endl;
	else {
		cout << "Building generalized hypertree done in " << difftime(end, start) << " sec";
		cout << " (generalized hypertree-width: " << HT->getHTreeWidth() << ")." << endl << endl;

		HT->shrink(false);
	}

	return HT;
}
