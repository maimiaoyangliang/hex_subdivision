#include "api.hxx"
#include "ckoutcom.hxx"
#include "lists.hxx"
#include "fileinfo.hxx"
#include "kernapi.hxx"

void save_sat_file(const char* filename, ENTITY_LIST& savelist) {
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