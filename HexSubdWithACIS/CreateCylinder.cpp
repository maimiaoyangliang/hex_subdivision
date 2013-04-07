#include <set>
#include <IOSTREAM>

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

#include "Subdivision.h"
#include "HexModel.h"
// #include "HexCell.h"
// #include "HexFace.h"
// #include "HexEdge.h"
// #include "HexVertex.h"


extern void save_sat_file(const char*, ENTITY_LIST&);


void create_cylinder_with_hole() {
	BODY* cylinder1;
	outcome res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10),5,5,5,NULL,cylinder1);
	check_outcome(res);
	
	BODY* cylinder2;
	res = api_solid_cylinder_cone(SPAposition(0,0,0), SPAposition(0,0,10),2,2,2,NULL,cylinder2);
	check_outcome(res);
	
	res = api_subtract(cylinder2, cylinder1);
	check_outcome(res);
	
	ENTITY_LIST elist, wirelist;
	res = api_change_body_trans(cylinder1, NULL);
	check_outcome(res);
	elist.add(cylinder1);

	EDGE* bot_inside = cylinder1->lump()->shell()->face()->loop()->start()->edge();
	EDGE* top_inside = cylinder1->lump()->shell()->face()->loop()->next()->start()->edge();
	EDGE* bot_outside = cylinder1->lump()->shell()->face()->next()->loop()->start()->edge();
	EDGE* top_outside = cylinder1->lump()->shell()->face()->next()->loop()->next()->start()->edge();

/*	SPAposition sp = bot_inside->start_pos();
	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
	sp = top_inside->start_pos();
	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
	sp = bot_outside->start_pos();
	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
	sp = top_outside->start_pos();
	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
*/
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

// 	std::set< FACE* > fs;

	FACE* cone_inside = cylinder1->lump()->shell()->face_list();
	FACE* cone_outside = cone_inside->next();
	FACE* plane_bottom = cone_outside->next();
	FACE* plane_top = plane_bottom->next();

// 	fs.insert(cone_inside); fs.insert(cone_outside);
// 	fs.insert(plane_bottom); fs.insert(plane_top);
// 
// 	std::cout << fs.size() << std::endl;
// 
// 	SPAposition sp = cone_inside->loop()->start()->edge()->start_pos();
// 	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
// 	sp = cone_outside->loop()->start()->edge()->start_pos();
// 	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
// 	sp = plane_bottom->loop()->start()->edge()->start_pos();
// 	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());
// 	sp = plane_top->loop()->start()->edge()->start_pos();
// 	acis_printf("%f,%f,%f\n", sp.x(), sp.y(), sp.z());

	FACE* acis_face[] = {
			plane_bottom, plane_bottom, plane_bottom, plane_bottom,
			plane_top, plane_top,plane_top,plane_top,
			cone_inside, cone_inside, cone_inside, cone_inside,
			cone_outside, cone_outside, cone_outside, cone_outside,
			NULL, NULL, NULL, NULL
	};



	save_sat_file(".\\model\\mycylinder-origin.sat", elist);
	
	hex_subdiv::hs_model hexModel;
	
	const size_t VERT_NUM = 16;
	using hex_subdiv::hs_point;
	hs_point vert_pos[VERT_NUM] = {
		hs_point(2,0,0),  hs_point(0,2,0),  hs_point(-2,0,0),  hs_point(0,-2,0),
			hs_point(2,0,10), hs_point(0,2,10), hs_point(-2,0,10), hs_point(0,-2,10),
			hs_point(5,0,0),  hs_point(0,5,0),  hs_point(-5,0,0),  hs_point(0,-5,0),
			hs_point(5,0,10), hs_point(0,5,10), hs_point(-5,0,10), hs_point(0,-5,10)
	};
	size_t i = 0;
	for (; i < VERT_NUM; ++i) {
		hexModel.add_vert(vert_pos[i]);
	}
	
	const size_t EDGE_NUM = 32;
	size_t vert_idx_in_edge[EDGE_NUM][2] = { // indices of vertex in edge
		{0,1},{1,2},{2,3},{3,0},		 // bottom inside
		{8,9},{9,10},{10,11},{11,8},     // bottom outside
		{0,8},{1,9},{2,10},{3,11},       // bottom link
		{4,5},{5,6},{6,7},{7,4},         // top inside
		{12,13},{13,14},{14,15},{15,12}, // top outside
		{4,12},{5,13},{6,14},{7,15},	 // top link
		{0,4},{1,5},{2,6},{3,7},		 // side inside
		{8,12},{9,13},{10,14},{11,15}	 // side outside
	};


	for (i = 0; i < EDGE_NUM; ++i) {
		size_t i1 = vert_idx_in_edge[i][0];
		size_t i2 = vert_idx_in_edge[i][1];
//		hexModel.add_edge(wirelist, vert_pos[i1], vert_pos[i2], i1, i2, geom_edge[i]);
		hexModel.add_edge(i1, i2);
	}

	hexModel.set_acis_edge(geom_edge, EDGE_NUM);
	
	const size_t FACE_NUM = 20;
	/*
	size_t vert_ind_in_face[FACE_NUM][4] = { // indices of vertex in face
	{0,1,9,8}, {1,2,10,9}, {2,3,11,10}, {3,0,8,11}, // bottom
	{4,5,13,12}, {5,6,14,13}, {6,7,15,14}, {7,4,12,15}, // top
	{0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}, // side inside
	{8,9,13,12}, {9,10,14,13}, {10,11,15,14}, {11,8,12,15}, // side outside
	{0,8,12,4}, {1,9,13,5}, {2,10,14,6}, {3,11,15,7} // side link
};*/
	size_t edge_idx_in_face[FACE_NUM][4] = { // indices of edge in face
		{0,8,4,9},{1,9,5,10},{2,10,6,11},{3,11,7,8}, // bottom 
		{12,20,16,21},{13,21,17,22},{14,22,18,23},{15,23,19,20}, // top 
		{0,25,12,24},{1,26,13,25},{2,27,14,26},{3,24,15,27}, // side inside 
		{4,29,16,28},{5,30,17,29},{6,31,18,30},{7,28,19,31}, // side outside
		{8,28,20,24},{9,29,21,25},{10,30,22,26},{11,31,23,27} // bottom 
	};

	
	for (i = 0; i < FACE_NUM; ++i) {
//		hexModel.add_face( /*vert_ind_in_face[i],*/ edge_ind_in_face[i], 4, acis_face[i]);
		hexModel.add_face(edge_idx_in_face[i], 4);
	}

	hexModel.set_acis_face(acis_face, FACE_NUM);
	
	const size_t CELL_NUM = 4;
	/*
	size_t vert_ind_in_cell[CELL_NUM][8] = {
	{0,1,9,8,4,5,13,12},{1,2,10,9,5,6,14,13},
	{2,3,11,10,6,7,15,14},{3,0,8,11,7,4,12,15}
	};
	size_t edge_ind_in_cell[CELL_NUM][12] = {
	{0,8,4,9,12,20,16,21,24,25,29,28},{1,10,5,9,13,22,17,21,25,26,30,29},
	{2,11,6,10,14,23,18,22,26,27,31,30},{3,8,7,11,15,20,19,23,27,24,28,31}
};*/
	size_t face_idx_in_cell[CELL_NUM][6] = {
		{0,4,8,16,12,17},{1,5,9,17,13,18},
		{2,6,10,18,14,19},{3,7,11,19,15,16}
	};
	for (i = 0; i < CELL_NUM; ++i) {
		hexModel.add_cell(face_idx_in_cell[i], 6);
	}
	
	ENTITY_LIST wl;
 	hex_subdiv::hs_subdiv sub(&hexModel);
	const size_t SUBDIVISION_NUM = 2;
	for (size_t s = 0; s < SUBDIVISION_NUM; ++s) {
		wl.clear();
		sub.inter_subdiv(wl);
	}



/*	BODY* wb;
	res = api_unite_edges(wirelist, wb, NULL);
	check_outcome(res);
	wirelist.clear();
	wirelist.add(wb);*/

	wl.add(cylinder1);
	save_sat_file(".\\model\\mycylinder-inter.sat", wl);
	save_sat_file(".\\model\\mycylinder-wire.sat", wirelist);
	
}
