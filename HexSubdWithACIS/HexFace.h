#ifndef HEXFACE_H_
#define HEXFACE_H_

#include "HexVarDef.h"

class FACE;

namespace HexMesh {
	

	
	class HexFace {
	public:
		HexFace();
		HexFace(const HexFace&);
		HexFace& operator= (const HexFace&);
		
		virtual ~HexFace() {}
		
		void set_acis_face(FACE* acis_face) { _acis_face = acis_face; }
		void set_cell(size_t in, size_t out) { cellIndices[0] = in; cellIndices[1] = out; }
		void add_edge(size_t ind) { edgeIndices.insert(ind); }
		void add_vert(size_t ind) { vertexIndices.insert(ind); }
		
		int_set_iter first_vert()  { return vertexIndices.begin(); }
		int_set_iter end_vert()  { return vertexIndices.end(); }
		size_t vert_size() const { return vertexIndices.size(); }
		
		int_set_iter first_edge()  { return edgeIndices.begin(); }
		int_set_iter end_edge()  { return edgeIndices.end(); }
		size_t edge_size() const { return edgeIndices.size(); }
		
		void set_fst_cell(size_t id) { cellIndices[0] = id; }
		void set_snd_cell(size_t id) { cellIndices[1] = id; }
		int fst_cell()  { return cellIndices[0]; }
		int snd_cell()  { return cellIndices[1]; }
		const FACE* acis_face() const { return _acis_face; }

		void clear_vert() { vertexIndices.clear(); }
		void clear_edge() { edgeIndices.clear(); }
		void clear_all() { clear_vert(); clear_edge(); cellIndices[0] = cellIndices[1] = -1; _acis_face = NULL; }
		
	private:
		int_set vertexIndices;
		int_set edgeIndices;
		int  cellIndices[2];
		FACE*   _acis_face;
	};

}

#endif