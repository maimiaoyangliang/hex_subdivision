#ifndef HS_POINT_H_
#define HS_POINT_H_

namespace hex_subdiv {
	
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
		hs_point& unify();
		inline void	print() const;
		
	private:
		double _x, _y, _z;
	};
	
	hs_point operator+ (const hs_point&, const hs_point&);
	hs_point operator- (const hs_point&, const hs_point&);
	
	hs_point operator* (const hs_point&, double);
	hs_point operator* (double, const hs_point&);
	
	hs_point operator/ (const hs_point&, double);
	
	
} // namespace

#endif