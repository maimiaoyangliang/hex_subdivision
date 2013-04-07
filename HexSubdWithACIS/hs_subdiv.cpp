#pragma warning(disable:4786)

#include <SET>
#include <IOSTREAM>
#include <CASSERT>

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


namespace hex_subdiv {
	
	hs_subdiv::hs_subdiv(hs_model* hex_model)
		: phexmodel(new hs_model(*hex_model))
		, subdiv_counter(0) {}
	
	hs_subdiv::~hs_subdiv() {
		if (phexmodel)
			delete phexmodel;
	}
	

	
	// calculate cell point coordinate by iterating each cell
	void hs_subdiv::calc_cell_point(point_vector& cell_points) {
		
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
			
			if (face_iter->fst_cell() != -1 && face_iter->snd_cell() != -1) { 
				// inner face
				cent = cent * 0.5 + (cell_points[face_iter->fst_cell()]
					+ cell_points[face_iter->snd_cell()]) * 0.25;
			} else { 
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
	void hs_subdiv::calc_edge_point(point_vector& edge_points
								  , point_vector& face_points
								  , point_vector& cell_points
								  ) {

		hs_point cent;
		edge_vector_iter edge_iter = phexmodel->first_edge();
		for (; edge_iter != phexmodel->end_edge(); ++edge_iter) {
			// centroid of each edge
			cent = ( phexmodel->vert_at(edge_iter->start_vert()).coord() + 
				phexmodel->vert_at(edge_iter->end_vert()).coord() ) * 0.5;

			size_t csize = edge_iter->cell_size();
			size_t fsize = edge_iter->face_size(); 

			if (csize == fsize) {
				// inner edge
				hs_point avg_face_point;
				int_set_iter face_iter = edge_iter->first_face();
				for (; face_iter != edge_iter->end_face(); ++face_iter) {
					avg_face_point += face_points[*face_iter] ;
				}
				
				avg_face_point /= fsize;
				
				hs_point avg_cell_point;
				int_set_iter cell_iter = edge_iter->first_cell();
				for (; cell_iter != edge_iter->end_cell(); ++cell_iter) {
					avg_cell_point += cell_points[*cell_iter];
				}
				
				avg_cell_point /= csize;
				// average of midpoint, face point and cell point
				cent = (cent + avg_face_point + avg_cell_point) / 3;
				
			} else {

				SPAposition spa_pos( cent.x(), cent.y(), cent.z());
				
				if ( 2 == fsize ) { 
					// crease edge
					// find a vertex in the curve which is adjacent to crease edge
					assert( edge_iter->acis_edge() );
					const curve& ccurv = edge_iter->acis_edge()->geometry()->equation();
					SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
					cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
					
				} else { 
					// ordinary border edge
					// find a vertex in the surface which is adjacent to edge
					std::set< const FACE* > face_set;
					int_set_iter face_iter = edge_iter->first_face();
					for (; face_iter != edge_iter->end_face(); ++face_iter) {
						face_set.insert( phexmodel->face_at(*face_iter).acis_face() );
						/*spa_face = phexmodel->face_at(*face_iter).acis_face();
						if (spa_face) {
							const surface& csurf = spa_face->geometry()->equation();
							SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
							hs_point new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
							cent = new_pos;
							break;
						}*/
					}
					// can not find a surface adjacent to edge
					//assert( face_iter == edge_iter->end_face() || spa_face != NULL );

					face_set.erase(NULL);
					assert( 1 == face_set.size() );
					const FACE* spa_face = *(face_set.begin());
					const surface& csurf = spa_face->geometry()->equation();
					SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
					cent.set_coord( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );	
				}
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
	void hs_subdiv::calc_vert_point(point_vector& vert_points
								  , point_vector& edge_points
								  , point_vector& face_points
								  , point_vector& cell_points
								  ) {
		
		hs_point cent;
		vert_vector_iter vert_iter = phexmodel->first_vert();
		for (; vert_iter != phexmodel->end_vert(); ++ vert_iter) {

			cent = vert_iter->coord();

			int fsize = vert_iter->face_size();
			if ( 12 == fsize ) {
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
					avg_face_point += face_points[*face_iter];
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
				
				cent = (cent * (fsize - 6) + avg_mid_edge * 3 
					+ avg_face_point * 2 + avg_cell_point) / fsize;
				
			} else {
				// border point
				int_set_iter edge_iter = vert_iter->first_edge();
				const EDGE* spa_edge = NULL;
				for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
					spa_edge = phexmodel->edge_at(*edge_iter).acis_edge();
					if (spa_edge) break;
				}
				
				SPAposition spa_pos( cent.x(), cent.y(), cent.z());
				
				if (spa_edge) {  // the vertex locate in border edge which has geometry curve
					const curve& ccurv = spa_edge->geometry()->equation();
					SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
					hs_point new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
					cent = new_pos;
				} else { // the vertex locate in border edge which which has geometry surface instead of curve
					int_set_iter face_iter = vert_iter->first_face();
					for (; face_iter != vert_iter->end_face(); ++face_iter) {
						const FACE* spa_face = phexmodel->face_at(*face_iter).acis_face();
						if (spa_face) {
							const surface& csurf = spa_face->geometry()->equation();
							SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
							hs_point new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
							cent = new_pos;
							break;
						}
					}
				}
			}
			
			vert_points.push_back(cent);
			
		}
		
	}
	
	
	
	
	/************************************************************************/
	/* laplacian smoothing interior points                                  */
	/************************************************************************/
	void hs_subdiv::laplacian_smoothing(hs_model* pNewModel) {
		if ( !pNewModel ) return;
		
		int_set vert_set;
		size_t vert_ind = 0;
		vert_vector_iter vert_iter = pNewModel->first_vert();
		for (; vert_iter != pNewModel->end_vert(); ++vert_iter, ++vert_ind) {
			if ( 6 == vert_iter->edge_size() ) {
				int_set_iter edge_iter = vert_iter->first_edge();
				for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
					hs_edge& hex_edge = pNewModel->edge_at(*edge_iter);
					vert_set.insert( hex_edge.start_vert() );
					vert_set.insert( hex_edge.end_vert() );
				}
				vert_set.erase(vert_ind);
				
				hs_point spoint;
				int_set_iter viter = vert_set.begin();
				for (; viter != vert_set.end(); ++viter) {
					spoint += pNewModel->vert_at(*viter).coord();
				}
				spoint /= vert_set.size();
				
				vert_iter->set_coord(spoint);
				vert_set.clear();
			}
			
		}
	}
	
	/************************************************************************/
	/* change new vertices to interpolate geometry model.                   */
	/************************************************************************/
	/*
	void Subdivision::ChangeNewVertices() {
	// find all border face
	size_t face_ind = 0;
	face_vector_iter face_iter = m_pHexModel->first_face();
	for (; face_iter != m_pHexModel->end_face(); ++face_iter, ++face_ind) {
	if (face_iter->snd_cell() == -1 || face_iter->fst_cell() == -1) {
	
	  SPAposition spa_pos( facePoints[face_ind].x(), 
	  facePoints[face_ind].y(), 
	  facePoints[face_ind].z() );
	  
		assert(face_iter->acis_face() != NULL); 
		const surface& csurf = face_iter->acis_face()->geometry()->equation();
		SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
		Point3D new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
		
		  std::cout << "new pos" << std::endl;
		  new_pos.print();
		  facePoints[face_ind].print();
		  
			facePoints[face_ind] = new_pos;
			
			  }
			  }
			  
				// find all border edge
				size_t edge_ind = 0;
				edge_vector_iter edge_iter = m_pHexModel->first_edge();
				for (; edge_iter != m_pHexModel->end_edge(); ++edge_iter, ++edge_ind) {
				// test border edge
				if (edge_iter->cell_size() != edge_iter->face_size()) {
				SPAposition spa_pos( edgePoints[edge_ind].x(), 
				edgePoints[edge_ind].y(), 
				edgePoints[edge_ind].z());
				
				  if (edge_iter->acis_edge()) { // border edge which has geometry curve
				  const curve& ccurv = edge_iter->acis_edge()->geometry()->equation();
				  SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
				  Point3D new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
				  edgePoints[edge_ind] = new_pos;
				  } else { // border edge which has geometry surface instead of curve
				  int_set_iter face_iter = edge_iter->first_face();
				  for (; face_iter != edge_iter->end_face(); ++face_iter) {
				  const FACE* spa_face = m_pHexModel->face(*face_iter).acis_face();
				  if (spa_face) {
				  const surface& csurf = spa_face->geometry()->equation();
				  SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
				  Point3D new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
				  edgePoints[edge_ind] = new_pos;
				  break;
				  }
				  }
				  }
				  
					}
					}
					
					  // find all border vertex
					  size_t vert_ind = 0;
					  vert_vector_iter vert_iter = m_pHexModel->first_vert();
					  for (; vert_iter != m_pHexModel->end_vert(); ++vert_iter, ++vert_ind) {
					  if (12 != vert_iter->cell_size()) {
					  int_set_iter edge_iter = vert_iter->first_edge();
					  EDGE* spa_edge = NULL;
					  for (; edge_iter != vert_iter->end_edge(); ++edge_iter) {
					  spa_edge = m_pHexModel->edge(*edge_iter).acis_edge();
					  if (spa_edge) break;
					  }
					  
						SPAposition spa_pos( newVertices[vert_ind].x(), 
						newVertices[vert_ind].y(), 
						newVertices[vert_ind].z());
						
						  if (spa_edge) {  // the vertex locate in border edge which has geometry curve
						  const curve& ccurv = spa_edge->geometry()->equation();
						  SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
						  Point3D new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
						  newVertices[vert_ind] = new_pos;
						  } else { // the vertex locate in border edge which which has geometry surface instead of curve
						  int_set_iter face_iter = vert_iter->first_face();
						  for (; face_iter != vert_iter->end_face(); ++face_iter) {
						  const FACE* spa_face = m_pHexModel->face(*face_iter).acis_face();
						  if (spa_face) {
						  const surface& csurf = spa_face->geometry()->equation();
						  SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
						  Point3D new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
						  newVertices[vert_ind] = new_pos;
						  break;
						  }
						  }
						  }
						  
							}
							}
							}
*/


/************************************************************************/
/*             new vertex link                                          */
/************************************************************************/
void hs_subdiv::create_vert_list(hs_model* pNewModel,
								 point_vector& newVertices, 
								 point_vector& edgePoints, 
								 point_vector& facePoints, 
								 point_vector& cellPoints) {
	
	hs_vert vert;
	std::vector<hs_point>::const_iterator newVertIter;
	
	newVertIter = newVertices.begin();
	for (; newVertIter != newVertices.end(); ++newVertIter) {
		vert.set_coord(*newVertIter);
		pNewModel->add_vert(vert);
	}
	
	newVertIter = edgePoints.begin();
	for (; newVertIter != edgePoints.end(); ++newVertIter) {
		vert.set_coord(*newVertIter);
		pNewModel->add_vert(vert);
	}
	
	newVertIter = facePoints.begin();
	for (; newVertIter != facePoints.end(); ++newVertIter) {
		vert.set_coord(*newVertIter);
		pNewModel->add_vert(vert);
	}
	
	newVertIter = cellPoints.begin();
	for (; newVertIter != cellPoints.end(); ++newVertIter) {
		vert.set_coord(*newVertIter);
		pNewModel->add_vert(vert);
	}
	
}
/************************************************************************/
/*    new edge link                                                     */
/************************************************************************/
void hs_subdiv::create_edge_list(hs_model* pNewModel, 
								 point_vector& newVertices, 
								 point_vector& edgePoints,
								 point_vector& facePoints) {
	
	hs_edge edge;
	int vertIndex = 0;
	
	int pos1, pos2;
	edge_vector_iter edgeIter = phexmodel->first_edge();
	for (; edgeIter != phexmodel->end_edge(); ++edgeIter, ++vertIndex) {
		// one old edge split to new edges
		edge.set_acis_edge( const_cast< EDGE* >( edgeIter->acis_edge() ) );
		
		pos1 = edgeIter->start_vert();
		pos2 = vertIndex + newVertices.size();
		pNewModel->add_edge(pos1, pos2);
		
		pos1 = edgeIter->end_vert();
		pNewModel->add_edge(pos1, pos2);
		
	}
	
	vertIndex = 0;
	face_vector_iter faceIter = phexmodel->first_face();
	for (; faceIter != phexmodel->end_face(); ++faceIter, ++vertIndex) {
		// create four new edges in one face
		pos2 = vertIndex + newVertices.size() + edgePoints.size();
		
		int_set_iter edgeOfFaceIter = faceIter->first_edge();
		for (; edgeOfFaceIter != faceIter->end_edge(); ++edgeOfFaceIter) {
			pos1 = *edgeOfFaceIter + newVertices.size();
			
			pNewModel->add_edge(pos1, pos2);
		}
	}
	
	vertIndex = 0;
	cell_vector_iter cellIter = phexmodel->first_cell();
	for (; cellIter != phexmodel->end_cell(); ++cellIter, ++vertIndex) {
		// create six new edges in one cell
		pos2 = vertIndex + newVertices.size() + edgePoints.size() + facePoints.size();
		
		int_set_iter faceOfCellIter = cellIter->first_face();
		for (; faceOfCellIter != cellIter->end_face(); ++faceOfCellIter) {
			pos1 = *faceOfCellIter + newVertices.size() + edgePoints.size();
			
			pNewModel->add_edge(pos1, pos2);
			
		}
	}
	
}
/************************************************************************/
/*    new face link                                                     */
/************************************************************************/
void hs_subdiv::create_face_list(hs_model* pNewModel,
								 point_vector& newVertices,
								 point_vector& edgePoints,
								 point_vector& facePoints) {
	
	// in every face, it will subdivide one face to four faces
	int_set_iter SetIntIter;
	int i, j;
	
	int newFaceIndex;
	int faceIndex = 0;
	const int numOfNewFacesInFace = 4;
	hs_face faceInFace[numOfNewFacesInFace];
	face_vector_iter faceIter = phexmodel->first_face();
	for (; faceIter != phexmodel->end_face(); ++faceIter, ++faceIndex) {
		
		int facePointIndex = faceIndex + newVertices.size() + edgePoints.size();
		
		int vertexIndexInFace[4];
		SetIntIter = faceIter->first_vert();
		for (i = 0; SetIntIter != faceIter->end_vert() && i < 4; ++SetIntIter, ++i) {
			vertexIndexInFace[i] = *SetIntIter;				
		}
		
		int edgePointIndexInFace[4];
		SetIntIter = faceIter->first_edge();
		for (i = 0; SetIntIter != faceIter->end_edge() && i < 4; ++SetIntIter, ++i) {
			edgePointIndexInFace[i] = *SetIntIter + newVertices.size();
		}
		
		for (i = 0; i < numOfNewFacesInFace; ++ i) {
			
			// vertices in face
			faceInFace[i].add_vert(facePointIndex);
			faceInFace[i].add_vert(vertexIndexInFace[i]);
			
			int edgePointIndexInIthFace[2];
			int k = 0;
			
			SetIntIter = pNewModel->vert_at(vertexIndexInFace[i]).first_edge();
			for (; SetIntIter != pNewModel->vert_at(vertexIndexInFace[i]).end_edge(); ++SetIntIter) {
				for (j = 0; j < 4; ++j) {
					if (pNewModel->edge_at(*SetIntIter).start_vert() == edgePointIndexInFace[j] ||
						pNewModel->edge_at(*SetIntIter).end_vert() == edgePointIndexInFace[j]) {
						
						if (k < 2)
							edgePointIndexInIthFace[k++] = edgePointIndexInFace[j];
						// put the vertex into face
						faceInFace[i].add_vert(edgePointIndexInFace[j]);
						// put the edge into face
						faceInFace[i].add_edge(*SetIntIter);
					}
				}
			}
			
			newFaceIndex = i + numOfNewFacesInFace * faceIndex;
			
			SetIntIter = faceInFace[i].first_vert();
			for (; SetIntIter != faceInFace[i].end_vert(); ++SetIntIter) {
				pNewModel->vert_at(*SetIntIter).add_face(newFaceIndex);
			}
			
			// edges in face
			SetIntIter = pNewModel->vert_at(facePointIndex).first_edge();
			for (; SetIntIter != pNewModel->vert_at(facePointIndex).end_edge(); ++SetIntIter) {
				for (k = 0; k < 2; ++ k) {
					if (pNewModel->edge_at(*SetIntIter).start_vert() == edgePointIndexInIthFace[k] ||
						pNewModel->edge_at(*SetIntIter).end_vert() == edgePointIndexInIthFace[k])
						faceInFace[i].add_edge(*SetIntIter);
				}
			}
			
			SetIntIter = faceInFace[i].first_edge();
			for (; SetIntIter != faceInFace[i].end_edge(); ++SetIntIter) {
				pNewModel->edge_at(*SetIntIter).add_face(newFaceIndex);
			}
			
			// four new faces has same acis_face with the old face
			faceInFace[i].set_acis_face(const_cast< FACE* >(faceIter->acis_face()));
			
			pNewModel->add_face(faceInFace[i]);
			
			faceInFace[i].clear_vert();
			faceInFace[i].clear_edge();
			
		}
		
	}
	
	// in every cell, it will create 12 new faces
	const int numOfNewFacesInCell = 12;
	hs_face faceInCell[numOfNewFacesInCell];
	int cellIndex = 0;
	cell_vector_iter cellIter = phexmodel->first_cell();
	for (; cellIter != phexmodel->end_cell(); ++cellIter, ++cellIndex) {
		
		int cellPointIndex = cellIndex + newVertices.size() + edgePoints.size() + facePoints.size();
		
		int facePointIndexInCell[6];
		SetIntIter = cellIter->first_face();
		for (i = 0; SetIntIter != cellIter->end_face() && i < 6; 
		++SetIntIter, ++i) {
			facePointIndexInCell[i] = *SetIntIter + newVertices.size() + edgePoints.size();
		}
		
		int edgePointIndexInCell[12];
		SetIntIter = cellIter->first_edge();
		for (i = 0; SetIntIter != cellIter->end_edge() && i < 12; 
		++SetIntIter, ++i) {
			edgePointIndexInCell[i] = *SetIntIter + newVertices.size();
		}
		
		for (i = 0; i < numOfNewFacesInCell; ++ i) {
			
			faceInCell[i].add_vert(cellPointIndex);
			faceInCell[i].add_vert(edgePointIndexInCell[i]);
			
			int facePointIndexInIthCell[2];
			int k = 0;
			
			SetIntIter = pNewModel->vert_at(edgePointIndexInCell[i]).first_edge();
			for (; SetIntIter != pNewModel->vert_at(edgePointIndexInCell[i]).end_edge();
			++SetIntIter) {
				
				for (j = 0; j < 6; ++ j) {
					
					if (pNewModel->edge_at(*SetIntIter).start_vert() == facePointIndexInCell[j] ||
						pNewModel->edge_at(*SetIntIter).end_vert() == facePointIndexInCell[j]) {
						if (k < 2) 
							facePointIndexInIthCell[k++] = facePointIndexInCell[j];
						faceInCell[i].add_vert(facePointIndexInCell[j]);
						
						faceInCell[i].add_edge(*SetIntIter);
					}
				}
				
			} 
			
			newFaceIndex = i + numOfNewFacesInCell * cellIndex + numOfNewFacesInFace * faceIndex;
			
			SetIntIter = faceInCell[i].first_vert();
			for (; SetIntIter != faceInCell[i].end_vert(); ++SetIntIter) {
				pNewModel->vert_at(*SetIntIter).add_face(newFaceIndex);
			}
			// edges in face
			SetIntIter = pNewModel->vert_at(cellPointIndex).first_edge();
			for (; SetIntIter != pNewModel->vert_at(cellPointIndex).end_edge(); ++SetIntIter) {
				for (k = 0; k < 2; ++k) {
					if (pNewModel->edge_at(*SetIntIter).start_vert() == facePointIndexInIthCell[k] ||
						pNewModel->edge_at(*SetIntIter).end_vert() == facePointIndexInIthCell[k])
						faceInCell[i].add_edge(*SetIntIter);
				}
			}
			
			SetIntIter = faceInCell[i].first_edge();
			for (; SetIntIter != faceInCell[i].end_edge(); ++SetIntIter) {
				pNewModel->edge_at(*SetIntIter).add_face(newFaceIndex);
			}
			
			faceInCell[i].set_acis_face(static_cast< FACE* >(NULL));
			
			pNewModel->add_face(faceInCell[i]);
			
			faceInCell[i].clear_vert();
			faceInCell[i].clear_edge();
			
		}
	}
}
/************************************************************************/
/*  new cell link                                                       */
/************************************************************************/
void hs_subdiv::create_cell_list(hs_model* pNewModel,
								 point_vector& newVertices,
								 point_vector& edgePoints,
								 point_vector& facePoints) {
	
	int_set_iter iter;
	int i, j;
	
	int newCellIndex;
	const int numOfNewCellsInCell = 8;
	hs_cell cellInCell[numOfNewCellsInCell];
	int cellIndex = 0;
	cell_vector_iter cellIter = phexmodel->first_cell();
	for (; cellIter != phexmodel->end_cell(); ++cellIter, ++cellIndex) {
		
		int cellPointIndex = cellIndex + newVertices.size() + edgePoints.size() + facePoints.size();
		
		int vertexIndexInCell[8];
		iter = cellIter->first_vert();
		for (i = 0; iter != cellIter->end_vert() && i < 8;
		++iter, ++i) {
			vertexIndexInCell[i] = *iter;
		}
		
		int edgePointIndexInCell[12];
		iter = cellIter->first_edge();
		for (i = 0; iter != cellIter->end_edge() && i < 12;
		++iter, ++i) {
			edgePointIndexInCell[i] = *iter + newVertices.size();
		}
		
		int facePointIndexInCell[6];
		iter = cellIter->first_face();
		for (i = 0; iter != cellIter->end_face() && i < 6; ++iter, ++i) {
			facePointIndexInCell[i] = *iter + newVertices.size() + edgePoints.size();
		}
		
		for (i = 0; i < numOfNewCellsInCell; ++ i) {
			
			// vertex
			cellInCell[i].add_vert(cellPointIndex);
			cellInCell[i].add_vert(vertexIndexInCell[i]);
			
			int k = 0;
			int edgePointIndexInIthCell[3];
			iter = pNewModel->vert_at(vertexIndexInCell[i]).first_edge();
			for (; iter != pNewModel->vert_at(vertexIndexInCell[i]).end_edge(); ++iter)
			{
				for (j = 0; j < 12; ++ j) 
				{
					if (pNewModel->edge_at(*iter).start_vert() == edgePointIndexInCell[j] ||
						pNewModel->edge_at(*iter).end_vert() == edgePointIndexInCell[j])
					{
						if (k < 3)
							edgePointIndexInIthCell[k++] = edgePointIndexInCell[j];
						
						cellInCell[i].add_vert(edgePointIndexInCell[j]);
						
						cellInCell[i].add_edge(*iter);
					}
				}
			}
			
			k = 0;
			int facePointIndexInIthCell[3];
			iter = pNewModel->vert_at(vertexIndexInCell[i]).first_face();
			for (; iter != pNewModel->vert_at(vertexIndexInCell[i]).end_face(); ++iter)
			{
				for (j = 0; j < 6; ++ j) 
				{
					int_set_iter vertOfFaceIter = pNewModel->face_at(*iter).first_vert();
					for (; vertOfFaceIter != pNewModel->face_at(*iter).end_vert(); ++vertOfFaceIter)
					{
						if (*vertOfFaceIter == facePointIndexInCell[j]) 
						{
							if (k < 3) 
								facePointIndexInIthCell[k++] = facePointIndexInCell[j];
							
							cellInCell[i].add_vert(facePointIndexInCell[j]);
							
							cellInCell[i].add_face(*iter);
							
						}
					}
				}
			}
			
			newCellIndex = i + numOfNewCellsInCell * cellIndex;
			
			iter = cellInCell[i].first_vert();
			for (; iter != cellInCell[i].end_vert(); ++ iter) {
				pNewModel->vert_at(*iter).add_cell(newCellIndex);
			}
			
			// edge
			for (k = 0; k < 3; ++ k) {
				iter = pNewModel->vert_at(facePointIndexInIthCell[k]).first_edge();
				for (; iter != pNewModel->vert_at(facePointIndexInIthCell[k]).end_edge(); ++iter) {
					for (j = 0; j < 3; ++j) {
						if (pNewModel->edge_at(*iter).start_vert() == edgePointIndexInIthCell[j] ||
							pNewModel->edge_at(*iter).end_vert() == edgePointIndexInIthCell[j]) {
							cellInCell[i].add_edge(*iter);
						}
					}
					
					if (pNewModel->edge_at(*iter).start_vert() == cellPointIndex ||
						pNewModel->edge_at(*iter).end_vert() == cellPointIndex) {
						cellInCell[i].add_edge(*iter);
					}
				}
			}
			
			iter = cellInCell[i].first_edge();
			for (; iter != cellInCell[i].end_edge(); ++iter) {
				pNewModel->edge_at(*iter).add_cell(newCellIndex);
			}
			
			// face
			iter = pNewModel->vert_at(cellPointIndex).first_face();
			for (; iter != pNewModel->vert_at(cellPointIndex).end_face(); ++iter) {
				int_set_iter vertOfFaceIter = pNewModel->face_at(*iter).first_vert();
				for (; vertOfFaceIter != pNewModel->face_at(*iter).end_vert(); ++vertOfFaceIter) {
					for (j = 0; j < 3; ++j) {
						if (*vertOfFaceIter == edgePointIndexInIthCell[j]) {
							cellInCell[i].add_face(*iter);
						}
					}
				}
			}
			
			iter = cellInCell[i].first_face();
			for (; iter != cellInCell[i].end_face(); ++iter) {
				if (pNewModel->face_at(*iter).fst_cell() == -1) 
					pNewModel->face_at(*iter).set_fst_cell(newCellIndex);
				else if (pNewModel->face_at(*iter).snd_cell() == -1) {
					pNewModel->face_at(*iter).set_snd_cell(newCellIndex);
				}
			}
			
			pNewModel->add_cell(cellInCell[i]);
			
			cellInCell[i].clear_all();
			
		}
		
	}
}

/************************************************************************/
/*  Hexahedron Approximating Subdivision                                */
/************************************************************************/
void hs_subdiv::inter_subdiv(ENTITY_LIST& elist) {
	
	point_vector newVertices;
	point_vector edgePoints;
	point_vector facePoints;
	point_vector cellPoints;
	
	calc_cell_point(cellPoints);
	calc_face_point(facePoints, cellPoints);
	calc_edge_point(edgePoints, facePoints, cellPoints);
	calc_vert_point(newVertices, edgePoints, facePoints, cellPoints);
	
	//	ChangeNewVertices();
	
	// new model
	hs_model* pNewModel = new hs_model;
	
	create_vert_list(pNewModel, newVertices, edgePoints, facePoints, cellPoints);
	create_edge_list(pNewModel, newVertices, edgePoints, facePoints);
	create_face_list(pNewModel, newVertices, edgePoints, facePoints);
	create_cell_list(pNewModel, newVertices, edgePoints, facePoints);
	
	const size_t LAP_NUM = 5;
	for (size_t i = 0; i < LAP_NUM; ++i) {
		laplacian_smoothing(pNewModel);
	}
	
	
	pNewModel->acis_wire(elist);
	
	//	FreeMemory();
	
	delete phexmodel;
	phexmodel = pNewModel;
	
	print_info();
}

void hs_subdiv::print_info() {
	std::cout << std::endl;
	std::cout << "####### subdivision counter: " << ++subdiv_counter << " #######" << std::endl;
	std::cout << "#vertices: " << phexmodel->vert_size() << std::endl;
	std::cout << "#edges:    " << phexmodel->edge_size() << std::endl;
	std::cout << "#faces:    " << phexmodel->face_size() << std::endl;
	std::cout << "#cells:    " << phexmodel->cell_size() << std::endl;
}


/************************************************************************/
/*  Hexahedron Interpolating Subdivision                                */
/************************************************************************/
/*void Subdivision::InterpolateSubdivision() {

  CalcCellPoints();
  CalcFacePointsInter();
  //CalcFacePoints();
  CalcEdgePointsInter();
  CalcNewVerticesInter();
  
	// new model
	HexModel* pNewModel = new HexModel;
	
	  NewVertexList(pNewModel);
	  NewEdgeList(pNewModel);
	  NewFaceList(pNewModel);
	  NewCellList(pNewModel);
	  
		FreeMemory();
		
		  delete m_pHexModel;
		  m_pHexModel = pNewModel;
		  
			std::cerr << std::endl;
			std::cerr << "####### subdivision counter: " << ++subdiv_counter << " #######" << std::endl;
			std::cerr << "#vertices: " << m_pHexModel->vert_size() << std::endl;
			std::cerr << "#edges:    " << m_pHexModel->edge_size() << std::endl;
			std::cerr << "#faces:    " << m_pHexModel->face_size() << std::endl;
			std::cerr << "#cells:    " << m_pHexModel->cell_size() << std::endl;
}*/

}
