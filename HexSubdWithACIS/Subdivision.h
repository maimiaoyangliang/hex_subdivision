#ifndef SUBDIVISION_H_
#define SUBDIVISION_H_

#include <VECTOR>

#include "Point3D.h"

class ENTITY_LIST;

namespace hex_subdiv {

	class hs_model;
	
	class hs_subdiv {
	public:
		hs_subdiv(hex_subdiv::hs_model* hex_model);
		
		virtual ~hs_subdiv();
		
//		void ApproximateSubdivision(ENTITY_LIST&);
		void inter_subdiv(ENTITY_LIST&);
		
	private:

		typedef std::vector<hs_point> point_vector;

		void calc_cell_point(point_vector&);
		void calc_face_point(point_vector&, point_vector&);
		void calc_edge_point(point_vector&, point_vector&, point_vector&);
		void calc_vert_point(point_vector&, point_vector&, point_vector&, point_vector&);

//		void ChangeNewVertices();
		void laplacian_smoothing(hs_model*);

		void create_vert_list(hs_model* pNewModel, point_vector&, point_vector&, point_vector&, point_vector&);
		void create_edge_list(hs_model* pNewModel, point_vector&, point_vector&, point_vector&);
		void create_face_list(hs_model* pNewModel, point_vector&, point_vector&, point_vector&);
		void create_cell_list(hs_model* pNewModel, point_vector&, point_vector&, point_vector&);
		
//		void FreeMemory();
		
	private:
		hs_model* phexmodel;
		size_t	  subdiv_counter;	// count the number of  subdivision

	};
}


#endif