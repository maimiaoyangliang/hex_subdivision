#include "HexCell.h"

namespace HexMesh {
	
	HexCell::HexCell(const HexCell& c) 
		: vertexIndices(c.vertexIndices)
		, edgeIndices(c.edgeIndices)
		, faceIndices(c.faceIndices) {}
	
	HexCell& HexCell::operator= (const HexCell& c) {
		if (this == &c)
			return *this;
		
		vertexIndices = c.vertexIndices;
		edgeIndices   = c.edgeIndices;
		faceIndices   = c.faceIndices;
		
		return *this;
	}

}
