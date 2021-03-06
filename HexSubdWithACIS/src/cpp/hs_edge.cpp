#include "hs_edge.h"

using namespace hex_subdiv;

hs_edge::hs_edge() {
	vert_idx[0] = vert_idx[1] = -1;
	_acis_edge = NULL;
	_type = INNER_EDGE;
}

hs_edge::hs_edge(const hs_edge& e) {
	vert_idx[0] = e.vert_idx[0];
	vert_idx[1] = e.vert_idx[1];
	face_idx = e.face_idx;
	cell_idx = e.cell_idx;
	_acis_edge = e._acis_edge;
	_type = e._type;
}

hs_edge& hs_edge::operator= (const hs_edge& e) {
	if (this == &e)
		return *this;
	vert_idx[0] = e.vert_idx[0];
	vert_idx[1] = e.vert_idx[1];
	face_idx = e.face_idx;
	cell_idx = e.cell_idx;
	_acis_edge = e._acis_edge;
	_type = e._type;
	
	return *this;
}

