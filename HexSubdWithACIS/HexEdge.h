#ifndef HEXEDGE_H_
#define HEXEDGE_H_

#include "HexVarDef.h"
	
class EDGE;

namespace HexMesh {
	


	class HexEdge {
	public:
		HexEdge();
		HexEdge(const HexEdge&);
		HexEdge& operator= (const HexEdge&);
		
		virtual ~HexEdge(){}

		void add_face(size_t ind) { faceIndices.insert(ind); }
		void add_cell(size_t ind) { cellIndices.insert(ind); }
		
		void set_acis_edge(EDGE* acis_edge) { _acis_edge = acis_edge; }
		void set_start_vert(size_t v) { vertexIndices[0] = v; }
		void set_end_vert(size_t v) { vertexIndices[1] = v; }

		size_t start_vert() const { return vertexIndices[0]; }
		size_t end_vert() const { return vertexIndices[1]; }

		int_set_iter first_face()  { return faceIndices.begin(); }
		int_set_iter end_face()  { return faceIndices.end(); }
		size_t face_size() const { return faceIndices.size(); }

		int_set_iter first_cell()  { return cellIndices.begin(); }
		int_set_iter end_cell()  { return cellIndices.end(); }
		size_t cell_size() const { return cellIndices.size(); }

		EDGE* acis_edge() const { return _acis_edge; }

		
	private:
		int_set faceIndices;
		int_set cellIndices;
		size_t  vertexIndices[2];
		EDGE*   _acis_edge;
		
	};
	
} // namespace

#endif