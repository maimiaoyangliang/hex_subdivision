#ifndef HEXMODEL_H_
#define HEXMODEL_H_

#include <VECTOR>
#include "HexVertex.h"
#include "HexEdge.h"
#include "HexFace.h"
#include "HexCell.h"

class FACE;
class EDGE;
class ENTITY_LIST;

namespace HexMesh {
	
	class Point3D;

	typedef std::vector<HexVertex> vert_vector;
	typedef std::vector<HexVertex>::iterator vert_vector_iter;
	typedef std::vector<HexEdge> edge_vector;
	typedef std::vector<HexEdge>::iterator edge_vector_iter;
	typedef std::vector<HexFace> face_vector;
	typedef std::vector<HexFace>::iterator face_vector_iter;
	typedef std::vector<HexCell> cell_vector;
	typedef std::vector<HexCell>::iterator cell_vector_iter;
	
	class HexModel {
	public:
		HexModel() {}
		HexModel(const HexModel& rhs) 
			: vertices(rhs.vertices) 
			, edges(rhs.edges)
			, faces(rhs.faces)
			, cells(rhs.cells) {}

		virtual ~HexModel();
		
		void add_vert(const HexVertex& hex_vert) { vertices.push_back(hex_vert); }
		void add_edge(const HexEdge& hex_edge) { edges.push_back(hex_edge); }
		void add_face(const HexFace& hex_face) { faces.push_back(hex_face); }
		void add_cell(const HexCell& hex_cell) { cells.push_back(hex_cell); }
		
		void add_vert(const Point3D&);
		void add_edge(size_t, size_t);
//		void add_edge(ENTITY_LIST&, const Point3D&, const Point3D&, size_t, size_t, EDGE*);
		void add_face(const size_t ei[], size_t);
//		void add_face(const size_t ei[], size_t, FACE*);
		void add_cell(const size_t ci[], size_t);

		void acis_wire(ENTITY_LIST&);
		void set_acis_edge(EDGE* [], size_t);
		void set_acis_face(FACE* [], size_t);

		HexVertex& vertex(size_t ind)  { return vertices[ind]; }
		HexEdge& edge(size_t ind)  { return edges[ind]; }
		HexFace& face(size_t ind)  { return faces[ind]; }
		HexCell& cell(size_t ind)  { return cells[ind]; }
		
		vert_vector_iter first_vert()  { return vertices.begin(); }
		vert_vector_iter end_vert()  { return vertices.end(); }
		size_t vert_size() const { return vertices.size(); }
		
		edge_vector_iter first_edge()  { return edges.begin(); }
		edge_vector_iter end_edge()  { return edges.end(); }
		size_t edge_size() const { return edges.size(); }
		
		face_vector_iter first_face()  { return faces.begin(); }
		face_vector_iter end_face()  { return faces.end(); }
		size_t face_size() const { return faces.size(); }
		
		cell_vector_iter first_cell()  { return cells.begin(); }
		cell_vector_iter end_cell()  { return cells.end(); }
		size_t cell_size() const { return cells.size(); }
		
	private:
		vert_vector vertices;
		edge_vector edges;
		face_vector faces;
		cell_vector cells;	
	};
	
} // namespace

#endif