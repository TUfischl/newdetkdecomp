#include "HingeDecomp.h"
#include "Hypergraph.h"
#include "Hypertree.h"
#include "Hingetree.h"
#include "Vertex.h"
#include "DetKDecomp.h"

HingeDecomp::~HingeDecomp()
{
}

Hingetree * HingeDecomp::buildHingetree()
{
	HE_VEC edges;

	// Order hyperedges heuristically
	edges = MyHg->getMCSOrder();


	MyHinge = new Hingetree(edges);

	decomp();
	
	return MyHinge;
}

Hypertree * HingeDecomp::buildHypertree(Hingetree *hinge)
{
	Hypertree *htree{ nullptr };

	//Build a hypergraph from current hinge
	Hypergraph hg_hinge;
	hg_hinge.setParent(MyHg);
	for (auto he : hinge->allEdges())
		hg_hinge.insertEdge(he);

	DetKDecomp *decomp = new DetKDecomp(&hg_hinge, MyK, true);

	htree = decomp->buildHypertree();

	delete decomp;

	// Expand pruned hypertree nodes
	if (htree != nullptr) {
		//Build hypertrees for child nodes
		for (list<Hingeedge*>::const_iterator it = hinge->getChildren().begin(); it != hinge->getChildren().end() && htree != nullptr; it++) {
			Hypertree *child = buildHypertree((*it)->dest);
			if (child != nullptr) {
				//Insert child into HTree
				Hypertree *childRoot = child->findCoverNode((*it)->label);
				Hypertree *childParent = child->findCoverNode((*it)->label);
				childRoot->setRoot();
				childParent->insChild(childRoot);
			}
			else {
				delete htree;
				htree = nullptr;
			}
		}
	}

	return htree;
}

void HingeDecomp::decomp(Hingetree ** hinge)
{
	Hyperedge* e{ nullptr };
	vector<VE_VEC*> connectors;
	vector<HE_VEC*> partitions;
	//HE_VEC *edges;
	int parts;

	while (e = (*hinge)->selectUnused(MyUsed)) {
		MyHg->resetEdgeLabels(-2);
		MyHg->resetVertexLabels();

		for (auto edge : (*hinge)->allEdges()) {
			if (edge == e)
				edge->setLabel(-1);
			else
				edge->setLabel(0);
		}

		for (auto v : e->allVertices())
			v->setLabel(-1);

		parts = separate((*hinge)->getEdges(), partitions, connectors);

		if (parts > 1) {
			vector<Hingetree*> trees;

			for (auto part : partitions) 
				trees.push_back(new Hingetree(*part));

			int rootPart;
			Hingetree* rootHinge;
			Hingeedge* parent = (*hinge)->getParent();
			//Now connect the Hinges to the tree, start with finding a root
			if (parent != nullptr)
				rootPart = parent->label->getLabel() - 1;
			else
				rootPart = 0;
			rootHinge = trees[rootPart];
			rootHinge->setParent(parent);

			for (int i = 0; i < parts; i++) {
				for (list<Hingeedge*>::iterator it = (*hinge)->getChildren().begin(); it != (*hinge)->getChildren().end(); it++)
					if ((*it)->label->getLabel() == i + 1) {
						trees[i]->getChildren().push_back(*it);
						Hingeedge* p = new Hingeedge;
						p->label = (*it)->label;
						p->dest = trees[i];
						delete (*it)->dest->getParent();
						(*it)->dest->setParent(p);
					}


				if (i != rootPart) {
					Hingeedge* he = new Hingeedge;
					Hingeedge* pe = new Hingeedge;

					he->dest = trees[i];
					pe->dest = rootHinge;
					pe->label = he->label = e;
					trees[i]->setParent(pe);
					rootHinge->getChildren().push_back(he);
				}
			}
			
			delete (*hinge);
			(*hinge) = rootHinge;
		}

		for (int i = 0; i < parts; i++) {
			delete partitions[i];
			delete connectors[i];
		}
		partitions.clear();
		connectors.clear();
		
		MyUsed.insert(e);
	}

	(*hinge)->setMinimal();

	for (list<Hingeedge*>::iterator it = (*hinge)->getChildren().begin(); it != (*hinge)->getChildren().end(); it++)
		decomp(&((*it)->dest));
	
}
