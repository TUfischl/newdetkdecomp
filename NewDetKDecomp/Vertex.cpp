#include "Vertex.h"

int G_VertexID = 0;

Vertex::Vertex(const string & name) : Vertex(++G_VertexID, name)
{
}

Vertex::~Vertex()
{
}
