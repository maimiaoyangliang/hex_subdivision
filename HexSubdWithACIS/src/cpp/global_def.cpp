/** stl header files **/
#include <FSTREAM>
#include <IOSTREAM>
#include <SSTREAM>
#include <STRING>
#include <VECTOR>

/** ACIS header files **/
#include "acis_mgr.h"
#include "hs_model.h"
#include "hs_subdiv.h"
#include "hs_example.h"
#include "lists.hxx"
#include "cstrapi.hxx"
#include "alltop.hxx"

#include "global_def.h"

/*
template<template <class, class> class Container, class T, class V> 
void print_jacobian(const char*, Container<T, V>&);*/
extern void print_jacobian(const char*, const std::vector<double>&);

template<typename T>
std::string to_string(T i) {
	std::stringstream ss;
	std::string str;
	ss << i; ss >> str;
	return str;
}

void test_gear_coarse() {
	
	acis_mgr acis("constructors");
	
	hex_subdiv::hs_model hex_model;
	
	hex_subdiv::hs_example hex_example;
	
	ENTITY_LIST gear_model;
	std::string origin_model_path("..\\model\\gear\\sat\\gear.sat");
	std::string new_model_path("..\\model\\gear\\sat\\coarse_gear_model.sat");
	hex_example.make_gear_coarse(origin_model_path.c_str(), gear_model);
	acis.save_sat_file(new_model_path.c_str(), gear_model);

	ENTITY_LIST spa_model;
	hex_example.gear_coarse(spa_model, hex_model, new_model_path.c_str());

	/** get initial mesh **/
	ENTITY_LIST init_mesh;
	hex_model.acis_wire(init_mesh);	
	init_mesh.add(spa_model);
	acis.save_sat_file("..\\model\\gear\\sat\\coarse_init_mesh.sat", init_mesh);

	const size_t NUM_OF_SUB = 2;
	std::stringstream ss;
	std::string itos;
	/** no laplacian smoothing **/
	{
		hex_subdiv::hs_subdiv sub(&hex_model);
		for (size_t s = 0; s < NUM_OF_SUB; ++s) {
			sub.inter_subdiv();
			
			/** compute each cell jacobian value and write to file **/
			std::vector<double> jacob;
			sub.jacobian(jacob);
			std::string jacob_path("..\\model\\gear\\txt\\coarse_nolap_jacobian_");
			jacob_path += to_string(s + 1) + ".txt";
			print_jacobian(jacob_path.c_str(), jacob);
			
			/** save model to .obj file **/
			std::string obj_path("..\\model\\gear\\obj\\coarse_nolap_sub_");
			obj_path += to_string(s + 1) + ".obj";
			sub.model()->save_file(obj_path.c_str(), "obj");
			/** save model to .sat file **/
			ENTITY_LIST sublist_1;
			sub.model()->acis_wire(sublist_1);
			sublist_1.add(spa_model);
			std::string sat_file("..\\model\\gear\\sat\\coarse_nolap_sub_");
			sat_file += to_string(s + 1) + ".sat";
			acis.save_sat_file(sat_file.c_str(), sublist_1);
		}
	}
	/** laplacian smoothing **/
	{
		hex_subdiv::hs_subdiv sub(&hex_model);
		for (size_t s = 0; s < NUM_OF_SUB; ++s) {
			sub.inter_subdiv();
			sub.laplacian_smoothing_adaptive();
			
			/** compute each cell jacobian value and write to file **/
			std::vector<double> jacob;
			sub.jacobian(jacob);
			std::string jacob_path("..\\model\\gear\\txt\\coarse_jacobian_");
			jacob_path += to_string(s + 1) + ".txt";
			print_jacobian(jacob_path.c_str(), jacob);
			
			/** save model to .obj file **/
			std::string obj_path("..\\model\\gear\\obj\\coarse_sub_");
			obj_path += to_string(s + 1) + ".obj";
			sub.model()->save_file(obj_path.c_str(), "obj");
			/** save model to .sat file **/
			ENTITY_LIST sublist_1;
			sub.model()->acis_wire(sublist_1);
			sublist_1.add(spa_model);
			std::string sat_file("..\\model\\gear\\sat\\coarse_gear_sub_");
			sat_file += to_string(s + 1) + ".sat";
			acis.save_sat_file(sat_file.c_str(), sublist_1);
		}
	}
	
}

void test_gear_compact() {
	
	acis_mgr acis("constructors");
	
	hex_subdiv::hs_model hex_model;
	
	hex_subdiv::hs_example hex_example;
	
	ENTITY_LIST gear_model;
	std::string origin_model_path("..\\model\\gear\\sat\\gear.sat");
	std::string new_model_path("..\\model\\gear\\sat\\compact_gear_model.sat");
	hex_example.make_gear_compact(origin_model_path.c_str(), gear_model);
	acis.save_sat_file(new_model_path.c_str(), gear_model); 
	ENTITY_LIST spa_model;
	hex_example.gear_compact(spa_model, hex_model, new_model_path.c_str());

	/** get initial mesh **/
	ENTITY_LIST init_mesh;
	hex_model.acis_wire(init_mesh);	
	init_mesh.add(spa_model);
	acis.save_sat_file("..\\model\\gear\\sat\\compact_init_mesh.sat", init_mesh);

	const size_t NUM_OF_SUB = 2;
	std::stringstream ss;
	std::string itos;
	/** no laplacian smoothing **/
	{
		hex_subdiv::hs_subdiv sub(&hex_model);
		for (size_t s = 0; s < NUM_OF_SUB; ++s) {
			sub.inter_subdiv();
			
			/** compute each cell jacobian value and write to file **/
			std::vector<double> jacob;
			sub.jacobian(jacob);
			std::string jacob_path("..\\model\\gear\\txt\\compact_nolap_jacobian_");
			jacob_path += to_string(s + 1) + ".txt";
			print_jacobian(jacob_path.c_str(), jacob);
			
			/** save model to .obj file **/
			std::string obj_path("..\\model\\gear\\obj\\compact_nolap_sub_");
			obj_path += to_string(s + 1) + ".obj";
			sub.model()->save_file(obj_path.c_str(), "obj");
			/** save model to .sat file **/
			ENTITY_LIST sublist_1;
			sub.model()->acis_wire(sublist_1);
			sublist_1.add(spa_model);
			std::string sat_file("..\\model\\gear\\sat\\compact_nolap_sub_");
			sat_file += to_string(s + 1) + ".sat";
			acis.save_sat_file(sat_file.c_str(), sublist_1);
		}
	}
	
	/** laplacian smoothing **/
	{
		hex_subdiv::hs_subdiv sub(&hex_model);
		for (size_t s = 0; s < NUM_OF_SUB; ++s) {
			sub.inter_subdiv();
			sub.laplacian_smoothing_adaptive();
			
			/** compute each cell jacobian value and write to file **/
			std::vector<double> jacob;
			sub.jacobian(jacob);
			std::string jacob_path("..\\model\\gear\\txt\\compact_jacobian_");
			jacob_path += to_string(s + 1) + ".txt";
			print_jacobian(jacob_path.c_str(), jacob);
			
			/** save model to .obj file **/
			std::string obj_path("..\\model\\gear\\obj\\compact_sub_");
			obj_path += to_string(s + 1) + ".obj";
			sub.model()->save_file(obj_path.c_str(), "obj");
			/** save model to .sat file **/
			ENTITY_LIST sublist_1;
			sub.model()->acis_wire(sublist_1);
			sublist_1.add(spa_model);
			std::string sat_file("..\\model\\gear\\sat\\compact_gear_sub_");
			sat_file += to_string(s + 1) + ".sat";
			acis.save_sat_file(sat_file.c_str(), sublist_1);
		}
	}
	
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


/** write jacobian vector to file **/
/*
template<template <class, class> class Container, class T, class V> 
void print_jacobian(const char*, Container<T, V>&)*/
void print_jacobian(const char* filepath, const std::vector<double>& jaco_vec) {
	std::fstream fs(filepath, std::ios::out);
	if ( !fs.is_open() ) {
		std::cerr << filepath << "fails to open..." << std::endl; 
		return;
	}

	/*Container<T>*/
	std::vector<double>::const_iterator itr = jaco_vec.begin();
	for (size_t j = 0; itr != jaco_vec.end(); ++itr) {
		fs << "cell " << j++ << " jacobian: " << *itr << std::endl;
	}
	fs.close();
	std::cout << filepath << " is saved successfully..." << std::endl;
}
