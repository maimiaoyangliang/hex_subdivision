#include "hs_face.h"

#include <algorithm>
#include <ITERATOR>
#include <CASSERT>


using namespace hex_subdiv;

hs_face::hs_face() {
	cell_idx[0] = cell_idx[1] = -1;
	_acis_face = NULL;
	_type = BORDER_FACE;
}

hs_face::hs_face(const hs_face& f) {
	vert_idx = f.vert_idx;
	edge_idx   = f.edge_idx;
	cell_idx[0] = f.cell_idx[0];
	cell_idx[1] = f.cell_idx[1];
	_acis_face = f._acis_face;
	_type = f._type;
}

hs_face& hs_face::operator= (const hs_face& f) {
	if (this == &f)
		return *this;
	vert_idx = f.vert_idx;
	edge_idx   = f.edge_idx;
	cell_idx[0] = f.cell_idx[0];
	cell_idx[1] = f.cell_idx[1];
	_acis_face = f._acis_face;
	_type = f._type;
	
	return *this;
}

