#pragma warning(disable:4786)

#include <IOSTREAM>
#include <FSTREAM>
#include <CSTDIO>
#include <CSTRING>

#include "hs_model.h"
#include "hs_point.h"

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
	
	void hs_model::print_vert(const char* filepath) const {
		if ( filepath ) {
			FILE* file = fopen(filepath, "w");
			if ( file ) {
				size_t idx = 0;
				char* vtname[] = {"INNER_VERT", "ORDINARY_VERT", "CREASE_VERT", "CORNER_VERT"};
				std::vector<hs_vert>::const_iterator vitr = vertices.begin();
				for (; vitr != vertices.end(); ++vitr) {
					hs_point& pnt = vitr->coord();
					fprintf(file, "%d: [%f,%f,%f] %s\n", idx++, 
						pnt.x(), pnt.y(), pnt.z(), vtname[ (size_t)vitr->type() ]);
				}
				fclose(file);
				printf("%s is saved successfully...\n", filepath);
			} else {
				printf("%s fails to open...\n", filepath);
			}
			
		} else {
			std::cout << "********all vertex********" << std::endl;
			size_t idx = 0;
			char* vtname[] = {"INNER_VERT", "ORDINARY_VERT", "CREASE_VERT", "CORNER_VERT"};
			std::vector<hs_vert>::const_iterator vitr = vertices.begin();
			for (; vitr != vertices.end(); ++vitr) {
				hs_point& pnt = vitr->coord();
				std::cout << idx++ << ": ["
					<< pnt.x() << ", "
					<< pnt.y() << ", "
					<< pnt.z() << "] "
					<< vtname[ (size_t)vitr->type() ]
					<< std::endl;
			}
		} 
	}

	void hs_model::print_edge(const char* filepath) const {
		char* etname[] = {"INNER_EDGE", "ORDINARY_EDGE", "CREASE_EDGE"};
		if ( filepath ) {
			FILE* file = fopen(filepath, "w");
			if ( file ) {
				size_t idx = 0;
				std::vector<hs_edge>::const_iterator eitr = edges.begin();
				for (; eitr != edges.end(); ++eitr) {
					fprintf(file, "%d: [%d %d] %s\n", idx++, 
						eitr->start_vert(), eitr->end_vert(), etname[ (size_t)eitr->type() ]);
				}
				fclose(file);
				printf("%s is saved successfully...\n", filepath);
			} else {
				printf("%s fails to open...\n", filepath);
			}
		

		} else {
			std::cout << "********all vertex********" << std::endl;
			size_t idx = 0;
			std::vector<hs_edge>::const_iterator eitr = edges.begin();
			for (; eitr != edges.end(); ++eitr) {
				printf("%d: [%d %d] %s\n", idx++, 
					eitr->start_vert(), eitr->end_vert(), etname[ (size_t)eitr->type() ]);
			}
		} 
	}
	
	void hs_model::print_face(const char* filepath) const {
		char* ftname[] = {"INNER_FACE", "BORDER_FACE"};
		if ( filepath ) {
			std::fstream fs(filepath, std::ios::out);
			if ( fs.is_open() ) {
				size_t idx = 0;
				std::vector<hs_face>::const_iterator fitr = faces.begin();
				for (; fitr != faces.end(); ++fitr) {
					int_set_citer vitr = fitr->first_vert();
					fs << idx++ << " [" << *vitr; 
					for (++vitr; vitr != fitr->end_vert(); ++vitr) {
						fs << ", " << *vitr;
					}
					fs << "] " << ftname[ (size_t)fitr->type() ] << "\n";
				}
				fs.close();
				printf("%s is saved successfully...\n", filepath);
			} else {
				printf("%s fails to open...\n", filepath);
			}

		} else {
			std::cout << "********all face********" << std::endl;
			size_t idx = 0;
			std::vector<hs_face>::const_iterator fitr = faces.begin();
			for (; fitr != faces.end(); ++fitr) {
				int_set_citer vitr = fitr->first_vert();
				std::cout << idx++ << "[" << *vitr; 
				for (++vitr; vitr != fitr->end_vert(); ++vitr) {
					std::cout << ", " << *vitr;
				}
				std::cout << "] " << ftname[ (size_t)fitr->type() ] << std::endl;
			}
		}
	}

	void hs_model::print_cell(const char* filepath) const {
		if ( filepath ) {
			std::fstream fs( filepath, std::ios::out );
			if ( fs.is_open() ) {
				size_t idx = 0;
				std::vector<hs_cell>::const_iterator citr = cells.begin();
				for (; citr != cells.end(); ++citr) {
					int_set_citer vitr = citr->first_vert();
					fs << idx++ << "[" << *vitr;
					for (++vitr; vitr != citr->end_vert(); ++vitr) {
						fs << ", " << *vitr;
					}
					fs << "]\n";
				}
				fs.close();
				printf("%s is saved successfully...\n", filepath);
			} else {
				printf("%s fails to open...\n", filepath);
			}
			

		} else {
			std::cout << "********all cell********" << std::endl;
			size_t idx = 0;
			std::vector<hs_cell>::const_iterator citr = cells.begin();
			for (; citr != cells.end(); ++citr) {
				int_set_citer vitr = citr->first_vert();
				std::cout << idx++ << "[" << *vitr;
				for (++vitr; vitr != citr->end_vert(); ++vitr) {
					std::cout << ", " << *vitr;
				}
				std::cout << "]" << std::endl;
			}
		}	
	}
	
	void hs_model::acis_wire(ENTITY_LIST& elist) const {
		
		edge_vector_citer edge_iter = edges.begin();
		for (; edge_iter != edges.end(); ++edge_iter) {
			hs_point& start = vertices[edge_iter->start_vert()].coord();
			hs_point& end = vertices[edge_iter->end_vert()].coord();

			SPAposition spa_start( start.x(), start.y(), start.z() );
			SPAposition spa_end( end.x(), end.y(), end.z() );

			EDGE* acis_edge;
			check_outcome( api_curve_line(spa_start, spa_end, acis_edge) ); 
			elist.add(acis_edge);
		}
	}
	
	
	void hs_model::set_acis_edge(EDGE* acis_edge[], size_t esize) {
		
		assert( edges.size() == esize );
		
		edge_vector_iter edge_iter = edges.begin();
		for (size_t i = 0; edge_iter != edges.end(); ++edge_iter, ++i) {
			edge_iter->set_acis_edge(acis_edge[i]);
		}
	}
	
	void hs_model::set_acis_face(FACE* acis_face[], size_t fsize) {
		
		assert( faces.size() == fsize );
		
		face_vector_iter face_iter = faces.begin();
		for (size_t i = 0; face_iter != faces.end(); ++face_iter, ++i) {
			face_iter->set_acis_face(acis_face[i]);
		}
	}
	
	
	void hs_model::add_vert(const hs_point& pos, vert_type vtype) {
		hs_vert hex_vert(pos.x(), pos.y(), pos.z());
		hex_vert.set_type(vtype);
		vertices.push_back(hex_vert);
	}
	
	void hs_model::add_edge(size_t sp
		, size_t ep
		, EDGE* spa_edge
		, edge_type etype
		) {

		hs_edge hex_edge;
		hex_edge.set_start_vert(sp);
		hex_edge.set_end_vert(ep);
		hex_edge.set_type(etype);
		hex_edge.set_acis_edge(spa_edge);
		edges.push_back(hex_edge);
		
		size_t eidx = edges.size() - 1;
		vertices[sp].add_edge(eidx);
		vertices[ep].add_edge(eidx);
	}
	

	void hs_model::add_face_by_edges(const size_t edge_idx[]
		, size_t esize
		, FACE* spa_face
		, face_type ftype
		) {

		hs_face hex_face;
		hex_face.set_type(ftype);
		hex_face.set_acis_face(spa_face);
		faces.push_back(hex_face);
		
		size_t fidx = faces.size() - 1;
		int_set vert_of_edge;
		
		for (size_t i = 0; i < esize; ++i) {
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
	
	
	void hs_model::add_cell_by_faces(const size_t fidx[], size_t fsz) {
		
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
		
		cells.push_back(hex_cell);
	}
	
	void hs_model::add_cell_by_verts( const size_t vidx[], size_t vsz) {
		int_set face_idx;
		for (size_t i = 0; i < vsz; ++i) {
			hs_vert& vert = vertices[ vidx[i] ];
			int_set_iter fitr = vert.first_face();
			for (; fitr != vert.end_face(); ++fitr) { 
				size_t count = 0;
				hs_face& face = faces[ *fitr ];
				int_set_iter vitr = face.first_vert();
				for (; vitr != face.end_vert(); ++vitr) {
					for (size_t j = 0; j < vsz; ++j) {
						if (*vitr ==  vidx[j]) {
							++count;
						}
					}
				}
				if (face.vert_size() == count) {
					face_idx.insert(*fitr);
				}
			}
		}

		assert( 6 == face_idx.size() );
		size_t fidx[6];
		size_t* pfidx = fidx;
		int_set_iter fitr = face_idx.begin();
		for (; fitr != face_idx.end(); ++fitr) {
			*pfidx++ = *fitr;
		}
		add_cell_by_faces(fidx, 6);
	}
	
	void hs_model::add_face_by_verts( const size_t vidx[], size_t vsz,
		FACE* spa_face, face_type ftype) {
		int_set edge_idx;
		for (size_t i = 0; i < vsz; ++i) {  
			hs_vert& vert = vert_at( vidx[i] ); 
			int_set_iter eitr = vert.first_edge();
			for (; eitr != vert.end_edge(); ++eitr) {
				hs_edge& edge = edges[ *eitr ];
				size_t vi = edge.start_vert() == vidx[i] ? 
					edge.end_vert() : edge.start_vert(); 
				for (size_t j = 0; j < vsz; ++j) {
					if (vidx[j] == vi) {
						edge_idx.insert( *eitr );
					}
				}
			}
		}

		assert( 4 == edge_idx.size() );
		size_t eidx[4];
		size_t* peidx = eidx;
		int_set_iter eitr = edge_idx.begin();
		for (; eitr != edge_idx.end();) {
			*peidx++ = *eitr++;
		}
		add_face_by_edges(eidx, 4, spa_face, ftype);
	}
	
	void hs_model::check_face() const {
		face_vector_citer fcitr = faces.begin();
		for (size_t idx = 0; fcitr != faces.end(); ++fcitr, ++idx) {
			/*std::cout << idx << ": type " << fcitr->type() << ", acis_face "
				<< fcitr->acis_face() << std::endl;*/
			switch( fcitr->type() ) {
			case INNER_FACE: /*assert( fcitr->acis_face() == NULL ); break;*/
				if (fcitr->acis_face() != NULL) {
					std::cout << idx << " face fails to check..." << std::endl;
					std::cout << "type: " << fcitr->type() << ", acis_face: "
						<< fcitr->acis_face() << std::endl;
					return;
				}
				break;
			case BORDER_FACE: /*assert( fcitr->acis_face() != NULL ); break;*/
				if (fcitr->acis_face() == NULL) {
					std::cout << idx << " face fails to check..." << std::endl;
					std::cout << "type: " << fcitr->type() << ", acis_face: "
						<< fcitr->acis_face() << std::endl;
					return;
				}
				break;
			}
		}
		std::cout << "success to check faces of model..." << std::endl;
	}
	
	void hs_model::check_edge() const {
		edge_vector_citer ecitr = edges.begin();
		for (size_t idx = 0; ecitr != edges.end(); ++ecitr, ++idx) {
// 			std::cout << idx << ": type " << ecitr->type() 
// 				<< ", acis_edge " << ecitr->acis_edge() << std::endl;
			switch( ecitr->type() ) {
			case INNER_EDGE: case ORDINARY_EDGE:  
// 				assert( ecitr->acis_edge() == NULL ); 
				if (ecitr->acis_edge() != NULL) {
					std::cout << idx << " edge fails to check..." << std::endl;
					std::cout << "type: " << ecitr->type() << ", acis_edge: "
						<< ecitr->acis_edge() << std::endl;
					return;
				}
				break;
			case CREASE_EDGE:
// 				assert( ecitr->acis_edge() != NULL );
				if (ecitr->acis_edge() == NULL) {
					std::cout << idx << " edge fails to check..." << std::endl;
					std::cout << "type: " << ecitr->type() << ", acis_edge: "
						<< ecitr->acis_edge() << std::endl;
					return;
				}
				break;			
			}
		}
		std::cout << "success to check edges of model..." << std::endl;
	}
	
	void hs_model::write_vert( const char* filepath) const {
		if ( filepath ) {
			std::fstream fs(filepath, std::ios::out);
			if ( fs.is_open() ) {
				vert_vector_citer vitr = vertices.begin();
				for (; vitr != vertices.end(); ++vitr) {
					fs << vitr->coord().x() << " " 
						<< vitr->coord().y() << " " 
						<< vitr->coord().z() << "\n";
				}
				fs.close();
				std::cout << filepath << " is saved successfully..." << std::endl;
			} else {
				std::cerr << "ERROR IN OPEN FILE: " << filepath << std::endl;
			}
		}	
	}
	
	void hs_model::write_edge( const char* filepath ) const {
		if ( filepath ) {
			std::fstream fs(filepath, std::ios::out);
			if ( fs.is_open() ) {
				edge_vector_citer eitr = edges.begin();
				for (; eitr != edges.end(); ++eitr) {
					fs << eitr->start_vert() << " " 
						<< eitr->end_vert() << "\n";
				}
				fs.close();
				std::cout << filepath << " is saved successfully..." << std::endl;
			} else {
				std::cerr << "ERROR IN OPEN FILE: " << filepath << std::endl;
			}
		}
	}
	
	void hs_model::save_file( const char* filepath, const char* file_ext ) const {
		if ( !filepath ) return;
		std::fstream fs(filepath, std::ios::out);
		if ( !fs.is_open() ) return;

		if ( strcmp(file_ext, "obj") == 0 ) {
			vert_vector_citer vcitr = vertices.begin();
			for (; vcitr != vertices.end(); ++vcitr) {
				fs << "v " << vcitr->coord().x()
					<< " " << vcitr->coord().y()
					<< " " << vcitr->coord().z()
					<< "\n";
			}
			face_vector_citer fcitr = faces.begin();
			for (size_t fidx = 0; fcitr != faces.end(); ++fcitr, ++fidx) {
				size_t vidx[4];
				sort_edge_of_face(vidx, fidx);
				fs << "f " << ++vidx[0] << " " << ++vidx[1] <<" " 
					<< ++vidx[2] << " " << ++vidx[3] << "\n";
			}

		} else if ( strcmp(file_ext, "vm") == 0 ) {
			fs << "VM ASCII\n";
			fs << "VERTICES\n" << vert_size() << "\n";
			vert_vector_citer vcitr = vertices.begin();
			for (; vcitr != vertices.end(); ++vcitr) {
				fs << vcitr->coord().x()
					<< " " << vcitr->coord().y()
					<< " " << vcitr->coord().z()
					<< "\n";
			}
			fs << "EDGES\n" << edge_size() << "\n";
			edge_vector_citer ecitr = edges.begin();
			for (; ecitr != edges.end(); ++ecitr) {
				fs << ecitr->start_vert() << " "
					<< ecitr->end_vert() << "\n";
			}
			fs << "FACES\n" << face_size() << "\n";
			face_vector_citer fcitr = faces.begin();
			for (; fcitr != faces.end(); ++fcitr) {
				fs << fcitr->edge_size() << " ";
				int_set_citer fecitr = fcitr->first_edge();
				for (; fecitr != fcitr->end_edge(); ++fecitr) {
					fs << *fecitr << " ";
				}
				fs << "\n";
			}
			fs << "POLYHEDRA\n" << cell_size() << "\n";
			cell_vector_citer ccitr = cells.begin();
			for (; ccitr != cells.end(); ++ccitr) {
				fs << ccitr->face_size() << " ";
				int_set_citer cfitr = ccitr->first_face();
				for (; cfitr != ccitr->end_face(); ++cfitr ) {
					fs << *cfitr << " ";
				}
				fs << "\n";
			}
		}
	
		fs.close();
		std::cout << filepath << " is saved successfully..." << std::endl;
	}	
	
	
	void hs_model::sort_edge_of_face( size_t vidx[], size_t idx ) const {
		const hs_face& face = faces[idx];
		assert( face.vert_size() == 4 );
		int_set etmp;
		int_set_citer eitr = face.first_edge();
		for (; eitr != face.end_edge(); ++eitr) {
			etmp.insert(*eitr);
		}
		eitr = etmp.begin();
		size_t itmp = edges[*eitr].start_vert();
		for (size_t i = 0; i < 4; ++i) {
			vidx[i] = itmp;
			etmp.erase(*eitr);
			eitr = etmp.begin();
			for (; eitr != etmp.end(); ++eitr) {
				if ( vidx[i] == edges[*eitr].start_vert() ) {
					itmp = edges[*eitr].end_vert();
					break;
				} else if (	vidx[i] == edges[*eitr].end_vert() ) {
					itmp = edges[*eitr].start_vert();
					break;
				}
			}
		}
	}
	
	
	
	
	
		
} // namespace
