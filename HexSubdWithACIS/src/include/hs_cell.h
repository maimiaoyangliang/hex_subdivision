#ifndef HS_CELL_H_
#define HS_CELL_H_

#include "hex_subdiv.h"

namespace hex_subdiv {
	
	class hs_cell {
	public:
		hs_cell() {}
		hs_cell(const hs_cell&);
		hs_cell& operator= (const hs_cell&);
		
		virtual ~hs_cell() {}
		
		void add_vert(size_t idx) { vert_idx.insert(idx); }
		void add_edge(size_t idx) { edge_idx.insert(idx); }
		void add_face(size_t idx) { face_idx.insert(idx); }
		
		int_set_citer first_vert() const { return vert_idx.begin(); }
		int_set_citer first_face() const { return face_idx.begin(); }
		int_set_iter first_vert()  { return vert_idx.begin(); }
		int_set_iter first_edge()  { return edge_idx.begin(); }
		int_set_iter first_face()  { return face_idx.begin(); }
		
		int_set_citer end_vert() const { return vert_idx.end(); }
		int_set_citer end_face() const { return face_idx.end(); }
		int_set_iter end_vert()  { return vert_idx.end(); }
		int_set_iter end_edge()  { return edge_idx.end(); }
		int_set_iter end_face()  { return face_idx.end(); }
		
		size_t vert_size() const { return vert_idx.size(); }
		size_t edge_size() const { return edge_idx.size(); }
		size_t face_size() const { return face_idx.size(); }
		
		void clear_all() { vert_idx.clear(); edge_idx.clear(); face_idx.clear(); }
				
	private:
		int_set vert_idx;
		int_set edge_idx;
		int_set face_idx;
	};
	
} // end namespace

#endif