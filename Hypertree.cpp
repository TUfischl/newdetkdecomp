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



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Hypertree::Hypertree(Hypergraph *hg) : MyHg{ hg }
{
}


Hypertree::~Hypertree()
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		delete *ChildIter;
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


void Hypertree::labelCovEdges(Hypergraph *HGraph)
{
	bool bCovered;

	// Label all nodes in the chi-set
	for(auto v : MyChi)
		v->setLabel(1);

	// Label all hyperedges covered by the chi-set
	for(auto he : HGraph->allEdges()) {
		bCovered = true;
		for(auto v : he->allVertices())
			if(v->getLabel() == 0) {
				bCovered = false;
				break;
			}
		if(bCovered)
			he->setLabel(1);
	}

	// Reset node labels in the chi-set
	for (auto v : MyChi)
		v->setLabel(0);

	// Label hyperedges covered by the children
	for(auto child : MyChildren)
		child->labelCovEdges(HGraph);
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


void Hypertree::selCovHTNodes(vector<Hypertree *> &CovNodes, bool bStrict)
{   
	// Add all nodes of hyperedges that occur the first time to the chi-set
	for(auto he : MyLambda)
		if(CovNodes[he->getLabel()] == nullptr)
			CovNodes[he->getLabel()] = this;
		else
			if(!bStrict && (MyLambda.size() < CovNodes[he->getLabel()]->getLambda()->size()))
				CovNodes[he->getLabel()] = this;

	// Set the chi-set for all children
	for(auto child : MyChildren)
		child->selCovHTNodes(CovNodes);
}


void Hypertree::setChi_Conn(Hypergraph *HGraph)
{
	int i{ 0 };

	HGraph->resetVertexLabels();
	// Label all nodes that occur in the chi-sets of at least two subtrees by -1
	for(auto child : MyChildren)
		child->labelChiSets(++i);

	if(MyParent != nullptr)
		// Label all nodes that occur in the chi-set of the parent and of at least one subtree by -1
		for(auto chi : MyParent->allChi())
			if(chi->getLabel() > 0)
				chi->setLabel(-1);

	// Put all nodes in the actual chi-set that are labeled with -1
	for(auto v : HGraph->allVertices())
		if(v->getLabel() == -1)
			MyChi.insert(v);
	
	// Set the chi-sets for all children
	for(auto child : MyChildren)
		child->setChi_Conn(HGraph);
}


bool Hypertree::isCyclic_pvt()
{
	if(iMyLabel != 0)
		return true;
	iMyLabel = 1;

	// Check cyclicity for all children
	for(auto child : MyChildren)
		if(child->isCyclic())
			return true;

	return false;
}


Vertex *Hypertree::checkCond2()
{
	Vertex *Witness;
	VE_SET::iterator NodeIter1, NodeIter2;
	list<Hypertree *>::iterator ChildIter;

	// Check "connectedness" for each child and label forbidden nodes
	for(auto child : MyChildren) {
		if((Witness = child->checkCond2()) != nullptr)
			return Witness;
		for(auto v : child->allChi()) {
			if(MyChi.find(v) == MyChi.end())
				v->setLabel(1);
		}
	}

	// Check whether there occur forbidden nodes in the actual chi-set
	for(auto v : MyChi)
		if(v->getLabel() != 0)
			return v;

	return nullptr;
}


Hypertree *Hypertree::checkCond3()
{
	Hypertree *Witness;

	// Label all variables of hyperedges in the lambda-set
	for(auto he : MyLambda)
		for(auto v : he->allVertices())
			v->setLabel(1);

	// Check whether there is some variable in the chi-set that is not labeled
	for(auto v : MyChi)
		if(v->getLabel() == 0)
			return this;

	// Reset all labels of variables of hyperedges in the lambda-set
	for(auto he : MyLambda)
		for(auto v : he->allVertices())
			v->setLabel(0);

	// Check third condition for all children
	for(auto child : MyChildren)
		if((Witness = child->checkCond3()) != nullptr)
			return Witness;

	return nullptr;
}


void Hypertree::reduceLambdaTopDown()
{
	size_t iNbrOfNodes, iNbrOfHEdges;
	bool bFinalOccurrence;
	VE_SET Nodes;
	HE_SET HEdges, Lambda, CovEdges;
	SetCover SC(MyHg);

	// Compute an upper bound for the memory needed in this method
	iNbrOfNodes = 0;
	for(auto he : MyLambda)
		iNbrOfNodes += he->getNbrOfVertices();
	iNbrOfHEdges = MyLambda.size();
	if(MyParent != nullptr)
		iNbrOfHEdges += MyParent->MyLambda.size();

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

	if(MyParent != nullptr)
		// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
		for(auto he : MyParent->MyLambda) {
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
	VE_SET Nodes;
	HE_SET HEdges, Lambda, CovEdges;
	SetCover SC(MyHg);
	Hypertree *Child = this;

	resetLabels();
	while(Child != nullptr) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for(auto ch : MyChildren)
				if(ch->getLabel() == 0) {
					bChildFound = true;
					Child = ch;
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

		if(Child->MyParent != nullptr)
			// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
			for(auto he : Child->MyParent->MyLambda) {
				for(auto v : he->allVertices())
					if(v->getLabel() > 0)
						v->setLabel(-1);
			}

		for(auto he : Lambda) {
			// Check whether the hyperedge does not occur in the parent
			bFinalOccurrence = true;
			if((Child->MyParent != nullptr) && (Child->MyParent->MyLambda.find(he) != Child->MyParent->MyLambda.end()))
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
		Child->iMyLabel = 1;
		Child = Child->MyParent;
	}
}


void Hypertree::writeGMLNodes(ofstream &GMLFile)
{
	int i, *iOrder;
	HE_SET::iterator LambdaIter;
	VE_SET::iterator ChiIter;
	list<Hypertree *>::iterator ChildIter;
	Vertex **Chi;
	Hyperedge **Lambda;

	Lambda = new Hyperedge*[MyLambda.size()];
	iOrder = new int[MyLambda.size()];
	if((Lambda == NULL) || (iOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::writeGMLNodes");
	// Copy the lambda-set into an array
	for(i=0, LambdaIter = MyLambda.begin(); LambdaIter != MyLambda.end(); i++, LambdaIter++) {
		Lambda[i] = *LambdaIter;
		iOrder[i] = (*LambdaIter)->getLabel();
	}
	sortPointers((void **)Lambda, iOrder, 0, (int)MyLambda.size()-1);

	GMLFile << "  node [" << endl;
	GMLFile << "    id " << iMyLabel << endl;
	GMLFile << "    label \"{";

	// Write lambda-set
	if(MyLambda.size() > 0)
		GMLFile << Lambda[0]->getName();
	for(i=1; i < (int)MyLambda.size(); i++)
		GMLFile << ", " << Lambda[i]->getName();

	delete [] Lambda;
	delete [] iOrder;

	GMLFile << "}    {";

	Chi = new Vertex*[MyChi.size()];
	iOrder = new int[MyChi.size()];
	if((Chi == NULL) || (iOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::writeGMLNodes");
	// Copy the chi-set into an array
	for(i=0, ChiIter = MyChi.begin(); ChiIter != MyChi.end(); i++, ChiIter++) {
		Chi[i] = *ChiIter;
		iOrder[i] = (*ChiIter)->getLabel();
	}
	sortPointers((void **)Chi, iOrder, 0, (int)MyChi.size()-1);

	// Write chi-set
	if(MyChi.size() > 0)
		GMLFile << Chi[0]->getName();
	for(i=1; i < (int)MyChi.size(); i++)
		GMLFile << ", " << Chi[i]->getName();

	delete [] Chi;
	delete [] iOrder;
	
	GMLFile << "}\"" << endl;
	GMLFile << "    vgj [" << endl;
	GMLFile << "      labelPosition \"in\"" << endl;
	GMLFile << "      shape \"Rectangle\"" << endl;
	GMLFile << "    ]" << endl;
	GMLFile << "  ]" << endl << endl;

	// Write GML nodes for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLNodes(GMLFile);
}


void Hypertree::writeGMLEdges(ofstream &GMLFile)
{
	list<Hypertree *>::iterator ChildIter;

	if(MyParent != NULL) {
		GMLFile << "  edge [" << endl;
		GMLFile << "    source " << MyParent->getLabel() << endl;
		GMLFile << "    target " << iMyLabel << endl;
		GMLFile << "  ]" << endl << endl;
	}

	// Write GML edges for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLEdges(GMLFile);
}


void Hypertree::outputToGML(Hypergraph *HGraph, char *cNameOfFile)
{
	int i;
	ofstream GMLFile;
	
	// Set labels to write the output in a uniform order
	setIDLabels();
	for(i=0; i < HGraph->getNbrOfVertices(); i++)
		HGraph->getVertex(i)->setLabel(i);
	for(i=0; i < HGraph->getNbrOfEdges(); i++)
		HGraph->getEdge(i)->setLabel(i);

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


void Hypertree::insChi(Vertex *Vertex)
{
	MyChi.insert(Vertex);
}


void Hypertree::insLambda(Hyperedge *Edge)
{
	MyLambda.insert(Edge);
}


void Hypertree::setParent(Hypertree *Parent)
{
	MyParent = Parent;
}


Hypertree *Hypertree::getParent()
{
	return MyParent;
}


void Hypertree::insChild(Hypertree *Child, bool bSetParent)
{
	MyChildren.push_back(Child);
	if(bSetParent)
		Child->setParent(this);
}


bool Hypertree::remChild(Hypertree *Child)
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((*ChildIter) == Child)
			break;
	if(ChildIter != MyChildren.end()) {
		// If the child was found, remove it
		MyChildren.erase(ChildIter);
		return true;
	}

	return false;
}


void Hypertree::remChildren()
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->setParent(nullptr);
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
	bMyCut = bCut;
}


bool Hypertree::isCut()
{
	return bMyCut;
}


Hypertree *Hypertree::getCutNode()
{
	Hypertree *HTree = nullptr;

	if(bMyCut)
		return this;

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
	if(MyParent != nullptr) {
		// Make parent node to the root
		MyParent->setRoot();

		// Remove actual node from the child set of the parent node
		if(!MyParent->remChild(this))
			writeErrorMsg("Invalid parent/child relation.", "Hypertree::setRoot");

		// Consider parent node as child of the actual node
		insChild(MyParent);

		// Make actual node to the root
		MyParent = nullptr;
	}
}


bool Hypertree::isRoot()
{
	return MyParent == nullptr;
}


size_t Hypertree::getHTreeWidth()
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


size_t Hypertree::getTreeWidth()
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


VE_SET *Hypertree::getChi()
{
	return &MyChi;
}


HE_SET *Hypertree::getLambda()
{
	return &MyLambda;
}


void Hypertree::setLabel(int iLabel)
{
	iMyLabel = iLabel;
}


int Hypertree::getLabel()
{
	return iMyLabel;
}


void Hypertree::resetLabels()
{
	iMyLabel = 0;

	for(auto child : MyChildren)
		child->resetLabels();
}


int Hypertree::setIDLabels(int iStartID)
{
	list<Hypertree *>::iterator ChildIter;

	iMyLabel = iStartID;
	for(auto child : MyChildren)
		iStartID = child->setIDLabels(iStartID + 1);

	return iStartID;
}


bool Hypertree::isCyclic()
{
	resetLabels();
	return isCyclic_pvt();
}


void Hypertree::shrinkByLambda()
{
	bool bChildFound;
	Hypertree *Child = this;
	list<Hypertree *>::iterator ChildIter;

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
			if (isSubset(Child->MyLambda, (*ChildIter)->MyLambda)) {

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
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Remove all child nodes whose lambda-sets are subsets of the lambda-set of the actual node
		for (ChildIter = Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if (isSubset((*ChildIter)->MyLambda, Child->MyLambda)) {

				// Move the chi-set from the child to the actual node
				for (auto v : (*ChildIter)->MyChi)
					Child->MyChi.insert(v);
				
				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Label the node and go up to the parent node
		Child->iMyLabel = 1;
		Child = Child->MyParent;
	}
}

void Hypertree::shrinkByChi()
{
	bool bChildFound;
	Hypertree *Child = this;
	list<Hypertree *>::iterator ChildIter, ChildIter2;
	VE_SET::iterator ChiIter;
	HE_SET::iterator LambdaIter;
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
			if (isSubset(Child->MyChi, (*ChildIter)->MyChi)) {

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

				// Remove the child node (its contents is now stored in the actual node)
				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Remove all child nodes whose chi-sets are subsets of the chi-set of the actual node
		for (ChildIter = Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if (isSubset((*ChildIter)->MyChi, Child->MyChi)) {

				// Move the pointers from the child to the actual node
				for (auto ptr : (*ChildIter)->MyPointers)
					Child->MyPointers.insert(ptr);

				for (auto ch : (*ChildIter)->MyChildren)
					Child->insChild(ch);
				(*ChildIter)->MyChildren.clear();
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Label the node and go up to the parent node
		Child->iMyLabel = 1;
		Child = Child->MyParent;
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
	VE_SET Nodes(MyChi);
	HE_SET HEdges(MyLambda), CovEdges;
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


void Hypertree::setChi(Hypergraph *HGraph, bool bStrict)
{
	int i;
	vector<Hypertree *> CovNodes(HGraph->getNbrOfEdges());

	for(i=0; i < HGraph->getNbrOfEdges(); i++) {
		HGraph->getEdge(i)->setLabel(i);
		CovNodes[i] = nullptr;
	}

	selCovHTNodes(CovNodes, bStrict);
	for(i=0; i < HGraph->getNbrOfEdges(); i++)
		if(CovNodes[i] != nullptr)
			for(auto v : HGraph->getEdge(i)->allVertices())
				CovNodes[i]->getChi()->insert(v);
	
	setChi_Conn(HGraph);
}


void Hypertree::setLambda(Hypergraph *HGraph)
{
	/*
	VE_SET::iterator ChiIter;
	HE_SET::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter; */
	VE_SET Nodes;
	HE_SET HEdges, CovEdges1, CovEdges2;
	SetCover SC(MyHg);
	

	// Select all nodes in the chi-set and the corresponding hyperedges
	for(auto v : MyChi) {
		Nodes.insert(v);
		for(auto he : HGraph->allVertexNeighbors(v))
			HEdges.insert(he);
	} 

	// Apply set covering
	CovEdges1 = SC.cover(Nodes, HEdges);

	if(!MyLambda.empty()) {
		// Mark all nodes that are already covered by hyperedges in the lambda-set
		HGraph->resetVertexLabels();
		for(auto he : MyLambda)
			for(auto v : he->allVertices())
				v->setLabel(1);

		Nodes.clear();
		HEdges.clear();
		// Choose all nodes in the chi-set that are not covered
		for(auto v : MyChi)
			if(v->getLabel() == 0) {
				Nodes.insert(v);
				for (auto he : HGraph->allVertexNeighbors(v))
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
		child->setLambda(HGraph);
}


void Hypertree::resetLambda(Hypergraph *HGraph)
{
	VE_SET Nodes;
	HE_SET HEdges, CovEdges;
	SetCover SC(HGraph);

	// Choose all nodes in the chi-set to be covered
	for(auto v : MyChi) {
		Nodes.insert(v);
		for (auto he : HGraph->allVertexNeighbors(v))
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
		child->setLambda(HGraph);
}

Hypertree * Hypertree::findCoverNode(Hyperedge * edge)
{
	Hypertree *node{ nullptr };

	if (MyLambda.find(edge) != MyLambda.end())
		return this;

	for (auto child : MyChildren)
		if ((node = child->findCoverNode(edge)) != nullptr)
			return node;

	return nullptr;
}


Hyperedge *Hypertree::checkCond1(Hypergraph *HGraph)
{
	HGraph->resetEdgeLabels();
	HGraph->resetVertexLabels();

	// Label all hyperedges covered by some chi-set
	labelCovEdges(HGraph);

	// Search for hyperedges that are not labeled
	for(int i=0; i < HGraph->getNbrOfEdges(); i++)
		if(HGraph->getEdge(i)->getLabel() == 0)
			return HGraph->getEdge(i);

	return nullptr;
}


Vertex *Hypertree::checkCond2(Hypergraph *HGraph)
{
	HGraph->resetVertexLabels();
	return checkCond2();
}


Hypertree *Hypertree::checkCond3(Hypergraph *HGraph)
{
	HGraph->resetVertexLabels();
	return checkCond3();
}


Hypertree *Hypertree::checkCond4(Hypergraph *HGraph)
{
	Hypertree *Witness;

	// Label all variables occurring in some chi-set of the subtree
	HGraph->resetVertexLabels();
	labelChiSets();

	// Compute the intersection with the variables of hyperedges in the lambda-set
	for(auto he : MyLambda)
		for(auto v : he->allVertices())
			if(v->getLabel() != 0)
				v->setLabel(2);

	// Compute the set difference between the intersection and the chi-set
	for(auto v : MyChi)
		v->decLabel();
	
	// Check whether the set difference is empty
	for(auto v : HGraph->allVertices())
		if(v->getLabel() > 1)
			return this;

	// Check fourth condition for all children
	for(auto child : MyChildren)
		if((Witness = child->checkCond4(HGraph)) != nullptr)
			return Witness;

	return nullptr;
}

