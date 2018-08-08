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


// hypergraph-properties V2.0
//
// Reference paper: W. Fischl, G. Gottlob and R. Pichler,
// Hypergraph Decomposition Methods: From Theory to Practice,
// Submitted for publication.
//
// Note: This program is a prototype implementation and does in no sense
// claim to be the most efficient way of implementing hypergraph-properties. Moreover,
// several parts of the code have been developed within an implementation
// framework for evaluating several decomposition algorithms. These parts of 
// the code may therefore be unnecessary or are formulated in a more general 
// way than would be necessary for hypergraph-properties.


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


char *cInpFile, *cOutFile;



int main(int argc, char **argv)
{
	int iRandomInit, K = 0;
	bool bDef;
	time_t start, end;
	Hypergraph HG;
	Parser *P;
	Hypertree *HT;

	cout << "*** hypergraph-properties (version 2.0) ***" << endl << endl;

	// Check command line arguments and initialize random number generator
	usage(argc, argv, &K, &bDef);
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
	HG.buildHypergraph(*P);
	if (!HG.isConnected())
		cerr << "Warning: Hypergraph is not connected." << endl;
	time(&end);
	cout << "Building hypergraph done in " << difftime(end, start) << " sec." << endl << endl;
	delete P;

	cout << "Vertices: " << HG.getNbrOfVertices() << endl;
	cout << "Edges: " << HG.getNbrOfEdges() << endl;
	
	cout << "Degree: " << HG.degree() << endl; 
	cout << "Arity: " << HG.arity() << endl;
	cout << "BIP: " << HG.bip(2) << endl;
	cout << "3-BIP: " << HG.bip(3) << endl;
	cout << "4-BIP: " << HG.bip(4) << endl;
	cout << "VC-dim: " << HG.vcDimension() << endl;
        
        

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


		// Write usage error message
		if ((!*bDef && (argc < 2)) || (*bDef && (argc < 3)) || (i < argc - 1)) {
			cerr << "Usage: " << argv[0] << " <filename>" << endl;
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
