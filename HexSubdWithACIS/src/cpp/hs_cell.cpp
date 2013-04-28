#include "hs_cell.h"

using namespace hex_subdiv;

hs_cell::hs_cell(const hs_cell& c) 
: vert_idx(c.vert_idx)
, edge_idx(c.edge_idx)
, face_idx(c.face_idx) {}

hs_cell& hs_cell::operator= (const hs_cell& c) {
	if (this == &c)
		return *this;
	
	vert_idx = c.vert_idx;
	edge_idx = c.edge_idx;
	face_idx = c.face_idx;
	
	return *this;
}
