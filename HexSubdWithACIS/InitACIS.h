#ifndef INITACIS_H_
#define INITACIS_H_
// disable lots of map warning
#pragma warning(disable: 4786)

#include <MAP>
#include <STRING>

#include "ckoutcom.hxx"
//class outcome;

class InitACIS {
public:
	InitACIS(const std::string&, bool = true);
	~InitACIS();

private:
	InitACIS(const InitACIS&);
	InitACIS& operator = (const InitACIS&);

	inline void check_result(const outcome&, const std::string& );
	inline std::string create_string( const std::string& );
	void init_map();

	typedef outcome (*FunPtr)();
	std::map<std::string, FunPtr> funMap;
	std::string cond;
	bool modeller;
};

#endif