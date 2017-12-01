// Models a hypertree node.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPERTREE)
#define CLS_HYPERTREE

#include "Globals.h"

#include <list>
#include <set>

using namespace std;

class Hypergraph;
class Hyperedge;
class Vertex;

class Hypertree  
{
private:
	// Pointer to the hypergraph corresponding to this hypertree
	Hypergraph *MyHg;

	// Pointer to the parent node in the hypertree
	Hypertree *MyParent{ nullptr };

	// Set of pointers to the children in the hypertree
	list<Hypertree *> MyChildren;

	// Set of pointers to nodes representing the chi-set
	VE_SET MyChi;

	// Set of pointers to edges representing the lambda-set
	HE_SET MyLambda;

	// Set of pointers used for the construction of a hypertree
	set<void *> MyPointers;

	// Set of IDs used for the construction of a hypertree
	set<int> MyIDs;

	// Label of the hypertree node (useful for miscellaneous computations)
	int iMyLabel{ 0 };

	// Indicates whether the actual hypertree-branch was cut
	bool bMyCut{ false };

	// Labels all hyperedges covered by the chi-set of some node
	void labelCovEdges(Hypergraph *HGraph);

	// Labels the variables in the chi-sets of all nodes
	void labelChiSets(int iLabel = 1);

	// Selects for each hyperedge a hypertree node to cover its nodes
	void selCovHTNodes(vector<Hypertree *> &CovNodes, bool bStrict = true);

	// Sets the chi-set such that the chi-labellings are connected
	void setChi_Conn(Hypergraph *HGraph);

	// The method checks whether the hypertree contains cycles
	bool isCyclic_pvt();

	// Checks condition 2
	Vertex *checkCond2();

	// Checks condition 3
	Hypertree *checkCond3();

	// Reduces the labellings in the lambda-sets
	void reduceLambdaTopDown();

	// Reduces the labellings in the lambda-sets
	void reduceLambdaBottomUp();

	// Removes redundant nodes by comparing the lambda sets
	void shrinkByLambda();

	// Removes redundant nodes by comparing the chi sets
	void shrinkByChi();

	// Writes hypertree nodes into a GML file
	void writeGMLNodes(ofstream &GMLFile);

	// Writes hypertree edges into a GML file
	void writeGMLEdges(ofstream &GMLFile);

public:
	// Constructor
	Hypertree(Hypergraph *Hg);

	// Destructor
	virtual~Hypertree();

	// Writes hypertree to GML format file
	void outputToGML(Hypergraph *HGraph, char *cNameOfFile);
    
	// Inserts a node into the chi-set 
	void insChi(Vertex *Vertex);

	// Inserts an edge into the lambda-set 
	void insLambda(Hyperedge *Edge);

	// Sets pointer to the parent node
	void setParent(Hypertree *Parent);

	// Returns pointer to the parent node
	Hypertree *getParent();

	// Inserts a pointer to a child
	void insChild(Hypertree *Child, bool bSetParent = true);

	// Removes a pointer to a child
	bool remChild(Hypertree *Child);

	// Removes all pointers to children
	void remChildren(bool SetParent = true, Hypertree *NewParent = nullptr);
	
	// Inserts a pointer into the pointer set;
	// these pointers can be used for the construction of hypertrees
	void insPointer(void *Ptr);

	// Inserts an ID into the ID set;
	// these IDs can be used for the construction of hypertrees
	void insID(int iID);

	// Sets the cut status
	void setCut(bool bCut = true);

	// Returns true if the actual hypertree-node was cut; otherwise false
	bool isCut();

	// Returns a cut tree node within the subtree rooted at
	// the actual tree node
	Hypertree *getCutNode();

	// Returns the tree node within the subtree rooted at the 
	// actual tree node, whose pointer list contains Ptr
	Hypertree *getHTNode(void *Ptr);

	// Returns the tree node within the subtree rooted at the 
	// actual tree node, whose ID list contains iID
	Hypertree *getHTNode(int iID);

	// Sets the actual tree node as root of the hypertree
	void setRoot();

	// Returns true iff the hypertree node does not have a parent
	bool isRoot();

	// Returns the hypertreewidth, i.e., the maximum number of elements 
	// in the lambda-set over all nodes in the subtree
	size_t getHTreeWidth();

	// Returns the treewidth, i.e., the maximum number of elements 
	// in the chi-set over all nodes in the subtree
	size_t getTreeWidth();

	// Returns the chi-set labelling the hypertree-node
	VE_SET *getChi();

	auto allChi() -> decltype(make_iterable(MyChi.begin(), MyChi.end())) {
		return make_iterable(MyChi.begin(), MyChi.end());
	}

	// Returns the lambda-set labelling the hypertree-node
	HE_SET *getLambda();

	auto allLambda() -> decltype(make_iterable(MyLambda.begin(), MyLambda.end())) {
		return make_iterable(MyLambda.begin(), MyLambda.end());
	}

	// Returns all children
	auto allChildren() -> decltype(make_iterable(MyChildren.begin(), MyChildren.end())) {
		return make_iterable(MyChildren.begin(), MyChildren.end());
	}

	// Sets the label of the hypertree-node
	void setLabel(int iLabel);

	// Returns the label of the hypertree-node
	int getLabel();

	// Sets labels of all hypertree-nodes in the subtree to zero
	void resetLabels();

	// Sets the labels of all hypertree-nodes in the subtree to a unique ID
	int setIDLabels(int iStartID = 1);

	// Checks whether the hypertree contains cycles
	bool isCyclic();

	// Removes vertices from Chi not in parameter vertices
	void reduceChi(VE_SET *vertices);


	// Removes redundant nodes (default compares Chi-Labels, else compares Lambda-Labels)
	void shrink(bool bLambda = false);

	// WF, 29.10.2017 - not supported by new data structures
	// Swaps all chi- and lambda-sets
	// void swapChiLambda();

	// Eliminates hyperedges that are already covered
	void elimCovEdges();

	// Reduces the labellings in the lambda-sets
	void reduceLambda();
	
	// Sets the chi-sets based on the lambda-sets
	void setChi(Hypergraph *HGraph, bool bStrict = true);

	// Sets the lambda-sets based on the chi-sets
	void setLambda(Hypergraph *HGraph);

	// Resets the lambda-set to a possibly smaller set
	void resetLambda(Hypergraph *HGraph);

	// Finds the first node in the hypertree that covers the hyperedge
	Hypertree* findCoverNode(Hyperedge *edge);

	// Finds the first node in the hypertree that has *edge in the Lambda label
	Hypertree* findNodeByLambda(Hyperedge *edge);

	// Checks hypertree condition 1
	Hyperedge *checkCond1(Hypergraph *HGraph);

	// Checks hypertree condition 2
	Vertex *checkCond2(Hypergraph *HGraph);

	// Checks hypertree condition 3
	Hypertree *checkCond3(Hypergraph *HGraph);

	// Checks hypertree condition 4
	Hypertree *checkCond4(Hypergraph *HGraph);
};


#endif // !defined(CLS_HYPERTREE)

