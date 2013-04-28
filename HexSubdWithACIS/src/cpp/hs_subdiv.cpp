#pragma warning(disable:4786)

#include <SET>
#include <IOSTREAM>
#include <CASSERT> 
#include <LIMITS>

#include "hs_cell.h"
#include "hs_model.h"
#include "hs_face.h"
#include "hs_edge.h"
#include "hs_vert.h"
#include "hs_subdiv.h"

#include "alltop.hxx"
#include "surdef.hxx"
#include "surface.hxx"
#include "position.hxx"
#include "curve.hxx"
#include "curdef.hxx"
#include "lists.hxx"

using namespace hex_subdiv;



hs_subdiv::hs_subdiv(hs_model* hex_model) 
	: phexmodel(new hs_model(*hex_model))
	, subdiv_counter(0)
	, lapalace_counter(0) 
	{}

hs_subdiv::~hs_subdiv() {
	if (phexmodel)
		delete phexmodel;
}


// calculate cell point coordinate by iterating each cell	
void hs_subdiv::calc_cell_point( point_vector& cell_points ) {
	
	hs_point cent;
	cell_vector_iter cell_iter = phexmodel->first_cell();
	for (; cell_iter != phexmodel->end_cell(); ++cell_iter) {
		cent.zero();
		int_set_iter vert_iter = cell_iter->first_vert();
		for (; vert_iter != cell_iter->end_vert(); ++ vert_iter) {
			cent += phexmodel->vert_at(*vert_iter).coord();
		}
		cent /= cell_iter->vert_size(); 
		cell_points.push_back(cent);
	}
}


// calculate face point coordinate by iterating each face
// there are 2 classes face: 
//	1) border face, which has 1 adjacent cell;
//	2) inner face, which has 2 adjacent cells.
void hs_subdiv::calc_face_point(point_vector& face_points, point_vector& cell_points) {
	
	hs_point cent;
	face_vector_iter face_iter = phexmodel->first_face();
	for (; face_iter != phexmodel->end_face(); ++face_iter) {
		cent.zero();
		int_set_iter vert_iter = face_iter->first_vert();
		for (; vert_iter != face_iter->end_vert(); ++vert_iter) {
			cent += phexmodel->vert_at(*vert_iter).coord();
		}
		cent /= face_iter->vert_size();
		
		//if (face_iter->fst_cell() != -1 && face_iter->snd_cell() != -1) {
		if ( face_iter->type() == INNER_FACE ) {
			// inner face
			cent = cent * 0.5 + (cell_points[face_iter->fst_cell()]
				+ cell_points[face_iter->snd_cell()]) * 0.25;
		} else if ( face_iter->type() == BORDER_FACE ) { 
			// boder face
			// find a vertex in the surface 
			assert( NULL != face_iter->acis_face() ); 
			SPAposition spa_pos( cent.x(), cent.y(), cent.z() );
			const surface& csurf = face_iter->acis_face()->geometry()->equation();
			SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
			cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );			
		}
		
		face_points.push_back(cent);	
	}
}


// calculate edge point coordinate by iterating each edge in model
// there are 3 classes edge: 
//	1) inner edge, whose number of adjacent faces is same with adjacent cells;
//	2) crease edge, which has 2 adjacent border faces
//	3) ordinary border edge( except crease ), 
//     which has 2 adjacent border faces and other inner faces
void hs_subdiv::calc_edge_point(point_vector& edge_points, point_vector& cell_points) {	
	
	size_t idx = 0;
	hs_point cent;
	edge_vector_iter edge_iter = phexmodel->first_edge();
	for (; edge_iter != phexmodel->end_edge(); ++edge_iter, ++idx) {
		// centroid of each edge
		cent = ( phexmodel->vert_at(edge_iter->start_vert()).coord() + 
			phexmodel->vert_at(edge_iter->end_vert()).coord() ) * 0.5;
		
		if ( edge_iter->type() == INNER_EDGE ) {
			// inner edge
			hs_point avg_face_point;
			int_set_iter face_iter = edge_iter->first_face();
			for (; face_iter != edge_iter->end_face(); ++face_iter) {
				hs_point avg_cent_face;
				hs_face& f = phexmodel->face_at(*face_iter);
				int_set_iter vitr = f.first_vert();
				for (; vitr != f.end_vert(); ++vitr) {
					avg_cent_face += phexmodel->vert_at(*vitr).coord();
				}
				avg_face_point += avg_cent_face / f.vert_size();
			} 
			avg_face_point /= edge_iter->face_size();
			
			hs_point avg_cell_point;
			int_set_iter cell_iter = edge_iter->first_cell();
			for (; cell_iter != edge_iter->end_cell(); ++cell_iter) {
				avg_cell_point += cell_points[*cell_iter];
			} 
			avg_cell_point /= edge_iter->cell_size();
			
			// average of midpoint, face point and cell point
			size_t n = edge_iter->face_size();
			cent = ((n - 3) * cent + 2 * avg_face_point + avg_cell_point) / n;
			
		} else if ( edge_iter->type() == CREASE_EDGE ) {
			// crease edge
			// find a vertex in the curve which is adjacent to crease edge
			assert( NULL != edge_iter->acis_edge() );
			SPAposition spa_pos( cent.x(), cent.y(), cent.z());
			const curve& ccurv = edge_iter->acis_edge()->geometry()->equation();
			SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
			cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
			
		} else if ( edge_iter->type() == ORDINARY_EDGE ) { 
			// ordinary border edge
			// find a vertex in the surface which is adjacent to edge
			std::set< const FACE* > face_set;
			int_set_iter face_iter = edge_iter->first_face();
			for (; face_iter != edge_iter->end_face(); ++face_iter) {
				face_set.insert( phexmodel->face_at(*face_iter).acis_face() );
			}
			// ordinary edge have 2 same adjacent surface
			face_set.erase(NULL); 
			assert( 1 == face_set.size() );
			const FACE* spa_face = *(face_set.begin());
			const surface& csurf = spa_face->geometry()->equation();
			SPAposition spa_pos( cent.x(), cent.y(), cent.z() );
			SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
			cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );	
		}
		
		edge_points.push_back(cent);	
	}
}


// calculate vertex point by iterating each vertex
// there are 4 classes vertex: 
//	1) inner vertex, 
//	2) corner point, 
//	3) point on crease 
//	4) ordinary border point on adjacent surface
void hs_subdiv::calc_vert_point(point_vector& vert_points, point_vector& cell_points) {
	
	hs_point cent;
	vert_vector_iter vert_iter = phexmodel->first_vert();
	for (; vert_iter != phexmodel->end_vert(); ++ vert_iter) {
		
		cent = vert_iter->coord();
		
		if ( vert_iter->type() == INNER_VERT ) {
			// inner point
			hs_point avg_cell_point;
			int_set_iter cell_iter = vert_iter->first_cell();
			for (; cell_iter != vert_iter->end_cell(); ++cell_iter) {
				avg_cell_point += cell_points[*cell_iter];
			}
			
			avg_cell_point /= vert_iter->cell_size();
			
			hs_point avg_face_point;
			int_set_iter face_iter = vert_iter->first_face();
			for (; face_iter != vert_iter->end_face(); ++face_iter) {
				hs_point avg_cent_face;
				hs_face& f = phexmodel->face_at(*face_iter);
				int_set_iter vitr = f.first_vert();
				for (; vitr != f.end_vert(); ++vitr) {
					avg_cent_face += phexmodel->vert_at(*vitr).coord();
				}
				avg_face_point += avg_cent_face / f.vert_size();
			}
			
			avg_face_point /= vert_iter->face_size();
			
			hs_point avg_mid_edge;
			int_set_iter edge_iter = vert_iter->first_edge();
			for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
				avg_mid_edge += (
					phexmodel->vert_at(phexmodel->edge_at(*edge_iter).start_vert()).coord() +
					phexmodel->vert_at(phexmodel->edge_at(*edge_iter).end_vert()).coord() 
					) * 0.5;
			}
			
			avg_mid_edge /= vert_iter->edge_size();
			
			cent = (cent + avg_mid_edge * 3 + avg_face_point * 3 + avg_cell_point) / 8;
			
		} else if ( vert_iter->type() == CREASE_VERT ){
			// crease vertex
			// find the crease curve of crease vertex using set
			std::set< const EDGE* > edge_set;
			int_set_iter edge_iter = vert_iter->first_edge();
			for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
				edge_set.insert( phexmodel->edge_at(*edge_iter).acis_edge() );
			}
			edge_set.erase( NULL );
			// crease vertex only have 2 same crease curve 
			// and other adjacent edge is null
			assert( 1 == edge_set.size() );
			
			SPAposition spa_pos( cent.x(), cent.y(), cent.z());
			const EDGE* spa_edge = *(edge_set.begin());
			const curve& ccurv = spa_edge->geometry()->equation();
			SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
			cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
			
		} else if ( vert_iter->type() == ORDINARY_VERT ) { 
			// ordinary vertex
			// find the unique adjacent border surface using set
			std::set< const FACE* > face_set;
			int_set_iter face_iter = vert_iter->first_face();
			for (; face_iter != vert_iter->end_face(); ++face_iter) {
				face_set.insert( phexmodel->face_at(*face_iter).acis_face() );
			}
			face_set.erase( NULL );
			assert( 1 == face_set.size() );
			const FACE* spa_face = *(face_set.begin());
			const surface& csurf = spa_face->geometry()->equation();
			SPAposition spa_pos( cent.x(), cent.y(), cent.z() );
			SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
			cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
		} else {
			// corner vertex
			assert( vert_iter->type() == CORNER_VERT );
		}
		
		vert_points.push_back(cent);
	}	
}


// laplacian smoothing is used to each vertex
// for 4 classes vertex, using different methods
// and adaptive to do it
double hs_subdiv::laplacian_smoothing() {
	
	assert(phexmodel != NULL);
	
	double change_sum = 0;
	
	int_set vert_set;
	vert_vector_iter vert_iter = phexmodel->first_vert();
	for (size_t vert_idx = 0; vert_iter != phexmodel->end_vert(); ++vert_iter, ++vert_idx) {
		if ( vert_iter->type() == INNER_VERT ) {
			// update inner vertex using midpoint of its all 1-neighborhood points
			int_set_iter edge_iter = vert_iter->first_edge();
			for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
				// edges of adjacent to inner vertex are inner
				hs_edge& hex_edge = phexmodel->edge_at(*edge_iter);
				assert( hex_edge.type() == INNER_EDGE );
				vert_set.insert( hex_edge.start_vert() );
				vert_set.insert( hex_edge.end_vert() );
			}
			
			vert_set.erase(vert_idx);	
			hs_point spoint;
			int_set_iter viter = vert_set.begin();
			for (; viter != vert_set.end(); ++viter) {
				spoint += phexmodel->vert_at(*viter).coord();
			}
			spoint /= vert_set.size();
			change_sum += (spoint - vert_iter->coord()).length();
			vert_iter->set_coord(spoint); 
			
		} else if ( vert_iter->type() == ORDINARY_VERT ) {
			// update ordinary vertex coordinate 
			// using average of its 1-neighborhood points on adjacent 
			// ordinary edges, then project it to surface.
			int_set_iter edge_iter = vert_iter->first_edge();
			for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
				hs_edge& hex_edge = phexmodel->edge_at(*edge_iter);
				if ( hex_edge.type() == ORDINARY_EDGE ) {
					vert_set.insert( hex_edge.start_vert() );
					vert_set.insert( hex_edge.end_vert() );
				}
			}
			vert_set.erase(vert_idx);	
			hs_point spoint;
			int_set_iter viter = vert_set.begin();
			for (; viter != vert_set.end(); ++viter) {
				spoint += phexmodel->vert_at(*viter).coord();
			}
			spoint /= vert_set.size();
			
			std::set< const FACE* > face_set;
			int_set_iter face_iter = vert_iter->first_face();
			for (; face_iter != vert_iter->end_face(); ++face_iter) {
				hs_face& hex_face = phexmodel->face_at(*face_iter);
				if ( hex_face.type() == BORDER_FACE ) {
					face_set.insert( phexmodel->face_at(*face_iter).acis_face() );
				}
			}
			// vertex has unique adjacent border surface
			assert( 1 == face_set.size() );
			const FACE* spa_face = *(face_set.begin());
			const surface& csurf = spa_face->geometry()->equation();
			SPAposition spa_pos( spoint.x(), spoint.y(), spoint.z() );
			SPAposition new_pos( csurf.eval_position( csurf.param(spa_pos) ) );
			hs_point hs_pos( new_pos.x(), new_pos.y(), new_pos.z() );
			change_sum += (hs_pos - vert_iter->coord()).length();
			vert_iter->set_coord( hs_pos );
			
		} else if ( vert_iter->type() == CREASE_VERT ) {
			// update crease vertex coordinate
			// using average of its 1-neighborhood points on adjacent 
			// crease edge, then project midpoint to the curve
			std::set< const EDGE* > edge_set;
			int_set_iter edge_iter = vert_iter->first_edge();
			for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
				hs_edge& hex_edge = phexmodel->edge_at(*edge_iter);
				if ( hex_edge.type() == CREASE_EDGE ) {
					vert_set.insert( hex_edge.start_vert() );
					vert_set.insert( hex_edge.end_vert() );
				}
				edge_set.insert( hex_edge.acis_edge() );
			}
			vert_set.erase(vert_idx);	
			hs_point spoint;
			int_set_iter viter = vert_set.begin();
			for (; viter != vert_set.end(); ++viter) {
				spoint += phexmodel->vert_at(*viter).coord();
			}
			spoint /= vert_set.size();
			// crease vertex has unique adjacent crease curve
			edge_set.erase(NULL);
			assert( edge_set.size() == 1);
			const EDGE* spa_edge = *(edge_set.begin());
			const curve& ccurv = spa_edge->geometry()->equation();
			SPAposition spa_pos( spoint.x(), spoint.y(), spoint.z() );
			SPAposition new_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
			hs_point hs_pos( new_pos.x(), new_pos.y(), new_pos.z() );
			change_sum += (hs_pos - vert_iter->coord()).length();
			vert_iter->set_coord(hs_pos);
			
		} else {
			assert ( vert_iter->type() == CORNER_VERT );
		}
		
		vert_set.clear();	
	}
	
	return change_sum;
}

// create new vertex list 
// new vertex has same type with old vertex.
// new edge vertex has same type with old edge, such as inner, ordinary and crease
// new face vertex has same type with old face, such as inner, (ordinary) border
// new cell vertex only has INNER type
void hs_subdiv::create_vert_list(hs_model* pmodel
								 , point_vector& vert_points
								 , point_vector& edge_points
								 , point_vector& face_points
								 , point_vector& cell_points
								 ) 
{
	point_vector_iter vert_iter;
	size_t idx = 0;
	vert_iter = vert_points.begin();
	for (; vert_iter != vert_points.end(); ++vert_iter, ++idx) {
		pmodel->add_vert(*vert_iter, phexmodel->vert_at(idx).type() );
	}
	
	idx = 0;
	vert_iter = edge_points.begin();
	for (; vert_iter != edge_points.end(); ++vert_iter, ++idx) {
		pmodel->add_vert(*vert_iter, 
			static_cast<vert_type>( phexmodel->edge_at(idx).type() ));
	}
	
	idx = 0;
	vert_iter = face_points.begin();
	for (; vert_iter != face_points.end(); ++vert_iter, ++idx) {
		pmodel->add_vert(*vert_iter,
			static_cast<vert_type>( phexmodel->face_at(idx).type() ) );
	}
	
	vert_iter = cell_points.begin();
	for (; vert_iter != cell_points.end(); ++vert_iter) {
		pmodel->add_vert(*vert_iter, INNER_VERT);
	}
	
}

// create new edge list
// old edge split to 2 new edges with same type
// new face edge has same type with old face, such as inner or (ordinary) border
// new cell edge has only one type: INNER_EDGE
void hs_subdiv::create_edge_list(hs_model* pmodel) {
	
	size_t vsize = phexmodel->vert_size();
	size_t esize = phexmodel->edge_size();
	size_t fsize = phexmodel->face_size();
	
	size_t pos1, pos2;
	pos2 = vsize; 
	edge_vector_iter edge_iter = phexmodel->first_edge();
	for (; edge_iter != phexmodel->end_edge(); ++edge_iter) {
		// one old edge split to new edges
		EDGE* spa_edge = const_cast< EDGE* >( edge_iter->acis_edge() ) ;
		
		pos1 = edge_iter->start_vert();
		pmodel->add_edge(pos1, pos2, spa_edge, edge_iter->type());
		
		pos1 = edge_iter->end_vert();
		pmodel->add_edge(pos1, pos2, spa_edge, edge_iter->type());
		++pos2;
	}
	
	pos2 = vsize + esize; 
	face_vector_iter face_iter = phexmodel->first_face();
	for (; face_iter != phexmodel->end_face(); ++face_iter) {
		// create four new edges in each face
		int_set_iter eiter = face_iter->first_edge();
		for (; eiter != face_iter->end_edge(); ++eiter) {
			pos1 = *eiter + vsize; 
			pmodel->add_edge(pos1, pos2, NULL,
				static_cast<edge_type>( face_iter->type() ) );
		}
		++pos2;
	}
	
	pos2 = vsize + esize + fsize; 
	cell_vector_iter cell_iter = phexmodel->first_cell();
	for (; cell_iter != phexmodel->end_cell(); ++cell_iter) {
		// create six new edges in one cell
		int_set_iter fiter = cell_iter->first_face();
		for (; fiter != cell_iter->end_face(); ++fiter) {
			pos1 = *fiter + vsize + esize; 
			pmodel->add_edge(pos1, pos2, NULL, INNER_EDGE);				
		}
		++pos2;
	}
	
}

// create new face list 
// old face split 4 new face with same type
// there are 12 faces created in each cell with INNER_FACE type
void hs_subdiv::create_face_list(hs_model* pmodel) {
	
	size_t vsize = phexmodel->vert_size();
	size_t esize = phexmodel->edge_size();
	size_t fsize = phexmodel->face_size();
	
	// in each face, 4 new faces are created 
	size_t fp_idx = vsize + esize;
	face_vector_iter face_itr = phexmodel->first_face();
	for (; face_itr != phexmodel->end_face(); ++face_itr, ++fp_idx) {
		// find all edge points in each face
		int_set ep_set; // edge point set
		int_set_iter edge_itr = face_itr->first_edge();
		for (; edge_itr != face_itr->end_edge(); ++edge_itr) {
			ep_set.insert(*edge_itr + vsize);
		} 
		
		// according to the vertex of face, create new face
		int_set_iter vert_itr = face_itr->first_vert();
		for (; vert_itr != face_itr->end_vert(); ++vert_itr) {
			// iterating the adjacent edge of vertex to find 2 edge
			// in the face
			int_set eidx_set; // edge index set
			int_set sepidx_set; // selected edge point index 
			hs_vert& hex_vert = pmodel->vert_at(*vert_itr);
			int_set_iter eitr = hex_vert.first_edge(); 
			for (; eitr != hex_vert.end_edge(); ++eitr) { 
				hs_edge& edge = pmodel->edge_at(*eitr);
				size_t sp = edge.start_vert();
				size_t ep = edge.end_vert(); 
				int_set_iter ep_itr = ep_set.begin(); 
				for (; ep_itr != ep_set.end(); ++ep_itr) {
					if ( *ep_itr == sp || *ep_itr == ep ) {
						eidx_set.insert(*eitr);
						sepidx_set.insert(*ep_itr);
					}
				}
			} 
			
			hs_vert& face_point = pmodel->vert_at(fp_idx);
			int_set_iter fe_itr = face_point.first_edge();
			for (; fe_itr != face_point.end_edge(); ++fe_itr) {
				hs_edge& edge = pmodel->edge_at(*fe_itr);
				size_t sp = edge.start_vert();
				size_t ep = edge.end_vert();
				int_set_iter sepitr = sepidx_set.begin(); 
				for (; sepitr != sepidx_set.end(); ++sepitr) {
					if ( *sepitr == sp || *sepitr == ep ) {
						eidx_set.insert(*fe_itr); 
					}
				}
			}  
			
			assert( 4 == eidx_set.size() );
			size_t* eidx = new size_t[4];
			size_t* peidx = eidx;
			int_set_iter itr = eidx_set.begin();
			for (; itr != eidx_set.end(); ) {
				*peidx++ = *itr++;
			}
			
			pmodel->add_face_by_edges(eidx, 4, 
				const_cast< FACE* >( face_itr->acis_face() ), 
				face_itr->type()
				);
			delete[] eidx;
		}
	}
	
	// in each cell, 12 new faces are created
	cell_vector_iter cell_iter = phexmodel->first_cell();
	for (size_t cidx = 0; cell_iter != phexmodel->end_cell(); ++cell_iter, ++cidx) {
		int_set fe_set, ce_set; // face edges set and cell edges set
		int_set_iter fiter = cell_iter->first_face();
		for (; fiter != cell_iter->end_face(); ++fiter) {
			size_t fe_idx = 2 * esize + 4 * (*fiter);
			const size_t NUM_OF_FACE_EDGES = 4;
			size_t i = 0;
			for (; i < NUM_OF_FACE_EDGES; ++i) { 
				fe_set.insert(fe_idx++);
			}
			size_t ce_idx = 2 * esize + 4 * fsize + 6 * cidx;
			const size_t NUM_OF_CELL_EDGES = 6;
			for (i = 0; i < NUM_OF_CELL_EDGES; ++i) {
				ce_set.insert(ce_idx++);
			}
		}
		int_set_iter eiter = cell_iter->first_edge();
		for (; eiter != cell_iter->end_edge(); ++eiter) {
			size_t ep_idx = *eiter + vsize; // edge point index
			int_set edge_of_face_set; // select 2 face edge adjacent to the edge point
			int_set fp_set; // face point set
			int_set_iter fe_itr = fe_set.begin();
			for (; fe_itr != fe_set.end(); ++fe_itr) {
				// find 2 face edges according to 1 edge point
				size_t sv = pmodel->edge_at(*fe_itr).start_vert();
				size_t ev = pmodel->edge_at(*fe_itr).end_vert();
				if ( sv == ep_idx || ev == ep_idx ) {
					edge_of_face_set.insert(*fe_itr);
					fp_set.insert(sv); fp_set.insert(ev);
				}
			} 
			
			assert( 2 == edge_of_face_set.size() ); 
			fp_set.erase(ep_idx);
			assert( 2 == fp_set.size() ); // only 2 face point are selected
			int_set_iter ce_itr = ce_set.begin();
			for (; ce_itr != ce_set.end(); ++ce_itr) {
				// find 2 cell edges according to 2 face point
				size_t sv = pmodel->edge_at(*ce_itr).start_vert();
				size_t ev = pmodel->edge_at(*ce_itr).end_vert();
				int_set_iter fp_itr = fp_set.begin();
				size_t fp1 = *fp_itr++; 
				size_t fp2 = *fp_itr;
				if ( sv == fp1 || sv == fp2 || ev == fp1 || ev == fp2 ) {
					edge_of_face_set.insert(*ce_itr);
				}
			}
			
			assert( 4 == edge_of_face_set.size() );
			size_t* edge_of_face = new size_t[4];
			int_set_iter eofs = edge_of_face_set.begin();
			for (size_t i = 0; eofs != edge_of_face_set.end();) {
				edge_of_face[i++] = *eofs++;
			}
			pmodel->add_face_by_edges(edge_of_face, 4, NULL, INNER_FACE);
			delete[] edge_of_face;
		} // for -- edge
	} // for -- cell
}
	
// create new cell list by iterating every cell
// each cell is splitted to 8 new cells
void hs_subdiv::create_cell_list(hs_model* pmodel) {
	size_t vsize = phexmodel->vert_size();
	size_t esize = phexmodel->edge_size();
	size_t fsize = phexmodel->face_size();
	
	size_t cell_idx = vsize + esize + fsize;
	cell_vector_iter cell_itr = phexmodel->first_cell();
	for (; cell_itr != phexmodel->end_cell(); ++cell_itr, ++cell_idx) {
		// find all face point of the cell
		int_set fp_set; // face point set
		int_set_iter cf_itr = cell_itr->first_face();
		for (; cf_itr != cell_itr->end_face(); ++cf_itr) {
			fp_set.insert( *cf_itr + vsize + esize );
		}
		// find all edge point of the cell
		int_set ep_set; // edge point set
		int_set_iter ce_itr = cell_itr->first_edge();
		for (; ce_itr != cell_itr->end_edge(); ++ce_itr) {
			ep_set.insert( *ce_itr + vsize );
		}
		// create each face according to each vertex of cell
		int_set_iter vert_iter = cell_itr->first_vert();
		for (; vert_iter != cell_itr->end_vert(); ++vert_iter) {
			
			int_set face_of_cell_set; // cell faces set 
			
			hs_vert& hex_vert = pmodel->vert_at(*vert_iter);
			int_set_iter vf_itr = hex_vert.first_face();
			for (; vf_itr != hex_vert.end_face(); ++vf_itr) {
				// iterate faces of the vertex to find the 3 faces 
				// in the cell according to face points of the cell
				hs_face& hex_face = pmodel->face_at(*vf_itr);
				int_set_iter vfv_itr = hex_face.first_vert();
				for (; vfv_itr != hex_face.end_vert(); ++vfv_itr) {
					int_set_iter fp_itr = fp_set.begin();
					for (; fp_itr != fp_set.end(); ++fp_itr) {
						if ( *fp_itr == *vfv_itr) {
							face_of_cell_set.insert(*vf_itr);
						} // if
					} // for fp_itr
				} // for vfv_itr
			} // for vf_itr
			
			// find 3 edge points adjacent to the vertex by 
			// iterating all adjacent edges of the vertex
			int_set ep_selected_set; // selected edge point set
			int_set_iter ve_itr = hex_vert.first_edge();
			for (; ve_itr != hex_vert.end_edge(); ++ve_itr) {
				int_set_iter ep_itr = ep_set.begin();
				for (; ep_itr != ep_set.end(); ++ep_itr) {
					hs_edge& hex_edge = pmodel->edge_at(*ve_itr);
					if ( *ep_itr == hex_edge.start_vert() || 
						*ep_itr == hex_edge.end_vert() ) {
						ep_selected_set.insert(*ep_itr);
					}
				}
			}
			
			// according the selected 3 edge points, find 3 cell faces of cell point by
			// iterating all its adjacent faces
			hs_vert& cell_point = pmodel->vert_at(cell_idx);
			int_set_iter focp_itr = cell_point.first_face(); // face of cell point
			for (; focp_itr != cell_point.end_face(); ++focp_itr) {
				hs_face& face = pmodel->face_at(*focp_itr);
				int_set_iter iter = face.first_vert();
				for (; iter != face.end_vert(); ++iter) {
					int_set_iter eps_itr = ep_selected_set.begin(); 
					for (; eps_itr != ep_selected_set.end(); ++eps_itr) {
						if ( *eps_itr == *iter ) {
							face_of_cell_set.insert(*focp_itr);
						}
					}
				}
			}
			
			assert( 6 == face_of_cell_set.size() );
			size_t* fidx_of_cell = new size_t[6];
			size_t* pfidx = fidx_of_cell;
			int_set_iter fiter = face_of_cell_set.begin();
			for (; fiter != face_of_cell_set.end(); ) {
				*pfidx++ = *fiter++;
			}
			
			pmodel->add_cell_by_faces(fidx_of_cell, 6);
			delete[] fidx_of_cell;
		}
	}
}


// hexahedral interpolate subdivision
void hs_subdiv::inter_subdiv() {
	
	point_vector vert_points;
	point_vector edge_points;
	point_vector face_points;
	point_vector cell_points;
	
	calc_cell_point(cell_points);
	calc_face_point(face_points, cell_points);
	calc_edge_point(edge_points, cell_points);
	calc_vert_point(vert_points, cell_points);
	
	// new model
	hs_model* p_new_model = new hs_model;
	
	create_vert_list(p_new_model, vert_points, edge_points, face_points, cell_points); 
	create_edge_list(p_new_model);
	create_face_list(p_new_model);
	create_cell_list(p_new_model);
	
	
	delete phexmodel;
	phexmodel = p_new_model;
	
	++subdiv_counter;
}

void hs_subdiv::print_info() const { 
	std::cout << "########################################" << std::endl;
	std::cout << "Number of Subdivision:         " << subdiv_counter << std::endl;
	std::cout << "Number of Laplacian Smoothing: " << lapalace_counter << std::endl;
	std::cout << "Number of Vertices:            " << phexmodel->vert_size() << std::endl;
	std::cout << "Number of Edges:               " << phexmodel->edge_size() << std::endl;
	std::cout << "Number of Faces:               " << phexmodel->face_size() << std::endl;
	std::cout << "Number of Cells:               " << phexmodel->cell_size() << std::endl;
}

void hs_subdiv::laplacian_smoothing_adaptive() {
	
	double change = laplacian_smoothing();
	double precision = change / 10.0;
	++lapalace_counter;
	size_t max_lap_smooth = 20;
	while ( change > precision && lapalace_counter < max_lap_smooth ) {
		change = laplacian_smoothing();  
		++lapalace_counter;
	} 
}

// calculate the jacobian value of each cell
// it is minimal mixed product of each vertex of cell
void hs_subdiv::jacobian( std::vector<double>& jacob) {
	
	cell_vector_iter cell_itr = phexmodel->first_cell();
	for (; cell_itr != phexmodel->end_cell(); ++cell_itr) {
		int_set vert_set;
		double min_jacob = std::numeric_limits<double>::max();
		int_set_iter vitr = cell_itr->first_vert();
		for (; vitr != cell_itr->end_vert(); ++vitr) {
			int_set_iter eitr = cell_itr->first_edge();
			for (; eitr != cell_itr->end_edge(); ++eitr) {
				hs_edge& he = phexmodel->edge_at(*eitr);
				size_t sp = he.start_vert();
				size_t ep = he.end_vert();
				if ( *vitr == sp ) {
					vert_set.insert(ep);
				} else if ( *vitr == ep ) {
					vert_set.insert(sp);
				}
			} 
			
			assert( 3 == vert_set.size() );
			hs_point vec[3];
			hs_point vert_pos = phexmodel->vert_at(*vitr).coord();
			int_set_iter vsitr = vert_set.begin();
			for (size_t i = 0; vsitr != vert_set.end(); ++vsitr) {
				vec[i] = phexmodel->vert_at(*vsitr).coord();
				vec[i] -= vert_pos; 
				vec[i++].unit();
			}
			double jac = vec[0].dot( vec[1].cross( vec[2] ) );
			if ( jac < 0) {
				jac = vec[0].dot( vec[2].cross( vec[1] ) );
			}
			min_jacob = jac < min_jacob ? jac : min_jacob;
			vert_set.clear();
		}
		
		jacob.push_back(min_jacob);
	}
}


