#include "HexEdge.h"

namespace HexMesh {

	HexEdge::HexEdge() {
		vertexIndices[0] = vertexIndices[1] = -1;
		_acis_edge = NULL;
	}
	
	HexEdge::HexEdge(const HexEdge& e) {
		vertexIndices[0] = e.vertexIndices[0];
		vertexIndices[1] = e.vertexIndices[1];
		faceIndices = e.faceIndices;
		cellIndices = e.cellIndices;
		_acis_edge = e._acis_edge;
	}
	
	HexEdge& HexEdge::operator= (const HexEdge& e) {
		if (this == &e)
			return *this;
		vertexIndices[0] = e.vertexIndices[0];
		vertexIndices[1] = e.vertexIndices[1];
		faceIndices = e.faceIndices;
		cellIndices = e.cellIndices;
		_acis_edge = e._acis_edge;
		
		return *this;
	}

}
