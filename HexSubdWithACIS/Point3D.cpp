#include <CASSERT>
#include <CMATH>
#include <CSTDLIB>
#include <IOSTREAM>

#include "Point3D.h"

namespace HexMesh {
	
#define DELTA (1.0e-20)
	
	// operator
	Point3D& Point3D::operator= (const Point3D& p) {
		if (this == &p) return *this;
		_x = p._x; _y = p._y; _z = p._z;
		
		return *this;
	}
	
	Point3D& Point3D::operator+= (const Point3D &p) {
		_x += p._x; _y += p._y; _z += p._z;
		return *this;
	}
	
	Point3D& Point3D::operator-= (const Point3D &p) {
		_x -= p._x; _y -= p._y; _z -= p._z;
		return *this;
	}
	
	Point3D& Point3D::operator*= (double f){
		_x *= f; _y *= f; _z *= f;
		return *this;
	}
	
	Point3D& Point3D::operator/= (double d){
		assert(d > DELTA || d < -DELTA);
		_x /= d; _y /= d; _z /= d;
		return *this;
	}
	
	double Point3D::dot(const Point3D &p){
		return _x * p._x + _y * p._y + _z * p._z;
	}
	
	Point3D Point3D::cross(const Point3D &p){
		return Point3D(_y*p._z - _z*p._y, _z*p._x - _x*p._z, _x*p._y - _y*p._x);
	}
	
	double Point3D::length() {
		return sqrt(_x * _x + _y * _y + _z * _z);
	}
	
	void Point3D::zero() {
		_x = _y = _z = 0.0;
	}
	
	void Point3D::print() const{
		std::cout << "[" << _x << ", "
			<< _y << ", " << _z << "]" << std::endl;
	}

	Point3D& Point3D::unify() {
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

	Point3D operator+ (const Point3D& p1, const Point3D& p2) {
		return Point3D(p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z());
	}

	Point3D operator- (const Point3D& p1, const Point3D& p2) {
		return Point3D(p1.x() - p2.x(), p1.y() - p2.y(), p1.z() - p2.z());
	}

	Point3D operator* (const Point3D& p, double d) {
		return Point3D(p.x() * d, p.y() * d, p.z() * d);
	}

	Point3D operator* (double d, const Point3D& p) {
		return p * d;
	}

	Point3D operator/ (const Point3D& p, double d) {
		assert( d > DELTA || d < -DELTA);
		return p * (1 / d);
	}
	
	
}// namespace