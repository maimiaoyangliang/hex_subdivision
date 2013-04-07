#ifndef HS_MODEL_H_
#define HS_MODEL_H_

#include <VECTOR>

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
	
	class hs_model {
	public:
		hs_model() {}
		hs_model(const hs_model& rhs) 
			: vertices(rhs.vertices) 
			, edges(rhs.edges)
			, faces(rhs.faces)
			, cells(rhs.cells) {}
		
		virtual ~hs_model();
		
		void add_vert(const hs_vert& hex_vert) { vertices.push_back(hex_vert); }
		void add_edge(const hs_edge& hex_edge) { edges.push_back(hex_edge); }
		void add_face(const hs_face& hex_face) { faces.push_back(hex_face); }
		void add_cell(const hs_cell& hex_cell) { cells.push_back(hex_cell); }
		
		void add_vert(const hs_point&);
		void add_edge(size_t, size_t);
		void add_face(const size_t ei[], size_t);
		void add_cell(const size_t ci[], size_t);
		
		void acis_wire(ENTITY_LIST&);
		void set_acis_edge(EDGE* [], size_t);
		void set_acis_face(FACE* [], size_t);
		
		hs_vert& vert_at(size_t idx)  { return vertices[idx]; }
		hs_edge& edge_at(size_t idx) { return edges[idx]; }
		hs_face& face_at(size_t idx) { return faces[idx]; }
		hs_cell& cell_at(size_t idx) { return cells[idx]; }
		
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
		
	private:
		vert_vector vertices;
		edge_vector edges;
		face_vector faces;
		cell_vector cells;	
	};
	
} // namespace

#endif