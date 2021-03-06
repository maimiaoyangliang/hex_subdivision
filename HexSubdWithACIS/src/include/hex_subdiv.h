#ifndef HEX_SUBDIV_H_
#define HEX_SUBDIV_H_

#pragma warning(disable:4786)

#include <SET>
#include <LIMITS>

namespace hex_subdiv {

	typedef std::set<size_t> int_set;
	typedef std::set<size_t>::iterator int_set_iter;
	typedef std::set<size_t>::const_iterator int_set_citer;
	
	const double dlimit = std::numeric_limits<double>::max();
	
} // end namespace

#endif

