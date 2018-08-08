// Models a hypertree node.
//
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPERTREE)
#define CLS_HYPERTREE

#include "Globals.h"
#include "FractionalEdgeCover.h"

#include <list>
#include <set>

using namespace std;

class Hypergraph;
class Hyperedge;
class Vertex;

class Hypertree: public std::enable_shared_from_this<Hypertree>
{
private:
	// Unique identification of Hypertree
	uint MyId;

	// Pointer to the hypergraph corresponding to this hypertree
	shared_ptr<Hypergraph> MyHg;

	// Pointer to the parent node in the hypertree
	std::weak_ptr<Hypertree> MyParent;

	// Set of pointers to the children in the hypertree
	list<std::shared_ptr<Hypertree>> MyChildren;

	// Set of pointers to nodes representing the chi-set
	VertexSet MyChi;

	// Set of pointers to edges representing the lambda-set
	HyperedgeSet MyLambda;

	// Pointer to a fractional edge cover for the chi-set of this hypertree-node
	unique_ptr<FractionalEdgeCover> MyFec{ nullptr };

	// Set of pointers used for the construction of a hypertree
	set<void *> MyPointers;

	// Set of IDs used for the construction of a hypertree
	set<int> MyIDs;

	// Label of the hypertree node (useful for miscellaneous computations)
	int MyLabel{ 0 };

	// Indicates whether the actual hypertree-branch was cut
	bool MyCut{ false };

	// Removes from edges all hyperedges covered by the chi-set of some node
	void removeCoveredEdges(list<HyperedgeSharedPtr> &edges) const;

	// Labels the variables in the chi-sets of all nodes
	void labelChiSets(int iLabel = 1);

	// Collects all vertices stored in the chi sets of the subtree rooted at this
	void collectChiSets(VertexSet &vertices) const;

	// Selects for each hyperedge a hypertree node to cover its nodes
	void selCovHTNodes(vector<Hypertree*> &CovNodes, bool bStrict = true);

	// Sets the chi-set such that the chi-labellings are connected
	void setChi_Conn();

	// Reduces the labellings in the lambda-sets
	void reduceLambdaTopDown();

	// Reduces the labellings in the lambda-sets
	void reduceLambdaBottomUp();

	// Removes redundant nodes by comparing the lambda sets
	void shrinkByLambda();

	// Removes redundant nodes by comparing the chi sets
	void shrinkByChi();

	// Writes hypertree nodes into a GML file
	void writeGMLNodes(ofstream &GMLFile) const;

	// Writes hypertree edges into a GML file
	void writeGMLEdges(ofstream &GMLFile) const;

	std::shared_ptr<Hypertree> getPtr() {
		return shared_from_this();
	}

public:
	// Constructor
	Hypertree(const shared_ptr<Hypergraph> &Hg);

	// Copy Constructor for hypertreeSharedPtr
	std::shared_ptr<Hypertree> clone() const;
	
	// Writes hypertree to GML format file
	void outputToGML(const string &cNameOfFile) const;
    
	// Inserts a node into the chi-set 
	void insChi(const VertexSharedPtr &Vertex);

	// Inserts an edge into the lambda-set 
	void insLambda(const HyperedgeSharedPtr &Edge);

	// Sets pointer to the parent node
	void setParent(const std::weak_ptr<Hypertree> &Parent);

	// Returns pointer to the parent node
	const std::weak_ptr<Hypertree> getParent() const;

	// Inserts a pointer to a child
	void insChild(const std::shared_ptr<Hypertree> &Child, bool bSetParent = true);

	// Removes a pointer to a child
	bool remChild(const std::shared_ptr<Hypertree> &Child);

	// Removes all pointers to children
	void remChildren(bool SetParent = true, const std::shared_ptr<Hypertree> &NewParent = nullptr);
	
	// Inserts a pointer into the pointer set;
	// these pointers can be used for the construction of hypertrees
	void insPointer(void *Ptr);

	// Inserts an ID into the ID set;
	// these IDs can be used for the construction of hypertrees
	void insID(int iID);

	// Sets a fractional edge cover into this hypertree-node
	void setFec(unique_ptr<FractionalEdgeCover> &fec) {
		MyFec = move(fec);
	}

	unique_ptr<FractionalEdgeCover> &getFec() {
		return MyFec;
	}

	// Sets the cut status
	void setCut(bool bCut = true);

	// Returns true if the actual hypertree-node was cut; otherwise false
	bool isCut() const;

	// Returns a cut tree node within the subtree rooted at
	// the actual tree node
	std::shared_ptr<Hypertree> getCutNode();

	// Returns the tree node within the subtree rooted at the 
	// actual tree node, whose pointer list contains Ptr
	Hypertree *getHTNode(void *Ptr);

	// Returns the tree node within the subtree rooted at the 
	// actual tree node, whose ID list contains iID
	Hypertree *getHTNode(int iID);

	// Sets the actual tree node as root of the hypertree
	void setRoot();

	// Returns true iff the hypertree node does not have a parent
	bool isRoot() const;

	// Returns the hypertreewidth, i.e., the maximum number of elements 
	// in the lambda-set over all nodes in the subtree
	size_t getHTreeWidth() const;

	// Returns the treewidth, i.e., the maximum number of elements 
	// in the chi-set over all nodes in the subtree
	size_t getTreeWidth() const;

	// Returns the chi-set labelling the hypertree-node
	const VertexSet &getChi() const { return MyChi; }

	auto allChi() const -> decltype(make_iterable(MyChi.begin(), MyChi.end())) {
		return make_iterable(MyChi.begin(), MyChi.end());
	}

	// Returns the lambda-set labelling the hypertree-node
	const HyperedgeSet &getLambda() const { return MyLambda; };

	auto allLambda() const -> decltype(make_iterable(MyLambda.begin(), MyLambda.end())) {
		return make_iterable(MyLambda.begin(), MyLambda.end());
	}

	// Returns all children
	auto allChildren() const -> decltype(make_iterable(MyChildren.begin(), MyChildren.end())) {
		return make_iterable(MyChildren.begin(), MyChildren.end());
	}

	// Sets the label of the hypertree-node
	void setLabel(int iLabel);

	// Returns the label of the hypertree-node
	int getLabel() const {
		return MyLabel;
	};

	// Sets labels of all hypertree-nodes in the subtree to zero
	void resetLabels();

	// Sets the labels of all hypertree-nodes in the subtree to a unique ID
	int setIDLabels(int iStartID = 1);

	// Removes vertices from Chi not in parameter vertices
	void reduceChi(const VertexSet &vertices);

	// Removes redundant nodes (default compares Chi-Labels, else compares Lambda-Labels)
	// TODO: Lambda! Shrinking has to be done differently with fractional edge covers
	// 2018-07-30: Chi Shrinking done, has to be tested
	void shrink(bool bLambda = false);

	// WF, 29.10.2017 - not supported by new data structures
	// Swaps all chi- and lambda-sets
	// void swapChiLambda();

	// Eliminates hyperedges that are already covered
	void elimCovEdges();

	// Reduces the labellings in the lambda-sets
	void reduceLambda();
	
	// Sets the chi-sets based on the lambda-sets
	void setChi(bool bStrict = true);

	// Sets the lambda-sets based on the chi-sets
	void setLambda();

	// Resets the lambda-set to a possibly smaller set
	void resetLambda();

	// Finds the first node in the hypertree that covers the hyperedge
	std::shared_ptr<Hypertree> findCoverNode(const HyperedgeSharedPtr &edge);

	// Finds the first node in the hypertree that has *edge in the Lambda label
	std::shared_ptr<Hypertree> findNodeByLambda(const HyperedgeSharedPtr &edge);

	// Checks hypertree condition 1
	list<HyperedgeSharedPtr> checkCond1() const;

	// Checks hypertree condition 2
	VertexSharedPtr checkCond2() const { list<VertexSharedPtr> lst; return checkCond2(lst); }
	VertexSharedPtr checkCond2(list<VertexSharedPtr> &forbidden) const;

	// Checks hypertree condition 3
	std::shared_ptr<const Hypertree> checkCond3() const;

	// Checks hypertree condition 4
	std::shared_ptr<const Hypertree> checkCond4() const;

	bool verify(bool hd = true, ostream &out = cout);
	
};

using HypertreeSharedPtr = std::shared_ptr<Hypertree>;

#endif // !defined(CLS_HYPERTREE)

