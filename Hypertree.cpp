// Hypertree.cpp: implementation of the Hypertree class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Hypergraph.h"
#include "Hyperedge.h"
#include "Vertex.h"
#include "Globals.h"
#include "Hypertree.h"
#include "SetCover.h"

uint G_HTID{ 0 };


//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


void Hypertree::removeCoveredEdges(list<HyperedgeSharedPtr> &edges) const
{
	for (auto edge_it = edges.begin(); edge_it != edges.end(); ) {
		if ((*edge_it)->isCoveredBy(MyChi))
			edge_it = edges.erase(edge_it);
		else
			edge_it++;
	}
    
	for (auto child_it = MyChildren.cbegin(); child_it != MyChildren.cend() && edges.size() > 0; child_it++) 
		(*child_it)->removeCoveredEdges(edges);
}


void Hypertree::labelChiSets(int iLabel)
{
	// Label all nodes in the chi-set
	for(auto v : MyChi)
		if(v->getLabel() >= 0)
			if((v->getLabel() > 0) && (v->getLabel() < iLabel))
				v->setLabel(-1);
			else
				v->setLabel(iLabel);

	// Label all nodes in the chi-sets of the children
	for(auto child : MyChildren)
		child->labelChiSets(iLabel);
}

void Hypertree::collectChiSets(VertexSet & vertices) const
{
	for (auto v : MyChi)
		vertices.insert(v);

	for (auto child : MyChildren)
		child->collectChiSets(vertices);
}


void Hypertree::selCovHTNodes(vector<Hypertree*> &CovNodes, bool bStrict)
{   
	// Add all nodes of hyperedges that occur the first time to the chi-set
	for(auto he : MyLambda)
		if(CovNodes[he->getLabel()] == nullptr)
			CovNodes[he->getLabel()] = this;
		else
			if(!bStrict && (MyLambda.size() < CovNodes[he->getLabel()]->getLambda().size()))
				CovNodes[he->getLabel()] = this;

	// Set the chi-set for all children
	for(auto child : MyChildren)
		child->selCovHTNodes(CovNodes);
}


void Hypertree::setChi_Conn()
{
	int i{ 0 };
	
	MyHg->setVertexLabels();
	// Label all nodes that occur in the chi-sets of at least two subtrees by -1
	for(auto child : MyChildren)
		child->labelChiSets(++i);

	if (!MyParent.expired()) {
		HypertreeSharedPtr parent = MyParent.lock();
		// Label all nodes that occur in the chi-set of the parent and of at least one subtree by -1
		for (auto chi : parent->allChi())
			if (chi->getLabel() > 0)
				chi->setLabel(-1);
	}

	// Put all nodes in the actual chi-set that are labeled with -1
	for(auto v : MyHg->allVertices())
		if(v->getLabel() == -1)
			MyChi.insert(v);
	
	// Set the chi-sets for all children
	for(auto child : MyChildren)
		child->setChi_Conn();
}


void Hypertree::reduceLambdaTopDown()
{
	size_t iNbrOfNodes, iNbrOfHEdges;
	bool bFinalOccurrence;
	VertexSet Nodes;
	HyperedgeSet HEdges, Lambda, CovEdges;
	SetCover SC(MyHg);
	HypertreeSharedPtr parent{ nullptr };

	if (!MyParent.expired())
		parent = MyParent.lock();

	// Compute an upper bound for the memory needed in this method
	iNbrOfNodes = 0;
	for(auto he : MyLambda)
		iNbrOfNodes += he->getNbrOfVertices();
	iNbrOfHEdges = MyLambda.size();
	if(parent != nullptr)
		iNbrOfHEdges += parent->MyLambda.size();

	/*
	Lambda = new CompSet((int)MyLambda.size());
	Nodes = new CompSet(iNbrOfNodes);
	HEdges = new CompSet(iNbrOfHEdges);
	if((Lambda == NULL) || (Nodes == NULL) || (HEdges == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::reduceLambdaTopDown"); */

	// Reset the labels of the nodes in the actual hyperedges
	for(auto he : MyLambda) {
		Lambda.insert(he);
		HEdges.insert(he);
		for(auto v : he->allVertices())
			v->setLabel(0);
	}
	MyLambda.clear();

	// Label all nodes in the actual hyperedges that occur in the hyperedges of at least two children by -1
	int i = 1;
	for (auto child : MyChildren) {
		for (auto he : child->MyLambda)
			for (auto v : he->allVertices())
				if (v->getLabel() >= 0) {
					if ((v->getLabel() > 0) && (v->getLabel() < i))
						v->setLabel(-1);
					else
						v->setLabel(i);
				}
		i++;
	}

	if(parent != nullptr)
		// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
		for(auto he : parent->MyLambda) {
			HEdges.insert(he);
			for(auto v : he->allVertices())
				if(v->getLabel() > 0)
					v->setLabel(-1);
		}

	for(auto he : Lambda) {
		// Check whether the hyperedge does not occur in any child
		bFinalOccurrence = true;
		for(auto child : MyChildren)
			if(child->MyLambda.find(he) != child->MyLambda.end()) {
				bFinalOccurrence = false;
				break;
			}

		// If so, the hyperedge must not be removed
		if(bFinalOccurrence) {
			MyLambda.insert(he);
			for(auto v : he->allVertices())
				v->setLabel(0);
		}
	}

	// Select all nodes in the actual hyperedges that are marked: they must be covered
	for(auto he : Lambda) {
		for(auto v : he->allVertices())
			if(v->getLabel() == -1)
				Nodes.insert(v);
	}

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	// Copy all covering hyperedges into the lambda-set
	MyLambda.insert(CovEdges.begin(), CovEdges.end());

	// If the size of the lambda-set has been increased
	if((int)MyLambda.size() >= Lambda.size()) {
		MyLambda = Lambda;
	}

	// Reduce the lambda-sets of all children
	for(auto child : MyChildren)
		child->reduceLambdaTopDown();
}


void Hypertree::reduceLambdaBottomUp()
{
	size_t iNbrOfNodes, iNbrOfHEdges;
	bool bFinalOccurrence, bChildFound;
	VertexSet Nodes;
	HyperedgeSet HEdges, Lambda, CovEdges;
	SetCover SC(MyHg);
	Hypertree *Child = this;
	HypertreeSharedPtr child_parent{ nullptr };


	resetLabels();
	while(Child != nullptr) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for(auto ch : MyChildren)
				if(ch->getLabel() == 0) {
					bChildFound = true;
					Child = ch.get();
					break;
				}
		} while(bChildFound);

		// Compute an upper bound for the memory needed in this method
		iNbrOfNodes = 0;
		for(auto he : Child->MyLambda)
			iNbrOfNodes += he->getNbrOfVertices();
		iNbrOfHEdges = Child->MyLambda.size();
		for(auto ch : Child->MyChildren)
			iNbrOfHEdges += ch->MyLambda.size();


		// Reset the labels of the nodes in the actual hyperedges
		for(auto he : Child->MyLambda) {
			Lambda.insert(he);
			HEdges.insert(he);
			for(auto v : he->allVertices())
				v->setLabel(0);
		}
		Child->MyLambda.clear();

		// Label all nodes in the actual hyperedges that occur in the hyperedges of at least two children by -1
		int i{ 1 };
		for (auto ch : Child->MyChildren) {
			for (auto he : ch->MyLambda) {
				HEdges.insert(he);
				for (auto v : he->allVertices())
					if (v->getLabel() >= 0) {
						if ((v->getLabel() > 0) && (v->getLabel() < i))
							v->setLabel(-1);
						else
							v->setLabel(i);
					}
			}
			i++;
		}
		
		child_parent = nullptr;
		if (!Child->MyParent.expired())
			child_parent = Child->MyParent.lock();

		if (child_parent != nullptr)
			// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
			for(auto he : child_parent->MyLambda) {
				for(auto v : he->allVertices())
					if(v->getLabel() > 0)
						v->setLabel(-1);
			}

		for(auto he : Lambda) {
			// Check whether the hyperedge does not occur in the parent
			bFinalOccurrence = true;
			if((child_parent != nullptr) && (child_parent->MyLambda.find(he) != child_parent->MyLambda.end()))
				bFinalOccurrence = false;

			// If so, the hyperedge must not be removed
			if(bFinalOccurrence) {
				Child->MyLambda.insert(he);
				for(auto v : he->allVertices())
					v->setLabel(0);
			}
		}

		// Select all nodes in the actual hyperedges that are marked: they must be covered
		for(auto he : Lambda)
			for(auto v : he->allVertices())
				if(v->getLabel() == -1)
					Nodes.insert(v);

		// Apply set covering
		CovEdges = SC.cover(Nodes, HEdges);

		// Copy all covering hyperedges into the lambda-set
		MyLambda.insert(CovEdges.begin(),CovEdges.end());

		
		// If the size of the lambda-set has been increased
		if(Child->MyLambda.size() >= Lambda.size()) {
			Child->MyLambda.clear();
			Child->MyLambda.insert(Lambda.begin(),Lambda.end());
		}

		// Label the node and go up to the parent node
		Child->MyLabel = 1;
		Child = child_parent.get();
	}
}


void Hypertree::writeGMLNodes(ofstream &GMLFile) const
{
	int i;
	vector<int> iOrder;
	HyperedgeSet::const_iterator LambdaIter;
	VertexSet::const_iterator ChiIter;
	vector<VertexSharedPtr> Chi;
	vector<HyperedgeSharedPtr> Lambda;

	iOrder.clear();
	iOrder.resize(MyLambda.size());
	Lambda.resize(MyLambda.size());
	// Copy the lambda-set into an array
	for(i=0, LambdaIter = MyLambda.begin(); LambdaIter != MyLambda.end(); i++, LambdaIter++) {
		Lambda[i] = *LambdaIter;
		iOrder[i] = (*LambdaIter)->getId();
	}
	sortVectors<HyperedgeSharedPtr>(Lambda, iOrder, 0, (int)MyLambda.size()-1);

	GMLFile << "  node [" << endl;
	GMLFile << "    id " << MyLabel << endl;
	GMLFile << "    label \"{";

	// Write lambda-set
	if(MyLambda.size() > 0)
		GMLFile << Lambda[0]->getName();
	for(i=1; i < (int)MyLambda.size(); i++)
		GMLFile << ", " << Lambda[i]->getName();

	GMLFile << "}    {";

	if (MyFec) {
		HyperedgeVector edges = MyFec->getEdges();
		vector<double> weights = MyFec->getWeights();
		HyperedgeVector GammaH(edges.size());
		vector<double> GammaW(edges.size());
		
		// Copy the gamma-set into an array
		HyperedgeVector::iterator edgesIt;
		vector<double>::iterator weightsIt;
		for (i = 0, edgesIt = edges.begin(), weightsIt = weights.begin();
			edgesIt != edges.end(), weightsIt != weights.end();
			i++, edgesIt++, weightsIt++) {

			GammaH[i] = *edgesIt;
			GammaW[i] = *weightsIt;
		}
		// TODO we can delete all of these lines and write directly the vectors
		//sortPointers((void **)GammaH, iOrder, 0, (int)edges->size() - 1);
		//sortPointers((void **)GammaW, iOrder, 0, (int)weights->size() - 1);

		// Write gamma-set
		if (edges.size() > 0) {
			GMLFile << GammaH[0]->getName() << "=" << GammaW[0];
		}
		for (i = 1; i < (int)edges.size(); i++) {
			GMLFile << ", " << GammaH[i]->getName() << "=" << GammaW[i];
		}


		GMLFile << "}    {";

	}

	iOrder.clear();
	iOrder.resize(MyChi.size());
	Chi.resize(MyChi.size());
	// Copy the chi-set into an array
	for(i=0, ChiIter = MyChi.begin(); ChiIter != MyChi.end(); i++, ChiIter++) {
		Chi[i] = *ChiIter;
		iOrder[i] = (*ChiIter)->getId();
	}
	sortVectors<VertexSharedPtr>(Chi, iOrder, 0, (int)MyChi.size()-1);

	// Write chi-set
	if(MyChi.size() > 0)
		GMLFile << Chi[0]->getName();
	for(i=1; i < (int)MyChi.size(); i++)
		GMLFile << ", " << Chi[i]->getName();
	
	GMLFile << "}\"" << endl;
	GMLFile << "    vgj [" << endl;
	GMLFile << "      labelPosition \"in\"" << endl;
	GMLFile << "      shape \"Rectangle\"" << endl;
	GMLFile << "    ]" << endl;
	GMLFile << "  ]" << endl << endl;

	// Write GML nodes for all subtrees
	for(auto ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLNodes(GMLFile);
}


void Hypertree::writeGMLEdges(ofstream &GMLFile) const
{
	HypertreeSharedPtr parent{ nullptr };

	if (!MyParent.expired())
		parent = MyParent.lock();

	if (parent != nullptr) {
		GMLFile << "  edge [" << endl;
		GMLFile << "    source " << parent->MyLabel << endl;
		GMLFile << "    target " << MyLabel << endl;
		GMLFile << "  ]" << endl << endl;
	}

	// Write GML edges for all subtrees
	for(auto ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLEdges(GMLFile);
}


Hypertree::Hypertree(const shared_ptr<Hypergraph> & Hg) : MyHg{ Hg }
{
	MyId = G_HTID++;
}

std::shared_ptr<Hypertree> Hypertree::clone() const
{
	std::shared_ptr<Hypertree> ht = make_shared<Hypertree>(MyHg);

	ht->MyChi = MyChi;
	ht->MyLambda = MyLambda;

	ht->MyPointers = MyPointers;
    ht->MyIDs = MyIDs;
	ht->MyLabel = MyLabel;
	ht->MyCut = MyCut;


	for (auto child : MyChildren) {
		ht->insChild(child->clone());
	}

	return ht;
}

void Hypertree::outputToGML(const string &cNameOfFile) const
{
	ofstream GMLFile;
	
	GMLFile.open(cNameOfFile, ios::out);

	// Check if file opening was successful
	if(!GMLFile.is_open())
		writeErrorMsg("Error opening file.", "Hypertree::outputToGML");

	GMLFile << "graph [" << endl << endl;
	GMLFile << "  directed 0" << endl << endl;

 	writeGMLNodes(GMLFile);  // Write hypertree nodes in GML format
	writeGMLEdges(GMLFile);  // Write hypertree edges in GML format

	GMLFile << "]" << endl;

	GMLFile.close();
}


void Hypertree::insChi(const VertexSharedPtr &Vertex)
{
	MyChi.insert(Vertex);
}


void Hypertree::insLambda(const HyperedgeSharedPtr &Edge)
{
	MyLambda.insert(Edge);
}


void Hypertree::setParent(const weak_ptr<Hypertree> &Parent)
{
	MyParent = Parent;
}


const weak_ptr<Hypertree> Hypertree::getParent() const
{
	return MyParent;
}


void Hypertree::insChild(const std::shared_ptr<Hypertree> &Child, bool bSetParent)
{
	MyChildren.push_back(Child);
	if (bSetParent)
		Child->setParent(shared_from_this());
}


bool Hypertree::remChild(const std::shared_ptr<Hypertree> &Child)
{
	list<std::shared_ptr<Hypertree>>::iterator ChildIter;

	for (ChildIter = MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if ((*ChildIter) == Child) {
			// If the child was found, remove it
			MyChildren.erase(ChildIter);
			return true;
		}

	return false;
}


void Hypertree::remChildren(bool SetParent, const std::shared_ptr<Hypertree> &NewParent)
{
	list<HypertreeSharedPtr>::iterator ChildIter;

	if (SetParent)
		for (ChildIter = MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
			(*ChildIter)->setParent(NewParent);
	MyChildren.clear();
}


void Hypertree::insPointer(void *Ptr)
{
	MyPointers.insert(Ptr);
}


void Hypertree::insID(int iID)
{
	MyIDs.insert(iID);
}


void Hypertree::setCut(bool bCut)
{
	MyCut = bCut;
}


bool Hypertree::isCut() const
{
	return MyCut;
}


HypertreeSharedPtr Hypertree::getCutNode()
{
	HypertreeSharedPtr HTree = nullptr;

	if(MyCut)
		return shared_from_this();

	// Search for cut nodes in the subtrees rooted at the child nodes
	for(auto child : MyChildren)
		if((HTree = child->getCutNode()) != nullptr)
			return HTree;

	return nullptr;
}


Hypertree *Hypertree::getHTNode(void *Ptr)
{
	Hypertree *HTree = nullptr;
	set<void *>::iterator SetIter;

	// Search for Ptr in the actual node
	SetIter = MyPointers.find(Ptr);
	if(SetIter != MyPointers.end())
		return this;  // Ptr was found in the actual node

	// Search for Ptr in the subtrees rooted at the child nodes
	for(auto child : MyChildren)
		if((HTree = child->getHTNode(Ptr)) != nullptr)
			return HTree;

	return nullptr;
}


Hypertree *Hypertree::getHTNode(int iID)
{
	Hypertree *HTree = nullptr;
	set<int>::iterator SetIter;

	// Search for Ptr in the actual node
	SetIter = MyIDs.find(iID);
	if(SetIter != MyIDs.end())
		return this;  // Ptr was found in the actual node

	// Search for Ptr in the subtrees rooted at the child nodes
	for(auto child : MyChildren)
		if((HTree = child->getHTNode(iID)) != nullptr)
			return HTree;

	return nullptr;
}


void Hypertree::setRoot()
{
	HypertreeSharedPtr parent{ nullptr };

	if (!MyParent.expired()) {
		parent = MyParent.lock();

		// Make parent node to the root
		parent->setRoot();

		// Remove actual node from the child set of the parent node
		if (!parent->remChild(shared_from_this()))
			writeErrorMsg("Invalid parent/child relation.", "Hypertree::setRoot");

		// Consider parent node as child of the actual node
		insChild(parent);

		// Make actual node to the root
		MyParent.reset();
	}
}


bool Hypertree::isRoot() const
{
	return MyParent.expired();
}


size_t Hypertree::getHTreeWidth() const
{
	size_t iTmp, iTreeWidth;

	// Set treewidth to the cardinality of the actual lambda-set
	iTreeWidth = MyLambda.size();  
	
	// Compute maximum between treewidths of all subtrees and the actual cardinality
	for(auto child : MyChildren) {
		iTmp = child->getHTreeWidth();
		if(iTmp > iTreeWidth)
			iTreeWidth = iTmp;
	}

	return iTreeWidth;
}


size_t Hypertree::getTreeWidth() const
{
	size_t iTmp, iTreeWidth;

	// Set treewidth to the cardinality of the actual lambda-set
	iTreeWidth = MyChi.size() - 1;  
	
	// Compute maximum between treewidths of all subtrees and the actual cardinality
	for(auto child : MyChildren) {
		iTmp = child->getTreeWidth();
		if(iTmp > iTreeWidth)
			iTreeWidth = iTmp;
	}

	return iTreeWidth;
}


void Hypertree::setLabel(int iLabel)
{
	MyLabel = iLabel;
}


void Hypertree::resetLabels()
{
	MyLabel = 0;

	for(auto child : MyChildren)
		child->resetLabels();
}


int Hypertree::setIDLabels(int iStartID)
{
	MyLabel = iStartID;
	for(auto child : MyChildren)
		iStartID = child->setIDLabels(iStartID + 1);

	return iStartID;
}


void Hypertree::reduceChi(const VertexSet &vertices)
{
	for (auto it = MyChi.begin(); it != MyChi.end(); )
		if (vertices.find(*it) == vertices.end())
			it = MyChi.erase(it);
		else
			it++;
}


void Hypertree::shrinkByLambda()
{
	bool bChildFound;
	HypertreeSharedPtr Child = shared_from_this();
	list<HypertreeSharedPtr>::iterator ChildIter;

	resetLabels();
	while (Child != nullptr) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for (auto ch : Child->MyChildren)
				if (ch->getLabel() == 0) {
					bChildFound = true;
					Child = ch;
					break;
				}
		} while (bChildFound);

		// Remove the actual node if its lambda-set is a subset of the lambda-set of one of its children
		for (ChildIter = Child->MyChildren.begin(); ChildIter != MyChildren.end();)
			if (isSubset<HyperedgeSharedPtr>(Child->MyLambda, (*ChildIter)->MyLambda)) {

				// Move the lambda-set from the child to the actual node
				for (auto he : (*ChildIter)->MyLambda)
					Child->MyLambda.insert(he);

				// Move the chi-set from the child to the actual node
				for (auto v : (*ChildIter)->MyChi)
					Child->MyChi.insert(v);

				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				// Remove the child node (its contents is now stored in the actual node)
				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Remove all child nodes whose lambda-sets are subsets of the lambda-set of the actual node
		for (ChildIter = Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if (isSubset<HyperedgeSharedPtr>((*ChildIter)->MyLambda, Child->MyLambda)) {

				// Move the chi-set from the child to the actual node
				for (auto v : (*ChildIter)->MyChi)
					Child->MyChi.insert(v);
				
				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Label the node and go up to the parent node
		Child->MyLabel = 1;
		if (Child->MyParent.expired())
			Child = nullptr;
		else
			Child = Child->MyParent.lock();
	}
}

void Hypertree::shrinkByChi()
{
	bool bChildFound;
	HypertreeSharedPtr Child = shared_from_this();
	list<HypertreeSharedPtr>::iterator ChildIter, ChildIter2;
	VertexSet::iterator ChiIter;
	HyperedgeSet::iterator LambdaIter;
	set<void *>::iterator PtrIter;  

	resetLabels();
	while (Child != nullptr) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for (auto ch : Child->MyChildren)
				if (ch->getLabel() == 0) {
					bChildFound = true;
					Child = ch;
					break;
				}
		} while (bChildFound);

		// Remove the actual node if its chi-set is a subset of the chi-set of one of its children
		for (ChildIter = Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if (isSubset<VertexSharedPtr>(Child->MyChi, (*ChildIter)->MyChi)) {

				// Move the chi-set from the child to the actual node
				for (auto v : (*ChildIter)->MyChi)
					Child->MyChi.insert(v);

				// Move the lambda-set from the child to the actual node
				Child->MyLambda.clear();
				for (auto he : (*ChildIter)->MyLambda)
					Child->MyLambda.insert(he);

				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				// Move the fractional edge cover from the child to the actual node
				Child->setFec((*ChildIter)->getFec());

				// Remove the child node (its contents is now stored in the actual node)
				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Remove all child nodes whose chi-sets are subsets of the chi-set of the actual node
		for (ChildIter = Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if (isSubset<VertexSharedPtr>((*ChildIter)->MyChi, Child->MyChi)) {

				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Label the node and go up to the parent node
		Child->MyLabel = 1;
		if (Child->MyParent.expired())
			Child = nullptr;
		else
			Child = Child->MyParent.lock();
	}
}


void Hypertree::shrink(bool bLambda)
{
	if (bLambda)
		shrinkByLambda();
	else
		shrinkByChi();
}


/*****
 * WF: 29.10.2017 - not supported by new data structures
 *****
void Hypertree::swapChiLambda()
{
	list<Hypertree *>::iterator ChildIter;
	HE_SET Tmp;
	VE_SET::iterator ChiIter;
	HE_SET::iterator LambdaIter;

	// Swap chi- and lambda set labelling the actual node
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		Tmp.insert(*LambdaIter);
	MyLambda.clear();
	for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++)
		MyLambda.insert((Hyperedge *)(*ChiIter));
	MyChi.clear();
	for(LambdaIter=Tmp.begin(); LambdaIter != Tmp.end(); LambdaIter++)
		MyChi.insert((Node *)(*LambdaIter));
	Tmp.clear();

	// Swap chi- and lambda set for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->swapChiLambda();
}
*/

void Hypertree::elimCovEdges()
{
	VertexSet Nodes(MyChi);
	HyperedgeSet HEdges(MyLambda), CovEdges;
	SetCover SC(MyHg);

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	// Copy the covering hyperedges into the lambda-set
	MyLambda = CovEdges;

	// Eliminate covered hyperedges in all subtrees
	for(auto child : MyChildren)
		child->elimCovEdges();
}


void Hypertree::reduceLambda()
{
	reduceLambdaTopDown();
	reduceLambdaBottomUp();
}


void Hypertree::setChi(bool bStrict)
{
	int i = 0;
	vector<Hypertree*> CovNodes(MyHg->getNbrOfEdges());

	for(auto e : MyHg->allEdges()) {
		e->setLabel(i);
		CovNodes[i++] = nullptr;
	}

	i = 0;
	selCovHTNodes(CovNodes, bStrict);
	for (auto e : MyHg->allEdges()) {
		if (CovNodes[i] != nullptr)
			for (auto v : e->allVertices())
				CovNodes[i]->insChi(v);
		i++;
	}
	
	setChi_Conn();
}


void Hypertree::setLambda()
{
	/*
	VE_SET::iterator ChiIter;
	HE_SET::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter; */
	VertexSet Nodes;
	HyperedgeSet HEdges, CovEdges1, CovEdges2;
	SetCover SC(MyHg);
	

	// Select all nodes in the chi-set and the corresponding hyperedges
	for(auto v : MyChi) {
		Nodes.insert(v);
		for(auto he : MyHg->allVertexNeighbors(v))
			HEdges.insert(he);
	} 

	// Apply set covering
	CovEdges1 = SC.cover(Nodes, HEdges);

	if(!MyLambda.empty()) {
		// Mark all nodes that are already covered by hyperedges in the lambda-set
		MyHg->setVertexLabels();
		for(auto he : MyLambda)
			for(auto v : he->allVertices())
				v->setLabel(1);

		Nodes.clear();
		HEdges.clear();
		// Choose all nodes in the chi-set that are not covered
		for(auto v : MyChi)
			if(v->getLabel() == 0) {
				Nodes.insert(v);
				for (auto he : MyHg->allVertexNeighbors(v))
					HEdges.insert(he);
			}

		// Apply set covering
		CovEdges2 = SC.cover(Nodes, HEdges);

		if(CovEdges2.size() < CovEdges1.size())
			CovEdges1 = CovEdges2;
	}

	// Copy all covering hyperedges into the lambda-set
	for(auto he : CovEdges1)
		MyLambda.insert(he);

	// Set the lambda-sets for all subtrees
	for(auto child : MyChildren)
		child->setLambda();
}


void Hypertree::resetLambda()
{
	VertexSet Nodes;
	HyperedgeSet HEdges, CovEdges;
	SetCover SC(MyHg);

	// Choose all nodes in the chi-set to be covered
	for(auto v : MyChi) {
		Nodes.insert(v);
		for (auto he : MyHg->allVertexNeighbors(v))
			HEdges.insert(he);
	}

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	if(CovEdges.size() < (int)MyLambda.size()) {
		MyLambda.clear();
		// Copy all covering hyperedges into the lambda-set
		MyLambda = CovEdges;
	}

	// Set the lambda-sets for all subtrees
	for(auto child : MyChildren)
		child->setLambda();
}

HypertreeSharedPtr Hypertree::findCoverNode(const HyperedgeSharedPtr &edge)
{
	HypertreeSharedPtr node{ nullptr };

	if (MyLambda.find(edge) != MyLambda.end())
		return shared_from_this();

	for (auto child : MyChildren)
		if ((node = child->findCoverNode(edge)) != nullptr)
			return node;

	return nullptr;
}

HypertreeSharedPtr Hypertree::findNodeByLambda(const HyperedgeSharedPtr &edge)
{
	HypertreeSharedPtr node{ nullptr };
	HyperedgeSet::iterator it = MyLambda.find(edge);

	if (it != MyLambda.end())
		return shared_from_this();

	for (auto it = MyChildren.begin(); it != MyChildren.end() && node == nullptr; it++)
		node = (*it)->findNodeByLambda(edge);

	return node;
}


list<HyperedgeSharedPtr> Hypertree::checkCond1() const
{
	list<HyperedgeSharedPtr> edges;
	
	for (auto e : MyHg->allEdges())
        edges.push_back(e);

	// removes all hyperedges covered by some chi-set
	removeCoveredEdges(edges); 

	return edges;
}

VertexSharedPtr Hypertree::checkCond2(list<VertexSharedPtr> &forbidden) const
{
	VertexSharedPtr Witness;
	list<Hypertree *>::iterator ChildIter;

	// Check "connectedness" for each child and label forbidden nodes
	for (auto child : MyChildren) {
		if ((Witness = child->checkCond2(forbidden)) != nullptr)
			return Witness;
		for (auto v : child->allChi()) {
			if (MyChi.find(v) == MyChi.end())
				forbidden.push_back(v);
		}
	}

	// Check whether there occur forbidden nodes in the actual chi-set
	for (auto v : MyChi)
		if (find(forbidden.begin(),forbidden.end(),v) != forbidden.end())
			return v;

	return nullptr;
} 


std::shared_ptr<const Hypertree> Hypertree::checkCond3() const
{
	VertexSet lambda;

	for (auto he : MyLambda)
		for (auto v : he->allVertices())
			lambda.insert(v);

	//Check if each vertex in MyChi is covered by MyLambda
	for (auto v : MyChi)
		if (lambda.find(v) == lambda.end())
			return shared_from_this();

	//check cond 3 for all children
	for (auto child : MyChildren) {
		auto witness = child->checkCond3();
		if (witness != nullptr)
			return witness;
	}

	return nullptr;
}


std::shared_ptr<const Hypertree> Hypertree::checkCond4() const
{
	VertexSet comp;

	// Get all vertices covered by the subtree rooted at this
	collectChiSets(comp);

	// Remove all vertices of the current chi label
	for (auto &v : MyChi)
		comp.erase(v);

	// Check if some vertex of comp is still in an edge of the lambda label
	for (auto &he : MyLambda)
		for (auto &v : he->allVertices())
			if (comp.find(v) != comp.end())
				return shared_from_this();
	

	// Check fourth condition for all children
	for (auto &child : MyChildren) {
		auto witness = child->checkCond4();
		if (witness != nullptr)
			return shared_from_this();
	}

	return nullptr;
}

bool Hypertree::verify(bool hd, ostream &out)
{
	bool bAllCondSat = true;
	VertexSharedPtr WitnessNode;
	HyperedgeSharedPtr WitnessEdge;
	shared_ptr<const Hypertree> WitnessTree;
	list<HyperedgeSharedPtr> witnessCond1;

	// Check acyclicity of the hypertree
	this->resetLabels();
	//if (HT->isCyclic())
	//	writeErrorMsg("Hypertree contains cycles.", "verify");
	this->setIDLabels();

	// Check condition 1
	out << "Condition 1: ";
	out.flush();
	if ((witnessCond1 = checkCond1()).empty())
		out << "satisfied." << endl;
	else {
		out << "violated! (see atoms \"";
		for (auto &e : witnessCond1)
			cout << e->getName() << " ";
		out << "\")" << endl;
		bAllCondSat = false;
	}

	// Check condition 2
	out << "Condition 2: ";
	cout.flush();
	if ((WitnessNode = checkCond2()) == nullptr)
		out << "satisfied." << endl;
	else {
		out << "violated! (see variable \"" << WitnessNode->getName() << "\")" << endl;
		bAllCondSat = false;
	}

	// Check condition 3
	out << "Condition 3: ";
    out.flush();
	if ((WitnessTree = checkCond3()) == nullptr)
		out << "satisfied." << endl;
	else {
		out << "violated! (see hypertree node \"" << WitnessTree->getLabel() << "\")" << endl;
		bAllCondSat = false;
	}

	if (hd) {
		// Check condition 4
		out << "Condition 4: ";
		out.flush();
		if ((WitnessTree = checkCond4()) == NULL)
			out << "satisfied." << endl;
		else
			out << "violated! (see hypertree node \"" << WitnessTree->getLabel() << "\")" << endl;
		// (see hypertree node \"" << WitnessTree->getLabel() << "\")" << endl;
	}

	return bAllCondSat;
}

