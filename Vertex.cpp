#include "Vertex.h"

int G_VertexID = 0;

Vertex::Vertex(const string & name) : Vertex(++G_VertexID, name)
{
}

Vertex::~Vertex()
{
}

std::ostream & operator<<(std::ostream & out, const Vertex & v)
{
	out << v.getName();

	return out;
}
