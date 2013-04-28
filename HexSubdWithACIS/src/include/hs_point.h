#ifndef HS_POINT_H_
#define HS_POINT_H_

#include <CASSERT>
#include "hex_subdiv.h"

namespace hex_subdiv {

// absolute value
#define ABS( var ) ((var) < 0 ? (-(var)) : (var))
#define EPSILON (1.0e-20)
	
	class hs_point {
		
	public:
		hs_point() : _x(0.0f), _y(0.0f), _z(0.0f) {}
		hs_point(double x, double y, double z) : _x(x), _y(y), _z(z) {}
		hs_point(const hs_point & p) : _x(p._x), _y(p._y), _z(p._z) {}
		
		virtual ~hs_point(){}
		
		// operator
		hs_point& operator=  (const hs_point &);
		hs_point& operator+= (const hs_point &);
		hs_point& operator-= (const hs_point &);
		hs_point& operator*= (double f);
		hs_point& operator/= (double d);
		
		double x() const { return _x; }
		double y() const { return _y; }
		double z() const { return _z; }

		void set_x(double x) { _x = x; }
		void set_y(double y) { _y = y; }
		void set_z(double z) { _z = z; }
		void set_coord(double x, double y, double z) { _x = x; _y = y; _z = z; }
		
		double	dot(const hs_point &);
		double	length();
		void	zero();
		hs_point cross(const hs_point &);
		hs_point& unit();
		void	print() const;
		
	private:
		double _x, _y, _z;
	};
	

	
	inline hs_point operator+ (const hs_point& p1, const hs_point& p2) {
		return hs_point(p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z());
	}
	
	inline hs_point operator- (const hs_point& p1, const hs_point& p2) {
		return hs_point(p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z());
	}
	
	inline hs_point operator* (const hs_point& p, double d) {
		return hs_point(p.x() * d, p.y() * d, p.z() * d);
	}
	
	inline hs_point operator* (double d, const hs_point& p) {
		return p * d;
	}
	
	inline hs_point operator/ (const hs_point& p, double d) {
		assert( d > EPSILON || d < -EPSILON);
		return p * ( 1 / d );
	}
	
	inline bool operator== (const hs_point& p1, const hs_point& p2) {
		return (ABS(p1.x() - p2.x()) < EPSILON) 
			&& (ABS(p1.y() - p2.y()) < EPSILON) 
			&& (ABS(p1.z() - p2.z()) < EPSILON);
	}
	
} // namespace

#endif