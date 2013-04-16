#ifndef INIT_ACIS_H_
#define INIT_ACIS_H_
// disable lots of map warning
#pragma warning(disable: 4786)

#include <MAP>
#include <STRING>

#include "ckoutcom.hxx" 

class ENTITY_LIST;

class acis_mgr {
	
public:
	acis_mgr(const std::string&, bool = true);
	~acis_mgr();

	void save_sat_file(const char*, ENTITY_LIST&);
	
private:
	acis_mgr(const acis_mgr&);
	acis_mgr& operator = (const acis_mgr&);
	
	inline void check_result(const outcome&, const std::string& );
	inline std::string create_string( const std::string& );
	void init_map();
	
	typedef outcome (*FunPtr)();
	std::map<std::string, FunPtr> funMap;
	
	std::string cond;
	bool modeller;
};

#endif