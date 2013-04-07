#include "HexFace.h"

namespace HexMesh {
	
	HexFace::HexFace() {
		cellIndices[0] = cellIndices[1] = -1;
		_acis_face = NULL;
	}
	
	HexFace::HexFace(const HexFace& f) {
		vertexIndices = f.vertexIndices;
		edgeIndices   = f.edgeIndices;
		cellIndices[0] = f.cellIndices[0];
		cellIndices[1] = f.cellIndices[1];
		_acis_face = f._acis_face;
	}
	
	HexFace& HexFace::operator= (const HexFace& f) {
		if (this == &f)
			return *this;
		vertexIndices = f.vertexIndices;
		edgeIndices   = f.edgeIndices;
		cellIndices[0] = f.cellIndices[0];
		cellIndices[1] = f.cellIndices[1];
		_acis_face = f._acis_face;
		
		return *this;
	}

}
