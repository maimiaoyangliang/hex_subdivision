#include "InitACIS.h"

// acis header files
#include "cstrapi.hxx" // constructors
#include "kernapi.hxx" // modeller, kernel
#include "boolapi.hxx" // booleans
#include "sweepapi.hxx" // sweeping
#include "blendapi.hxx" // blending
#include "coverapi.hxx" // covering
#include "sboolapi.hxx" // sbooleans
#include "af_api.hxx" // faceter

 


InitACIS::InitACIS(const std::string& rhs, bool m):cond(rhs), funMap(), modeller(m) {
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

InitACIS::~InitACIS() {
	
	std::string funName = create_string("terminate");
	FunPtr fptr = funMap[funName];
	outcome res = fptr();
	check_result(res, funName);
	
	if (modeller) {
		res = api_stop_modeller();
		check_result(res, "api_stop_modeller");
	}
	
}

std::string InitACIS::create_string(const std::string& rhs) {
	return "api_" + rhs + "_" + cond;
}

void InitACIS::check_result(const outcome& res, const std::string& funName) {
	acis_printf("proceeding: %s \n", funName.c_str());
	if ( !res.ok() ) {
		acis_printf("ERROR IN FUNCTION %s !\n",funName.c_str());
	}
}

void InitACIS::init_map() {
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