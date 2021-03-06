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

using namespace hex_subdiv;

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

void hs_example::gear_coarse(ENTITY_LIST& init_mesh, hs_model& hex_model, const char* filepath) {
	
	ENTITY_LIST glist;
	FILE* file = acis_fopen(filepath, "r"); 
	check_outcome( api_restore_entity_list(file, TRUE, glist) );
	acis_fclose(file);
	
	init_mesh.add(glist);
	
	glist.init();
	std::vector< FACE* > face_vec;
	get_faces_of_gear(static_cast< BODY* >( glist.first() ), face_vec);
	get_faces_of_gear(static_cast< BODY* >( glist.next() ), face_vec);
	std::vector< VERTEX* > spa_verts_vec;
	std::vector< SPAposition > verts_vec;
	size_t novar; /** number of vertices a round of gear**/
	get_vertices_of_gear_coarse(face_vec, verts_vec, spa_verts_vec, novar);
	
	add_vertices_to_model_coarse(hex_model, verts_vec, novar);
	add_edges_to_model_coarse(hex_model, face_vec, spa_verts_vec, novar);
	add_faces_to_model_coarse(hex_model, face_vec, spa_verts_vec, novar);
	add_cells_to_model_coarse(hex_model, novar);
}

void hs_example::gear_compact(ENTITY_LIST& init_mesh, hs_model& hex_model, const char* filepath) {
	
	ENTITY_LIST glist;
	FILE* file = acis_fopen(filepath, "r"); 
	check_outcome( api_restore_entity_list(file, TRUE, glist) );
	acis_fclose(file);
	
	init_mesh.add(glist);
	
	glist.init();
	std::vector< FACE* > face_vec;
	get_faces_of_gear(static_cast< BODY* >( glist.first() ), face_vec);
	get_faces_of_gear(static_cast< BODY* >( glist.next() ), face_vec);
	std::vector< VERTEX* > spa_verts_vec;
	std::vector< SPAposition > verts_vec;
	size_t novar; /** number of vertices a round of gear**/
	get_vertices_of_gear_compact(face_vec, verts_vec, spa_verts_vec, novar);
	
	add_vertices_to_model_compact(hex_model, verts_vec, novar);
	add_edges_to_model_compact(hex_model, face_vec, spa_verts_vec, novar);
	add_faces_to_model_compact(hex_model, face_vec, spa_verts_vec, novar);
	add_cells_to_model_compact(hex_model, novar);
}

void hs_example::make_gear_coarse( const char* filepath, ENTITY_LIST& elist) {
	
	ENTITY_LIST glist;
	FILE* file = acis_fopen(filepath, "r");
	outcome res = api_restore_entity_list(file, TRUE, glist);
	check_outcome(res);
	BODY* gbody = static_cast< BODY* >( glist[0] );
	
	FACE* cylinder[1];
	double radius = hs_point(13, -1.5, 0).length();
	double start_degree = atan(1.5 / 13) * 180 / M_PI; 
	double end_degree = 360 - start_degree;
	SPAposition center = SPAposition(0,0,-12.5);
	SPAvector normal = SPAvector(0,0,25);
	res = api_face_cylinder_cone(center, normal, radius, radius,
		start_degree,end_degree,1,NULL,cylinder[0]);
	check_outcome(res); 
	
	BODY* gnewbody;
	res = api_mk_by_faces(NULL, 1, cylinder, gnewbody);
	check_outcome(res);
	
	elist.add(gbody);
	elist.add(gnewbody);
}

void hs_example::make_gear_compact(const char* filepath, ENTITY_LIST& elist) {
	
	ENTITY_LIST glist;
	FILE* file = acis_fopen(filepath, "r");
	outcome res = api_restore_entity_list(file, TRUE, glist);
	check_outcome(res);
	BODY* gbody = static_cast< BODY* >( glist[0] );
	
	const size_t NUM_FACE_INS = 4;
	FACE* cylinder[NUM_FACE_INS];
	double radius[NUM_FACE_INS] = {hs_point(13, -1.5, 0).length(), 23, 29, 35};
	double start_degree[NUM_FACE_INS] = {atan(1.5 / 13) * 180 / M_PI, 0, 0, 0}; 
	double end_degree[NUM_FACE_INS] = {360 - start_degree[0], 360, 360, 360};
	SPAposition center[NUM_FACE_INS] = {SPAposition(0,0,-12.5), SPAposition(0,0,-3.5), SPAposition(0,0,-3.5), SPAposition(0,0,-3.5)};
	SPAvector normal[NUM_FACE_INS] = {SPAvector(0,0,25), SPAvector(0,0,7), SPAvector(0,0,7), SPAvector(0,0,7)};
	for (size_t i = 0; i < NUM_FACE_INS; ++i) {
		res = api_face_cylinder_cone(center[i], normal[i], radius[i], radius[i],
			start_degree[i],end_degree[i],1,NULL,cylinder[i]);
		check_outcome(res);
		
	}
	BODY* gnewbody;
	res = api_mk_by_faces(NULL, NUM_FACE_INS, cylinder, gnewbody);
	check_outcome(res);
	
	elist.add(gbody);
	elist.add(gnewbody);
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


void hs_example::get_vertices_of_gear_coarse( std::vector< FACE* >& face_vec, 
											 std::vector< SPAposition >& verts_vec, std::vector< VERTEX* >& spa_verts_vec, size_t& novar) 
{
	FACE* gface;
	EDGE* gedge;
	LOOP* gloop;
	COEDGE* gcoedge;
	
	std::vector< VERTEX* > top_verts_tmp, bot_verts_tmp;
	// top vertices
	gface = face_vec[85];
	gloop = gface->loop()->next();
	gcoedge = gloop->start();
	get_vertices_by_coedge(gcoedge, top_verts_tmp); 
	// bottom vertices
	gface = face_vec[83];
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
	gface = face_vec[85];
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
	gface = face_vec[11];
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
	gface = face_vec[11];
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
		FACE* ftemp = face_vec[i];
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
	
	gface = face_vec[86];
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
		FACE* ftemp = face_vec[i];
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
	
	gface = face_vec[4];
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
		FACE* ftemp = face_vec[i];
		EDGE* etemp = ftemp->loop()->start()->edge();
		
		SPAposition gpos( i == 0 ? etemp->start_pos() : etemp->end_pos() );
		top_verts.push_back(gpos);
		
		gpos.set_z( gpos.z() - 25 );
		bot_verts.push_back(gpos);
	}
	
	gface = face_vec[4];
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
		FACE* ftemp = face_vec[2];
		EDGE* etemp = ftemp->loop()->start()->next()->edge();
		
		SPAposition gpos( i == 0 ? etemp->end_pos() : etemp->start_pos() );
		top_verts.push_back(gpos);
		
		gpos.set_z( gpos.z() - 25 );
		bot_verts.push_back(gpos);
	}
	
	gface = face_vec[86];
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
	gface = face_vec[3]; 
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

void hs_example::get_vertices_of_gear_compact( std::vector< FACE* >& face_vec, 
											  std::vector< SPAposition >& verts_vec, std::vector< VERTEX* >& spa_verts_vec, size_t& novar) 
{
	FACE* gface;
	EDGE* gedge;
	LOOP* gloop;
	COEDGE* gcoedge;
	
	std::vector< VERTEX* > top_verts_tmp, bot_verts_tmp;
	// top vertices
	gface = face_vec[85];
	gloop = gface->loop()->next();
	gcoedge = gloop->start();
	get_vertices_by_coedge(gcoedge, top_verts_tmp); 
	// bottom vertices
	gface = face_vec[83];
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
	gface = face_vec[85];
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
	gface = face_vec[11];
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
	
	gface = face_vec[86];
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
	
	gface = face_vec[87];
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
	
	gface = face_vec[88];
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
	gface = face_vec[11];
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
		FACE* ftemp = face_vec[i];
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
	
	gface = face_vec[89];
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
		FACE* ftemp = face_vec[i];
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
	
	gface = face_vec[4];
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
		FACE* ftemp = face_vec[i];
		EDGE* etemp = ftemp->loop()->start()->edge();
		
		SPAposition gpos( i == 0 ? etemp->start_pos() : etemp->end_pos() );
		top_verts.push_back(gpos);
		
		gpos.set_z( gpos.z() - 25 );
		bot_verts.push_back(gpos);
	}
	
	gface = face_vec[4];
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
		FACE* ftemp = face_vec[2];
		EDGE* etemp = ftemp->loop()->start()->next()->edge();
		
		SPAposition gpos( i == 0 ? etemp->end_pos() : etemp->start_pos() );
		top_verts.push_back(gpos);
		
		gpos.set_z( gpos.z() - 25 );
		bot_verts.push_back(gpos);
	}
	
	gface = face_vec[89];
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
	gface = face_vec[3]; 
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

/** novar: number of vertices a round of gear **/
void hs_example::add_vertices_to_model_coarse( hs_model& hex_model, 
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

void hs_example::add_vertices_to_model_compact( hs_model& hex_model, 
											   std::vector< SPAposition >& verts_vec, size_t novar) 
{	
	std::vector< SPAposition >::iterator vitr = verts_vec.begin();
	for (size_t level = 0; level < 2; ++level) { 		
		for (size_t ctrl = 0; ctrl < 13; ++ctrl) {
			for (size_t sz = 0; sz < novar && vitr != verts_vec.end(); ) {
				vert_type vtype; 
				switch (ctrl) {
				case 0: case 1:
					vtype = CORNER_VERT; break;
				case 2: case 3: case 7: case 12:
					vtype = CREASE_VERT; break;
				case 4: case 5: case 6:
					vtype = ORDINARY_VERT; break;
				case 8:
					if (sz < 2) { vtype = CREASE_VERT;} 
					else		{ vtype = INNER_VERT;}
					break;
				case 9: 
					if (sz < 2) { vtype = CREASE_VERT;}
					else		{ vtype = ORDINARY_VERT;}
					break;
				case 10: 
					if (sz < 2) { vtype = CORNER_VERT;}
					else		{ vtype = CREASE_VERT;}
					break;
				case 11:
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

void hs_example::add_edges_to_model_coarse( hs_model& hex_model, std::vector< FACE* >& face_vec,
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
					spa_face = (level == 0) ? face_vec[85] : face_vec[83]; 
					spa_edge = spa_face->loop()->start()->edge();
					etype = CREASE_EDGE; break;
				case 1: 
					spa_face = (level == 0) ? face_vec[11] : face_vec[9];
					spa_edge = spa_face->loop()->next()->start()->edge();
					etype = CREASE_EDGE; break;
				case 2:
					spa_face = (level == 0) ? face_vec[11] : face_vec[9];
					spa_edge = spa_face->loop()->start()->edge();
					etype = CREASE_EDGE; break;
				case 3:
					if (st == 0) { etype = ORDINARY_EDGE; }
					else	     { etype = INNER_EDGE; } 
					break;
				case 5: { 
					spa_face = (level == 0) ? face_vec[3] : face_vec[7]; 
					COEDGE* spa_coedge = (level == 0) 
						? spa_face->loop()->next()->start()->next()->next()
						: spa_face->loop()->start();
					spa_edge = spa_coedge->next()->edge();
					etype = CREASE_EDGE; break;
						}
				case 7:
					spa_face = (level == 0) ? face_vec[3] : face_vec[7];
					spa_edge = (level == 0) ? spa_face->loop()->start()->edge() 
						: spa_face->loop()->next()->start()->edge();
					etype = CREASE_EDGE; break;
				case 4: 
					etype = ORDINARY_EDGE; break;
				case 6:
					if (st == 0) { 
						spa_edge = (level == 0)
							? face_vec[2]->loop()->start()->next()->edge()
							: face_vec[2]->loop()->start()->next()->next()->next()->edge();
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
							? face_vec[0]->loop()->start()->edge() 
							: face_vec[1]->loop()->start()->edge();
						} else { etype = ORDINARY_EDGE;}
						break;
				case 7:
					if ( st < 2 ) { 
						etype = CREASE_EDGE; 
						if (level == 0) {
							spa_edge = (st == 0) 
								? face_vec[0]->loop()->start()->next()->edge() 
								: face_vec[1]->loop()->start()->next()->next()->next()->edge();
						} else if (level == 1) {
							spa_edge = (st == 0) 
								? face_vec[0]->loop()->start()->next()->next()->next()->edge() 
								: face_vec[1]->loop()->start()->next()->edge();
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
				spa_edge = face_vec[st]->loop()->start()->next()->next()->edge();
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
					spa_edge = face_vec[st]->loop()->start()->next()->next()->edge();
					etype = CREASE_EDGE;
					} else { etype = INNER_EDGE; }
					break;
			case 6:
				if (st < 2) {
					spa_edge = face_vec[st]->loop()->start()->edge();
					etype = CREASE_EDGE;
					} else { etype = ORDINARY_EDGE; }
					break;
			}
			hex_model.add_edge(vidx, vidx + HALF_VERTS_NUM, spa_edge, etype);
			++st; ++vidx;
		} 
	}	
}

void hs_example::add_edges_to_model_compact( hs_model& hex_model, std::vector< FACE* >& face_vec,
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
		for (size_t ctrl = 0; ctrl < 11; ++ctrl) { 
			
			if (7 == ctrl || 8 == ctrl) { st = 1; ++vidx;} 
			else { st = 0; }
			for (; st < novar; ++st, ++vidx) {
				edge_type etype; 
				EDGE* spa_edge = NULL;
				FACE* spa_face = NULL;
				
				switch (ctrl) {
				case 0: 
					spa_face = (level == 0) ? face_vec[85] : face_vec[83]; 
					spa_edge = spa_face->loop()->start()->edge();
					etype = CREASE_EDGE; break;
				case 1: 
					spa_face = (level == 0) ? face_vec[11] : face_vec[9];
					spa_edge = spa_face->loop()->next()->start()->edge();
					etype = CREASE_EDGE; break;
				case 5:
					spa_face = (level == 0) ? face_vec[11] : face_vec[9];
					spa_edge = spa_face->loop()->start()->edge();
					etype = CREASE_EDGE; break;
				case 6:
					if (st == 0) { etype = ORDINARY_EDGE; }
					else	     { etype = INNER_EDGE; } 
					break;
				case 8: { 
					spa_face = (level == 0) ? face_vec[3] : face_vec[7]; 
					COEDGE* spa_coedge = (level == 0) 
						? spa_face->loop()->next()->start()->next()->next()
						: spa_face->loop()->start();
					spa_edge = spa_coedge->next()->edge();
					etype = CREASE_EDGE; break;
						}
				case 10:
					spa_face = (level == 0) ? face_vec[3] : face_vec[7];
					spa_edge = (level == 0) ? spa_face->loop()->start()->edge() 
						: spa_face->loop()->next()->start()->edge();
					etype = CREASE_EDGE; break;
				case 2: case 3: case 4: case 7: 
					etype = ORDINARY_EDGE; break;
				case 9:
					if (st == 0) { 
						spa_edge = (level == 0)
							? face_vec[2]->loop()->start()->next()->edge()
							: face_vec[2]->loop()->start()->next()->next()->next()->edge();
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
		for (ctrl = 0; ctrl < 12; ++ctrl) { 
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
				case 1: case 2: case 3: case 4: case 5: case 6: case 11:
					etype = ORDINARY_EDGE; break;
				case 7:
					etype = INNER_EDGE; break;
				case 8:
					if ( st < 2 ) { etype = ORDINARY_EDGE; }
					else	      { etype = INNER_EDGE; }
					break;
				case 9: 
					if ( st < 2 ) { 
						etype = CREASE_EDGE; 
						spa_edge = (st == 0) 
							? face_vec[0]->loop()->start()->edge() 
							: face_vec[1]->loop()->start()->edge();
						} else { etype = ORDINARY_EDGE;}
						break;
				case 10:
					if ( st < 2 ) { 
						etype = CREASE_EDGE; 
						if (level == 0) {
							spa_edge = (st == 0) 
								? face_vec[0]->loop()->start()->next()->edge() 
								: face_vec[1]->loop()->start()->next()->next()->next()->edge();
						} else if (level == 1) {
							spa_edge = (st == 0) 
								? face_vec[0]->loop()->start()->next()->next()->next()->edge() 
								: face_vec[1]->loop()->start()->next()->edge();
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
				spa_edge = face_vec[st]->loop()->start()->next()->next()->edge();
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
	for (size_t ctrl = 0; ctrl < 10; ++ctrl) {
		for (size_t st = 0; st < novar; ) {
			edge_type etype; 
			EDGE* spa_edge = NULL;
			switch(ctrl) {
			case 0: case 1:
				spa_edge = get_edge_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[vidx + 2 * novar]);
				assert ( spa_edge != NULL );
				etype = CREASE_EDGE; break;
			case 2: case 3: case 4: case 5: case 6: case 7: 
				etype = INNER_EDGE; break;
			case 8:
				if (st < 2) {
					spa_edge = face_vec[st]->loop()->start()->next()->next()->edge();
					etype = CREASE_EDGE;
					} else { etype = INNER_EDGE; }
					break;
			case 9:
				if (st < 2) {
					spa_edge = face_vec[st]->loop()->start()->edge();
					etype = CREASE_EDGE;
					} else { etype = ORDINARY_EDGE; }
					break;
			}
			hex_model.add_edge(vidx, vidx + HALF_VERTS_NUM, spa_edge, etype);
			++st; ++vidx;
		} 
	}
	
}

void hs_example::add_faces_to_model_coarse( hs_model& hex_model, std::vector< FACE* >& face_vec, 
										   std::vector< VERTEX* >& spa_verts_vec, size_t novar) 
{
	const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2);
	size_t vidx = 0;
	for (size_t level = 0; level < 2; ++level) {
		vidx = level * HALF_VERTS_NUM; 
		FACE* spa_face = (0 == level) ? face_vec[85] : face_vec[83];
		for(size_t st = 0; st < novar; st += 2, vidx += 2) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + novar, end + novar};
			hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
		}
		
		for (size_t ctrl = 0; ctrl < 8; ++ctrl) { 
			face_type ftype;
			spa_face = NULL;
			switch (ctrl) {
			case 0: spa_face = (0 == level) ? face_vec[85] : face_vec[83]; ftype = BORDER_FACE; break;
			case 1: spa_face = (0 == level) ? face_vec[10] : face_vec[8]; ftype = BORDER_FACE; break;
			case 2: spa_face = (0 == level) ? face_vec[11] : face_vec[9]; ftype = BORDER_FACE; break;	
			case 5: spa_face = face_vec[4]; ftype = BORDER_FACE; break;
			case 6: case 7:
				spa_face = (0 == level) ? face_vec[3] : face_vec[7]; ftype = BORDER_FACE; break;
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
			FACE* spa_face = (st == 0) ? face_vec[2] : NULL;
			face_type ftype = (st == 0) ? BORDER_FACE : INNER_FACE;
			hex_model.add_face_by_verts(verts,4,spa_face,ftype);
		}
		
		spa_face = (0 == level) ? face_vec[5] : face_vec[6];
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
				if (st == 0) { spa_face = face_vec[2]; ftype = BORDER_FACE; }
				else { ftype = INNER_FACE; }
				break;
			case 4:
				spa_face = face_vec[4]; ftype = BORDER_FACE; break;
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
					spa_face = face_vec[st]; 
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
			spa_face = face_vec[st];
			} else { ftype = INNER_FACE; } 
			size_t verts[] = {vidx, vidx + novar, vidx + 2*novar, vidx + 3*novar};
			hex_model.add_face_by_verts(verts,4,spa_face,ftype);
			
			size_t bot_vidx = vidx + HALF_VERTS_NUM;
			size_t bot_verts[] = {bot_vidx, bot_vidx + novar, bot_vidx + 2*novar, bot_vidx + 3*novar};
			hex_model.add_face_by_verts(bot_verts,4,spa_face,ftype);
	}
}

void hs_example::add_faces_to_model_compact( hs_model& hex_model, std::vector< FACE* >& face_vec, 
											std::vector< VERTEX* >& spa_verts_vec, size_t novar) 
{
	const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2);
	size_t vidx = 0;
	for (size_t level = 0; level < 2; ++level) {
		vidx = level * HALF_VERTS_NUM; 
		FACE* spa_face = (0 == level) ? face_vec[85] : face_vec[83];
		for(size_t st = 0; st < novar; st += 2, vidx += 2) {
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1;
			size_t verts[] = {vidx, end, vidx + novar, end + novar};
			hex_model.add_face_by_verts(verts,4,spa_face,BORDER_FACE);
		}
		
		for (size_t ctrl = 0; ctrl < 11; ++ctrl) { 
			face_type ftype;
			spa_face = NULL;
			switch (ctrl) {
			case 0: spa_face = (0 == level) ? face_vec[85] : face_vec[83]; ftype = BORDER_FACE; break;
			case 1: spa_face = (0 == level) ? face_vec[10] : face_vec[8]; ftype = BORDER_FACE; break;
			case 2: case 3: case 4: case 5:
				spa_face = (0 == level) ? face_vec[11] : face_vec[9]; ftype = BORDER_FACE; break;	
			case 8: spa_face = face_vec[4]; ftype = BORDER_FACE; break;
			case 9: case 10:
				spa_face = (0 == level) ? face_vec[3] : face_vec[7]; ftype = BORDER_FACE; break;
			case 6: case 7:
				ftype = INNER_FACE; break;
			}
			
			if ( 9 == ctrl || 8 == ctrl || 7 == ctrl ) { ++vidx; st = 1;} 
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
			FACE* spa_face = (st == 0) ? face_vec[2] : NULL;
			face_type ftype = (st == 0) ? BORDER_FACE : INNER_FACE;
			hex_model.add_face_by_verts(verts,4,spa_face,ftype);
		}
		
		spa_face = (0 == level) ? face_vec[5] : face_vec[6];
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
	
	for (size_t ctrl = 0; ctrl < 8; ++ctrl) {
		
		if ( 7 == ctrl ) { ++vidx; st = 1;}
		else { st = 0; }
		for(; st < novar; ++st, ++vidx) {
			face_type ftype; 
			FACE* spa_face = NULL; 
			switch ( ctrl ) {
			case 0: case 1: case 2: case 3: case 4: case 5:
				ftype = INNER_FACE; break;
			case 6:
				if (st == 0) { spa_face = face_vec[2]; ftype = BORDER_FACE; }
				else { ftype = INNER_FACE; }
				break;
			case 7:
				spa_face = face_vec[4]; ftype = BORDER_FACE; break;
			} 
			size_t end = (st + 1 == novar) ? vidx - novar + 1 : vidx + 1; 
			size_t verts[] = {vidx, end, vidx + HALF_VERTS_NUM, end + HALF_VERTS_NUM};
			hex_model.add_face_by_verts(verts,4,spa_face,ftype);
		}
	}
	
	vidx = 0;
	for (ctrl = 0; ctrl < 9; ++ctrl) {
		for (st = 0; st < novar; ++st, ++vidx) {
			face_type ftype; 
			FACE* spa_face = NULL;
			switch ( ctrl ) {
			case 0:
				ftype = BORDER_FACE;
				spa_face = get_face_by_2_vert(spa_verts_vec[vidx], spa_verts_vec[vidx + 3 * novar]);
				assert ( spa_face != NULL );
				break;
			case 1: case 2: case 3: case 4: case 5: case 6: case 7:
				ftype = INNER_FACE; break;
			case 8:
				if (st < 2) {
					ftype = BORDER_FACE;
					spa_face = face_vec[st]; 
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
			spa_face = face_vec[st];
		} else { ftype = INNER_FACE; } 
		size_t verts[] = {vidx, vidx + novar, vidx + 2*novar, vidx + 3*novar};
		hex_model.add_face_by_verts(verts,4,spa_face,ftype);
		
		size_t bot_vidx = vidx + HALF_VERTS_NUM;
		size_t bot_verts[] = {bot_vidx, bot_vidx + novar, bot_vidx + 2*novar, bot_vidx + 3*novar};
		hex_model.add_face_by_verts(bot_verts,4,spa_face,ftype);
	}
}

void hs_example::add_cells_to_model_coarse( hs_model& hex_model, size_t novar) {
	
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

void hs_example::add_cells_to_model_compact( hs_model& hex_model, size_t novar) {
	
	const size_t HALF_VERTS_NUM = static_cast< size_t >( hex_model.vert_size() / 2 );
	size_t vidx = 0; 
	for (size_t ctrl = 0; ctrl < 9; ++ctrl) {
		size_t st;
		if ( 8 == ctrl ) { ++vidx; st = 1;}
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

void hs_example::mechanicalpart( hs_model& hex_model, ENTITY_LIST& elist) {
	BODY *top_part_out, *top_part_in, *top_block;
	outcome res = api_solid_cylinder_cone(SPAposition(-2.5,0,10), SPAposition(2.5,0,10), 
		10, 10, 10, NULL, top_part_out);
	check_outcome(res);
	
	res = api_solid_block(SPAposition(-2.5, 10, 10), SPAposition(2.5, -10, -5), top_block);
	check_outcome(res);
	
	check_outcome( api_unite(top_block, top_part_out) );
	
	res = api_solid_cylinder_cone(SPAposition(-2.5,0,10), SPAposition(2.5,0,10), 
		5, 5, 5, NULL, top_part_in);
	check_outcome(res);
	
	check_outcome( api_subtract(top_part_in, top_part_out) );
	
	res = api_change_body_trans(top_part_out, NULL);
	check_outcome(res);
	
	BODY *bot_out, *bot_in;
	res = api_solid_cylinder_cone(SPAposition(0,-20,-10), SPAposition(0,20,-10), 
		10, 10, 10, NULL, bot_out);
	check_outcome(res);
	
	res = api_solid_cylinder_cone(SPAposition(0,-20,-10), SPAposition(0,20,-10), 
		5, 5, 5, NULL, bot_in);
	check_outcome(res);
	
	check_outcome( api_subtract(bot_in, bot_out) );
	
	check_outcome( api_unite(bot_out, top_part_out) );
	
	elist.add(top_part_out);
	
	/** add vertices to model **/
	using namespace hex_subdiv;
	const size_t VERT_NUM = 68;
	hs_point vert_pos[VERT_NUM] = { 
		// top 1
		hs_point(2.5,0,20), 
			hs_point(2.5,-10,10), hs_point(2.5,-10,5),
			hs_point(2.5,10,5), hs_point(2.5,10,10), 
			hs_point(2.5,0,15), hs_point(2.5,-5,10), hs_point(2.5,0,5), hs_point(2.5,5,10), 	 
			// top 2
			hs_point(-2.5,0,20), 
			hs_point(-2.5,-10,10), hs_point(-2.5,-10,5),
			hs_point(-2.5,10,5), hs_point(-2.5,10,10), 
			hs_point(-2.5,0,15), hs_point(-2.5,-5,10), hs_point(-2.5,0,5), hs_point(-2.5,5,10),  
			// bottom 1
			hs_point(2.5,-20,-5.669873),hs_point(-2.5,-20,-5.669873),
			hs_point(-5,-20,-10),hs_point(0,-20,-15),hs_point(5,-20,-10),
			hs_point(2.5,-20,-0.317542),hs_point(-2.5,-20,-0.317542),
			hs_point(-10,-20,-10),hs_point(0,-20,-20),hs_point(10,-20,-10),
			// bottom 2
			hs_point(2.5,-10,-5.669873),hs_point(-2.5,-10,-5.669873),
			hs_point(-5,-10,-10),hs_point(0,-10,-15),hs_point(5,-10,-10),
			hs_point(2.5,-10,-0.317542),hs_point(-2.5,-10,-0.317542),
			hs_point(-10,-10,-10),hs_point(0,-10,-20),hs_point(10,-10,-10),
			// bottom 3
			hs_point(2.5,0,-5.669873),hs_point(-2.5,0,-5.669873),
			hs_point(-5,0,-10),hs_point(0,0,-15),hs_point(5,0,-10),
			hs_point(2.5,0,-0.317542),hs_point(-2.5,0,-0.317542),
			hs_point(-10,0,-10),hs_point(0,0,-20),hs_point(10,0,-10),
			// bottom 4
			hs_point(2.5,10,-5.669873),hs_point(-2.5,10,-5.669873),
			hs_point(-5,10,-10),hs_point(0,10,-15),hs_point(5,10,-10),
			hs_point(2.5,10,-0.317542),hs_point(-2.5,10,-0.317542),
			hs_point(-10,10,-10),hs_point(0,10,-20),hs_point(10,10,-10),
			// bottom 5
			hs_point(2.5,20,-5.669873),hs_point(-2.5,20,-5.669873),
			hs_point(-5,20,-10),hs_point(0,20,-15),hs_point(5,20,-10),
			hs_point(2.5,20,-0.317542),hs_point(-2.5,20,-0.317542),
			hs_point(-10,20,-10),hs_point(0,20,-20),hs_point(10,20,-10)
	};
	vert_type vtype[VERT_NUM] = {
		// TOP 1
		CREASE_VERT, 
			CORNER_VERT, CREASE_VERT,
			CREASE_VERT, CORNER_VERT,
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT,
			// TOP 2
			CREASE_VERT, 
			CORNER_VERT, CREASE_VERT,
			CREASE_VERT, CORNER_VERT,
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT,
			// BOTTOM 1
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT,
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT,
			// BOTTOM 2
			ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			CORNER_VERT, CORNER_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			// BOTTOM 3
			ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			CREASE_VERT, CREASE_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			// BOTTOM 4
			ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			CORNER_VERT, CORNER_VERT, ORDINARY_VERT, ORDINARY_VERT, ORDINARY_VERT,
			// BOTTOM 5
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT,
			CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT, CREASE_VERT
	};
	
	for (size_t i = 0; i < VERT_NUM; ++i) {
		hex_model.add_vert(vert_pos[i], vtype[i]);
	}
	
	/** add edges to model **/
	const size_t EDGE_NUM = 156;
	size_t vidx_of_edge[EDGE_NUM][2] = {
		// top 1  16
		{53,3},{3,4},{4,0},{0,1},{1,2},{2,33},
		{43,7},{3,7},{4,8},{0,5},{1,6},{2,7},
		{5,6},{6,7},{7,8},{8,5},
		// top 2   16
		{54,12},{12,13},{13,9},{9,10},{10,11},{11,34},
		{44,16},{12,16},{13,17},{9,14},{10,15},{11,16},
		{14,15},{15,16},{16,17},{17,14},
		// top 3  9
		{0,9},{1,10},{2,11},{3,12},{4,13},
		{5,14},{6,15},{7,16},{8,17},
		// bottom 1 15
		{18,19},{19,20},{20,21},{21,22},{22,18},
		{23,24},{24,25},{25,26},{26,27},{27,23},
		{18,23},{19,24},{20,25},{21,26},{22,27},
		// bottom 2 15
		{28,29},{29,30},{30,31},{31,32},{32,28},
		{33,34},{34,35},{35,36},{36,37},{37,33},
		{28,33},{29,34},{30,35},{31,36},{32,37},
		// bottom 3 15
		{38,39},{39,40},{40,41},{41,42},{42,38},
		{43,44},{44,45},{45,46},{46,47},{47,43},
		{38,43},{39,44},{40,45},{41,46},{42,47},
		// bottom 4 15
		{48,49},{49,50},{50,51},{51,52},{52,48},
		{53,54},{54,55},{55,56},{56,57},{57,53},
		{53,48},{49,54},{50,55},{51,56},{52,57},
		// bottom 5 15
		{58,59},{59,60},{60,61},{61,62},{62,58},
		{63,64},{64,65},{65,66},{66,67},{67,63},
		{63,58},{59,64},{60,65},{61,66},{62,67},
		// bottom 6 10
		{18,28},{19,29},{20,30},{21,31},{22,32},
		{23,33},{24,34},{25,35},{26,36},{27,37},
		// bottom 7 10
		{38,28},{39,29},{40,30},{41,31},{42,32},
		{43,33},{44,34},{45,35},{46,36},{47,37},
		// bottom 8 10
		{38,48},{39,49},{40,50},{41,51},{42,52},
		{43,53},{44,54},{45,55},{46,56},{47,57},
		// bottom 9 10
		{58,48},{59,49},{60,50},{61,51},{62,52},
		{63,53},{64,54},{65,55},{66,56},{67,57}
	};
	edge_type etype[EDGE_NUM] = {
		// top 1
		CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			// top 2
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			// top 3
			ORDINARY_EDGE,CREASE_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,CREASE_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 1
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 2
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,
			// bottom 3
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			INNER_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,
			// bottom 4
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,INNER_EDGE,
			// bottom 5
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,CREASE_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 6
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 7
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE, CREASE_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 8
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			CREASE_EDGE, CREASE_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			// bottom 9
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,
			ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE,ORDINARY_EDGE
	};
	SHELL* spa_shell = top_part_out->lump()->shell();
	EDGE* spa_edge[EDGE_NUM] = {
		// top 1
		edge_at(face_at(spa_shell, 6)->loop(), 2),edge_at(face_at(spa_shell, 6)->loop(), 2),
			edge_at(face_at(spa_shell, 6)->loop(), 3),edge_at(face_at(spa_shell, 6)->loop(), 3),
			edge_at(face_at(spa_shell, 6)->loop(), 4),edge_at(face_at(spa_shell, 6)->loop(), 4),
			NULL,NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 6)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 6)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 6)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 6)->loop()->next(), 1),
			// top 2
			edge_at(face_at(spa_shell, 9)->loop(), 4),edge_at(face_at(spa_shell, 9)->loop(), 4),
			edge_at(face_at(spa_shell, 9)->loop(), 3),edge_at(face_at(spa_shell, 9)->loop(), 3),
			edge_at(face_at(spa_shell, 9)->loop(), 2),edge_at(face_at(spa_shell, 9)->loop(), 2),
			NULL,NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 9)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 9)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 9)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 9)->loop()->next(), 1),
			// top 3
			NULL,edge_at(face_at(spa_shell,5)->loop(),3),NULL,NULL,edge_at(face_at(spa_shell,8)->loop(),3),
			NULL,NULL,NULL,NULL,
			// bottom 1
			edge_at(face_at(spa_shell, 3)->loop(), 1),edge_at(face_at(spa_shell, 3)->loop(), 1),
			edge_at(face_at(spa_shell, 3)->loop(), 1),edge_at(face_at(spa_shell, 3)->loop(), 1),
			edge_at(face_at(spa_shell, 3)->loop(), 1),
			edge_at(face_at(spa_shell, 3)->loop()->next(), 1),edge_at(face_at(spa_shell, 3)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 3)->loop()->next(), 1),edge_at(face_at(spa_shell, 3)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 3)->loop()->next(), 1),
			NULL,NULL,NULL,NULL,NULL,
			// bottom 2
			NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 5)->loop(), 1),NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			// bottom 3
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			// bottom 4
			NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 8)->loop(), 1),NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			// bottom 5
			edge_at(face_at(spa_shell, 4)->loop(), 1),edge_at(face_at(spa_shell, 4)->loop(), 1),
			edge_at(face_at(spa_shell, 4)->loop(), 1),edge_at(face_at(spa_shell, 4)->loop(), 1),
			edge_at(face_at(spa_shell, 4)->loop(), 1),
			edge_at(face_at(spa_shell, 4)->loop()->next(), 1),edge_at(face_at(spa_shell, 4)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 4)->loop()->next(), 1),edge_at(face_at(spa_shell, 4)->loop()->next(), 1),
			edge_at(face_at(spa_shell, 4)->loop()->next(), 1),
			NULL,NULL,NULL,NULL,NULL,
			// bottom 6
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			// bottom 7
			NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 6)->loop(), 1), edge_at(face_at(spa_shell, 9)->loop(), 1),NULL,NULL,NULL,
			// bottom 8
			NULL,NULL,NULL,NULL,NULL,
			edge_at(face_at(spa_shell, 6)->loop(), 1), edge_at(face_at(spa_shell, 9)->loop(), 1),NULL,NULL,NULL,
			// bottom 9
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL
	};
	
	for (i = 0; i < EDGE_NUM; ++i) {
		hex_model.add_edge(vidx_of_edge[i][0],vidx_of_edge[i][1],spa_edge[i], etype[i]);
	}
	
	/** add faces to model **/
	const size_t FACE_NUM = 113;
	size_t vidx_of_face[FACE_NUM][4] = {
		// TOP 1  6
		{3,7,43,53},{3,4,8,7},{4,0,5,8},{5,0,1,6},{1,2,7,6},{7,2,33,43},
			// TOP 2  6
		{12,16,44,54},{12,13,17,16},{13,9,14,17},{14,9,10,15},{10,11,16,15},{16,11,34,44},
		// TOP 3  16
		{7,43,44,16},{3,7,16,12},{4,8,17,13},{5,0,9,14},{1,6,15,10},{2,7,16,11},
		{3,53,54,12},{3,4,13,12},{0,4,13,9},{0,1,10,9},{1,2,11,10},{2,33,34,11},
		{5,6,15,14},{6,7,16,15},{7,8,17,16},{8,5,14,17},
		// BOTTOM 1   25
		{18,19,24,23},{19,20,25,24},{20,21,26,25},{21,22,27,26},{22,18,23,27}, 
		{28,29,34,33},{29,30,35,34},{30,31,36,35},{31,32,37,36},{32,28,33,37}, 
		{38,39,44,43},{39,40,45,44},{40,41,46,45},{41,42,47,46},{42,38,43,47}, 
		{48,49,54,53},{49,50,55,54},{50,51,56,55},{51,52,57,56},{52,48,53,57}, 
		{58,59,64,63},{59,60,65,64},{60,61,66,65},{61,62,67,66},{62,58,63,67},
		// BOTTOM 2  20
		{18,19,28,29},{19,20,29,30},{20,21,30,31},{21,22,31,32},{22,18,32,28}, 
		{38,39,28,29},{39,40,29,30},{40,41,30,31},{41,42,31,32},{42,38,32,28}, 
		{38,39,48,49},{39,40,49,50},{40,41,50,51},{41,42,51,52},{42,38,52,48}, 
		{58,59,48,49},{59,60,49,50},{60,61,50,51},{61,62,51,52},{62,58,52,48},
		// BOTTOM 3  20
		{23,24,33,34},{24,25,34,35},{25,26,35,36},{26,27,36,37},{27,23,37,33},
		{33,34,43,44},{34,35,44,45},{35,36,45,46},{36,37,46,47},{37,33,47,43},
		{43,44,53,54},{44,45,54,55},{45,46,55,56},{46,47,56,57},{47,43,57,53},
		{53,54,63,64},{54,55,64,65},{55,56,65,66},{56,57,66,67},{57,53,67,63}, 
		// BOTTOM 4 20
		{18,23,28,33},{19,24,29,34},{20,25,30,35},{21,26,31,36},{22,27,32,37},
		{28,33,38,43},{29,34,39,44},{30,35,40,45},{31,36,41,46},{32,37,42,47},
		{38,43,48,53},{39,44,49,54},{40,45,50,55},{41,46,51,56},{42,47,52,57},
		{48,53,58,63},{49,54,59,64},{50,55,60,65},{51,56,61,66},{52,57,62,67}
	};
	
	face_type ftype[FACE_NUM] = {
		// TOP 1
		BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// TOP 2
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// TOP 3
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// BOTTOM 1
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// BOTTOM 2
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// BOTTOM 3
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			INNER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			INNER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,BORDER_FACE,
			// BOTTOM 4
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,
			INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE,INNER_FACE
	};
	
	FACE* spa_face[FACE_NUM] = {
		// TOP 1
		face_at(spa_shell,6),face_at(spa_shell,6),face_at(spa_shell,6),face_at(spa_shell,6),
			face_at(spa_shell,6),face_at(spa_shell,6),
			// TOP 2
			face_at(spa_shell,9),face_at(spa_shell,9),face_at(spa_shell,9),face_at(spa_shell,9),
			face_at(spa_shell,9),face_at(spa_shell,9),
			// TOP 3
			NULL,NULL,NULL,NULL,NULL,NULL,
			face_at(spa_shell,8),face_at(spa_shell,8),
			face_at(spa_shell,10),face_at(spa_shell,10),
			face_at(spa_shell,5),face_at(spa_shell,5),
			face_at(spa_shell,7),face_at(spa_shell,7),face_at(spa_shell,7),face_at(spa_shell,7),
			// BOTTOM 1
			face_at(spa_shell,3),face_at(spa_shell,3),face_at(spa_shell,3),face_at(spa_shell,3),face_at(spa_shell,3),
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			face_at(spa_shell,4),face_at(spa_shell,4),face_at(spa_shell,4),face_at(spa_shell,4),face_at(spa_shell,4),
			// BOTTOM 2
			face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),
			face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),
			face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),
			face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),face_at(spa_shell,2),
			// BOTTOM 3
			face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),
			NULL,face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),
			NULL,face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),
			face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),face_at(spa_shell,1),
			// bottom 4
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL,NULL,NULL
	};
	
	for (i = 0; i < FACE_NUM; ++i) {
		hex_model.add_face_by_verts(vidx_of_face[i], 4, spa_face[i], ftype[i]);
	}
	
	/** add cells to model **/
	const size_t CELL_NUM = 26;
	size_t vidx_of_cell[CELL_NUM][8] = {
		// TOP
		{3,7,43,53,12,16,44,54},{3,4,8,7,12,13,17,16},{4,0,5,8,13,9,14,17},
		{0,1,6,5,9,10,15,14},{1,2,7,6,10,11,16,15},{2,33,43,7,11,34,44,16},
		// BOTTOM
		{18,19,24,23,28,29,34,33},{19,20,25,24,29,30,35,34},{20,21,26,25,30,31,36,35},{21,22,27,26,31,32,37,36},{22,18,23,27,32,28,33,37},
		{28,29,34,33,38,39,44,43},{29,30,35,34,39,40,45,44},{30,31,36,35,40,41,46,45},{31,32,37,36,41,42,47,46},{32,28,33,37,42,38,43,47},
		{38,39,44,43,48,49,54,53},{39,40,45,44,49,50,55,54},{40,41,46,45,50,51,56,55},{41,42,47,46,51,52,57,56},{42,38,43,47,52,48,53,57},
		{48,49,54,53,58,59,64,63},{49,50,55,54,59,60,65,64},{50,51,56,55,60,61,66,65},{51,52,57,56,61,62,67,66},{52,48,53,57,62,58,63,67},
	};
	
	for (i = 0; i < CELL_NUM; ++i) {
		hex_model.add_cell_by_verts(vidx_of_cell[i], 8);
	}
} // END

/** find face locating at idx **/
FACE* hs_example::face_at( SHELL* spa_shell, int idx ) {
	FACE* fitr = spa_shell->first_face();
	while(--idx > 0 && fitr) {
		fitr = fitr->next();
	}
	assert( 0 == idx );
	return fitr;
}


EDGE* hs_example::edge_at( LOOP* spa_loop, int idx ) {
	COEDGE* ceitr = spa_loop->start();
	while(--idx > 0 && ceitr) {
		ceitr = ceitr->next();
	}
	assert( 0 == idx );
	return ceitr->edge();
}



