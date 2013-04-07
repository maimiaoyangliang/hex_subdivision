#include <CASSERT>
#include <CMATH>
#include <CSTDLIB>
#include <IOSTREAM>

#include "Point3D.h"

namespace hex_subdiv {
	
#define DELTA (1.0e-20)
	
	// operator
	hs_point& hs_point::operator= (const hs_point& p) {
		if (this == &p) return *this;
		_x = p._x; _y = p._y; _z = p._z;
		
		return *this;
	}
	
	hs_point& hs_point::operator+= (const hs_point &p) {
		_x += p._x; _y += p._y; _z += p._z;
		return *this;
	}
	
	hs_point& hs_point::operator-= (const hs_point &p) {
		_x -= p._x; _y -= p._y; _z -= p._z;
		return *this;
	}
	
	hs_point& hs_point::operator*= (double f){
		_x *= f; _y *= f; _z *= f;
		return *this;
	}
	
	hs_point& hs_point::operator/= (double d){
		assert(d > DELTA || d < -DELTA);
		_x /= d; _y /= d; _z /= d;
		return *this;
	}
	
	double hs_point::dot(const hs_point &p){
		return _x * p._x + _y * p._y + _z * p._z;
	}
	
	hs_point hs_point::cross(const hs_point &p){
		return hs_point(_y*p._z - _z*p._y, _z*p._x - _x*p._z, _x*p._y - _y*p._x);
	}
	
	double hs_point::length() {
		return sqrt(_x * _x + _y * _y + _z * _z);
	}
	
	void hs_point::zero() {
		_x = _y = _z = 0.0;
	}
	
	void hs_point::print() const{
		std::cout << "[" << _x << ", "
			<< _y << ", " << _z << "]" << std::endl;
	}

	hs_point& hs_point::unify() {
		double len = length();
		assert( len > DELTA );
		/*if( len < 1.0e-20 ) {
		_x = rand()%1000+20; _y = rand()%1000+20; _z = rand()%1000+20;
		len=length();
		_x/=len; _y/=len; _z/=len;
	} else */{
		len = 1.0f/len;
		_x *= len; _y *= len; _z *= len;
		}
		
		return *this;
	}

	hs_point operator+ (const hs_point& p1, const hs_point& p2) {
		return hs_point(p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z());
	}

	hs_point operator- (const hs_point& p1, const hs_point& p2) {
		return hs_point(p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z());
	}

	hs_point operator* (const hs_point& p, double d) {
		return hs_point(p.x() * d, p.y() * d, p.z() * d);
	}

	hs_point operator* (double d, const hs_point& p) {
		return p * d;
	}

	hs_point operator/ (const hs_point& p, double d) {
		assert( d > DELTA || d < -DELTA);
		return p * (1 / d);
	}
	
	
}// namespace