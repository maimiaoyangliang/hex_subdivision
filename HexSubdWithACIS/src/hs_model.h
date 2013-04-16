#ifndef HS_MODEL_H_
#define HS_MODEL_H_

#include <VECTOR>
#include <CASSERT>

#include "hs_vert.h"
#include "hs_edge.h"
#include "hs_face.h"
#include "hs_cell.h"

class FACE;
class EDGE;
class ENTITY_LIST;

namespace hex_subdiv {
	
	class hs_point;
	
	typedef std::vector<hs_vert> vert_vector;
	typedef std::vector<hs_edge> edge_vector;
	typedef std::vector<hs_face> face_vector;
	typedef std::vector<hs_cell> cell_vector;
	typedef std::vector<hs_vert>::iterator vert_vector_iter;
	typedef std::vector<hs_edge>::iterator edge_vector_iter;
	typedef std::vector<hs_face>::iterator face_vector_iter;
	typedef std::vector<hs_cell>::iterator cell_vector_iter;
	typedef std::vector<hs_vert>::const_iterator vert_vector_citer;
	typedef std::vector<hs_edge>::const_iterator edge_vector_citer;
	typedef std::vector<hs_face>::const_iterator face_vector_citer;
	typedef std::vector<hs_cell>::const_iterator cell_vector_citer;
	
	class hs_model {
	public:
		hs_model() {}
		hs_model(const hs_model& rhs) 
			: vertices(rhs.vertices) 
			, edges(rhs.edges)
			, faces(rhs.faces)
			, cells(rhs.cells) {}
		
		virtual ~hs_model();
		
		void add_vert(const hs_point&, vert_type);
		void add_edge(size_t, size_t, EDGE*, edge_type);
		void add_face_by_edges(const size_t[], size_t, FACE*, face_type);
		void add_face_by_verts(const size_t[], size_t, FACE*, face_type);
		void add_cell_by_faces(const size_t[], size_t);
		void add_cell_by_verts(const size_t[], size_t);
		
		void acis_wire(ENTITY_LIST&) const;
		void set_acis_edge(EDGE* [], size_t);
		void set_acis_face(FACE* [], size_t);
		
		hs_vert& vert_at(size_t idx) { assert( idx < vert_size() ); return vertices[idx]; }
		hs_edge& edge_at(size_t idx) { assert( idx < edge_size() ); return edges[idx]; }
		hs_face& face_at(size_t idx) { assert( idx < face_size() ); return faces[idx]; }
		hs_cell& cell_at(size_t idx) { assert( idx < cell_size() ); return cells[idx]; }
		
		vert_vector_iter first_vert()  { return vertices.begin(); }
		edge_vector_iter first_edge()  { return edges.begin(); }
		face_vector_iter first_face()  { return faces.begin(); }
		cell_vector_iter first_cell()  { return cells.begin(); }
		
		vert_vector_iter end_vert()  { return vertices.end(); }
		edge_vector_iter end_edge()  { return edges.end(); }
		face_vector_iter end_face()  { return faces.end(); }
		cell_vector_iter end_cell()  { return cells.end(); }
		size_t vert_size() const { return vertices.size(); }
		
		size_t edge_size() const { return edges.size(); }
		size_t face_size() const { return faces.size(); }
		size_t cell_size() const { return cells.size(); }

		void print_vert(const char* = NULL) const;
		void print_edge(const char* = NULL) const;
		void print_face(const char* = NULL) const;
		void print_cell(const char* = NULL) const;

		void write_vert(const char*) const;
		void write_edge(const char*) const;
		void save_file(const char*, const char* = "obj") const; 
		void check_face() const;
		void check_edge() const; 
		
	private:
		void sort_edge_of_face(size_t[], size_t) const;

	private:
		vert_vector vertices;
		edge_vector edges;
		face_vector faces;
		cell_vector cells;	
	};
	
} // namespace

#endif