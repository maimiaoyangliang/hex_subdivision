#pragma warning(disable:4786)

#include <SET>
#include <IOSTREAM>
#include <CASSERT>

#include "HexCell.h"
#include "HexModel.h"
#include "HexFace.h"
#include "HexEdge.h"
#include "HexVertex.h"
#include "Subdivision.h"

#include "alltop.hxx"
#include "surdef.hxx"
#include "surface.hxx"
#include "position.hxx"
#include "curve.hxx"
#include "curdef.hxx"
#include "lists.hxx"


namespace hex_subdiv {
	
	hs_subdiv::hs_subdiv(hex_subdiv::hs_model* hex_model)
		: phexmodel(new hs_model(*hex_model))
		, subdiv_counter(0) {}
	
	hs_subdiv::~hs_subdiv() {
		//		FreeMemory();
		if (phexmodel)
			delete phexmodel;
	}
	
	/*
	void Subdivision::FreeMemory() {
	newVertices.clear();
	edgePoints.clear();
	facePoints.clear();
	cellPoints.clear();
	}
	*/
	
	
	/************************************************************************/
	/* calculate cell points after volume subdivision                       */
	/************************************************************************/
	void hs_subdiv::calc_cell_point(std::vector<hs_point>& cellPoints) {
		
		hs_point cent;
		std::vector<hs_cell>::iterator cellIter = phexmodel->first_cell();
		for (; cellIter != phexmodel->end_cell(); ++ cellIter) {
			cent.zero();
			int_set_iter vertexOfCellIter = cellIter->first_vert();
			for (; vertexOfCellIter != cellIter->end_vert(); ++ vertexOfCellIter) {
				cent += phexmodel->vert_at(*vertexOfCellIter).coord();
			}
			cent /= cellIter->vert_size();
			cellPoints.push_back(cent);
		}
	}
	/************************************************************************/
	/* calculate face points after approximating subdivision                */
	/************************************************************************/
	void hs_subdiv::calc_face_point(std::vector<hs_point>& facePoints, 
		std::vector<hs_point>& cellPoints) {
		
		hs_point cent;
		face_vector_iter faceIter = phexmodel->first_face();
		for (; faceIter != phexmodel->end_face(); ++faceIter) {
			cent.zero();
			int_set_iter vertexOfFaceIter = faceIter->first_vert();
			for (; vertexOfFaceIter != faceIter->end_vert(); ++vertexOfFaceIter) {
				cent += phexmodel->vert_at(*vertexOfFaceIter).coord();
			}
			cent /= faceIter->vert_size();
			
			if (faceIter->fst_cell() != -1 && faceIter->snd_cell() != -1) {
				cent = cent * 0.5 + (cellPoints[faceIter->fst_cell()]
					+ cellPoints[faceIter->snd_cell()]) * 0.25;
			} else { // locate the vertex in the surface
				SPAposition spa_pos( cent.x(), cent.y(), cent.z() );
				
				assert( NULL != faceIter->acis_face() ); 
				const surface& csurf = faceIter->acis_face()->geometry()->equation();
				SPAposition new_spa_pos( csurf.eval_position( csurf.param(spa_pos) ) );
				hs_point new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
				
				cent = new_pos;
			}
			
			facePoints.push_back(cent);

		}
	}
	
	
	/************************************************************************/
	/* calculate edge points after approximating subdivision                */
	/************************************************************************/
	void hs_subdiv::calc_edge_point(std::vector<hs_point>& edgePoints,
		std::vector<hs_point>& facePoints, 
		std::vector<hs_point>& cellPoints) {
		
		// calculate edge point
		hs_point cent;
		edge_vector_iter edgeIter = phexmodel->first_edge();
		for (; edgeIter != phexmodel->end_edge(); ++edgeIter) {
			cent = (phexmodel->vert_at(edgeIter->start_vert()).coord() + 
				phexmodel->vert_at(edgeIter->end_vert()).coord()) * 0.5;
			int numOfCells = edgeIter->cell_size();
			int numOfFaces = edgeIter->face_size(); 
			// interior edge
			if (numOfCells == numOfFaces) {
				
				hs_point avgFacePoint;
				int_set_iter faceWithEdgeIter = edgeIter->first_face();
				for (; faceWithEdgeIter != edgeIter->end_face(); ++faceWithEdgeIter) {
					avgFacePoint += facePoints[*faceWithEdgeIter] ;
				}
				
				avgFacePoint /= numOfFaces;
				
				hs_point avgCellPoint;
				int_set_iter cellWithEdgeIter = edgeIter->first_cell();
				for (; cellWithEdgeIter != edgeIter->end_cell(); ++cellWithEdgeIter) {
					avgCellPoint += cellPoints[*cellWithEdgeIter];
				}
				
				avgCellPoint /= numOfCells;
				
				cent = (cent + avgFacePoint + avgCellPoint) / 3;
				
				}/* else {
				 
				   Point3D avgBorderFacePoint;
				   int coutBorderFaces = 0;
				   std::set<int>::const_iterator faceWithEdgeIter = edgeIter->first_face();
				   for (; faceWithEdgeIter != edgeIter->end_face(); ++faceWithEdgeIter) {
				   if (m_pHexModel->face(*faceWithEdgeIter).fst_cell() == -1 ||
				   m_pHexModel->face(*faceWithEdgeIter).snd_cell() == -1) {
				   avgBorderFacePoint += facePoints[*faceWithEdgeIter];
				   ++coutBorderFaces;
				   }
				   }
				   
					 avgBorderFacePoint /= coutBorderFaces;
					 
					   cent = (cent + avgBorderFacePoint) * 0.5;
					   
		}*/
			else {
				SPAposition spa_pos( cent.x(), 
					cent.y(), 
					cent.z());
				
				if (edgeIter->acis_edge()) { // border edge which has geometry curve
					const curve& ccurv = edgeIter->acis_edge()->geometry()->equation();
					SPAposition new_spa_pos( ccurv.eval_position( ccurv.param(spa_pos) ) );
					hs_point new_pos( new_spa_pos.x(), new_spa_pos.y(), new_spa_pos.z() );
					cent = new_pos;
				} else { // border edge which has geometry surface instead of curve
					int_set_iter face_iter = edgeIter->first_face();
					for (; face_iter != edgeIter->end_face(); ++face_iter) {
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
			
			
			edgePoints.push_back(cent);

		}
		
	}
	
	
	
	/************************************************************************/
	/* calculate new vertices after approximating subdivision               */
	/************************************************************************/
	void hs_subdiv::calc_vert_point(std::vector<hs_point>& newVertices,
		std::vector<hs_point>& edgePoints,
		std::vector<hs_point>& facePoints,
		std::vector<hs_point>& cellPoints) {
		
		hs_point cent;
		vert_vector_iter vertexIter = phexmodel->first_vert();
		for (; vertexIter != phexmodel->end_vert(); ++ vertexIter) 
		{
			cent = vertexIter->coord();
			int numOfFacesWithVert = vertexIter->face_size();
			
			if (numOfFacesWithVert == 12) // border vertex
			{
				hs_point avgCellPoint;
				int_set_iter cellWithVertIter = vertexIter->first_cell();
				for (; cellWithVertIter != vertexIter->end_cell(); ++cellWithVertIter) {
					avgCellPoint += cellPoints[*cellWithVertIter];
				}
				
				avgCellPoint /= vertexIter->cell_size();
				
				hs_point avgFacePoint;
				int_set_iter faceWithVertIter = vertexIter->first_face();
				for (; faceWithVertIter != vertexIter->end_face(); ++faceWithVertIter) {
					avgFacePoint += facePoints[*faceWithVertIter];
				}
				
				avgFacePoint /= vertexIter->face_size();
				
				hs_point avgEdgeMid;
				int_set_iter edgeWithVertIter = vertexIter->first_edge();
				for (; edgeWithVertIter != vertexIter->end_edge(); ++ edgeWithVertIter) {
					avgEdgeMid += (phexmodel->vert_at(phexmodel->edge_at(*edgeWithVertIter).start_vert()).coord() +
						phexmodel->vert_at(phexmodel->edge_at(*edgeWithVertIter).end_vert()).coord()) * 0.5;
				}
				
				avgEdgeMid /= vertexIter->edge_size();
				
				cent = (cent * (numOfFacesWithVert - 6) + avgEdgeMid * 3 
					+ avgFacePoint * 2 + avgCellPoint) / numOfFacesWithVert;
				
				
					}/* else {
					 
					   int numOfBorderFaces = 0;
					   Point3D avgBorderFacePoint;
					   std::set<int>::const_iterator borderFaceWithVertIter = vertexIter->first_face();
					   for (; borderFaceWithVertIter != vertexIter->end_face(); ++borderFaceWithVertIter)
					   {
					   if (m_pHexModel->face(*borderFaceWithVertIter).fst_cell() == -1 ||
					   m_pHexModel->face(*borderFaceWithVertIter).snd_cell() == -1)
					   {
					   ++ numOfBorderFaces;
					   avgBorderFacePoint += facePoints[*borderFaceWithVertIter];
					   }
					   }
					   
						 avgBorderFacePoint /= numOfBorderFaces;
						 
						   int numOfBorderEdges = 0;
						   Point3D avgBorderEdgeMid;
						   std::set<int>::const_iterator borderEdgeWithVertIter = vertexIter->first_edge();
						   for (; borderEdgeWithVertIter != vertexIter->end_edge(); ++borderEdgeWithVertIter)
						   {
						   if (m_pHexModel->edge(*borderEdgeWithVertIter).face_size() != 
						   m_pHexModel->edge(*borderEdgeWithVertIter).cell_size()) 
						   {
						   avgBorderEdgeMid += 
						   (m_pHexModel->vertex(m_pHexModel->edge(*borderEdgeWithVertIter).start_vert()).coord() +
						   m_pHexModel->vertex(m_pHexModel->edge(*borderEdgeWithVertIter).end_vert()).coord()) * 0.5;
						   ++ numOfBorderEdges;
						   }
						   }
						   
							 avgBorderEdgeMid /= numOfBorderEdges;
							 
							   cent = (cent * (numOfBorderFaces - 3) + 
							   avgBorderEdgeMid * 2 + avgBorderFacePoint) / numOfBorderFaces;
							   
		}*/
			
			else {
				int_set_iter edge_iter = vertexIter->first_edge();
				EDGE* spa_edge = NULL;
				for (; edge_iter != vertexIter->end_edge(); ++edge_iter) {
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
					int_set_iter face_iter = vertexIter->first_face();
					for (; face_iter != vertexIter->end_face(); ++face_iter) {
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
			
			newVertices.push_back(cent);

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
		edge.set_acis_edge( edgeIter->acis_edge() );
		
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
	
	std::cerr << std::endl;
	std::cerr << "####### subdivision counter: " << ++subdiv_counter << " #######" << std::endl;
	std::cerr << "#vertices: " << phexmodel->vert_size() << std::endl;
	std::cerr << "#edges:    " << phexmodel->edge_size() << std::endl;
	std::cerr << "#faces:    " << phexmodel->face_size() << std::endl;
	std::cerr << "#cells:    " << phexmodel->cell_size() << std::endl;
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
