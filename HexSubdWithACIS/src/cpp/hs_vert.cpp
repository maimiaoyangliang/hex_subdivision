#include "hs_vert.h"

namespace hex_subdiv {
	
	hs_vert::hs_vert() : coordinate(), _type(INNER_VERT) {}
	
	hs_vert::hs_vert(const hs_point& p, vert_type type) : coordinate(p), _type(type) {}
	
	hs_vert::hs_vert(double x, double y, double z, vert_type type) : coordinate(x,y,z), _type(type) {}
	
	hs_vert::hs_vert(const hs_vert& v)
		: coordinate(v.coordinate)
		, edge_idx(v.edge_idx)
		, face_idx(v.face_idx)
		, cell_idx(v.cell_idx)
		, _type(v._type) {}
	
	hs_vert& hs_vert::operator= (const hs_vert& v) {
		if (this == &v)
			return (*this);
		
		coordinate  = v.coordinate;
		edge_idx = v.edge_idx;
		face_idx = v.face_idx;
		cell_idx = v.cell_idx;
		_type = v._type;

		return (*this);
	}
	
} // namespace