#ifndef HEXFACE_H_
#define HEXFACE_H_

#include "HexVarDef.h"

class FACE;

namespace hex_subdiv {
	

	
	class hs_face {
	public:
		hs_face();
		hs_face(const hs_face&);
		hs_face& operator= (const hs_face&);
		
		virtual ~hs_face() {}
		
		void set_acis_face(FACE* acis_face) { _acis_face = acis_face; }
		void set_cell(size_t in, size_t out) { cell_idx[0] = in; cell_idx[1] = out; }

		void add_edge(size_t ind) { edge_idx.insert(ind); }
		void add_vert(size_t ind) { vert_idx.insert(ind); }
		
		int_set_iter first_vert()  { return vert_idx.begin(); }
		int_set_iter end_vert()  { return vert_idx.end(); }
		size_t vert_size() const { return vert_idx.size(); }
		
		int_set_iter first_edge()  { return edge_idx.begin(); }
		int_set_iter end_edge()  { return edge_idx.end(); }
		size_t edge_size() const { return edge_idx.size(); }
		
		void set_fst_cell(size_t id) { cell_idx[0] = id; }
		void set_snd_cell(size_t id) { cell_idx[1] = id; }
		int fst_cell()  { return cell_idx[0]; }
		int snd_cell()  { return cell_idx[1]; }
		const FACE* acis_face() const { return _acis_face; }

		void clear_vert() { vert_idx.clear(); }
		void clear_edge() { edge_idx.clear(); }
		void clear_all() { clear_vert(); clear_edge(); cell_idx[0] = cell_idx[1] = -1; _acis_face = NULL; }
		
	private:
		int_set vert_idx;
		int_set edge_idx;
		int		cell_idx[2];
		FACE*   _acis_face;
	};

}

#endif