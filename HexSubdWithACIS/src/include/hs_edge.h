#ifndef HS_EDGE_H_
#define HS_EDGE_H_

#include "hex_subdiv.h"

class EDGE;

namespace hex_subdiv {

	enum hs_edge_type {INNER_EDGE = 0, ORDINARY_EDGE, CREASE_EDGE};
	typedef hs_edge_type edge_type;
	
	class hs_edge {
	public:
		hs_edge();
		hs_edge(const hs_edge&);
		hs_edge& operator= (const hs_edge&);
		
		virtual ~hs_edge(){}
		
		void add_face(size_t idx) { face_idx.insert(idx); }
		void add_cell(size_t idx) { cell_idx.insert(idx); }
		
		void set_acis_edge(EDGE* acis_edge) { _acis_edge = acis_edge; }
		void set_start_vert(size_t v) { vert_idx[0] = v; }
		void set_end_vert(size_t v) { vert_idx[1] = v; }
		void set_type(edge_type type) { _type = type; }
		
		size_t start_vert() const { return vert_idx[0]; }
		size_t end_vert() const { return vert_idx[1]; }
		
		int_set_iter first_face()  { return face_idx.begin(); }
		int_set_iter first_cell()  { return cell_idx.begin(); }
		
		int_set_iter end_face()  { return face_idx.end(); }
		int_set_iter end_cell()  { return cell_idx.end(); }
		
		size_t face_size() const { return face_idx.size(); }
		size_t cell_size() const { return cell_idx.size(); }
		
		const EDGE* acis_edge() const { return _acis_edge; }		
		edge_type type() const { return _type; }
		
		
	private:
		int_set face_idx;
		int_set cell_idx;
		size_t  vert_idx[2];
		EDGE*   _acis_edge;
		edge_type _type;
		
	};
	
} // namespace

#endif