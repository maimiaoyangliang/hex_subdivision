#include <SET>
#include <IOSTREAM>
#include <FSTREAM>
#include <CMATH>
#include <algorithm>
#include <ITERATOR>


#include "ckoutcom.hxx"
#include "lists.hxx"
#include "cstrapi.hxx"
#include "alltop.hxx"
#include "boolapi.hxx"
#include "point.hxx"
#include "surface.hxx"
#include "surdef.hxx"
#include "curve.hxx"
#include "curdef.hxx"
#include "kernapi.hxx"
#include "coverapi.hxx"
#include "vector_utils.hxx"

#include "hs_subdiv.h"
#include "hs_model.h"
#include "hs_example.h"

namespace hex_subdiv {
	
	// create hexahedral initial subdivision mesh
	void hs_example::cylinder_6(hs_model& hex_model, ENTITY_LIST& elist) {
		
		BODY* cylinder1;
		outcome res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10),
			5, 5, 10, NULL, cylinder1);
		check_outcome(res);
		
		BODY* cylinder2;
		res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10),
			2, 2, 1, NULL, cylinder2);
		check_outcome(res);
		
		res = api_subtract(cylinder2, cylinder1);
		check_outcome(res);
		
		res = api_change_body_trans(cylinder1, NULL);
		check_outcome(res);
		
		elist.add(cylinder1);
		
		FACE* spa_face = cylinder1->lump()->shell()->face();
		EDGE* bot_inside = spa_face->loop()->start()->edge();
		EDGE* top_inside = spa_face->loop()->next()->start()->edge();
		EDGE* bot_outside = spa_face->next()->loop()->start()->edge();
		EDGE* top_outside = spa_face->next()->loop()->next()->start()->edge();
		
		FACE* cone_inside = spa_face;
		FACE* cone_outside = cone_inside->next();
		FACE* plane_bottom = cone_outside->next();
		FACE* plane_top = plane_bottom->next();
		
		const size_t VERT_NUM = 24;
		SPAposition vert_pos[VERT_NUM];
		const size_t PLOY_NUM = 6;
		const curve& bi_curve = bot_inside->geometry()->equation();
		size_t i;
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = bi_curve.eval_position( i * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i] = spos;
		}
		const curve& bo_curve = bot_outside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = bo_curve.eval_position((PLOY_NUM - i) * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + PLOY_NUM] = spos; 
		}
		const curve& ti_curve = top_inside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = ti_curve.eval_position((PLOY_NUM - i) * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + 2 * PLOY_NUM] = spos;
		}
		const curve& to_curve = top_outside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = to_curve.eval_position(i * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + 3 * PLOY_NUM] = spos;
		}
		
		using namespace hex_subdiv;
		for (i = 0; i < VERT_NUM; ++i) {
			hex_model.add_vert( hs_point(vert_pos[i].x(), vert_pos[i].y(), vert_pos[i].z()), CORNER_VERT);
		}
		
		const size_t EDGE_NUM = 48;
		size_t vidx_of_edge[EDGE_NUM][2] = {
			{0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,0},
			{6,7}, {7,8}, {8,9}, {9,10}, {10,11}, {11,6},
			{0,6}, {1,7}, {2,8}, {3,9}, {4,10}, {5,11},
			{12,13}, {13,14}, {14,15}, {15,16}, {16,17}, {17,12},
			{18,19}, {19,20}, {20,21}, {21,22}, {22,23}, {23,18},
			{12,18}, {13,19}, {14,20}, {15,21}, {16,22}, {17,23},
			{0,12}, {1,13}, {2,14}, {3,15}, {4,16}, {5,17},
			{6,18}, {7,19}, {8,20}, {9,21}, {10,22}, {11,23}
		};
		edge_type etype[EDGE_NUM] = { 
			CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE
		};
		EDGE* geom_edge[EDGE_NUM] = { 
			bot_inside,bot_inside,bot_inside,bot_inside, bot_inside, bot_inside,
				bot_outside,bot_outside,bot_outside,bot_outside, bot_outside, bot_outside,
				NULL,NULL,NULL,NULL,NULL,NULL,
				top_inside,top_inside,top_inside,top_inside,top_inside,top_inside,
				top_outside,top_outside,top_outside,top_outside,top_outside,top_outside,
				NULL,NULL,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL
		};
		for (i = 0; i < EDGE_NUM; ++i) {
			size_t i1 = vidx_of_edge[i][0];
			size_t i2 = vidx_of_edge[i][1];
			hex_model.add_edge(i1, i2, geom_edge[i], etype[i]);
		}
		
		const size_t FACE_NUM = 30;
		FACE* acis_face[] = { 
			plane_bottom, plane_bottom, plane_bottom, plane_bottom, plane_bottom, plane_bottom,
				plane_top, plane_top, plane_top, plane_top, plane_top, plane_top, 
				cone_inside, cone_inside, cone_inside, cone_inside, cone_inside, cone_inside,
				cone_outside, cone_outside, cone_outside, cone_outside, cone_outside, cone_outside,
				NULL, NULL, NULL, NULL, NULL, NULL
		};
		size_t eidx_of_face[FACE_NUM][4] = { // indices of edge in face
			{0,12,6,13},{1,13,7,14},{2,14,8,15},{3,15,9,16},{4,16,10,17},{5,17,11,12},
			{18,30,24,31},{19,31,25,32},{20,32,26,33},{21,33,27,34},{22,34,28,35},{23,35,29,30},
			{0,36,18,37},{1,37,19,38},{2,38,20,39},{3,39,21,40},{4,40,22,41},{5,41,23,36},
			{6,42,24,43},{7,43,25,44},{8,44,26,45},{9,45,27,46},{10,46,28,47},{11,47,29,42},
			{12,36,30,42},{13,37,31,43},{14,38,32,44},{15,39,33,45},{16,40,34,46},{17,41,35,47}
		};
		face_type ftype[FACE_NUM] = { 
			BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				INNER_FACE,  INNER_FACE,  INNER_FACE,  INNER_FACE,  INNER_FACE,  INNER_FACE
		};
		for (i = 0; i < FACE_NUM; ++i) {
			hex_model.add_face_by_edges(eidx_of_face[i], 4, acis_face[i], ftype[i]);
		}
		
		const size_t CELL_NUM = 6;
		size_t fidx_of_cell[CELL_NUM][6] = {
			{0,6,12,18,24,25},{1,7,13,19,25,26},
			{2,8,14,20,26,27},{3,9,15,21,27,28},
			{4,10,16,22,28,29},{5,11,17,23,29,24}
		};
		for (i = 0; i < CELL_NUM; ++i) {
			hex_model.add_cell_by_faces(fidx_of_cell[i], 6);
		}	
	}
	
	// create quadrilateral initial subdivision mesh
	void hs_example::cylinder_4(hs_model& hex_model, ENTITY_LIST& elist) {
		
		// create ACIS model
		BODY* cylinder1;
		outcome res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10), 
			5, 5, 5, NULL, cylinder1);
		check_outcome(res);
		
		BODY* cylinder2;
		res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10),
			2, 2, 2, NULL, cylinder2);
		check_outcome(res);
		
		res = api_subtract(cylinder2, cylinder1);
		check_outcome(res);
		
		res = api_change_body_trans(cylinder1, NULL);
		check_outcome(res);
		
		elist.add(cylinder1);
		
		FACE* spa_face = cylinder1->lump()->shell()->face();
		EDGE* bot_inside = spa_face->loop()->start()->edge();
		EDGE* top_inside = spa_face->loop()->next()->start()->edge();
		EDGE* bot_outside = spa_face->next()->loop()->start()->edge();
		EDGE* top_outside = spa_face->next()->loop()->next()->start()->edge();
		
		FACE* cone_inside = spa_face;
		FACE* cone_outside = cone_inside->next();
		FACE* plane_bottom = cone_outside->next();
		FACE* plane_top = plane_bottom->next();
		
		EDGE* geom_edge[] = { 
			bot_inside,bot_inside,bot_inside,bot_inside,
				bot_outside,bot_outside,bot_outside,bot_outside,
				NULL,NULL,NULL,NULL,
				top_inside,top_inside,top_inside,top_inside,
				top_outside,top_outside,top_outside,top_outside,
				NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL
		};
		
		FACE* acis_face[] = { 
			plane_bottom, plane_bottom, plane_bottom, plane_bottom,
				plane_top, plane_top,plane_top,plane_top,
				cone_inside, cone_inside, cone_inside, cone_inside,
				cone_outside, cone_outside, cone_outside, cone_outside,
				NULL, NULL, NULL, NULL
		};
		
		// add vertices to model
		const size_t VERT_NUM = 16;
		SPAposition vert_pos[VERT_NUM];
		const size_t PLOY_NUM = 4;
		const curve& bi_curve = bot_inside->geometry()->equation();
		size_t i;
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = bi_curve.eval_position( i * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i] = spos;
		}
		const curve& ti_curve = top_inside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = ti_curve.eval_position((PLOY_NUM - i) * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + PLOY_NUM] = spos;
		}
		const curve& bo_curve = bot_outside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = bo_curve.eval_position((PLOY_NUM - i) * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + 2 * PLOY_NUM] = spos; 
		}
		const curve& to_curve = top_outside->geometry()->equation();
		for (i = 0; i < PLOY_NUM; ++i) {
			SPAposition spos = to_curve.eval_position(i * 2 * M_PI / (double)PLOY_NUM);
			vert_pos[i + 3 * PLOY_NUM] = spos;
		}
		
		using namespace hex_subdiv;
		for (i = 0; i < VERT_NUM; ++i) {
			hex_model.add_vert( hs_point(vert_pos[i].x(), vert_pos[i].y(), vert_pos[i].z()), CORNER_VERT);
		}
		
		// add edges' top relationship to model
		const size_t EDGE_NUM = 32;
		size_t vert_idx_of_edge[EDGE_NUM][2] = { // indices of vertex in edge
			{0,1},{1,2},{2,3},{3,0},		 // bottom inside
			{8,9},{9,10},{10,11},{11,8},     // bottom outside
			{0,8},{1,9},{2,10},{3,11},       // bottom link
			{4,5},{5,6},{6,7},{7,4},         // top inside
			{12,13},{13,14},{14,15},{15,12}, // top outside
			{4,12},{5,13},{6,14},{7,15},	 // top link
			{0,4},{1,5},{2,6},{3,7},		 // side inside
			{8,12},{9,13},{10,14},{11,15}	 // side outside
		};
		edge_type etype[EDGE_NUM] = {
			CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				CREASE_EDGE, CREASE_EDGE, CREASE_EDGE, CREASE_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
				ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE, ORDINARY_EDGE,
		};
		for (i = 0; i < EDGE_NUM; ++i) {
			size_t i1 = vert_idx_of_edge[i][0];
			size_t i2 = vert_idx_of_edge[i][1];
			hex_model.add_edge(i1, i2, geom_edge[i], etype[i]);
		}
		
		// add faces' top relationship to model
		const size_t FACE_NUM = 20;
		size_t edge_idx_in_face[FACE_NUM][4] = { // indices of edge in face
			{0,8,4,9},{1,9,5,10},{2,10,6,11},{3,11,7,8}, // bottom 
			{12,20,16,21},{13,21,17,22},{14,22,18,23},{15,23,19,20}, // top 
			{0,25,12,24},{1,26,13,25},{2,27,14,26},{3,24,15,27}, // side inside 
			{4,29,16,28},{5,30,17,29},{6,31,18,30},{7,28,19,31}, // side outside
			{8,28,20,24},{9,29,21,25},{10,30,22,26},{11,31,23,27} // bottom 
		};
		face_type ftype[FACE_NUM] = {
			BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				BORDER_FACE, BORDER_FACE, BORDER_FACE, BORDER_FACE,
				INNER_FACE,  INNER_FACE,  INNER_FACE,  INNER_FACE
		};
		for (i = 0; i < FACE_NUM; ++i) {
			hex_model.add_face_by_edges(edge_idx_in_face[i], 4, acis_face[i], ftype[i]);
		} 
		
		// add cells' top relationship to model
		const size_t CELL_NUM = 4;
		size_t face_idx_of_cell[CELL_NUM][6] = {
			{0,4,8,16,12,17},{1,5,9,17,13,18},
			{2,6,10,18,14,19},{3,7,11,19,15,16}
		};
		for (i = 0; i < CELL_NUM; ++i) {
			hex_model.add_cell_by_faces(face_idx_of_cell[i], 6);
		}	
	}
	
	void hs_example::gear(ENTITY_LIST& init_mesh, hs_model& hex_model, const char* filepath) {

		ENTITY_LIST glist;
		FILE* file = acis_fopen(filepath, "r"); 
		check_outcome( api_restore_entity_list(file, TRUE, glist) );
		acis_fclose(file);

		init_mesh.add(glist);
		
		std::vector< FACE* > face_vec;
		get_faces_of_gear(static_cast< BODY* >( glist[0] ), face_vec);
		
		std::vector< VERTEX* > spa_verts_vec;
		std::vector< SPAposition > verts_vec;
		size_t novar; /** number of vertices a round of gear**/
		get_vertices_of_gear(face_vec, verts_vec, spa_verts_vec, novar);

		add_vertices_to_model(hex_model, verts_vec, novar);
		add_edges_to_model(hex_model, face_vec, spa_verts_vec, novar);
		add_faces_to_model(hex_model, face_vec, spa_verts_vec, novar);
		add_cells_to_model(hex_model, novar);
	}
	
	
	void hs_example::make_gear(const char* filepath, ENTITY_LIST& elist) {
		
		ENTITY_LIST glist;
		FILE* file = acis_fopen(filepath, "r");
		outcome res = api_restore_entity_list(file, TRUE, glist);
		check_outcome(res);
		BODY* gbody = static_cast< BODY* >( glist[0] );

		FACE* cylinder[1];
		double radius = hs_point(13, -1.5, 0).length();
		double start_degree = atan(1.5 / 13) * 180 / M_PI; 
		double end_degree = 360 - start_degree;
		res = api_face_cylinder_cone(SPAposition(0,0,-12.5), SPAvector(0,0,25), 
			radius, radius,start_degree,end_degree,1,NULL,cylinder[0]);
		check_outcome(res); 

		BODY* gnewbody;
		res = api_mk_by_faces(gbody ,1, cylinder, gnewbody);
		check_outcome(res);

		elist.add(gnewbody); 
	}
	
	EDGE* hs_example::get_edge_by_2_vert( VERTEX* v1, VERTEX* v2) {
		if ( !v1 || !v2 ) return NULL;
		ENTITY_LIST elist1, elist2;
		check_outcome( api_get_edges(v1, elist1) );
		check_outcome( api_get_edges(v2, elist2) );
		elist1.init(); elist2.init();
		EDGE* e1 = static_cast< EDGE* >( elist1.first() );
		while ( e1 ) {
			EDGE* e2 = static_cast< EDGE* >( elist2.first() );
			while ( e2 ) {
				if ( e1 == e2 ) { return e1; }
				e2 = static_cast< EDGE* >( elist2.next() );
			}
			e1 = static_cast< EDGE* >( elist1.next() );
		}
		return NULL;
	}
	
	void hs_example::get_vertices_by_coedge( COEDGE* start, std::vector< VERTEX* >& verts) {
		if ( !start ) return;
		assert( start->sense() == FORWARD );
		COEDGE* ptr = start;
		verts.push_back( ptr->end() );
		ptr = ptr->next(FORWARD);
		while ( ptr != start ) {
			verts.push_back( ptr->end() );
			ptr = ptr->next();
		}
	}
	
	void hs_example::print_vert( std::vector< VERTEX* >& verts, const char* filepath ) const { 
		if (filepath) {
			FILE* file = acis_fopen(filepath, "w");
			std::vector< VERTEX* >::const_iterator vcitr = verts.begin();
			for (size_t idx = 0; vcitr != verts.end(); ++vcitr) {
				SPAposition pos( (*vcitr)->geometry()->coords() ); 
				acis_printf("%d: [%f, %f, %f]\n", idx, pos.x(), pos.y(), pos.z());
				acis_fprintf(file, "%d: [%f, %f, %f]\n", idx++, pos.x(), pos.y(), pos.z());
			}
			acis_fclose(file);
		}  else { 
			std::vector< VERTEX* >::const_iterator vcitr = verts.begin();
			for (size_t idx = 0; vcitr != verts.end(); ++vcitr) {
				SPAposition pos( (*vcitr)->geometry()->coords() );
				acis_printf("%d: [%f, %f, %f]\n", idx++, pos.x(), pos.y(), pos.z());
			} 
		}
	}
	
	
	void hs_example::get_vertices_of_gear( std::vector< FACE* >& face_vec, 
		std::vector< SPAposition >& verts_vec, std::vector< VERTEX* >& spa_verts_vec, size_t& novar) 
	{
		FACE* gface;
		EDGE* gedge;
		LOOP* gloop;
		COEDGE* gcoedge;

		std::vector< VERTEX* > top_verts_tmp, bot_verts_tmp;
		// top vertices
		gface = face_vec[86];
		gloop = gface->loop()->next();
		gcoedge = gloop->start();
		get_vertices_by_coedge(gcoedge, top_verts_tmp); 
		// bottom vertices
		gface = face_vec[84];
		gloop = gface->loop()->next();
		gcoedge = gloop->start();
		get_vertices_by_coedge(gcoedge, bot_verts_tmp);
		assert( top_verts_tmp.size() == bot_verts_tmp.size() );
		std::vector< VERTEX* >::iterator bot_vitr = bot_verts_tmp.begin();
		std::advance(bot_vitr, 68);
		std::reverse(bot_verts_tmp.begin(), bot_vitr);
		std::reverse(bot_vitr, bot_verts_tmp.end()); 

		// 1
		std::vector< VERTEX* > top_spa_verts, bot_spa_verts;
		std::vector< SPAposition > top_verts, bot_verts;
		std::vector< VERTEX* >::iterator top_vitr;
		top_vitr = top_verts_tmp.begin();
		bot_vitr = bot_verts_tmp.begin();
		for (size_t i = 0; top_vitr != top_verts_tmp.end() && bot_vitr != bot_verts_tmp.end(); ) {
			if ( (i & 3) == 1 || (i & 3) == 2) {
				top_spa_verts.push_back( *top_vitr );
				bot_spa_verts.push_back( *bot_vitr );   
				top_verts.push_back( (*top_vitr)->geometry()->coords() );
				bot_verts.push_back( (*bot_vitr)->geometry()->coords() );
			}
			++top_vitr; ++bot_vitr; ++i;
		}
		novar = top_spa_verts.size();
		// 2
		top_vitr = top_verts_tmp.begin();
		bot_vitr = bot_verts_tmp.begin();
		for (i = 0; top_vitr != top_verts_tmp.end() && bot_vitr != bot_verts_tmp.end(); ) {
			if ( (i & 3) == 0 || (i & 3) == 3) {
				top_spa_verts.push_back( *top_vitr );
				bot_spa_verts.push_back( *bot_vitr ); 
				top_verts.push_back( (*top_vitr)->geometry()->coords() );
				bot_verts.push_back( (*bot_vitr)->geometry()->coords() );
			}
			++top_vitr; ++bot_vitr; ++i;
		}
 
		
		//3  
		gface = face_vec[86];
		gedge = gface->loop()->start()->edge();
		curve* gcurve = &( gedge->geometry()->equation_for_update() ); 
		size_t vsz = top_verts.size(); 
		for (i = 0; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			SPAposition v_pos( top_verts[idx] );
			SPAposition new_pos ( gcurve->eval_position( gcurve->param(v_pos) ) );
			top_verts.push_back(new_pos);

			new_pos.set_z(new_pos.z() - 10);
			bot_verts.push_back(new_pos);
		}


		// 4
		gface = face_vec[12];
		gloop = gface->loop()->next();
		gedge = gloop->start()->edge();
		gcurve = &( gedge->geometry()->equation_for_update() );
		vsz = top_verts.size(); 
		for (i = 0; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[ idx ];
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos); 

			gpos.set_z( gpos.z() - 7 );
			bot_verts.push_back(gpos);
		}

		// 5
		gface = face_vec[12];
		gloop = gface->loop();
		gedge = gloop->start()->edge();
		gcurve = &( gedge->geometry()->equation_for_update() );
		vsz = top_verts.size(); 
		for (i = 0; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos); 

			gpos.set_z( gpos.z() - 7 );
			bot_verts.push_back(gpos);
		}

		// 6
		vsz = top_verts.size(); 	
		for (i = 0; i < 2; ++i) {
			FACE* ftemp = face_vec[i + 1];
			EDGE* etemp = ftemp->loop()->start()->next()->next()->edge();
			gcurve = &(etemp->geometry()->equation_for_update());
			size_t idx = i + vsz - novar;
			SPAposition old_pos( top_verts[idx] );
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos);
			
			gpos.set_z( gpos.z() - 7 );
			bot_verts.push_back(gpos);
		}
		
		gface = face_vec[0];
		surface* gsurface = &( gface->geometry()->equation_for_update() ); 
		for (i = 2; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			SPApar_pos spa_param = gsurface->param( old_pos );
			SPAposition gpos( gsurface->eval_position( spa_param ) );
			top_verts.push_back(gpos);

			gpos.set_z( gpos.z() - 7 );
			bot_verts.push_back(gpos);
		}

		// 7
		vsz = top_verts.size(); 
		for (i = 0; i < 2; ++i) {
			FACE* ftemp = face_vec[i + 1];
			EDGE* etemp = ftemp->loop()->start()->edge();
			gcurve = &(etemp->geometry()->equation_for_update());
			size_t idx = i + vsz - novar;
			SPAposition old_pos ( top_verts[idx] );
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos);
			
			gpos.set_z( gpos.z() - 7 ); 
			bot_verts.push_back(gpos);
		}
		
		gface = face_vec[5];
		gsurface = &( gface->geometry()->equation_for_update() );
		for (i = 2; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			SPApar_pos spa_param = gsurface->param( old_pos );
			SPAposition gpos( gsurface->eval_position( spa_param ) );
			top_verts.push_back(gpos);

			gpos.set_z( gpos.z() - 7 );
			bot_verts.push_back(gpos);
		}

		// 8
		vsz = top_verts.size(); 
		for (i = 0; i < 2; ++i) {
			FACE* ftemp = face_vec[i + 1];
			EDGE* etemp = ftemp->loop()->start()->edge();
			
			SPAposition gpos( i == 0 ? etemp->start_pos() : etemp->end_pos() );
			top_verts.push_back(gpos);
			
			gpos.set_z( gpos.z() - 25 );
			bot_verts.push_back(gpos);
		}
		
		gface = face_vec[5];
		gedge = gface->loop()->start()->next()->edge();
		gcurve = &( gedge->geometry()->equation_for_update() );
		for (i = 2; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos);

			gpos.set_z( gpos.z() - 25 );
			bot_verts.push_back(gpos);
		}

		// 9
		vsz = top_verts.size(); 
		for (i = 0; i < 2; ++i) {
			FACE* ftemp = face_vec[3];
			EDGE* etemp = ftemp->loop()->start()->next()->edge();
			
			SPAposition gpos( i == 0 ? etemp->end_pos() : etemp->start_pos() );
			top_verts.push_back(gpos);
			
			gpos.set_z( gpos.z() - 25 );
			bot_verts.push_back(gpos);
		}
		
		gface = face_vec[0];
		gcoedge = gface->loop()->start();
		gedge = gcoedge->next()->next()->edge();
		gcurve = &( gedge->geometry()->equation_for_update() );
		for (i = 2; i < novar; ++i) {
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos);

			gpos.set_z( gpos.z() - 25 );
			bot_verts.push_back(gpos);
		}

		// 10
		gface = face_vec[4]; 
		gedge = gface->loop()->start()->edge();
		gcurve = &( gedge->geometry()->equation_for_update() );
		vsz = top_verts.size();
		for (i = 0; i < novar; ++i) { 
			size_t idx = i + vsz - novar;
			const SPAposition& old_pos = top_verts[idx];
			double spa_param = gcurve->param( old_pos );
			SPAposition gpos( gcurve->eval_position( spa_param ) );
			top_verts.push_back(gpos); 

			gpos.set_z( gpos.z() - 25 );
			bot_verts.push_back(gpos);
		}

		std::copy(top_verts.begin(), top_verts.end(), std::back_inserter(verts_vec) );
		std::copy(bot_verts.begin(), bot_verts.end(), std::back_inserter(verts_vec) );
		
		std::copy(top_spa_verts.begin(), top_spa_verts.end(), std::back_inserter(spa_verts_vec) ); 
		std::copy(bot_spa_verts.begin(), bot_spa_verts.end(), std::back_inserter(spa_verts_vec) );
	}
	
	void hs_example::get_faces_of_gear( ENTITY* ent, std::vector< FACE* >& face_vec) {

		ENTITY_LIST face_list;
		check_outcome( api_get_faces(ent, face_list) );
		face_list.init();
		FACE* face = static_cast< FACE* >( face_list.first() );
		while ( face ) {
			face_vec.push_back(face);
			face = static_cast< FACE* >( face_list.next() );
		} 
	}

	/** novar: number of vertices a round of gear **/
	void hs_example::add_vertices_to_model( hs_model& hex_model, 
		std::vector< SPAposition >& verts_vec, size_t novar) 
	{	
		std::vector< SPAposition >::iterator vitr = verts_vec.begin();
		for (size_t level = 0; level < 2; ++level) { 		
			for (size_t ctrl = 0; ctrl < 10; ++ctrl) {
				for (size_t sz = 0; sz < novar && vitr != verts_vec.end(); ) {
					vert_type vtype; 
					switch (ctrl) {
					case 0: case 1:
						vtype = CORNER_VERT; break;
					case 2: case 3: case 4: case 9:
						vtype = CREASE_VERT; break;
					case 5:
						if (sz < 2) { vtype = CREASE_VERT;} 
						else		{ vtype = INNER_VERT;}
						break;
					case 6: 
						if (sz < 2) { vtype = CREASE_VERT;}
						else		{ vtype = ORDINARY_VERT;}
						break;
					case 7: 
						if (sz < 2) { vtype = CORNER_VERT;}
						else		{ vtype = CREASE_VERT;}
						break;
					case 8:
						if (sz < 2) { vtype = CORNER_VERT;}
						else		{ vtype = ORDINARY_VERT;}
						break;
					} 
					
					SPAposition& spa_pos = *vitr;
					hs_point vert(spa_pos.x(), spa_pos.y(), spa_pos.z());
					hex_model.add_vert(vert, vtype);
					
					++sz; ++vitr;
				} // end for sz
			} // end for ctrl
		} // end for level	
	}
	
	void hs_example::add_edges_to_model( hs_model& hex_model, std::vector< FACE* >& face_vec,
		std::vector< VERTEX*>& spa_verts_vec, size_t novar)
	{
		const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2 );
		size_t vidx = 0; 
		for (size_t level = 0; level < 2; ++level) { 
			// 1  X---X---X
			for (size_t st = 0; st < novar; st += 2, vidx += 2) {
				size_t end, sidx, eidx;
				end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				sidx = st + level * novar * 2;
				eidx = (st + 1 == novar) ? sidx + 1 - novar : sidx + 1; 
				EDGE* spa_edge = get_edge_by_2_vert(spa_verts_vec[sidx], spa_verts_vec[eidx]);
				assert( spa_edge != NULL );
				hex_model.add_edge(vidx, end, spa_edge, CREASE_EDGE);
			} 

			// 2  
			for (st = 0; st < novar; ++st, ++vidx) { 
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				if (!(st & 1)) {  
					hex_model.add_edge(vidx, end, NULL, ORDINARY_EDGE);
				} else { 
					size_t sidx, eidx;
					sidx = st + novar + level * novar * 2;
					eidx = (st + 1 == novar) ? sidx + 1 - novar : sidx + 1; 
					EDGE* spa_edge = get_edge_by_2_vert(spa_verts_vec[sidx], spa_verts_vec[eidx]);
					assert( spa_edge != NULL );
					hex_model.add_edge(vidx, end, spa_edge, CREASE_EDGE);
				}
			} 

			//
			for (size_t ctrl = 0; ctrl < 8; ++ctrl) { 
				
				if (4 == ctrl || 5 == ctrl) { st = 1; ++vidx;} 
				else { st = 0; }
				for (; st < novar; ++st, ++vidx) {
					edge_type etype; 
					EDGE* spa_edge = NULL;
					FACE* spa_face = NULL;

					switch (ctrl) {
					case 0: 
						spa_face = (level == 0) ? face_vec[86] : face_vec[84]; 
						spa_edge = spa_face->loop()->start()->edge();
						etype = CREASE_EDGE; break;
					case 1: 
						spa_face = (level == 0) ? face_vec[12] : face_vec[10];
						spa_edge = spa_face->loop()->next()->start()->edge();
						etype = CREASE_EDGE; break;
					case 2:
						spa_face = (level == 0) ? face_vec[12] : face_vec[10];
						spa_edge = spa_face->loop()->start()->edge();
						etype = CREASE_EDGE; break;
					case 3:
						if (st == 0) { etype = ORDINARY_EDGE; }
						else	     { etype = INNER_EDGE; } 
						break;
					case 5: { 
						spa_face = (level == 0) ? face_vec[4] : face_vec[8]; 
						COEDGE* spa_coedge = (level == 0) 
							? spa_face->loop()->next()->start()->next()->next()
							: spa_face->loop()->start();
						spa_edge = spa_coedge->next()->edge();
						etype = CREASE_EDGE; break;
							}
					case 7:
						spa_face = (level == 0) ? face_vec[4] : face_vec[8];
						spa_edge = (level == 0) ? spa_face->loop()->start()->edge() 
							: spa_face->loop()->next()->start()->edge();
						etype = CREASE_EDGE; break;
					case 4: 
						etype = ORDINARY_EDGE; break;
					case 6:
						if (st == 0) { 
							spa_edge = (level == 0)
								? face_vec[3]->loop()->start()->next()->edge()
								: face_vec[3]->loop()->start()->next()->next()->next()->edge();
							etype = CREASE_EDGE;  
						} else { etype = ORDINARY_EDGE; }
						break;
					}
					size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1; 
					hex_model.add_edge(vidx, end, spa_edge, etype);
				}  
			}
			//     X
			//	   |
			//     X
			//     |
			//     X
			vidx = level * HALF_VERTS_NUM;
			for (ctrl = 0; ctrl < 9; ++ctrl) { 
				for (st = 0; st < novar; ++st, ++vidx) {
					edge_type etype;
					EDGE* spa_edge = NULL;
					switch ( ctrl ) {
					case 0: 
						{
							size_t spa_idx = st + level * novar * 2;
							spa_edge = get_edge_by_2_vert(spa_verts_vec[spa_idx], spa_verts_vec[spa_idx + novar]);
							assert ( spa_edge != NULL );
							etype = CREASE_EDGE; break;
						}
					case 1: case 2: case 3: case 8:
						etype = ORDINARY_EDGE; break;
					case 4:
						etype = INNER_EDGE; break;
					case 5:
						if ( st < 2 ) { etype = ORDINARY_EDGE; }
						else	      { etype = INNER_EDGE; }
						break;
					case 6: 
						if ( st < 2 ) { 
							etype = CREASE_EDGE; 
							spa_edge = (st == 0) 
								? face_vec[1]->loop()->start()->edge() 
								: face_vec[2]->loop()->start()->edge();
						} else { etype = ORDINARY_EDGE;}
						break;
					case 7:
						if ( st < 2 ) { 
							etype = CREASE_EDGE; 
							if (level == 0) {
								spa_edge = (st == 0) 
									? face_vec[1]->loop()->start()->next()->edge() 
									: face_vec[2]->loop()->start()->next()->next()->next()->edge();
							} else if (level == 1) {
								spa_edge = (st == 0) 
									? face_vec[1]->loop()->start()->next()->next()->next()->edge() 
									: face_vec[2]->loop()->start()->next()->edge();
							}
						} else { etype = ORDINARY_EDGE;} 
						break;
					}
					hex_model.add_edge(vidx, vidx + novar, spa_edge, etype);
				} 
			}	

			for (st = 0, vidx -= novar; st < novar; ++st, ++vidx) {
				EDGE* spa_edge = NULL;
				edge_type etype = INNER_EDGE;
				if ( st < 2 ) {
					spa_edge = face_vec[st + 1]->loop()->start()->next()->next()->edge();
					etype = CREASE_EDGE;
				}
				hex_model.add_edge(vidx, vidx - 3 * novar, spa_edge, etype);
			} 
		
			for (st = 0; st < novar; ++st, ++vidx) {  
				hex_model.add_edge(vidx, vidx - 5 * novar, NULL, ORDINARY_EDGE);
			} 			
		}

		//  X
		//  |\
		//1L| \<--this
		//  X  X
		//  |  | 2L
		std::vector< EDGE* > vtc1_edges, vtc2_edges;
		vidx = 0;
		for (size_t ctrl = 0; ctrl < 7; ++ctrl) {
			for (size_t st = 0; st < novar; ) {
				edge_type etype; 
				EDGE* spa_edge = NULL;
				switch(ctrl) {
				case 0: case 1:
					spa_edge = get_edge_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[vidx + 2 * novar]);
					assert ( spa_edge != NULL );
					etype = CREASE_EDGE; break;
				case 2: case 3: case 4: 
					etype = INNER_EDGE; break;
				case 5:
					if (st < 2) {
						spa_edge = face_vec[st + 1]->loop()->start()->next()->next()->edge();
						etype = CREASE_EDGE;
					} else { etype = INNER_EDGE; }
					break;
				case 6:
					if (st < 2) {
						spa_edge = face_vec[st + 1]->loop()->start()->edge();
						etype = CREASE_EDGE;
					} else { etype = ORDINARY_EDGE; }
					break;
				}
				hex_model.add_edge(vidx, vidx + HALF_VERTS_NUM, spa_edge, etype);
				++st; ++vidx;
			} 
		}
 
	}
	
	void hs_example::add_faces_to_model( hs_model& hex_model, std::vector< FACE* >& face_vec, 
		std::vector< VERTEX* >& spa_verts_vec, size_t novar) 
	{
		const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2);
		size_t vidx = 0;
		for (size_t level = 0; level < 2; ++level) {
			vidx = level * HALF_VERTS_NUM; 
			FACE* spa_face = (0 == level) ? face_vec[86] : face_vec[84];
			for(size_t st = 0; st < novar; st += 2, vidx += 2) {
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				size_t verts[] = {vidx, end, vidx + novar, end + novar};
				hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
			}
			
			for (size_t ctrl = 0; ctrl < 8; ++ctrl) { 
				face_type ftype;
				spa_face = NULL;
				switch (ctrl) {
				case 0: spa_face = (0 == level) ? face_vec[86] : face_vec[84]; ftype = BORDER_FACE; break;
				case 1: spa_face = (0 == level) ? face_vec[11] : face_vec[9]; ftype = BORDER_FACE; break;
				case 2: spa_face = (0 == level) ? face_vec[12] : face_vec[10]; ftype = BORDER_FACE; break;	
				case 5: spa_face = face_vec[5]; ftype = BORDER_FACE; break;
				case 6: case 7:
					spa_face = (0 == level) ? face_vec[4] : face_vec[8]; ftype = BORDER_FACE; break;
				case 3: case 4:
					ftype = INNER_FACE; break;
				}
				 
				if ( 6 == ctrl || 5 == ctrl || 4 == ctrl ) { ++vidx; st = 1;} 
				else { st = 0; }
				for(; st < novar; ++st, ++vidx) {
					size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
					size_t verts[] = {vidx, end, vidx + novar, end + novar};
					hex_model.add_face_by_verts(verts,4,spa_face,ftype);
				} 
			}  

			for(st = 0, vidx -= novar; st < novar; ++st, ++vidx) {
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				size_t verts[] = {vidx, end, vidx - 3*novar, end - 3*novar};
				FACE* spa_face = (st == 0) ? face_vec[3] : NULL;
				face_type ftype = (st == 0) ? BORDER_FACE : INNER_FACE;
				hex_model.add_face_by_verts(verts,4,spa_face,ftype);
			}
			
			spa_face = (0 == level) ? face_vec[6] : face_vec[7];
			for(st = 0; st < novar; ++st, ++vidx) {
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				size_t verts[] = {vidx, end, vidx - 5*novar, end - 5*novar};
				hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
			}		
		}

		vidx = 0;
		for(size_t st = 0; st < novar; st += 2, vidx += 2) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + HALF_VERTS_NUM, end + HALF_VERTS_NUM};
			FACE* spa_face = get_face_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[end + 2 * novar]);
			assert ( spa_face != NULL );
			hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
		}

		for(st = 0; st < novar; ++st, ++vidx) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + HALF_VERTS_NUM, end + HALF_VERTS_NUM};
			if (st & 1) {
				FACE* spa_face = get_face_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[end + 2 * novar]);
				assert ( spa_face != NULL );
				hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
			} else {
				hex_model.add_face_by_verts(verts,4,NULL,INNER_FACE);
			}
		}
		
		for (size_t ctrl = 0; ctrl < 5; ++ctrl) {
		
			if ( 4 == ctrl ) { ++vidx; st = 1;}
			else { st = 0; }
			for(; st < novar; ++st, ++vidx) {
				face_type ftype; 
				FACE* spa_face = NULL; 
				switch ( ctrl ) {
				case 0: case 1: case 2:
					ftype = INNER_FACE; break;
				case 3:
					if (st == 0) { spa_face = face_vec[3]; ftype = BORDER_FACE; }
					else { ftype = INNER_FACE; }
					break;
				case 4:
					spa_face = face_vec[5]; ftype = BORDER_FACE; break;
				} 
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1; 
				size_t verts[] = {vidx, end, vidx + HALF_VERTS_NUM, end + HALF_VERTS_NUM};
				hex_model.add_face_by_verts(verts,4,spa_face,ftype);
			}
		}
		 
		vidx = 0;
		for (ctrl = 0; ctrl < 6; ++ctrl) {
			for (st = 0; st < novar; ++st, ++vidx) {
				face_type ftype; 
				FACE* spa_face = NULL;
				switch ( ctrl ) {
				case 0:
					ftype = BORDER_FACE;
					spa_face = get_face_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[vidx + 3 * novar]);
					assert ( spa_face != NULL );
					break;
				case 1: case 2: case 3: case 4:
					ftype = INNER_FACE; break;
				case 5:
					if (st < 2) {
						ftype = BORDER_FACE;
						spa_face = face_vec[st + 1]; 
					} else { ftype = INNER_FACE; }
					break;
				}
				size_t verts[] = {vidx, vidx + novar, vidx + HALF_VERTS_NUM, vidx + novar + HALF_VERTS_NUM};
			    hex_model.add_face_by_verts(verts,4,spa_face,ftype);
			}
		}
		
		
		for (st = 0, vidx -= 2*novar; st < novar; ++st, ++vidx) {
			size_t verts[] = {vidx, vidx + novar, vidx + 4*novar, vidx + 5*novar}; 
			hex_model.add_face_by_verts(verts,4,NULL,INNER_FACE);
			
			size_t bot_vidx = vidx + HALF_VERTS_NUM;
			size_t bot_verts[] = {bot_vidx, bot_vidx + novar, bot_vidx + 4*novar, bot_vidx + 5*novar};
			hex_model.add_face_by_verts(bot_verts,4,NULL,INNER_FACE);
		}
		
		for (st = 0; st < novar; ++st, ++vidx) {
			face_type ftype;
			FACE* spa_face = NULL;
			if (st < 2) {
				ftype = BORDER_FACE;
				spa_face = face_vec[st + 1];
			} else { ftype = INNER_FACE; } 
			size_t verts[] = {vidx, vidx + novar, vidx + 2*novar, vidx + 3*novar};
			hex_model.add_face_by_verts(verts,4,spa_face,ftype);

			size_t bot_vidx = vidx + HALF_VERTS_NUM;
			size_t bot_verts[] = {bot_vidx, bot_vidx + novar, bot_vidx + 2*novar, bot_vidx + 3*novar};
			hex_model.add_face_by_verts(bot_verts,4,spa_face,ftype);
		}
	}
	
	void hs_example::add_cells_to_model( hs_model& hex_model, size_t novar) {
	
		const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2 );
		size_t vidx = 0; 
		for (size_t ctrl = 0; ctrl < 6; ++ctrl) {
			size_t st;
			if ( 5 == ctrl ) { ++vidx; st = 1;}
			else { st = 0; }
			while ( st < novar ) {
				size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
				size_t verts[] = {vidx, end, vidx + novar, end + novar,
					vidx + HALF_VERTS_NUM, end + HALF_VERTS_NUM, 
					vidx + novar + HALF_VERTS_NUM, end + novar + HALF_VERTS_NUM
				};
				hex_model.add_cell_by_verts(verts,8);
				if ( ctrl == 0) { st += 2; vidx += 2; }
				else { ++st; ++vidx; }
			}
		}
		vidx -= 2*novar;
		for (size_t st = 0; st < novar; ++st, ++vidx) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + novar, end + novar,
				vidx + 4*novar, end + 4*novar, 
				vidx + 5*novar, end + 5*novar};
			hex_model.add_cell_by_verts(verts,8);
			
			size_t bot_vidx = vidx + HALF_VERTS_NUM;
			end += HALF_VERTS_NUM;
			size_t bot_verts[] = {bot_vidx, end, bot_vidx + novar, end + novar,
				bot_vidx + 4*novar, end + 4*novar, 
				bot_vidx + 5*novar, end + 5*novar
			};
			hex_model.add_cell_by_verts(bot_verts,8);			
		}
		
		for (st = 1, ++vidx; st < novar; ++st, ++vidx) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + novar, end + novar,
				vidx + 2*novar, end + 2*novar, 
				vidx + 3*novar, end + 3*novar};
			hex_model.add_cell_by_verts(verts,8);
			
			size_t bot_vidx = vidx + HALF_VERTS_NUM;
			end += HALF_VERTS_NUM;
			size_t bot_verts[] = {bot_vidx, end, bot_vidx + novar, end + novar,
				bot_vidx + 2*novar, end + 2*novar, 
				bot_vidx + 3*novar, end + 3*novar};
			hex_model.add_cell_by_verts(bot_verts,8);	
		}
	}
	
	FACE* hs_example::get_face_by_2_vert( VERTEX* v1, VERTEX* v2) {
		if ( !v1 || !v2 ) return NULL;
		ENTITY_LIST face_list_1, face_list_2;
		check_outcome( api_get_faces(v1, face_list_1) );
		check_outcome( api_get_faces(v2, face_list_2) );
		face_list_1.init(); face_list_2.init();
		FACE* face1 = static_cast< FACE* >( face_list_1.first() );
		while ( face1 ) {
			FACE* face2 = static_cast< FACE* >( face_list_2.first() );
			while ( face2 ) {
				if ( face1 == face2 ) return face1;
				face2 = static_cast< FACE* >( face_list_2.next() );
			}
			face1 = static_cast< FACE* >( face_list_1.next() );
		}

		return NULL;
	}

} // namespace
