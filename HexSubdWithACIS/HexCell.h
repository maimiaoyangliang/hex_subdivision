#ifndef HEXCELL_H_
#define HEXCELL_H_

#include "HexVarDef.h"


namespace HexMesh {
	
	class HexCell {
	public:
		HexCell() {}
		HexCell(const HexCell&);
		HexCell& operator= (const HexCell&);
		
		virtual ~HexCell() {}
		
		void add_vert(size_t ind) { vertexIndices.insert(ind); }
		void add_edge(size_t ind) { edgeIndices.insert(ind); }
		void add_face(size_t ind) { faceIndices.insert(ind); }
		
		int_set_iter first_vert()  { return vertexIndices.begin(); }
		int_set_iter end_vert()  { return vertexIndices.end(); }
		size_t vert_size() const { return vertexIndices.size(); }
		
		int_set_iter first_edge()  { return edgeIndices.begin(); }
		int_set_iter end_edge()  { return edgeIndices.end(); }
		size_t edge_size() const { return edgeIndices.size(); }
		
		int_set_iter first_face()  { return faceIndices.begin(); }
		int_set_iter end_face()  { return faceIndices.end(); }
		size_t face_size() const { return faceIndices.size(); }

		void clear_all() { vertexIndices.clear(); edgeIndices.clear(); faceIndices.clear(); }
		
	private:
		int_set vertexIndices;
		int_set edgeIndices;
		int_set faceIndices;
	};
	
} // namespace

#endif