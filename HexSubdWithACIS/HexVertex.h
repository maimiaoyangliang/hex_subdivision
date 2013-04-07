#ifndef HEXVERTEX_H_
#define HEXVERTEX_H_


#include "Point3D.h"
#include "HexVarDef.h"

namespace hex_subdiv { 
	
	class hs_vert {
	public:
		hs_vert();
		hs_vert(double, double, double);
		hs_vert(const hs_point&);
		hs_vert(const hs_vert& );
		hs_vert& operator= (const hs_vert&);
		
		virtual ~hs_vert() {}
		
		hs_point coord() const { return coordinate; }
		
		void set_coord(const hs_point& p) { coordinate = p; }

		void add_edge(size_t idx) { edge_idx.insert(idx); }
		void add_face(size_t idx) { face_idx.insert(idx); }
		void add_cell(size_t idx) { cell_idx.insert(idx); }		
		
		int_set_iter first_edge()  { return edge_idx.begin(); }
		int_set_iter first_face()  { return face_idx.begin(); }
		int_set_iter first_cell()  { return cell_idx.begin(); }

		int_set_iter end_edge()  { return edge_idx.end(); }
		int_set_iter end_face()  { return face_idx.end(); }
		int_set_iter end_cell()  { return cell_idx.end(); }

		size_t edge_size()  { return edge_idx.size(); }
		size_t face_size() const { return face_idx.size(); }	
		size_t cell_size() const { return cell_idx.size(); }
		
	private:	
		hs_point	coordinate;             // the coordinate of vertex 
		int_set edge_idx;			// link of edges containing the vertex;
		int_set face_idx;			// link of faces containing the vertex;
		int_set cell_idx;			// link of cells containing the vertex;
	};
	
} // namespace

#endif