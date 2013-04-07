#ifndef POINT3D_H_
#define POINT3D_H_

namespace HexMesh {
	
	class Point3D {
	public:
		Point3D() : _x(0.0f), _y(0.0f), _z(0.0f) {}
		Point3D(double x, double y, double z) : _x(x), _y(y), _z(z) {}
		Point3D(const Point3D & p) : _x(p._x), _y(p._y), _z(p._z) {}
		
		virtual ~Point3D(){}
		
		// operator
		Point3D& operator=  (const Point3D &);
		Point3D& operator+= (const Point3D &);
		Point3D& operator-= (const Point3D &);
		Point3D& operator*= (double f);
		Point3D& operator/= (double d);
		
		double x() const { return _x; }
		double y() const { return _y; }
		double z() const { return _z; }
		
		double	dot(const Point3D &);
		double	length();
		Point3D cross(const Point3D &);
		Point3D& unify();
		void	zero();
		inline void	print() const;
		
	private:
		double _x, _y, _z;
	};
	
	Point3D operator+ (const Point3D&, const Point3D&);
	Point3D operator- (const Point3D&, const Point3D&);

	Point3D operator* (const Point3D&, double);
	Point3D operator* (double, const Point3D&);

	Point3D operator/ (const Point3D&, double);

	
} // namespace

#endif