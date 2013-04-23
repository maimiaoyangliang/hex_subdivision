#ifndef HS_EXAMPLE_H_
#define HS_EXAMPLE_H_

#include <VECTOR>

#include "position.hxx"

class FACE;
class EDGE;
class VERTEX;
class COEDGE;
class ENTITY;
class ENTITY_LIST;

namespace hex_subdiv {

	class hs_model;

	class hs_example {
	public:
		void cylinder_4(hs_model&, ENTITY_LIST&);
		void cylinder_6(hs_model&, ENTITY_LIST&);
		void gear_coarse(ENTITY_LIST&, hs_model&, const char* );
		void gear_compact(ENTITY_LIST&, hs_model&, const char* );
		void make_gear_compact(const char*, ENTITY_LIST&);
		void make_gear_coarse(const char*, ENTITY_LIST&);

	private:
		EDGE* get_edge_by_2_vert(VERTEX*, VERTEX*);
		FACE* get_face_by_2_vert(VERTEX*, VERTEX*);
		void  get_vertices_by_coedge(COEDGE*, std::vector< VERTEX* >&);
		void  print_vert(std::vector< VERTEX* >&, const char* = NULL) const; 

		// gear
		void get_faces_of_gear(ENTITY* , std::vector< FACE* >& );

		void get_vertices_of_gear_coarse( std::vector< FACE* >&, std::vector< SPAposition >&,
			std::vector< VERTEX* >& , size_t& );

		void add_vertices_to_model_coarse(hs_model&, std::vector< SPAposition >&, size_t);
		void add_edges_to_model_coarse(hs_model&, std::vector< FACE* >&, std::vector< VERTEX* >&, size_t);
		void add_faces_to_model_coarse(hs_model&, std::vector< FACE* >&, std::vector< VERTEX* >&, size_t);
		void add_cells_to_model_coarse(hs_model&, size_t);
		
		void get_vertices_of_gear_compact( std::vector< FACE* >&, std::vector< SPAposition >&,
			std::vector< VERTEX* >& , size_t& );
		void add_vertices_to_model_compact(hs_model&, std::vector< SPAposition >&, size_t);
		void add_edges_to_model_compact(hs_model&, std::vector< FACE* >&, std::vector< VERTEX* >&, size_t);
		void add_faces_to_model_compact(hs_model&, std::vector< FACE* >&, std::vector< VERTEX* >&, size_t);
		void add_cells_to_model_compact(hs_model&, size_t);
	};

} // namespace


#endif