#include "acis_mgr.h"

// acis header files
#include "cstrapi.hxx" // constructors
#include "kernapi.hxx" // modeller, kernel
#include "boolapi.hxx" // booleans
#include "sweepapi.hxx" // sweeping
#include "blendapi.hxx" // blending
#include "coverapi.hxx" // covering
#include "sboolapi.hxx" // sbooleans
#include "af_api.hxx" // faceter

#include "lists.hxx"
#include "fileinfo.hxx"



acis_mgr::acis_mgr(const std::string& rhs, bool m):cond(rhs), funMap(), modeller(m) {
	init_map();
	outcome res;
	if (modeller) {
		res = api_start_modeller(0);
		check_result(res, "api_start_modeller");
	}
	
	std::string funName = create_string("initialize");
	FunPtr fptr = funMap[funName];
	res = fptr();
	check_result(res, funName);
}

acis_mgr::~acis_mgr() {
	
	std::string funName = create_string("terminate");
	FunPtr fptr = funMap[funName];
	outcome res = fptr();
	check_result(res, funName);
	
	if (modeller) {
		res = api_stop_modeller();
		check_result(res, "api_stop_modeller");
	}
	
}

std::string acis_mgr::create_string(const std::string& rhs) {
	return "api_" + rhs + "_" + cond;
}

void acis_mgr::check_result(const outcome& res, const std::string& funName) {
	acis_printf("proceeding: %s \n", funName.c_str());
	if ( !res.ok() ) {
		acis_printf("ERROR IN FUNCTION %s !\n",funName.c_str());
	}
}

void acis_mgr::init_map() {
	funMap["api_initialize_booleans"]	  = api_initialize_booleans;
	funMap["api_terminate_booleans"]      = api_terminate_booleans;
	funMap["api_initialize_constructors"] = api_initialize_constructors;
	funMap["api_terminate_constructors"]  = api_terminate_constructors;
	funMap["api_initialize_sweeping"]     = api_initialize_sweeping;
	funMap["api_terminate_sweeping"]      = api_terminate_sweeping;
	funMap["api_initialize_blending"]     = api_initialize_blending;
	funMap["api_terminate_blending"]      = api_terminate_blending;
	funMap["api_initialize_covering"]     = api_initialize_covering;
	funMap["api_terminate_covering"]      = api_terminate_covering;
	funMap["api_initialize_kernel"]       = api_initialize_kernel;
	funMap["api_terminate_kernel"]        = api_terminate_kernel;
	funMap["api_initialize_sbooleans"]    = api_initialize_sbooleans;
	funMap["api_terminate_sbooleans"]     = api_terminate_sbooleans;
	funMap["api_initialize_faceter"]      = api_initialize_faceter;
	funMap["api_terminate_faceter"]       = api_terminate_faceter;
}


void acis_mgr::save_sat_file(const char* filename, ENTITY_LIST& savelist) {
	API_NOP_BEGIN
		FileInfo info;
	info.set_product_id("ZJU");
	info.set_units(1.0);
	outcome res = api_set_file_info(FileId|FileUnits,info);
	check_outcome(res);
	res = api_save_version(14, 0);
	check_outcome(res);
	
	FILE* fp = acis_fopen(filename,"w");
	if ( !fp ) {
		acis_printf("Don't open the file \n");
		return;
	}
	
    res = api_save_entity_list(fp, TRUE, savelist);
	check_outcome(res);
	
	acis_fclose(fp);
	acis_printf("The file %s is saved successfully ! \n", filename);
	API_NOP_END
}