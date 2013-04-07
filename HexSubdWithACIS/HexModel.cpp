#pragma warning(disable:4786)

#include <IOSTREAM>

#include "HexModel.h"
#include "Point3D.h"

#include "alltop.hxx"
#include "position.hxx"
#include "ckoutcom.hxx"
#include "cstrapi.hxx"
#include "lists.hxx"

namespace hex_subdiv {
	
	hs_model::~hs_model() {
		vertices.clear();
		edges.clear();
		faces.clear();
		cells.clear();
	}


	void hs_model::acis_wire(ENTITY_LIST& elist) {
		
		edge_vector_iter edge_iter = edges.begin();
		for (; edge_iter != edges.end(); ++edge_iter) {
			hs_point& start = vertices[edge_iter->start_vert()].coord();
			hs_point& end = vertices[edge_iter->end_vert()].coord();
			
			SPAposition spa_start( start.x(), start.y(), start.z() );
			SPAposition spa_end( end.x(), end.y(), end.z() );
			
			EDGE* acis_edge;
			outcome res = api_curve_line(spa_start, spa_end, acis_edge);
			check_outcome(res);
			
			elist.add(acis_edge);
		}
	}


	void hs_model::set_acis_edge(EDGE* acis_edge[], size_t size) {

		assert( edges.size() == size );

		edge_vector_iter edge_iter = edges.begin();
		for (size_t i = 0; edge_iter != edges.end(); ++edge_iter, ++i) {
			edge_iter->set_acis_edge(acis_edge[i]);
		}
	}

	void hs_model::set_acis_face(FACE* acis_face[], size_t size) {

		assert( faces.size() == size );

		face_vector_iter face_iter = faces.begin();
		for (size_t i = 0; face_iter != faces.end(); ++face_iter, ++i) {
			face_iter->set_acis_face(acis_face[i]);
		}
	}

	
	void hs_model::add_vert(const hs_point& pos) {
		hs_vert hex_vert(pos.x(), pos.y(), pos.z());
		add_vert(hex_vert);
	}
	
	void hs_model::add_edge(size_t sp, size_t ep) {
		hs_edge hex_edge;
		hex_edge.set_start_vert(sp);
		hex_edge.set_end_vert(ep);
		edges.push_back(hex_edge);

		size_t eidx = edges.size() - 1;
		vertices[sp].add_edge(eidx);
		vertices[ep].add_edge(eidx);
	}

/*
	void HexModel::add_edge(ENTITY_LIST& elist,
		const Point3D& pos1,
		const Point3D& pos2,
		size_t i1,  // index of vertex 1 is i1
		size_t i2,   // index of vertex 2 is i2
		EDGE* geom_edge
		)  {	
		
		HexMesh::HexEdge hex_edge;
		EDGE* acis_edge;
		SPAposition spa_pos1(pos1.x(), pos1.y(), pos1.z());
		SPAposition spa_pos2(pos2.x(), pos2.y(), pos2.z());
		outcome res = api_curve_line(spa_pos1, spa_pos2, acis_edge);
		check_outcome(res);
		
		elist.add(acis_edge);
		hex_edge.set_acis_edge(geom_edge);
		
		hex_edge.set_start_vert(i1);
		hex_edge.set_end_vert(i2);
		
		add_edge(hex_edge);	
		
		size_t ind = edge_size()-1;
		vertices[i1].add_edge(ind);
		vertices[i2].add_edge(ind);
	}	
*/

	void hs_model::add_face(const size_t edge_idx[], size_t size) {
		hs_face hex_face;
		faces.push_back(hex_face);

		size_t fidx = faces.size() - 1;
		int_set vert_of_edge;
		
		for (size_t i = 0; i < size; ++i) {
			face_at(fidx).add_edge(edge_idx[i]);
			edge_at(edge_idx[i]).add_face(fidx);
			vert_of_edge.insert(edge_at(edge_idx[i]).start_vert());
			vert_of_edge.insert(edge_at(edge_idx[i]).end_vert());
		}
		
		int_set_iter iter = vert_of_edge.begin();
		for(; iter != vert_of_edge.end(); ++iter) {
			face_at(fidx).add_vert(*iter);
			vert_at(*iter).add_face(fidx);
		}
	}
	
/*
	void HexModel::add_face(const size_t edge_indices[],
		size_t size,
		FACE* acis_face
		) {
		
		HexFace hex_face;
		hex_face.set_acis_face(acis_face);	
		
		add_face(hex_face);
		
		size_t ind = face_size() - 1;
		int_set vert_in_edge;
		
		for (size_t i = 0; i < size; ++i) {
			face(ind).add_edge(edge_indices[i]);
			edge(edge_indices[i]).add_face(ind);
			vert_in_edge.insert(edge(edge_indices[i]).start_vert());
			vert_in_edge.insert(edge(edge_indices[i]).end_vert());
		}
		
		int_set_iter iter = vert_in_edge.begin();
		for(; iter != vert_in_edge.end(); ++iter) {
			face(ind).add_vert(*iter);
			vertex(*iter).add_face(ind);
		}
		
	}
	
*/
	
	
	void hs_model::add_cell(const size_t fidx[],
		size_t fsz
		) {
		
		hs_cell hex_cell;
		int_set vert_of_cell;
		int_set edge_of_cell;
		
		size_t csz = cell_size();
		for (size_t i = 0; i < fsz; ++i) {
			hex_cell.add_face(fidx[i]);
			if ( -1 == face_at(fidx[i]).fst_cell()) {
				face_at(fidx[i]).set_fst_cell(csz);
			} else if ( -1 == face_at(fidx[i]).snd_cell()) {
				face_at(fidx[i]).set_snd_cell(csz);
			} else {
				std::cerr << "Error in add_hex_cell." << std::endl;
				return;
			}
			
			int_set_iter iter_vert = face_at(fidx[i]).first_vert();
			for(; iter_vert != face_at(fidx[i]).end_vert(); ++iter_vert) {
				vert_of_cell.insert(*iter_vert);
			}
			
			int_set_iter iter_edge = face_at(fidx[i]).first_edge();
			for(; iter_edge != face_at(fidx[i]).end_edge(); ++iter_edge) {
				edge_of_cell.insert(*iter_edge);
			}
		}
		
		int_set_iter iter;
		for (iter = vert_of_cell.begin(); iter != vert_of_cell.end(); ++iter) {
			hex_cell.add_vert(*iter);
			vert_at(*iter).add_cell(csz);
		}
		for (iter = edge_of_cell.begin(); iter != edge_of_cell.end(); ++iter) {
			hex_cell.add_edge(*iter);
			edge_at(*iter).add_cell(csz);
		}
		
		add_cell(hex_cell);
	}
	
	
	
}
