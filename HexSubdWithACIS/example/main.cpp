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
	acis.save_sat_file("..\\model\\bicy\\sat\\bicy_origin.sat", origlist);
	acis.save_sat_file("..\\model\\bicy\\sat\\bicy_sub_nolap.sat", wirelist);
	acis.save_sat_file("..\\model\\bicy\\sat\\bicy_sub_lap.sat", lap_smt_list);
}

void test_gear() {

	acis_mgr acis("booleans");
	
	ENTITY_LIST wirelist, init_mesh, sublist;
	hex_subdiv::hs_model hex_model;
	
	hex_subdiv::hs_example hex_example;
	
// 	hex_example.make_gear(".\\model\\gear\\sat\\gear.sat", wirelist);
// 	acis.save_sat_file(".\\model\\gear\\sat\\gear-model.sat", wirelist);
// 	wirelist.clear();

	hex_example.gear(wirelist, hex_model,"..\\model\\gear\\sat\\gear-model.sat");

	hex_model.acis_wire(init_mesh);

 	hex_subdiv::hs_subdiv sub(&hex_model);
	sub.inter_subdiv(); // first subdivision
//	sub.inter_subdiv(); // second subdivision
	sub.laplacian_smoothing_adaptive();

	/** compute each cell jacobian value and write to file **/
	std::vector<double> jacob;
	sub.jacobian(jacob);
	char* jacob_file_path = "..\\model\\gear\\txt\\jacobian.txt";
	FILE* file = fopen(jacob_file_path, "w");
	if ( file ) {
		std::vector<double>::iterator itr = jacob.begin();
		for (size_t j = 0; itr != jacob.end(); ++itr) {
			fprintf(file, "cell %d, jacobian: %f\n", j++, *itr);
		}
		fclose(file);
		printf("%s is saved successfully...\n", jacob_file_path);
	}
 
	/** save model to .obj file **/
	sub.model()->save_file("..\\model\\gear\\obj\\2_sub.obj", "obj");

	/** save model to .sat file **/
	sub.model()->acis_wire(sublist);
	sublist.add(wirelist);
	acis.save_sat_file("..\\model\\gear\\sat\\gear-sub.sat", sublist);
	acis.save_sat_file("..\\model\\gear\\sat\\init-mesh.sat", init_mesh);
}
