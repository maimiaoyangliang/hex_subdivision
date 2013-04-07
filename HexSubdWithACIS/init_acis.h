#ifndef INIT_ACIS_H_
#define INIT_ACIS_H_
// disable lots of map warning
#pragma warning(disable: 4786)

#include <MAP>
#include <STRING>

#include "ckoutcom.hxx"
//class outcome;

class init_acis {
	
public:
	init_acis(const std::string&, bool = true);
	~init_acis();
	
private:
	init_acis(const init_acis&);
	init_acis& operator = (const init_acis&);
	
	inline void check_result(const outcome&, const std::string& );
	inline std::string create_string( const std::string& );
	void init_map();
	
	typedef outcome (*FunPtr)();
	std::map<std::string, FunPtr> funMap;
	
	std::string cond;
	bool modeller;
};

#endif