#ifndef HS_SUBDIV_H_
#define HS_SUBDIV_H_

#include <VECTOR>

#include "hs_point.h"

class ENTITY_LIST;

namespace hex_subdiv {
	
	class hs_model;
	
	class hs_subdiv {
	public:
		hs_subdiv(hs_model* hex_model);
		
		virtual ~hs_subdiv();
		
		//		void ApproximateSubdivision(ENTITY_LIST&);
		void inter_subdiv();
		void laplacian_smoothing_adaptive();
		void jacobian(std::vector<double>&);
		void print_info() const;
		const hs_model* model() const { return phexmodel; }
		
	private:
		
		typedef std::vector<hs_point> point_vector;
		typedef std::vector<hs_point>::iterator point_vector_iter;
		
		void calc_cell_point(point_vector&);
		void calc_face_point(point_vector&, point_vector&);
		void calc_edge_point(point_vector&, point_vector&);
		void calc_vert_point(point_vector&, point_vector&);
		 
		double laplacian_smoothing();
		
		void create_vert_list(hs_model*, point_vector&, point_vector&, point_vector&, point_vector&);
		void create_edge_list(hs_model*);
		void create_face_list(hs_model*);
		void create_cell_list(hs_model*);
		
		
	private:
		hs_model* phexmodel;
		size_t	  subdiv_counter;	// count the number of  subdivision
		size_t    lapalace_counter;
		
	};
}


#endif