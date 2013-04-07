#include "HexVertex.h"

namespace HexMesh {
	
	HexVertex::HexVertex() : coordinate() {}
	
	HexVertex::HexVertex(const Point3D& p) : coordinate(p) {}
	
	HexVertex::HexVertex(double x, double y, double z) : coordinate(x,y,z) {}
	
	HexVertex::HexVertex(const HexVertex& v)
		: coordinate(v.coordinate)
		, edgeIndices(v.edgeIndices)
		, faceIndices(v.faceIndices)
		, cellIndices(v.cellIndices) {}
	
	HexVertex& HexVertex::operator= (const HexVertex& v) {
		if (this == &v)
			return (*this);
		
		coordinate  = v.coordinate;
		edgeIndices = v.edgeIndices;
		faceIndices = v.faceIndices;
		cellIndices = v.cellIndices;
		return (*this);
	}
	
} // namespace