#include <CSTDLIB>
#include <IOSTREAM>

#include "hs_point.h"

using namespace hex_subdiv;

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
	assert(d > EPSILON || d < -EPSILON);
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


void hs_point::print() const {
	std::cout << "[" << _x << ", "
		<< _y << ", " << _z << "]" << std::endl;
}

// vector unitization 
hs_point& hs_point::unit() {
	double len = length();
	assert( len > EPSILON );  
	len = 1.0f / len;
	_x *= len; _y *= len; _z *= len;
	
	return *this;
}
