#include "HexVertex.h"

namespace hex_subdiv {
	
	hs_vert::hs_vert() : coordinate() {}
	
	hs_vert::hs_vert(const hs_point& p) : coordinate(p) {}
	
	hs_vert::hs_vert(double x, double y, double z) : coordinate(x,y,z) {}
	
	hs_vert::hs_vert(const hs_vert& v)
		: coordinate(v.coordinate)
		, edge_idx(v.edge_idx)
		, face_idx(v.face_idx)
		, cell_idx(v.cell_idx) {}
	
	hs_vert& hs_vert::operator= (const hs_vert& v) {
		if (this == &v)
			return (*this);
		
		coordinate  = v.coordinate;
		edge_idx = v.edge_idx;
		face_idx = v.face_idx;
		cell_idx = v.cell_idx;
		return (*this);
	}
	
} // namespace