#ifndef HEXVERTEX_H_
#define HEXVERTEX_H_


#include "Point3D.h"
#include "HexVarDef.h"

namespace HexMesh { 
	
	class HexVertex {
	public:
		HexVertex();
		HexVertex(double, double, double);
		HexVertex(const Point3D&);
		HexVertex(const HexVertex& );
		HexVertex& operator= (const HexVertex&);
		
		virtual ~HexVertex() {}
		
		Point3D coord() const { return coordinate; }
		
		void set_coord(const Point3D& p) { coordinate = p; }
		void add_edge(size_t ind) { edgeIndices.insert(ind); }
		void add_face(size_t ind) { faceIndices.insert(ind); }
		void add_cell(size_t ind) { cellIndices.insert(ind); }
		
		
		int_set_iter first_edge()  { return edgeIndices.begin(); }
		int_set_iter end_edge()  { return edgeIndices.end(); }
		size_t edge_size()  { return edgeIndices.size(); }
		
		int_set_iter first_face()  { return faceIndices.begin(); }
		int_set_iter end_face()  { return faceIndices.end(); }
		size_t face_size() const { return faceIndices.size(); }
		
		int_set_iter first_cell()  { return cellIndices.begin(); }
		int_set_iter end_cell()  { return cellIndices.end(); }
		size_t cell_size() const { return cellIndices.size(); }
		
	private:	
		Point3D	coordinate;             // the coordinate of vertex 
		int_set edgeIndices;			// link of edges containing the vertex;
		int_set faceIndices;			// link of faces containing the vertex;
		int_set cellIndices;			// link of cells containing the vertex;
	};
	
} // namespace

#endif