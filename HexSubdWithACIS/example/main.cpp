#include "acis_mgr.h"
#include "hs_model.h"
#include "hs_subdiv.h"
#include "hs_example.h"

#include "lists.hxx"
#include "cstrapi.hxx"
#include "alltop.hxx"

#include <IOSTREAM>
#include <FSTREAM>


void test_cylinder(size_t, size_t);
void test_gear();


void main() {
	
	size_t type = 6;
	size_t sub_num = 3;
//	test_cylinder(type, sub_num);
	test_gear();

}

void test_cylinder(size_t type, size_t sub_num) {

	acis_mgr acis("booleans");
	
	hex_subdiv::hs_model hex_model;

	ENTITY_LIST cylist;
	hex_subdiv::hs_example hex_example;
	if ( type == 4 ) {
		hex_example.cylinder_4(hex_model, cylist);
	} else if ( type == 6 ) {
		hex_example.cylinder_6(hex_model, cylist);	
	} else { return; }
/*
	hex_model.print_vert();
	hex_model.print_edge();
	hex_model.print_face();
	hex_model.print_cell();
*/

	ENTITY_LIST origlist;
	hex_model.acis_wire(origlist);

	hex_subdiv::hs_subdiv sub(&hex_model); 
	ENTITY_LIST wirelist, lap_smt_list;
	for (size_t i = 0; i < sub_num; ++i) {
		wirelist.clear();
		sub.inter_subdiv();
		lap_smt_list.clear();
		sub.laplacian_smoothing_adaptive();
		sub.print_info();
	} 

	std::vector<double> jacob;
	sub.jacobian(jacob);
	std::vector<double>::iterator itr = jacob.begin();
	for (size_t j = 0; itr != jacob.end(); ++itr) {
		std::cout << j++ << ": " << *itr << std::endl;
	} 

	wirelist.add(cylist);
	origlist.add(cylist);
	lap_smt_list.add(cylist);
	acis.save_sat_file(".\\model\\bicy\\sat\\bicy_origin.sat", origlist);
	acis.save_sat_file(".\\model\\bicy\\sat\\bicy_sub_nolap.sat", wirelist);
	acis.save_sat_file(".\\model\\bicy\\sat\\bicy_sub_lap.sat", lap_smt_list);
}

void test_gear() {

	acis_mgr acis("booleans");
	
	ENTITY_LIST wirelist, sublist;
	hex_subdiv::hs_model hex_model;
	
	hex_subdiv::hs_example hex_example;
	
//	hex_example.make_gear(".\\model\\gear\\sat\\gear.sat", wirelist);
//	acis.save_sat_file(".\\model\\gear\\sat\\gear-model.sat", wirelist);
//	wirelist.clear();
	hex_example.gear(wirelist, hex_model,".\\model\\gear\\sat\\gear-model.sat");
//	wirelist.clear();
//	hex_model.acis_wire(wirelist);
// 	hex_model.check_edge();
// 	hex_model.check_face();
 	hex_subdiv::hs_subdiv sub(&hex_model);
	sub.inter_subdiv();
	sub.inter_subdiv();
	sub.laplacian_smoothing_adaptive();
	
	sub.model()->save_file(".\\model\\gear\\obj\\2_sub.obj", "obj");

// 	sub.model()->check_edge();
// 	sub.model()->check_face();
// 	sub.model()->write_vert(".\\model\\gear\\txt\\2_vert.txt");
// 	sub.model()->write_edge(".\\model\\gear\\txt\\2_edge.txt");
// 	sub.model()->print_edge(".\\model\\gear\\txt\\1_edge.txt");
// 	sub.model()->print_face(".\\model\\gear\\txt\\1_face.txt");
// 	sub.model()->print_cell(".\\model\\gear\\txt\\1_cell.txt");
// 	sub.model()->acis_wire(sublist);
// 	sublist.add(wirelist);
//	acis.save_sat_file(".\\model\\gear\\sat\\gear-test.sat", wirelist); 
//	show_wire_model(sublist, ".\\model\\gear\\txt\\2_vert.txt", ".\\model\\gear\\txt\\2_edge.txt");
// 	acis.save_sat_file(".\\model\\gear\\sat\\gear-sub.sat", sublist);
}
