using namespace std;

//#ifdef _LINUX
#include <coin/ClpSimplex.hpp>
//#elseif _WIN32
//#include "ClpSimplex.hpp"
//#endif

#include "FecCalculator.h"
#include "Hyperedge.h"
#include "Vertex.h"

#include "FractionalEdgeCover.h"

FecCalculator::FecCalculator(const HypergraphSharedPtr &hg) : MyHg{ hg }
{
}

FecCalculator::~FecCalculator()
{
}

FractionalEdgeCover FecCalculator::computeFEC(const VertexSet &chi, double &weight) const
{
	set<Map *, classcomp> nodeToIndexMap;
	set<Map *> edgeToIndexMap;
	vector<HyperedgeSharedPtr> indexToEdgeMap;
	HyperedgeSet edges = getIncidentEdges(chi, nodeToIndexMap, edgeToIndexMap, indexToEdgeMap);

	// TODO debug
	/*
	cout << endl;
	cout << "Chi: ";
	set<Node *>::iterator nodesIt;
	for (nodesIt = chi->begin(); nodesIt != chi->end(); nodesIt++)
	{
		cout << (*nodesIt)->getName() << " ";
	}
	cout << endl;

	cout << "Incident edges: ";
	set<Hyperedge *>::iterator edgesIt;
	for (edgesIt = edges->begin(); edgesIt != edges->end(); edgesIt++)
	{
		cout << (*edgesIt)->getName() << " ";
	}
	cout << endl;
	*/
	// TODO debug

	int columns = edges.size();
	int rows = chi.size();

	int numElems = columns * rows;
	double *elements = new double[numElems];
	for (int i = 0; i < numElems; i++)
	{
		elements[i] = 0;
	}
	// TODO debug
	/*
	cout << endl;
	for (int z = 0; z < numElems; z++) {
		cout << elements[z];
	}
	cout << endl;
	*/
	computeElements(chi, nodeToIndexMap, edges, edgeToIndexMap, elements);
	/*
	for (int z = 0; z < numElems; z++) {
		if (z % columns == 0) cout << endl;
		cout << elements[z];
	}
	cout << endl;
	*/
	// TODO debug

	double *objective = new double[columns];
	double *colLower = new double[columns];
	double *colUpper = new double[columns];
	for (int i = 0; i < columns; i++)
	{
		objective[i] = 1.0;
		colLower[i] = 0.0;
		colUpper[i] = 1.0;
	}

	double *rowLower = new double[rows];
	double *rowUpper = new double[rows];
	for (int i = 0; i < rows; i++)
	{
		rowLower[i] = 1.0;
		rowUpper[i] = COIN_DBL_MAX;
	}

	int *rowStart = new int[rows + 1];
	for (int i = 0; i < rows + 1; i++)
	{
		rowStart[i] = i * columns;
	}
	int *column = new int[numElems];
	for (int i = 0; i < numElems; i++)
	{
		column[i] = i % columns;
	}

	CoinPackedMatrix matrix(false, columns, rows, numElems, elements, column, rowStart, NULL);
	ClpSimplex model;
	model.setLogLevel(0);
	model.loadProblem(matrix, colLower, colUpper, objective, rowLower, rowUpper);
	model.dual();

	delete elements;
	delete objective;
	delete colLower;
	delete colUpper;
	delete rowLower;
	delete rowUpper;
	delete rowStart;
	delete column;

	double width = 0.0;
	const double *solution = model.getColSolution();
	HyperedgeVector fecEdges;
	vector<double> fecWeights;
	for (int i = 0; i < model.getNumCols(); i++)
	{
		width += solution[i];
		if (solution[i] != 0.0)
		{
			fecEdges.push_back(indexToEdgeMap.at(i));
			fecWeights.push_back(solution[i]);
		}
	}

	
	for (auto &m : edgeToIndexMap) 
		delete m;
	for (auto &m :  nodeToIndexMap) {
		delete m;
	}

	weight = width;
	FractionalEdgeCover fec(chi, fecEdges, fecWeights);
	return fec;
}

HyperedgeSet FecCalculator::getIncidentEdges(const VertexSet &chi, set<Map *, classcomp> &nodeToindexMap,
	set<Map *> &edgeToIndexMap, vector<HyperedgeSharedPtr> &indexToEdgeMap) const
{
	HyperedgeSet edges;
	int j = 0, k = 0;
	for (auto &v : chi)
	{
		Map *mv = new Map();
		mv->comp = v;
		mv->index = j++;
		nodeToindexMap.insert(mv);
		for (auto &e : MyHg->allVertexNeighbors(v))
		{
			if (edges.find(e) == edges.end())
			{
				edges.insert(e);
				Map *m = new Map();
				m->comp = e;
				m->index = k++;
				edgeToIndexMap.insert(m);
				indexToEdgeMap.push_back(e);
			}
		}
	}
	return edges;
}

void FecCalculator::computeElements
(const VertexSet &chi, set<Map *, classcomp> &nodeToIndexMap,
	const HyperedgeSet &edges, set<Map *> &edgeToIndexMap, double *elements) const
{
	int edgesNbr = edges.size();
	Map *temp = new Map();

	for (auto &m : edgeToIndexMap)
	{
		HyperedgeSharedPtr e = dynamic_pointer_cast<Hyperedge>(m->comp);
		int j = m->index;


		for (auto &v : e->allVertices())
		{
			if (chi.find(v) != chi.end())
			{
				temp->comp = v;
				Map *m = *(nodeToIndexMap.find(temp));
				// find v in nodeToIndexMap
				int i = m->index;
				elements[i * edgesNbr + j] = 1;
			}
		}
	}
	delete temp;
}
