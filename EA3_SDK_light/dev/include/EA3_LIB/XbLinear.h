// (c) 1998-2010 Electronic Arts Inc.

#ifndef XbLinear_h
#define XbLinear_h


#ifdef XB_LINEAR_USE_STREAMS
#include "EAiostream.h"
#endif
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4710 4514)
#endif
#include <float.h>
#include <math.h>
#include <string.h>

class XbMatrix;
class XbEuler;

#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif

#ifndef M_PIf
#define M_PIf             3.14159265f
#endif

#define dot3(a,b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])
#define dot4(a,b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3])

class XbVec3f {
  public:
    // default constructor does no initialization
    inline XbVec3f( void ) {
    }

#ifndef SWIG
    // these initialize the vector
    inline XbVec3f( const float v[3] ) {
      memcpy(vec,v,sizeof(vec));
    }
    inline XbVec3f( const double v[3] ) {
      vec[0] = (float)v[0]; vec[1] = (float)v[1]; vec[2] = (float)v[2];
    }
    inline XbVec3f( float x, float y, float z ) {
      vec[0] = x; vec[1] = y; vec[2] = z;
    }

    inline ~XbVec3f( void ) {
    }
#ifdef XB_LINEAR_USE_STREAMS
    // I/O
    inline EAistream& read( EAistream& is ) {
      return is >> vec[0] >> vec[1] >> vec[2];
    }
    inline EAostream& write( EAostream& os ) const {
      return os << vec[0] << ' ' << vec[1] << ' ' << vec[2];
    }

    friend EAistream& operator>>( EAistream &is, XbVec3f &v )
    {
	return is >> v.vec[0] >> v.vec[1] >> v.vec[2];
    }

    friend EAostream& operator<<( EAostream &os, const XbVec3f &v )
    {
	return os << v.vec[0] << ' ' << v.vec[1] << ' ' << v.vec[2];
    }
#endif
    // set value from array of 3 components
    inline XbVec3f& setValue( const float v[3] ) {
      memcpy(vec,v,sizeof(vec));
      return *this;
    }

    inline XbVec3f& setValue( const double v[3] ) {
      vec[0] = (float)v[0]; vec[1] = (float)v[1]; vec[2] = (float)v[2];
      return *this;
    }
#endif

    // sets value from 3 individual components
    inline XbVec3f& setValue( float x, float y, float z ) {
      vec[0] = x; vec[1] = y; vec[2] = z;
      return *this;
    }

#ifndef SWIG
    // returns array of 3 components
    inline const float *getValue( void ) const {
      return vec;
    }

    // get 3 individual components
    inline void getValue( float& x, float& y, float& z ) const {
      x = vec[0]; y = vec[1]; z = vec[2];
    }
    inline void getValue( double& x, double& y, double& z ) const {
      x = vec[0]; y = vec[1]; z = vec[2];
    }

    // get individual component by index
    inline float& operator[]( int i ) {return(vec[i]);}
    inline const float& operator[]( int i ) const {return(vec[i]);}
#endif

    // return length of vector
    inline float length( void ) const {
      return sqrtf(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
    }
    // return squared length of vector
    inline float sqrlength( void ) const {
      return (vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
    }

    // make vector unit length
    inline float normalize( void ) {        
      float r = length();
      if (r<0.000001f) {
        vec[0] = 1.0f;
        vec[1] = 0.0f;
        vec[2] = 0.0f;
      } else {
        vec[0]/=r;
        vec[1]/=r;
        vec[2]/=r;
      }
			// if 1.0+vec[i]*vec[i] == 1.0 zero out vec[i]
			// this prevents vectors like: 1,1e-05,0
			if (vec[0]*vec[0] < FLT_EPSILON) 
				vec[0] = 0.0f;
			if (vec[1]*vec[1] < FLT_EPSILON) 
				vec[1] = 0.0f;
			if (vec[2]*vec[2]< FLT_EPSILON) 
				vec[2] = 0.0f;
      return r;
    }

    // negates each component of vector
    inline XbVec3f& negate( void ) {
      vec[0] = -vec[0]; vec[1] = -vec[1]; vec[2] = -vec[2];
      return *this;
    }

    // right-handed cross product
    inline XbVec3f cross( const XbVec3f& v ) const {
      return XbVec3f( vec[1] * v.vec[2] - vec[2] * v.vec[1],
        vec[2] * v.vec[0] - vec[0] * v.vec[2],
        vec[0] * v.vec[1] - vec[1] * v.vec[0] );
    }

    // dot product
    inline float dot( const XbVec3f& v ) const {
      return(v[0]*vec[0]+v[1]*vec[1]+v[2]*vec[2]);
    }

    // angle between vectors
    inline float angleTo( const XbVec3f& v ) const {
      return (float)atan2( cross( v ).length(), dot( v ) );
    }
    // returns a perpendicular vector
    inline XbVec3f perpendicular( void ) const {
      double x = fabs( vec[0] );
      double y = fabs( vec[1] );
      double z = fabs( vec[2] );
      if ( x > y && x > z ) return XbVec3f( vec[1] + vec[2], -vec[0], -vec[0] );
      if ( y > x && y > z ) return XbVec3f( -vec[1], vec[0] + vec[2], -vec[1] );
      return XbVec3f( -vec[2], -vec[2], vec[0] + vec[1] );
    }

#ifndef SWIG
    inline XbVec3f& operator*=( float d ) {
      vec[0] *= d; vec[1] *= d; vec[2] *= d;
      return *this;
    };
    inline XbVec3f& operator/=( float d ) {
      vec[0] /= d; vec[1] /= d; vec[2] /= d;
      return *this;
    }
    inline XbVec3f& operator+=( const XbVec3f& v ) {
      vec[0] += v.vec[0]; vec[1] += v.vec[1]; vec[2] += v.vec[2];
      return *this;
    }
    inline XbVec3f& operator-=( const XbVec3f& v ) {
      vec[0] -= v.vec[0]; vec[1] -= v.vec[1]; vec[2] -= v.vec[2];
      return *this;
    }
    inline XbVec3f operator-( void ) const {
      return XbVec3f( -vec[0], -vec[1], -vec[2] );
    }
    inline bool operator==( const XbVec3f& v) const {
      return vec[0] == v.vec[0] && vec[1] == v.vec[1] && vec[2] == v.vec[2];
    }
	inline friend int  operator !=(const XbVec3f &v1, const XbVec3f &v2)
    { return !(v1 == v2); }

    inline friend XbVec3f operator*( const XbVec3f& v, float d ) {
      return XbVec3f( v.vec[0] * d, v.vec[1] * d, v.vec[2] * d );
    }
    inline friend XbVec3f operator*( float d, const XbVec3f& v ) {
      return XbVec3f( v.vec[0] * d, v.vec[1] * d, v.vec[2] * d );
    }
    inline friend XbVec3f operator/( const XbVec3f& v, float d ) {
      return XbVec3f( v.vec[0] / d, v.vec[1] / d, v.vec[2] / d );
    }
    inline friend XbVec3f operator+( const XbVec3f& a, const XbVec3f& b ) {
      return XbVec3f( a.vec[0] + b.vec[0], a.vec[1] + b.vec[1], a.vec[2] + b.vec[2] );
    }
    inline friend XbVec3f operator-( const XbVec3f& a, const XbVec3f& b ) {
      return XbVec3f( a.vec[0] - b.vec[0], a.vec[1] - b.vec[1], a.vec[2] - b.vec[2] );
    }

    friend void deg2rad( XbVec3f &v )
    {
        v.vec[0] *= (float)M_PI / 180.0f;
        v.vec[1] *= (float)M_PI / 180.0f;
        v.vec[2] *= (float)M_PI / 180.0f;
    }

    friend void rad2deg( XbVec3f &v )
    {
        v.vec[0] *= 180.0f / (float)M_PI;
        v.vec[1] *= 180.0f / (float)M_PI;
        v.vec[2] *= 180.0f / (float)M_PI;
    }
#endif  // #ifndef SWIG

  protected:
    float vec[3];
};

#ifndef SWIG

class XbVec2f
{
    public:

  // default constructor does no initialization
  inline XbVec2f()  {}

  // Constructor given an existing XbVec2f
  inline XbVec2f( const XbVec2f &v )    { setValue( v.vec ); }

  // Constructor given an array of 2 components
  inline XbVec2f( const float v[2] )    { setValue(v); }

  // Constructor given 2 individual components
  inline XbVec2f( float x, float y )    { setValue(x, y); }

  // Returns dot (inner) product of vector and another vector
  inline float dot( const XbVec2f &v ) const
    { return v.vec[0] * vec[0] + v.vec[1] * vec[1]; }

  // Returns pointer to array of 2 components
  inline const float  *getValue() const  { return vec; }

  // Returns 2 individual components
  inline void getValue(float &x, float &y) const
    { x = vec[0]; y = vec[1]; }

  // Returns geometric length of vector
  inline float length() const    { return sqrtf( dot( *this ) ); }

  // Negates each component of vector in place
  inline void negate()      { vec[0] *= -1.0; vec[1] *= -1.0; }

  // Changes vector to be unit length
  inline float normalize()
    {
        float len = length();
        *this /= len;
        return len;
    }

  // Sets value of vector from array of 2 components
  inline XbVec2f &setValue( const float v[2] )
    {
        vec[0] = v[0];
        vec[1] = v[1];
        return *this;
    }

  // Sets value of vector from 2 individual components
  inline XbVec2f &setValue( float x, float y )
    {
        vec[0] = x;
        vec[1] = y;
        return *this;
    }

  // Accesses indexed component of vector
  inline float &operator [](int i)     { return (vec[i]); }
  inline const float &operator [](int i) const   { return (vec[i]); }

  // Component-wise scalar multiplication and division operators
  inline XbVec2f &operator *=(float d)
    { return setValue( vec[0] * d, vec[1] * d ); }

  inline XbVec2f &operator /=(float d)
    { return setValue( vec[0] / d, vec[1] / d ); }

  // Component-wise vector addition and subtraction operators
  inline XbVec2f &operator +=(const XbVec2f &u)
    { return setValue( vec[0] + u.vec[0], vec[1] + u.vec[1] ); }

  inline XbVec2f &operator -=(const XbVec2f &u)
    { return setValue( vec[0] - u.vec[0], vec[1] - u.vec[1] ); }

  // Nondestructive unary negation - returns a new vector
  inline XbVec2f  operator -() const
    { return XbVec2f( -vec[0], -vec[1] ); }

  // Component-wise binary scalar multiplication and division operators
  inline friend XbVec2f operator *(const XbVec2f &v, float d)
    { return XbVec2f( v.vec[0] * d, v.vec[1] * d ); }
  inline friend XbVec2f operator *(float d, const XbVec2f &v)
    { return v * d; }
  inline friend XbVec2f operator /(const XbVec2f &v, float d)
    { return v * (1.0f / d); }

  // Component-wise binary vector addition and subtraction operators
  inline friend XbVec2f operator +(const XbVec2f &v1, const XbVec2f &v2)
    {
        return XbVec2f( v1.vec[0] + v2.vec[0],
            v1.vec[1] + v2.vec[1] );
    }

  inline friend XbVec2f  operator -(const XbVec2f &v1, const XbVec2f &v2);

  // Equality comparison operator
  inline friend int  operator ==(const XbVec2f &v1, const XbVec2f &v2)
    {
        return( v1.vec[0] == v2.vec[0] &&
              v1.vec[1] == v2.vec[1] );
    }
  inline friend int  operator !=(const XbVec2f &v1, const XbVec2f &v2)
    { return !(v1 == v2); }

  // Equality comparison within given tolerance - the square of the
  // length of the maximum distance between the two vectors
  inline bool equals(const XbVec2f &v, float tolerance) const;

    protected:
      float  vec[2];    // Storage for vector components
};

inline XbVec2f operator -(const XbVec2f &v1, const XbVec2f &v2)
    {
        return XbVec2f( v1.vec[0] - v2.vec[0],
            v1.vec[1] - v2.vec[1] );
    }
inline bool XbVec2f::equals(const XbVec2f &v, float tolerance) const
    {
        XbVec2f diff( *this - v );
        float sqLen = diff.dot( diff );
        return ( sqLen <= tolerance );
    }

#endif      // #ifndef SWIG

class XbRotation {
  public:
    // default constructor does no initialization
    inline XbRotation( void ) {
    }

#ifndef SWIG
    // construct from array of 4 quaternion components
    inline XbRotation( const float v[4] ) {
      setValue( v );
    }
    inline XbRotation( const double v[4] ) {
      setValue( v );
    }

    // construct from 4 individual quaternion components
    inline XbRotation( float q0, float q1, float q2, float q3 ) {
      setValue( q0, q1, q2, q3 );
    }

    // construct from rotation matrix
    inline XbRotation( const XbMatrix& m ) {
      setValue(m);
    }

    // construct from 3D rotation axis vector and angle in radians
    inline XbRotation( const XbVec3f& axis, float radians ) {
      setValue( axis, radians );
    }

    // construct rotation that rotates one direction vector to another
    inline XbRotation( const XbVec3f& from, const XbVec3f& to ) {
      setValue(from, to );
    }

    // construct from Euler angles
    inline XbRotation( const XbEuler& euler ) {
      setValue(euler);
    }
#endif      // #ifndef SWIG

    inline ~XbRotation( void ) {
    }

#ifndef SWIG
    // I/O
#ifdef XB_LINEAR_USE_STREAMS
    inline istream& read( istream& is ) {
      return is >> quat[0] >> quat[1] >> quat[2] >> quat[3];
    }

    inline ostream& write( ostream& os ) const {
      return os << quat[0] << ' ' << quat[1] << ' '
                << quat[2] << ' ' << quat[3];
    }

    friend istream& operator>>( istream &is, XbRotation &q )
    {
        return is >> q.quat[0] >> q.quat[1] >> q.quat[2] >> q.quat[3];
    }

    friend ostream& operator<<( ostream &os, const XbRotation &q )
    {
        return os << q.quat[0] << ' ' << q.quat[1] << ' '
	              << q.quat[2] << ' ' << q.quat[3];
    }
#endif
    // set value from array of 4 quaternion components
    inline XbRotation& setValue( const float q[4] ) {
      memcpy(quat,q,sizeof(quat));
      normalize();
      return *this;
    }
    inline XbRotation& setValue( const double q[4] ) {
      quat[0] = (float)q[0]; quat[1] = (float)q[1]; quat[2] = (float)q[2]; quat[3] = (float)q[3];
      normalize();
      return *this;
    }

    // set value from 4 individual quaternion components
    inline XbRotation& setValue( float q0, float q1, float q2, float q3 ) {
      quat[0] = q0; quat[1] = q1; quat[2] = q2; quat[3] = q3; 
      normalize();
      return *this;
    }

    // set value from a rotation matrix - behavior is undefined when
    // matrix is not a rotation matrix
    inline XbRotation& setValue( const XbMatrix& M );

    // set value from 3D rotation axis vector and angle in radians
    inline XbRotation& setValue( const XbVec3f& axis, float angle ) {
      float s = (float)sin( angle / 2 ) / axis.length();
      return setValue( s * axis[0], s * axis[1], s * axis[2], (float)cos( angle / 2 ) );
    }
      

    // set to rotation that rotates one direction vector to another
    inline XbRotation& setValue( const XbVec3f& from, const XbVec3f& to ) {
      XbVec3f f=from;
      f.normalize();
      XbVec3f t=to;
      t.normalize();
      float d = f.dot(t);
      if (d > 0.99999f) {
        setValue(0.0f,0.0f,0.0f,1.0f);
        return(*this);
      } 
      if (d < -0.99999f) {
        setValue(f.perpendicular(),(float)M_PI);
        return(*this);
      }
      return setValue( f.cross( t ), f.angleTo( t ) );
    }
#endif      // #ifndef SWIG

    // set via euler angles
    inline XbRotation& setValue( const XbEuler& euler);

#ifndef SWIG
    // return array of 4 quaternion components
    inline const float* getValue( void ) const {
      return quat;
    }
#endif

#ifndef SWIG
    // get 4 individual quaternion components
    inline void getValue( float& q0, float& q1, float& q2, float& q3 ) const {
      q0 = quat[0]; q1 = quat[1]; q2 = quat[2]; q3 = quat[3];
    }

    // get corresponding 3D rotation axis vector and angle in radians
    inline void getValue( XbVec3f& axis, float& a ) const {
      double cos_aover2 = quat[3];
      a = (float)acos( cos_aover2 ) * 2;
      double sin_aover2 = sqrt( 1.0 - cos_aover2 * cos_aover2 );
      axis.setValue( quat );
      if ( sin_aover2 != 0 ) axis /= (float)sin_aover2;
    }

    // get corresponding rotation matrix
    inline void getValue( XbMatrix& M ) const;
#endif

    // Changes a rotation to be its inverse
    inline XbRotation& invert( void ) {
      float n = dot4( quat, quat);
      quat[0] /= -n;
      quat[1] /= -n;
      quat[2] /= -n;
      quat[3] /= n;
      return *this;
    }

    // Returns the inverse of a rotation
    inline XbRotation inverse( void ) const {
      float n = dot4( quat, quat);
      return XbRotation( -quat[0] / n, -quat[1] / n, -quat[2] / n, quat[3] / n );
    }

#ifndef SWIG
    // compose another rotation with this one
    inline XbRotation& operator*=( const XbRotation& q ) {
        return setValue(
          quat[0]*q.quat[3]-quat[1]*q.quat[2]+quat[2]*q.quat[1]+quat[3]*q.quat[0],
          quat[0]*q.quat[2]+quat[1]*q.quat[3]-quat[2]*q.quat[0]+quat[3]*q.quat[1],
         -quat[0]*q.quat[1]+quat[1]*q.quat[0]+quat[2]*q.quat[3]+quat[3]*q.quat[2],
         -quat[0]*q.quat[0]-quat[1]*q.quat[1]-quat[2]*q.quat[2]+quat[3]*q.quat[3]);
    }

    inline bool operator==( const XbRotation& q) const {
      return quat[0] == q.quat[0] && quat[1] == q.quat[1] && quat[2] == q.quat[2] && quat[3] == q.quat[3];
    }
#endif

    // Puts the given vector through this rotation
    inline void multVec( const XbVec3f& src, XbVec3f& dst ) const {
      XbRotation r = inverse() * XbRotation( src[0], src[1], src[2], 0 ) * *this;
      dst.setValue( r.quat );
      dst *= src.length();
    }

    // Keep the axis the same. Multiply the angle of rotation by
    // the amount 'scaleFactor'
    // void scaleAngle( float scaleFactor );

    // Spherical linear interpolation: as t goes from 0 to 1, returned
    // value goes from "from" to "to"
    inline static XbRotation slerp( const XbRotation& q1, const XbRotation& q2, float u ) {
      double cs = dot4( q1.quat, q2.quat);
      if ( cs > 1.0 ) cs = 1.0f;
      if ( cs < -1.0 ) cs = -1.0f;
      double theta = acos( cs );
      double sx = sin( theta );
      if (sx < 1e-03) {
        return q1;
      }
      double s1 = sin( (1-u)*theta ) / sx;
      double s2 = sin( u*theta ) / sx;
      //printf("theta %f sx %f s1 %f s2 %f\n",theta,sx,s1,s2);
      return XbRotation(
        (float)(s1*q1.quat[0] + s2*q2.quat[0]),
        (float)(s1*q1.quat[1] + s2*q2.quat[1]),
        (float)(s1*q1.quat[2] + s2*q2.quat[2]),
        (float)(s1*q1.quat[3] + s2*q2.quat[3]) );
    }

    inline static float distance( const XbRotation& q1, const XbRotation& q2) {
      float q[4];
      q[0] = q1.quat[0]-q2.quat[0];
      q[1] = q1.quat[1]-q2.quat[1];
      q[2] = q1.quat[2]-q2.quat[2];
      q[3] = q1.quat[3]-q2.quat[3];
      return((float)sqrt(dot4( q, q)));
    }

    // Null rotation
    inline static XbRotation identity( void ) {
      return XbRotation( 0, 0, 0, 1 );
    }

#ifndef SWIG
    // compose rotations
    inline friend XbRotation operator*( const XbRotation& a, const XbRotation& b ) {
        return XbRotation(
            a.quat[0]*b.quat[3]-a.quat[1]*b.quat[2]+a.quat[2]*b.quat[1]+a.quat[3]*b.quat[0],
            a.quat[0]*b.quat[2]+a.quat[1]*b.quat[3]-a.quat[2]*b.quat[0]+a.quat[3]*b.quat[1],
            -a.quat[0]*b.quat[1]+a.quat[1]*b.quat[0]+a.quat[2]*b.quat[3]+a.quat[3]*b.quat[2],
            -a.quat[0]*b.quat[0]-a.quat[1]*b.quat[1]-a.quat[2]*b.quat[2]+a.quat[3]*b.quat[3]
            );
    }
#endif

    void normalize( void ) {
        float s = sqrtf( dot4( quat, quat) );
        quat[0] /= s; quat[1] /= s; quat[2] /= s; quat[3] /= s;
    }

#ifndef SWIG
    friend float dotProduct( const XbRotation &q1, const XbRotation &q2 )
    {
        return dot4( q1.quat, q2.quat );
    }
#endif

private:
    float quat[4];
};

class XbEuler {
  public:
    enum Order {
      XYZ = 0xa0,
      XYX = 0x11,
      XZY = 0x42,
      XZX = 0x33,
      YZX = 0x24,
      YZY = 0x55,
      YXZ = 0x86,
      YXY = 0x77,
      ZXY = 0x68,
      ZXZ = 0x99,
      ZYX = 0x0a,
      ZYZ = 0xbb,
    };

    enum Axes { STATIC, ROTATING };

    // default constructor does no initialization
    inline XbEuler( void );

#ifndef SWIG
    // these do initialization
    inline XbEuler( const XbVec3f& angles, Order, Axes );
    inline XbEuler( const XbRotation& rot, Order, Axes );
    inline XbEuler( const XbMatrix& mtx, Order, Axes );
#endif

    inline ~XbEuler( void );

#ifndef SWIG
    // I/O
#ifdef XB_LINEAR_USE_STREAMS
    inline istream& read( istream& is );
    inline ostream& write( ostream& os ) const;

    friend istream& operator>>( istream &is, XbEuler &e )
    {
        return is >> e.angles >> e.type;
    }

    friend ostream& operator<<( ostream &os, const XbEuler &e )
    {
        return os << e.angles << ' ' << e.type;
    }
#endif
#endif

    // get all parts of object
    inline void getValue( XbVec3f& angles, Order&, Axes& ) const;

    // get individual parts of object
    inline XbVec3f getAngles( void ) const;
    inline Order getOrder( void ) const;
    inline Axes getAxes( void ) const;

    // set all parts of object
    inline void setValue( const XbVec3f& angles, Order, Axes );
#ifndef SWIG
    inline void setValue( const XbRotation& rot, Order order, Axes axes );
#endif

    // set angles
    inline void setAngles( const XbVec3f& );

    // set order and/or axes - may cause the angles to be recomputed
    inline void setOrder( Order );
    inline void setAxes( Axes );
    inline void setOrderAndAxes( Order, Axes );

    // return alternate angles which achieve the same net rotation
    // - for non-repeated axes (e.g. XYZ, ZYX) the alternate form is
    // ( pi+angle[0], pi-angle[1], pi+angle[2] )
    // - for repeated axes (e.g. XYX, ZYZ) the alternate form is
    // ( pi+angle[0], -angle[1], pi+angle[2] )
    inline XbEuler alternate( void ) const;

    // using the alternate form and multiples of 2*pi return the
    // equivalent rotation in which the maximum distance to any
    // corresponding goal angle is smallest
    inline XbEuler closest( const XbVec3f& goal ) const;

#ifndef SWIG
    // comparison
    inline bool operator==( const XbEuler& x ) const;
#endif

    // get Euler type information
    inline void getInfo( int& i, int& j, int& k, bool& n, bool& s, bool& f ) const;

  protected:
    inline void setType( Order, Axes );
    inline void setValue( const XbMatrix& );

    XbVec3f angles;
    int type;
};

#ifndef SWIG
typedef float XbMat[4][4];
#endif

class XbMatrix {
  public:
    // default constructor does no initialization
    inline XbMatrix( void ) {
    }

#ifndef SWIG
    // construct from a 4x4 array of elements
    inline XbMatrix( const XbMat m) {
      setValue( m );
    }

    // construct from 16 individual elements
    inline XbMatrix(
      float m00, float m01, float m02, float m03,
      float m10, float m11, float m12, float m13,
      float m20, float m21, float m22, float m23,
      float m30, float m31, float m32, float m33
      )
    {
        mtx[0][0] = m00; mtx[0][1] = m01; mtx[0][2] = m02; mtx[0][3] = m03;
        mtx[1][0] = m10; mtx[1][1] = m11; mtx[1][2] = m12; mtx[1][3] = m13;
        mtx[2][0] = m20; mtx[2][1] = m21; mtx[2][2] = m22; mtx[2][3] = m23;
        mtx[3][0] = m30; mtx[3][1] = m31; mtx[3][2] = m32; mtx[3][3] = m33;
    }


    // from Euler angles
    inline XbMatrix( const XbEuler& euler) {
      setValue(euler);
    }
#endif      // #ifndef SWIG

    inline void setValue( const XbEuler& euler) {
       double ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
       int i,j,k; 
       bool n,s,f;
       euler.getInfo( i, j, k, n, s, f );
       float ex, ey, ez; 
       euler.getAngles().getValue( ex, ey, ez );
       if (f) {float t = ex; ex = ez; ez = t;}
       if (n) {ex = -ex; ey = -ey; ez = -ez;}
       ci = cos(ex); cj = cos(ey); ch = cos(ez);
       si = sin(ex); sj = sin(ey); sh = sin(ez);
       cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
       if (s) {
         mtx[i][i] = (float)cj;     mtx[j][i] =  (float)(sj*si);    mtx[k][i] =  (float)(sj*ci);
         mtx[i][j] = (float)(sj*sh);  mtx[j][j] = (float)(-cj*ss+cc); mtx[k][j] = (float)(-cj*cs-sc);
         mtx[i][k] = (float)(-sj*ch); mtx[j][k] =  (float)(cj*sc+cs); mtx[k][k] =  (float)(cj*cc-ss);
       } else {
         mtx[i][i] = (float)(cj*ch);  mtx[j][i] = (float)(sj*sc-cs);  mtx[k][i] = (float)(sj*cc+ss);
         mtx[i][j] = (float)(cj*sh);  mtx[j][j] = (float)(sj*ss+cc);  mtx[k][j] = (float)(sj*cs-sc);
         mtx[i][k] = (float)(-sj);    mtx[j][k] = (float)(cj*si);     mtx[k][k] = (float)(cj*ci);
       }
       mtx[3][0]=mtx[3][1]=mtx[3][2]=mtx[0][3]=mtx[1][3]=mtx[2][3]=0.0f;
       mtx[3][3]=1.0f;
    }

    inline ~XbMatrix( void ) {
    }

#ifndef SWIG
    // I/O
#ifdef XB_LINEAR_USE_STREAMS
    inline istream& read( istream& is );
    inline ostream& write( ostream& os ) const;

    friend istream& operator>>( istream &is, XbMatrix m )
    {
        return is
	        >> m.mtx[0][0] >> m.mtx[0][1] >> m.mtx[0][2] >> m.mtx[0][3]
	        >> m.mtx[1][0] >> m.mtx[1][1] >> m.mtx[1][2] >> m.mtx[1][3]
	        >> m.mtx[2][0] >> m.mtx[2][1] >> m.mtx[2][2] >> m.mtx[2][3]
	        >> m.mtx[3][0] >> m.mtx[3][1] >> m.mtx[3][2] >> m.mtx[3][3];
    }

    friend ostream& operator<<( ostream &os, const XbMatrix m )
    {
        return os
	        << m.mtx[0][0] << ' ' << m.mtx[0][1] << ' '
	        << m.mtx[0][2] << ' ' << m.mtx[0][3] << ' '
	        << m.mtx[1][0] << ' ' << m.mtx[1][1] << ' '
	        << m.mtx[1][2] << ' ' << m.mtx[1][3] << ' '
	        << m.mtx[2][0] << ' ' << m.mtx[2][1] << ' '
	        << m.mtx[2][2] << ' ' << m.mtx[2][3] << ' '
	        << m.mtx[3][0] << ' ' << m.mtx[3][1] << ' '
	        << m.mtx[3][2] << ' ' << m.mtx[3][3];
    }
#endif
    // set value from 4x4 array of elements
    inline void setValue( const XbMat m) {
      memcpy(mtx,m,sizeof(float)*16);
    }

    // set value from 16 individual elements

    inline void setValue(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
      )
    {
      mtx[0][0] = m00; mtx[0][1] = m01; mtx[0][2] = m02; mtx[0][3] = m03;
      mtx[1][0] = m10; mtx[1][1] = m11; mtx[1][2] = m12; mtx[1][3] = m13;
      mtx[2][0] = m20; mtx[2][1] = m21; mtx[2][2] = m22; mtx[2][3] = m23;
      mtx[3][0] = m30; mtx[3][1] = m31; mtx[3][2] = m32; mtx[3][3] = m33;
    }
#endif      // #ifndef SWIG

    // set to rotation matrix equivalent to given rotation
    inline void setRotate( const XbRotation& q );

    void setRotateX( float radian )
    {
        float sTh = sinf( radian );
        float cTh = cosf( radian );
        mtx[0][0] = 1.0; mtx[0][1] =  0.0; mtx[0][2] = 0.0; mtx[0][3] = 0.0;
        mtx[1][0] = 0.0; mtx[1][1] =  cTh; mtx[1][2] = sTh; mtx[1][3] = 0.0;
        mtx[2][0] = 0.0; mtx[2][1] = -sTh; mtx[2][2] = cTh; mtx[2][3] = 0.0;
        mtx[3][0] = 0.0; mtx[3][1] =  0.0; mtx[3][2] = 0.0; mtx[3][3] = 1.0;
    }

    void setRotateY( float radian )
    {
        float sTh = sinf( radian );
        float cTh = cosf( radian );
        mtx[0][0] = cTh; mtx[0][1] = 0.0; mtx[0][2] = -sTh; mtx[0][3] = 0.0;
        mtx[1][0] = 0.0; mtx[1][1] = 1.0; mtx[1][2] =  0.0; mtx[1][3] = 0.0;
        mtx[2][0] = sTh; mtx[2][1] = 0.0; mtx[2][2] =  cTh; mtx[2][3] = 0.0;
        mtx[3][0] = 0.0; mtx[3][1] = 0.0; mtx[3][2] =  0.0; mtx[3][3] = 1.0;
    }

    void setRotateZ( float radian )
    {
        float sTh = sinf( radian );
        float cTh = cosf( radian );
        mtx[0][0] =  cTh; mtx[0][1] = sTh; mtx[0][2] = 0.0; mtx[0][3] = 0.0;
        mtx[1][0] = -sTh; mtx[1][1] = cTh; mtx[1][2] = 0.0; mtx[1][3] = 0.0;
        mtx[2][0] =  0.0; mtx[2][1] = 0.0; mtx[2][2] = 1.0; mtx[2][3] = 0.0;
        mtx[3][0] =  0.0; mtx[3][1] = 0.0; mtx[3][2] = 0.0; mtx[3][3] = 1.0;
    }

    // set to uniform scale matrix
    inline void setScale( float s ) {
      memset(mtx,0,sizeof(mtx));
      mtx[0][0] = s;
      mtx[1][1] = s;
      mtx[2][2] = s;
      mtx[3][3] = 1.0f;
    }

#ifndef SWIG
    // set to non-uniform scale matrix
    inline void setScale( const XbVec3f& s ) {
      memset(mtx,0,sizeof(mtx));
      mtx[0][0] = s[0];
      mtx[1][1] = s[1];
      mtx[2][2] = s[2];
      mtx[3][3] = 1.0f;
    }
#endif

    // set to translation matrix
    inline void setTranslate( const XbVec3f& t ) {
      setValue(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        t[0], t[1], t[2], 1 );
    }

#ifndef SWIG
    // Composes the matrix based on a translation, rotation, scale,
    // orientation for scale, and center. The "center" is the
    // center point for scaling and rotation. The "scaleOrientation"
    // chooses the primary axes for the scale.
    inline void setTransform(
        const XbVec3f& translation,
        const XbRotation& rotation,
        const XbVec3f& scaleFactor,
        const XbRotation& scaleOrientation = XbRotation::identity(),
        const XbVec3f& center = XbVec3f( 0, 0, 0 ) );
#endif

    // set to identity matrix
    inline void makeIdentity( void );

#ifndef SWIG
    // get 4x4 array of elements
    inline void getValue( XbMat ) const;
    inline const XbMat& getValue( void ) const;

    // Cast: returns pointer to storage of first element
    inline operator float *( void ) {
      return (float *) mtx;
    }
    inline operator const float *( void ) const {
      return (const float *) mtx;
    }

    // Cast: returns reference to 4x4 array
    inline operator XbMat&( void ) {
      return mtx;
    }
    inline operator const XbMat&( void ) const {
      return mtx;
    }

    // Make it look like a usual matrix (so you can do m[3][2])
    inline float *operator[]( int i ) {
      return mtx[i];
    }
    inline const float *operator[]( int i ) const {
      return mtx[i];
    }
#endif      // #ifndef SWIG

#ifdef THIS_DOES_NOT_WORK
    // Decomposes the matrix into a translation, rotation, scale,
    // and scale orientation. Any projection information is discarded.
    // The decomposition depends upon choice of center point for
    // rotation and scaling, which is optional as the last parameter.
    // Note that if the center is 0, getTransform() is the same as
    // factor() where "t" is translation, "u" is rotation,
    // "s" is scaleFactor, and "r" is ScaleOrientation.
    inline void getTransform( XbVec3f& translation,
        XbRotation& rotation,
        XbVec3f& scaleFactor,
        XbRotation& scaleOrientation,
        const XbVec3f& center = XbVec3f( 0, 0, 0 ) ) const;
#else
    //  Decompose the matrix into translation, rotational angle, and scale factors.
    //  The rotational angle is in radians.
    inline void getTransform(
        XbVec3f& translation, XbVec3f& angles, XbVec3f& scaleFactor ) const;
#endif

    // get translation part of matrix
    inline XbVec3f getTranslation( void ) const;

    // determinant of upper-left 3x3 submatrix
    inline float det3( void ) const;

    // determinant of entire matrix
    inline float det4( void ) const;

    // Returns inverse of matrix.  Results are undefined for
    // singular matrices.
    inline XbMatrix inverse( void ) const;

    // Returns transpose of matrix
    inline XbMatrix transpose( void ) const;

    // Multiplies matrix by given matrix on right or left
    inline XbMatrix& multRight( const XbMatrix& m );
    inline XbMatrix& multLeft( const XbMatrix& m );

    // Multiplies matrix by given column vector, giving vector result
    inline void multMatrixVec( const XbVec3f& src, XbVec3f& dst ) const;
#ifndef SWIG
    inline void multMatrixVec( const float *src, float *dst ) const;
#endif

    // Multiplies given row vector by matrix, giving vector result
    inline void multVecMatrix( const XbVec3f& src, XbVec3f& dst ) const;
#ifndef SWIG
    inline void multVecMatrix( const float *src, float *dst ) const;
#endif


    // Multiplies given row vector by matrix, giving vector result
    // src is assumed to be a direction vector, so translation part of
    // matrix is ignored.
    inline void multDirMatrix( const XbVec3f& src, XbVec3f& dst ) const;
#ifndef SWIG
    inline void multDirMatrix( const float *src, float *dst ) const;
#endif

#ifndef SWIG
    inline XbMatrix& operator=( const XbMatrix &m ) {
      setValue(m);
      return *this;
    }
    inline XbMatrix& operator=( const XbMat m ) {
      setValue( m );
      return *this;
    }
    inline XbMatrix& operator=( const XbEuler& e ) {
      setValue(e);
      return(*this);
    }
    inline XbMatrix& operator=( const XbRotation& q );
    inline XbMatrix& operator*=( const XbMatrix& m ) {
      return *this = *this * m;
    }

    inline bool operator==( const XbMatrix& m) const {
      const float *a = *this;
      const float *b = m;
      return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3] &&
        a[4]==b[4] && a[5]==b[5] && a[6]==b[6] && a[7]==b[7] &&
        a[8]==b[8] && a[9]==b[9] && a[10]==b[10] && a[11]==b[11] &&
        a[12]==b[12] && a[13]==b[13] && a[14]==b[14] && a[15]==b[15];
    }

    // return identity matrix
    inline static XbMatrix identity( void );
#endif      // #ifndef SWIG

#define FEQUAL(x,y)     (fabs((x)-(y)) < EPSILON)

    inline bool isIdentity( float epsilon=0.0001 )
    {
      if( fabs( mtx[0][0] - 1.0 ) < epsilon &&
          fabs( mtx[1][1] - 1.0 ) < epsilon &&
          fabs( mtx[2][2] - 1.0 ) < epsilon &&
          fabs( mtx[3][0] ) < epsilon &&
          fabs( mtx[3][1] ) < epsilon &&
          fabs( mtx[3][2] ) < epsilon &&
          fabs( mtx[0][1] ) < epsilon &&
          fabs( mtx[0][2] ) < epsilon &&
          fabs( mtx[1][0] ) < epsilon &&
          fabs( mtx[1][2] ) < epsilon &&
          fabs( mtx[2][0] ) < epsilon &&
          fabs( mtx[2][1] ) < epsilon &&
          fabs( mtx[0][3] ) < epsilon &&
          fabs( mtx[1][3] ) < epsilon &&
          fabs( mtx[2][3] ) < epsilon &&
          fabs( mtx[3][3] - 1.0 ) < epsilon )
      {
        return true;
      }
      return false;
    }

#ifndef SWIG
    inline friend XbMatrix operator*( const XbMatrix& M1, const XbMatrix& M2 ) {
         return XbMatrix(
M1[0][0]*M2[0][0] + M1[0][1]*M2[1][0] + M1[0][2]*M2[2][0] + M1[0][3]*M2[3][0],
M1[0][0]*M2[0][1] + M1[0][1]*M2[1][1] + M1[0][2]*M2[2][1] + M1[0][3]*M2[3][1],
M1[0][0]*M2[0][2] + M1[0][1]*M2[1][2] + M1[0][2]*M2[2][2] + M1[0][3]*M2[3][2],
M1[0][0]*M2[0][3] + M1[0][1]*M2[1][3] + M1[0][2]*M2[2][3] + M1[0][3]*M2[3][3],
M1[1][0]*M2[0][0] + M1[1][1]*M2[1][0] + M1[1][2]*M2[2][0] + M1[1][3]*M2[3][0],
M1[1][0]*M2[0][1] + M1[1][1]*M2[1][1] + M1[1][2]*M2[2][1] + M1[1][3]*M2[3][1],
M1[1][0]*M2[0][2] + M1[1][1]*M2[1][2] + M1[1][2]*M2[2][2] + M1[1][3]*M2[3][2],
M1[1][0]*M2[0][3] + M1[1][1]*M2[1][3] + M1[1][2]*M2[2][3] + M1[1][3]*M2[3][3],
M1[2][0]*M2[0][0] + M1[2][1]*M2[1][0] + M1[2][2]*M2[2][0] + M1[2][3]*M2[3][0],
M1[2][0]*M2[0][1] + M1[2][1]*M2[1][1] + M1[2][2]*M2[2][1] + M1[2][3]*M2[3][1],
M1[2][0]*M2[0][2] + M1[2][1]*M2[1][2] + M1[2][2]*M2[2][2] + M1[2][3]*M2[3][2],
M1[2][0]*M2[0][3] + M1[2][1]*M2[1][3] + M1[2][2]*M2[2][3] + M1[2][3]*M2[3][3],
M1[3][0]*M2[0][0] + M1[3][1]*M2[1][0] + M1[3][2]*M2[2][0] + M1[3][3]*M2[3][0],
M1[3][0]*M2[0][1] + M1[3][1]*M2[1][1] + M1[3][2]*M2[2][1] + M1[3][3]*M2[3][1],
M1[3][0]*M2[0][2] + M1[3][1]*M2[1][2] + M1[3][2]*M2[2][2] + M1[3][3]*M2[3][2],
M1[3][0]*M2[0][3] + M1[3][1]*M2[1][3] + M1[3][2]*M2[2][3] + M1[3][3]*M2[3][3]);
    }
#endif  // #ifndef SWIG

    // factor matrix into u * s * transpose(u) * r * t where u and r
    // are rotations, s is scale and t is translation
    inline void factor( XbMatrix& u, XbMatrix& s, XbMatrix& r, XbMatrix& t ) const;

    // factor 3x3 submatrix into u * k * vt where u and vt are orthogonal
    // and k is diagonal
    inline void svd3( XbMatrix &u, XbMatrix &k, XbMatrix &vt ) const;

  protected:
    XbMat mtx;
};

#ifndef SWIG

XbRotation& XbRotation::setValue(const XbMatrix& M) {
  double tr = M[0][0]+M[1][1]+M[2][2];
  if ( tr > 0 ) { 
    float s = (float)sqrt( tr+1.0 );
    quat[3] = s*0.5f;
    s = 0.5f/s; 
    quat[0] = (M[1][2]-M[2][1])*s;
    quat[1] = (M[2][0]-M[0][2])*s;
    quat[2] = (M[0][1]-M[1][0])*s;
  } else {
    int nxt[3] = { 1, 2, 0 };
    int i = 0;  
    if (M[1][1] > M[0][0]) i = 1;
    if (M[2][2] > M[i][i]) i = 2;
    int j = nxt[i];
    int k = nxt[j];
    float s = (float)sqrt( M[i][i]-M[j][j]-M[k][k]+1.0 );
    quat[i] = s*0.5f;
    s = 0.5f/s;
    quat[3] = (M[j][k]-M[k][j])*s;
    quat[j] = (M[i][j]+M[j][i])*s;
    quat[k] = (M[i][k]+M[k][i])*s;
  }
  normalize();
  return *this;
}
XbRotation& XbRotation::setValue( const XbEuler& euler) {
  int i,j,k;
  bool n,s,f;
  euler.getInfo( i,j,k,n,s,f );
  float ex, ey, ez;
  euler.getAngles().getValue( ex, ey, ez );
  if (f) {float t = ex; ex = ez; ez = t;}
  if (n) ey = -ey;
  double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
  ti = ex*0.5; tj = ey*0.5; th = ez*0.5;
  ci = cos(ti);  cj = cos(tj);  ch = cos(th);
  si = sin(ti);  sj = sin(tj);  sh = sin(th);
  cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
  if (s) {
    quat[i] = (float)(cj*(cs + sc));
    quat[j] = (float)(sj*(cc + ss));
    quat[k] = (float)(sj*(cs - sc));
    quat[3] = (float)(cj*(cc - ss));
  } else {
    quat[i] = (float)(cj*sc - sj*cs);
    quat[j] = (float)(cj*ss + sj*cc);
    quat[k] = (float)(cj*cs - sj*sc);
    quat[3] = (float)(cj*cc + sj*ss);
  }
  if (n) quat[j] = -quat[j];
  return(*this);
}
void XbRotation::getValue( XbMatrix& M ) const {
  float n = dot4( quat, quat);
  float s = (n > 0.0f) ? (2.0f / n) : 0.0f;
  float xs = quat[0]*s,  ys = quat[1]*s,  zs = quat[2]*s;
  float wx = quat[3]*xs,  wy = quat[3]*ys, wz = quat[3]*zs;
  float xx = quat[0]*xs,  xy = quat[0]*ys, xz = quat[0]*zs;
  float yy = quat[1]*ys,  yz = quat[1]*zs, zz = quat[2]*zs;
  M[0][0] = 1.0f - (yy + zz); M[1][0] = xy - wz; M[2][0] = xz + wy;
  M[0][1] = xy + wz; M[1][1] = 1.0f - (xx + zz); M[2][1] = yz - wx;
  M[0][2] = xz - wy; M[1][2] = yz + wx; M[2][2] = 1.0f - (xx + yy);
  M[3][0]=M[3][1]=M[3][2]=M[0][3]=M[1][3]=M[2][3]=0.0f; M[3][3]=1.0f;
}

#ifdef XB_LINEAR_USE_STREAMS
istream& XbMatrix::read( istream& is ) {
	return is
	>> mtx[0][0] >> mtx[0][1] >> mtx[0][2] >> mtx[0][3]
	>> mtx[1][0] >> mtx[1][1] >> mtx[1][2] >> mtx[1][3]
	>> mtx[2][0] >> mtx[2][1] >> mtx[2][2] >> mtx[2][3]
	>> mtx[3][0] >> mtx[3][1] >> mtx[3][2] >> mtx[3][3];
}

ostream& XbMatrix::write( ostream& os ) const {
	return os
<< mtx[0][0] << ' ' << mtx[0][1] << ' ' << mtx[0][2] << ' ' << mtx[0][3] << ' '
<< mtx[1][0] << ' ' << mtx[1][1] << ' ' << mtx[1][2] << ' ' << mtx[1][3] << ' '
<< mtx[2][0] << ' ' << mtx[2][1] << ' ' << mtx[2][2] << ' ' << mtx[2][3] << ' '
<< mtx[3][0] << ' ' << mtx[3][1] << ' ' << mtx[3][2] << ' ' << mtx[3][3];
}
#endif
void XbMatrix::setRotate( const XbRotation& q ) {
	q.getValue( *this );
}

void XbMatrix::setTransform( const XbVec3f& translation,
		const XbRotation& rotation, const XbVec3f& scaleFactor,
		const XbRotation& scaleOrientation, const XbVec3f& center ) {
	XbMatrix t, r, s, o, c, cinv;
	t.setTranslate( translation );
	r.setRotate( rotation );
	s.setScale( scaleFactor );
	o.setRotate( scaleOrientation );
	c.setTranslate( center );
	cinv.setTranslate( -center );
	*this = cinv * o.transpose() * s * o * r * c * t;
}

void XbMatrix::makeIdentity( void ) {
	setValue(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 );
}

void XbMatrix::getValue( XbMat m ) const {
        memcpy(m,mtx,sizeof(float)*16);
	//m[0][0]=mtx[0][0]; m[0][1]=mtx[0][1]; m[0][2]=mtx[0][2]; m[0][3]=mtx[0][3];
	//m[1][0]=mtx[1][0]; m[1][1]=mtx[1][1]; m[1][2]=mtx[1][2]; m[1][3]=mtx[1][3];
	//m[2][0]=mtx[2][0]; m[2][1]=mtx[2][1]; m[2][2]=mtx[2][2]; m[2][3]=mtx[2][3];
	//m[3][0]=mtx[3][0]; m[3][1]=mtx[3][1]; m[3][2]=mtx[3][2]; m[3][3]=mtx[3][3];
}

const XbMat& XbMatrix::getValue( void ) const {
	return mtx;
}

typedef double Mat3[3][3];
typedef double Vec3[3];

// roots of x^3 + a[2] * x^2 + a[1] * cx + a[0] (assumes all are real)
inline void root3( const Vec3 a, Vec3 root ) {
	double q = ( 3*a[1] - a[2]*a[2] ) / 9.0;
	double r = ( 9*a[2]*a[1] - 27*a[0] - 2*a[2]*a[2]*a[2] ) / 54.0;
	double d = q*q*q + r*r;
	double m = -a[2] / 3.0;
	if ( d >= 0 ) {
		double s = pow( r,1.0/3.0 );
		root[0] = m + s * 2.0;
		root[1] = root[2] = m - s;
	} else {
		double rad = sqrt( -q );
		double theta = atan2( sqrt( -d ), r ) / 3.0;
		double bar = rad * cos( theta );
		double foo = sqrt(3.0) * rad * sin( theta );
		root[0] = m + 2.0 * bar;
		root[1] = m - bar - foo;
		root[2] = m - bar + foo;
	}
}

// find coefficients of characteristic polynomial - assumes s is symmetric
inline void charpoly( const Mat3 s, Vec3 a ) {
	double s01 = s[0][1] * s[0][1];
	double s12 = s[1][2] * s[1][2];
	double s20 = s[2][0] * s[2][0];
	a[2] = -(s[0][0]+s[1][1]+s[2][2]);
	a[1] = s[0][0]*s[1][1]+s[1][1]*s[2][2]+s[2][2]*s[0][0]-s01-s12-s20;
	a[0] = s01*s[2][2]+s12*s[0][0]+s20*s[1][1]-s[0][0]*s[1][1]*s[2][2]-2.0*s[0][1]*s[1][2]*s[2][0];
}

// find eigenvalues of symmetric matrix s
inline void eigenvals( const Mat3 s, Vec3 e ) {
	Vec3 a;
	charpoly( s, a );
	root3( a, e );
}

inline double dot( const Vec3 a, const Vec3 b ) {
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
	}

inline void cross( const Vec3 a, const Vec3 b, Vec3 c ) {
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

// find a vector perpendicular to "a"
inline void perpendicular( const Vec3 a, Vec3 b ) {
	double x = fabs( a[0] );
	double y = fabs( a[1] );
	double z = fabs( a[2] );
	if ( x > y && x > z ) { b[0]=a[1]+a[2]; b[1]=b[2]=-a[0]; } else
	if ( y > x && y > z ) { b[1]=a[2]+a[0]; b[2]=b[0]=-a[1]; } else
	{ b[2]=a[0]+a[1]; b[0]=b[1]=-a[2]; }
}

inline void scale( Vec3 v, double s ) {
	v[0] *= s; v[1] *= s; v[2] *= s;
}

inline void normalize( Vec3 v ) {
	scale( v, 1.0 / sqrt( dot( v, v ) ) );
}

inline void copy( const Vec3 a, Vec3 b ) {
	b[0]=a[0]; b[1]=a[1]; b[2]=a[2];
}

inline void mult( const Vec3 a, const XbMat m, Vec3 b ) {
	b[0] = a[0]*m[0][0] + a[1]*m[1][0] + a[2]*m[2][0];
	b[1] = a[0]*m[0][1] + a[1]*m[1][1] + a[2]*m[2][1];
	b[2] = a[0]*m[0][2] + a[1]*m[1][2] + a[2]*m[2][2];
}

//static void negate( Vec3 a ) {
	//a[0] = -a[0]; a[1] = -a[1]; a[2] = -a[2];
//}

// find eigenvector corresponding to eigenvalue e
// - assumes all eigenvalues are distinct
inline void eigenvec( const Mat3 m, double e, Vec3 v2 ) {
	Vec3 r0 = { m[0][0] - e, m[0][1], m[0][2] };
	Vec3 r1 = { m[1][0], m[1][1] - e, m[1][2] };
	Vec3 r2 = { m[2][0], m[2][1], m[2][2] - e };
	Vec3 v0, v1;
	cross( r0, r1, v0 );
	cross( r1, r2, v1 );
	cross( r2, r0, v2 );
	double d0 = dot( v0, v0 );
	double d1 = dot( v1, v1 );
	double d2 = dot( v2, v2 );
	if ( d0 > d1 && d0 > d2 ) copy( v0, v2 ); else
	if ( d1 > d0 && d1 > d2 ) copy( v1, v2 );
}

// find all eigenvectors of m given eigenvalues e[]
inline void eigenvecs( const Mat3 m, const Vec3 e, Vec3 v[3] ) {
	const int cmp[2][2][2][3] = {
		{ {{-1,-1,-1}, {2,1,0}}, {{1,0,2}, {1,2,0}} },
		{ {{0,2,1}, {2,0,1}}, {{0,1,2}, {-1,-1,-1}} },
		};
	const int *c = cmp[ e[0]<e[1] ][ e[1]<e[2] ][ e[2]<e[0] ];
	int lo = c[0]; int mid = c[1]; int hi = c[2];
	if ( e[hi]-e[lo] < 1e-5 ) {
		v[0][0] = v[1][1] = v[2][2] = 1;
		v[0][1] = v[0][2] = v[1][0] = v[1][2] = v[2][0] = v[2][1] = 0;
	} else if ( e[mid]-e[lo] < 1e-5 ) {
		eigenvec( m, e[hi], v[hi] );
		perpendicular( v[hi], v[mid] );
		cross( v[hi], v[mid], v[lo] );
	} else if ( e[hi]-e[mid] < 1e-5 ) {
		eigenvec( m, e[lo], v[lo] );
		perpendicular( v[lo], v[mid] );
		cross( v[lo], v[mid], v[hi] );
	} else {
		eigenvec( m, e[lo], v[lo] );
		eigenvec( m, e[mid], v[mid] );
		eigenvec( m, e[hi], v[hi] );
	}
}

void XbMatrix::svd3( XbMatrix& U, XbMatrix& K, XbMatrix& Vt ) const {
	Mat3 u;
	u[0][0]=mtx[0][0]*mtx[0][0]+mtx[0][1]*mtx[0][1]+mtx[0][2]*mtx[0][2];
	u[1][1]=mtx[1][0]*mtx[1][0]+mtx[1][1]*mtx[1][1]+mtx[1][2]*mtx[1][2];
	u[2][2]=mtx[2][0]*mtx[2][0]+mtx[2][1]*mtx[2][1]+mtx[2][2]*mtx[2][2];
	u[0][1]=u[1][0]=mtx[0][0]*mtx[1][0]+mtx[0][1]*mtx[1][1]+mtx[0][2]*mtx[1][2];
	u[1][2]=u[2][1]=mtx[1][0]*mtx[2][0]+mtx[1][1]*mtx[2][1]+mtx[1][2]*mtx[2][2];
	u[2][0]=u[0][2]=mtx[0][0]*mtx[2][0]+mtx[0][1]*mtx[2][1]+mtx[0][2]*mtx[2][2];
	Vec3 e, uv[3], vv[3];
	eigenvals( u, e );
	eigenvecs( u, e, uv );
	for ( int i = 0; i < 3; ++i ) {
		e[i] = sqrt( e[i] );
		normalize( uv[i] );
		mult( uv[i], mtx, vv[i] );
		scale( vv[i], 1.0 / e[i] );
	}
	U.setValue(
		(float)(uv[0][0]), (float)(uv[1][0]), (float)(uv[2][0]), 0,
		(float)(uv[0][1]), (float)(uv[1][1]), (float)(uv[2][1]), 0,
		(float)(uv[0][2]), (float)(uv[1][2]), (float)(uv[2][2]), 0,
		0, 0, 0, 1 );
	K.setScale( XbVec3f( e ) );
	Vt.setValue(
		(float)(vv[0][0]), (float)(vv[0][1]), (float)(vv[0][2]), 0,
		(float)(vv[1][0]), (float)(vv[1][1]), (float)(vv[1][2]), 0,
		(float)(vv[2][0]), (float)(vv[2][1]), (float)(vv[2][2]), 0,
		0, 0, 0, 1 );
}

void XbMatrix::factor( XbMatrix& u, XbMatrix& s, XbMatrix& r, XbMatrix& t )
		const {
	XbMatrix Vt;
	svd3( u, s, Vt );
	r = u * Vt;
	t.setTranslate( getTranslation() );
}

XbVec3f XbMatrix::getTranslation( void ) const {
	return XbVec3f( mtx[3][0], mtx[3][1], mtx[3][2] );
}

float XbMatrix::det3( void ) const {
	return	mtx[0][0]*(mtx[1][1]*mtx[2][2] - mtx[1][2]*mtx[2][1]) +
			mtx[0][1]*(mtx[1][2]*mtx[2][0] - mtx[1][0]*mtx[2][2]) +
			mtx[0][2]*(mtx[1][0]*mtx[2][1] - mtx[1][1]*mtx[2][0]);
}

float XbMatrix::det4( void ) const {
	return mtx[0][0] * (
		mtx[1][1] * ( mtx[2][2]*mtx[3][3] - mtx[2][3]*mtx[3][2] ) +
		mtx[1][2] * ( mtx[3][1]*mtx[2][3] - mtx[2][1]*mtx[3][3] ) +
		mtx[1][3] * ( mtx[2][1]*mtx[3][2] - mtx[3][1]*mtx[2][2] )
	) + mtx[0][1] * (
		mtx[1][0] * ( mtx[2][3]*mtx[3][2] - mtx[2][2]*mtx[3][3] ) +
		mtx[1][2] * ( mtx[2][0]*mtx[3][3] - mtx[3][0]*mtx[2][3] ) +
		mtx[1][3] * ( mtx[3][0]*mtx[2][2] - mtx[2][0]*mtx[3][2] )
	) + mtx[0][2] * (
		mtx[1][0] * ( mtx[2][1]*mtx[3][3] - mtx[3][1]*mtx[2][3] ) +
		mtx[1][1] * ( mtx[3][0]*mtx[2][3] - mtx[2][0]*mtx[3][3] ) +
		mtx[1][3] * ( mtx[2][0]*mtx[3][1] - mtx[3][0]*mtx[2][1] )
	) + mtx[0][3] * (
		mtx[1][0] * ( mtx[3][1]*mtx[2][2] - mtx[2][1]*mtx[3][2] ) +
		mtx[1][1] * ( mtx[2][0]*mtx[3][2] - mtx[3][0]*mtx[2][2] ) +
		mtx[1][2] * ( mtx[3][0]*mtx[2][1] - mtx[2][0]*mtx[3][1] )
	);
}

// JASON_OPT - caching of common sub-expressions
XbMatrix XbMatrix::inverse( void ) const
{
	float m20x31 = mtx[2][0]*mtx[3][1];
	float m20x32 = mtx[2][0]*mtx[3][2];
	float m20x33 = mtx[2][0]*mtx[3][3];
   	float m21x32 = mtx[2][1]*mtx[3][2];
	float m21x33 = mtx[2][1]*mtx[3][3];
	float m22x33 = mtx[2][2]*mtx[3][3];
	float m23x32 = mtx[2][3]*mtx[3][2];
	float m30x21 = mtx[3][0]*mtx[2][1];
	float m30x22 = mtx[3][0]*mtx[2][2];
	float m30x23 = mtx[3][0]*mtx[2][3];
	float m31x23 = mtx[3][1]*mtx[2][3];
	float m32x22 = mtx[3][1]*mtx[2][2];

	double d = mtx[0][0] * (
		mtx[1][1] * ( m22x33 - m23x32 ) +
		mtx[1][2] * ( m31x23 - m21x33 ) +
		mtx[1][3] * ( m21x32 - m32x22 )
	) + mtx[0][1] * (
		mtx[1][0] * ( m23x32 - m22x33 ) +
		mtx[1][2] * ( m20x33 - m30x23 ) +
		mtx[1][3] * ( m30x22 - m20x32 )
	) + mtx[0][2] * (
		mtx[1][0] * ( m21x33 - m31x23 ) +
		mtx[1][1] * ( m30x23 - m20x33 ) +
		mtx[1][3] * ( m20x31 - m30x21 )
	) + mtx[0][3] * (
		mtx[1][0] * ( m32x22 - m21x32 ) +
		mtx[1][1] * ( m20x32 - m30x22 ) +
		mtx[1][2] * ( m30x21 - m20x31 )
	);

	double id = 1.0 / d;

	return XbMatrix(
		(float)((mtx[1][1]*(m22x33 - m23x32) + mtx[1][2]*(m31x23-m21x33)+mtx[1][3]*(m21x32-m32x22)) * id),
		(float)((mtx[2][1]*(mtx[0][2]*mtx[3][3] - mtx[0][3]*mtx[3][2]) + mtx[2][2]*(mtx[0][3]*mtx[3][1]-mtx[0][1]*mtx[3][3])+mtx[2][3]*(mtx[0][1]*mtx[3][2]-mtx[0][2]*mtx[3][1])) * id),
		(float)((mtx[3][1]*(mtx[0][2]*mtx[1][3] - mtx[0][3]*mtx[1][2]) + mtx[3][2]*(mtx[0][3]*mtx[1][1]-mtx[0][1]*mtx[1][3])+mtx[3][3]*(mtx[0][1]*mtx[1][2]-mtx[0][2]*mtx[1][1])) * id),
		(float)((mtx[0][1]*(mtx[1][3]*mtx[2][2] - mtx[1][2]*mtx[2][3]) + mtx[0][2]*(mtx[1][1]*mtx[2][3]-mtx[1][3]*mtx[2][1])+mtx[0][3]*(mtx[1][2]*mtx[2][1]-mtx[1][1]*mtx[2][2])) * id),
		(float)((mtx[1][2]*(m20x33 - m30x23) + mtx[1][3]*(m30x22-m20x32)+mtx[1][0]*(m23x32-m22x33)) * id),
		(float)((mtx[2][2]*(mtx[0][0]*mtx[3][3] - mtx[3][0]*mtx[0][3]) + mtx[2][3]*(mtx[3][0]*mtx[0][2]-mtx[0][0]*mtx[3][2])+mtx[2][0]*(mtx[0][3]*mtx[3][2]-mtx[0][2]*mtx[3][3])) * id),
		(float)((mtx[3][2]*(mtx[0][0]*mtx[1][3] - mtx[1][0]*mtx[0][3]) + mtx[3][3]*(mtx[1][0]*mtx[0][2]-mtx[0][0]*mtx[1][2])+mtx[3][0]*(mtx[0][3]*mtx[1][2]-mtx[0][2]*mtx[1][3])) * id),
		(float)((mtx[0][2]*(mtx[2][0]*mtx[1][3] - mtx[1][0]*mtx[2][3]) + mtx[0][3]*(mtx[1][0]*mtx[2][2]-mtx[2][0]*mtx[1][2])+mtx[0][0]*(mtx[1][2]*mtx[2][3]-mtx[1][3]*mtx[2][2])) * id),
		(float)((mtx[1][3]*(m20x31 - m30x21) + mtx[1][0]*(m21x33-m31x23)+mtx[1][1]*(m30x23-m20x33)) * id),
		(float)((mtx[2][3]*(mtx[0][0]*mtx[3][1] - mtx[3][0]*mtx[0][1]) + mtx[2][0]*(mtx[0][1]*mtx[3][3]-mtx[0][3]*mtx[3][1])+mtx[2][1]*(mtx[3][0]*mtx[0][3]-mtx[0][0]*mtx[3][3])) * id),
		(float)((mtx[3][3]*(mtx[0][0]*mtx[1][1] - mtx[1][0]*mtx[0][1]) + mtx[3][0]*(mtx[0][1]*mtx[1][3]-mtx[0][3]*mtx[1][1])+mtx[3][1]*(mtx[1][0]*mtx[0][3]-mtx[0][0]*mtx[1][3])) * id),
		(float)((mtx[0][3]*(mtx[2][0]*mtx[1][1] - mtx[1][0]*mtx[2][1]) + mtx[0][0]*(mtx[1][3]*mtx[2][1]-mtx[1][1]*mtx[2][3])+mtx[0][1]*(mtx[1][0]*mtx[2][3]-mtx[2][0]*mtx[1][3])) * id),
		(float)((mtx[1][0]*(m32x22 - m21x32) + mtx[1][1]*(m20x32-m30x22)+mtx[1][2]*(m30x21-m20x31)) * id),
		(float)((mtx[2][0]*(mtx[0][2]*mtx[3][1] - mtx[0][1]*mtx[3][2]) + mtx[2][1]*(mtx[0][0]*mtx[3][2]-mtx[3][0]*mtx[0][2])+mtx[2][2]*(mtx[3][0]*mtx[0][1]-mtx[0][0]*mtx[3][1])) * id),
		(float)((mtx[3][0]*(mtx[0][2]*mtx[1][1] - mtx[0][1]*mtx[1][2]) + mtx[3][1]*(mtx[0][0]*mtx[1][2]-mtx[1][0]*mtx[0][2])+mtx[3][2]*(mtx[1][0]*mtx[0][1]-mtx[0][0]*mtx[1][1])) * id),
		(float)((mtx[0][0]*(mtx[1][1]*mtx[2][2] - mtx[1][2]*mtx[2][1]) + mtx[0][1]*(mtx[2][0]*mtx[1][2]-mtx[1][0]*mtx[2][2])+mtx[0][2]*(mtx[1][0]*mtx[2][1]-mtx[2][0]*mtx[1][1])) * id)
	);
}

XbMatrix XbMatrix::transpose( void ) const {
	return XbMatrix(
		mtx[0][0], mtx[1][0], mtx[2][0], mtx[3][0],
		mtx[0][1], mtx[1][1], mtx[2][1], mtx[3][1],
		mtx[0][2], mtx[1][2], mtx[2][2], mtx[3][2],
		mtx[0][3], mtx[1][3], mtx[2][3], mtx[3][3] );
}

XbMatrix& XbMatrix::multRight( const XbMatrix& m ) {
	return *this *= m;
}

XbMatrix& XbMatrix::multLeft( const XbMatrix& m ) {
	return *this = m * *this;
}

void XbMatrix::multMatrixVec( const XbVec3f& v1, XbVec3f& v2 ) const {
  float s =  mtx[3][0]*v1[0] + mtx[3][1]*v1[1] + mtx[3][2]*v1[2] + mtx[3][3];
	v2[0] = (mtx[0][0]*v1[0] + mtx[0][1]*v1[1] + mtx[0][2]*v1[2] + mtx[0][3])/s;
	v2[1] = (mtx[1][0]*v1[0] + mtx[1][1]*v1[1] + mtx[1][2]*v1[2] + mtx[1][3])/s;
	v2[2] = (mtx[2][0]*v1[0] + mtx[2][1]*v1[1] + mtx[2][2]*v1[2] + mtx[2][3])/s;
}

void XbMatrix::multVecMatrix( const XbVec3f& v1, XbVec3f& v2 ) const {
  float s = mtx[0][3]*v1[0] + mtx[1][3]*v1[1] + mtx[2][3]*v1[2] + mtx[3][3];
	v2[0] = (mtx[0][0]*v1[0] + mtx[1][0]*v1[1] + mtx[2][0]*v1[2] + mtx[3][0])/s;
	v2[1] = (mtx[0][1]*v1[0] + mtx[1][1]*v1[1] + mtx[2][1]*v1[2] + mtx[3][1])/s;
	v2[2] = (mtx[0][2]*v1[0] + mtx[1][2]*v1[1] + mtx[2][2]*v1[2] + mtx[3][2])/s;
}

void XbMatrix::multDirMatrix( const XbVec3f& v1, XbVec3f& v2 ) const {
	v2[0] = mtx[0][0]*v1[0] + mtx[1][0]*v1[1] + mtx[2][0]*v1[2];
	v2[1] = mtx[0][1]*v1[0] + mtx[1][1]*v1[1] + mtx[2][1]*v1[2];
	v2[2] = mtx[0][2]*v1[0] + mtx[1][2]*v1[1] + mtx[2][2]*v1[2];
}

void XbMatrix::multMatrixVec( const float *v1, float *v2 ) const {
  float s =  mtx[3][0]*v1[0] + mtx[3][1]*v1[1] + mtx[3][2]*v1[2] + mtx[3][3];
	v2[0] = (mtx[0][0]*v1[0] + mtx[0][1]*v1[1] + mtx[0][2]*v1[2] + mtx[0][3])/s;
	v2[1] = (mtx[1][0]*v1[0] + mtx[1][1]*v1[1] + mtx[1][2]*v1[2] + mtx[1][3])/s;
	v2[2] = (mtx[2][0]*v1[0] + mtx[2][1]*v1[1] + mtx[2][2]*v1[2] + mtx[2][3])/s;
}

void XbMatrix::multVecMatrix( const float *v1, float *v2 ) const {
  float s = mtx[0][3]*v1[0] + mtx[1][3]*v1[1] + mtx[2][3]*v1[2] + mtx[3][3];
	v2[0] = (mtx[0][0]*v1[0] + mtx[1][0]*v1[1] + mtx[2][0]*v1[2] + mtx[3][0])/s;
	v2[1] = (mtx[0][1]*v1[0] + mtx[1][1]*v1[1] + mtx[2][1]*v1[2] + mtx[3][1])/s;
	v2[2] = (mtx[0][2]*v1[0] + mtx[1][2]*v1[1] + mtx[2][2]*v1[2] + mtx[3][2])/s;
}
void XbMatrix::multDirMatrix( const float * v1, float *v2 ) const {
	v2[0] = mtx[0][0]*v1[0] + mtx[1][0]*v1[1] + mtx[2][0]*v1[2];
	v2[1] = mtx[0][1]*v1[0] + mtx[1][1]*v1[1] + mtx[2][1]*v1[2];
	v2[2] = mtx[0][2]*v1[0] + mtx[1][2]*v1[1] + mtx[2][2]*v1[2];
}

XbMatrix& XbMatrix::operator=( const XbRotation& q ) {
	setRotate( q );
	return *this;
}

XbMatrix XbMatrix::identity( void ) {
	return XbMatrix(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 );
}

XbEuler::XbEuler( void ) {
  setType(XbEuler::XYZ,XbEuler::STATIC);
}

XbEuler::XbEuler( const XbVec3f& v, Order order, Axes axes ) {
	angles = v;
	setType( order, axes );
}

XbEuler::XbEuler( const XbRotation& rot, Order order, Axes axes ) {
	setType( order, axes );
	XbMatrix m;
	m.setRotate( rot );
	setValue( m );
}

XbEuler::XbEuler( const XbMatrix& mtx, Order order, Axes axes ) {
	setType( order, axes );
	setValue( mtx );
}

XbEuler::~XbEuler( void ) {
}
#ifdef XB_LINEAR_USE_STREAMS
istream& XbEuler::read( istream& is ) {
	return angles.read( is ) >> type;
}

ostream& XbEuler::write( ostream& os ) const {
	return angles.write( os ) << ' ' << type;
}
#endif
void XbEuler::getValue( XbVec3f& a, Order& order, Axes& axes ) const {
	a = angles;
	order = getOrder();
	axes = getAxes();
}

XbVec3f XbEuler::getAngles( void ) const {
	return angles;
}

XbEuler::Order XbEuler::getOrder( void ) const {
	static const Order s[12] = {
		XYZ, XYX, XZY, XZX, YZX, YZY, YXZ, YXY, ZXY, ZXZ, ZYX, ZYZ
	};
	static const Order r[12] = {
		ZYX, XYX, YZX, XZX, XZY, YZY, ZXY, YXY, YXZ, ZXZ, XYZ, ZYZ
	};
	return type & 1 ? r[ type >> 1 ] : s[ type >> 1 ];
}

XbEuler::Axes XbEuler::getAxes( void ) const {
	return type & 1 ? ROTATING : STATIC;
}

void XbEuler::setValue( const XbVec3f& a, Order order, Axes axes ) {
	angles = a;
	setType( order, axes );
}

void XbEuler::setValue( const XbRotation& rot, Order order, Axes axes ) {
	setType( order, axes );
	XbMatrix m;
	m.setRotate( rot );
	setValue( m );
}

void XbEuler::setAngles( const XbVec3f& v ) {
	angles = v;
}

void XbEuler::setOrder( Order order ) {
	if ( order == getOrder() ) return;
	XbMatrix m( *this );
	setType( order, getAxes() );
	setValue( m );
}

void XbEuler::setAxes( Axes axes ) {
	if ( axes == getAxes() ) return;
	XbMatrix m( *this );
	setType( getOrder(), axes );
	setValue( m );
}

void XbEuler::setOrderAndAxes( Order order, Axes axes ) {
	if ( order == getOrder() && axes == getAxes() ) return;
	XbMatrix m( *this );
	setType( order, axes );
	setValue( m );
}

XbEuler XbEuler::alternate( void ) const {
	switch( getOrder() ) {
		case XYZ: case XZY: case YZX: case YXZ: case ZXY: case ZYX:
			return XbEuler( XbVec3f( (float)M_PI + angles[0], (float)M_PI - angles[1],
					(float)M_PI + angles[2] ), getOrder(), getAxes() );
    case XYX: case XZX: case YZY: case YXY: case ZXZ: case ZYZ: break;
	}	
	return XbEuler( XbVec3f( (float)M_PI + angles[0], -angles[1], (float)M_PI + angles[2] ),
			getOrder(), getAxes() );
}

inline double closest( double d ) {
	return d >= 0.0 ? int(d+.5) : int(d-.5);
}

inline double closer( double a, double b ) {
	const double twopi = 2 * M_PI;
	return a + closest( ( b - a ) / twopi ) * twopi;
}

inline XbVec3f closer( const XbVec3f& a, const XbVec3f& b ) {
	return XbVec3f( (float)closer( a[0], b[0] ), (float)closer( a[1], b[1] ),
			(float)closer( a[2], b[2] ) );
}

inline double max3( double a, double b, double c ) {
	if ( a > b ) b = a;
	if ( b > c ) c = b;
	return c;
}

inline double dist( const XbVec3f& a, const XbVec3f& b ) {
	XbVec3f d = a - b;
	return max3( fabs( d[0] ), fabs( d[1] ), fabs( d[2] ) );
}

XbEuler XbEuler::closest( const XbVec3f& e ) const {
	XbVec3f a1 = closer( angles, e );
	XbVec3f a2 = closer( alternate().angles, e );
	return XbEuler( dist( e, a1 ) < dist( e, a2 ) ? a1 : a2,
			getOrder(), getAxes() );
}

bool XbEuler::operator==( const XbEuler& x ) const {
	return angles == x.angles && type == x.type;
}

void XbEuler::getInfo( int& i, int& j, int& k, bool& n, bool& s, bool& f ) const {
	f = ((type & 1)==0)? false:true;
	s = ((type & 2)==0)? false:true;
	n = ((type & 4)==0)? false:true;
	int next[] = { 1, 2, 0, 1 };
	i = type >> 3;
	j = next[ i+n ];
	k = next[ i+1-n ];
}

void XbEuler::setType( Order order, Axes axes ) {
	type = axes == STATIC ? ( order & 0x0f ) << 1 : ( order & 0xf0 ) >> 3 | 1;
}

void XbEuler::setValue( const XbMatrix& inM ) {
	int i,j,k;
	bool n,s,f;
	getInfo( i,j,k,n,s,f );
	
	XbMatrix M = inM;
	float iScale = sqrtf(M[i][0]*M[i][0] + M[i][1]*M[i][1] + M[i][2]*M[i][2]);
	float jScale = sqrtf(M[j][0]*M[j][0] + M[j][1]*M[j][1] + M[j][2]*M[j][2]);
	float kScale = sqrtf(M[k][0]*M[k][0] + M[k][1]*M[k][1] + M[k][2]*M[k][2]);
	M[i][0] /= iScale; M[i][1] /= iScale; M[i][2] /= iScale;
	M[j][0] /= jScale; M[j][1] /= jScale; M[j][2] /= jScale;
	M[k][0] /= kScale; M[k][1] /= kScale; M[k][2] /= kScale;

	if (s) {
		double sy = sqrt(M[j][i]*M[j][i] + M[k][i]*M[k][i]);
		if (sy > 0.0001f) {
			angles[0] = (float)atan2(M[j][i], M[k][i]);
			angles[1] = (float)atan2((float)sy, M[i][i]);
			angles[2] = (float)atan2(M[i][j], -M[i][k]);
		} else {
			angles[0] = (float)atan2(-M[k][j], M[j][j]);
			angles[1] = (float)atan2((float)sy, M[i][i]);
			angles[2] = 0.0f;
		}
	} else {
		double cy = sqrt(M[i][i]*M[i][i] + M[i][j]*M[i][j]);
		if (cy > 0.0001f) {
			angles[0] = (float)atan2(M[j][k], M[k][k]);
			angles[1] = (float)atan2(-M[i][k],(float)cy);
			angles[2] = (float)atan2(M[i][j],M[i][i]);
		} else {
			angles[0] = (float)atan2(-M[k][j], M[j][j]);
			angles[1] = (float)atan2(-M[i][k],(float)cy);
			angles[2] = 0;
		}
	}
	if (n) {angles = -angles;}
	if (f) {float t = angles[0]; angles[0] = angles[2]; angles[2] = t;}
}

#if 0
//
// !!!!!!!!!!!!!!!!!!!! ERROR ERROR ERROR ERROR ERROR !!!!!!!!!!!!!!!!!!!
//
//  The distance calculation is wrong!!!!!!!
//
//
//  static double getClosestPointOnLine()
//
//  Get the closest point between a line and a point.
//
//  Parameters:
//    const XbVec3f& l1 : defines the line
//    const XbVec3f& l2 : defines the line
//    const XbVec3f& pt : point
//    XbVec3f& P       : closest point on the line
//    double& distance : closest distance
//
inline double getClosestPointOnLine(
                  const XbVec3f& l1,
                  const XbVec3f& l2,
                  const XbVec3f& pt, 
                  XbVec3f& P,
                  double& distance )
{
    double t2 = l2[0] * l1[0];
    double t4 = l1[0] * l1[0];
    double t6 = l2[1] * l1[1];
    double t8 = l1[1] * l1[1];
    double t10 = l2[2] * l1[2];
    double t12 = l1[2] * l1[2];
    double t13 = l2[0] * pt[0] - t2 - l1[0] * pt[0] + t4 + l2[1] *
                 pt[1] - t6 - l1[1] * pt[1] + t8 + l2[2] *
                 pt[2] - t10 - l1[2] * pt[2] + t12;
    double t = t13 / (l2[0] * l2[0] - 2.0 * t2 + t4 + l2[1] * l2[1] - 2.0 * t6 + t8 +
               l2[2] * l2[2] - 2.0 * t10 + t12);
    distance = t;

    P = (l1 + t * (l2 - l1));
    return t;
}
#endif

//
//  static double getClosestPointOnLine()
//
//  Reference: Graphics Gems II, Chapter 1.3
//
//  Get the closest point between a line and a point.
//
//  Parameters:
//    const XbVec3f& A    :
//    const XbVec3f& B    : A and B define the line segment
//    const XbVec3f& P    : point
//    XbVec3f& closestPt  : closest point on the line
//    float& sqrDistance  : closest distance
//    bool &between       : Is perpendicular projection of the point on the line
//                          inside the line segment?
//
//  Return:
//    float t :  0 <= t <= 1. Closest point on the line segment.
//
inline double getClosestPointOnLine(
                  const XbVec3f& A,
                  const XbVec3f& B,
                  const XbVec3f& P, 
                  XbVec3f& closestPt,
                  float& sqrDistance,
                  bool &between )
{
    //
    //  Find the dot product of the line segments: AP and AB.
    //
    XbVec3f AP( P - A );
    XbVec3f AB( B - A );

    //
    //  Notice: I don't normalize the vectors because I'm only checking
    //  for the sign of the dot product.
    //
    float Dot = AP.dot( AB );

    //
    //  If the dot product is less than 0, that means the closest point
    //  is beyond A outside the AB line segment.
    //
    if( Dot < 0 )
    {
        sqrDistance = AP.sqrlength();
        closestPt = A;
        between = false;
        return 0.0;
    }

    //
    //  Find the dot product of the line segments: PB and AB.
    //
    XbVec3f PB( B - P );

    //
    //  Notice: I don't normalize here either.
    //
    Dot = PB.dot( AB );

    //
    //  If the dot product is less than 0, that means the closest point 
    //  is beyond B outside the AB line segment.
    //
    if ( Dot < 0 )
    {
        sqrDistance = PB.sqrlength();
        closestPt = B;
        between = false;
        return 1.0;
    }

    //
    //  Find the perpendicular distance between the point and the line.
    //
    XbVec3f Cross = AP.cross( AB );
    float parallelogramSqrArea = Cross.sqrlength();
    sqrDistance = parallelogramSqrArea / AB.sqrlength();

    AP.normalize();
    AB.normalize();
    float APdotAB = AP.dot( AB );

    closestPt = A + APdotAB * AB;
    between = true;

    return APdotAB;
}

//
//  int pointDistanceFromLine2D()
//
//  Find the closest distance between a point, P and a line segment, AB.
//
//  Reference: Graphics Gems II, Chapter 1.3
//
//  Parameters:
//     int Ax,Ay,Bx,By   : (in) defines the line segment, AB
//     int Px,Py         : (in) defines the point
//     bool between      : (out) the perpendicular projection of the point is inside AB?
//
//  Returns:
//     int distSqr : the square distance
//
inline int pointDistanceFromLine2D(
               int Ax, int Ay, int Bx, int By,     // line
               int Px, int Py,                     // point
               bool &between )
{
    //
    //  Find the dot product of the line segments: AP and AB.
    //
    int ABx = Bx - Ax;
    int ABy = By - Ay;
    int APx = Px - Ax;
    int APy = Py - Ay;
    int Dot = APx * ABx + APy * ABy;

    //
    //  If the dot product is less than 0, that means the closest point
    //  is beyond A outside the AB line segment.
    //
    if( Dot < 0 )
    {
        int distSqr = APx * APx + APy * APy;
        between = false;
        return distSqr;
    }

    //
    //  Find the dot product of the line segments: PB and AB.
    //
    int PBx = Bx - Px;
    int PBy = By - Py;
    Dot = PBx * ABx + PBy * ABy;

    //
    //  If the dot product is 0, that means the closest point is inside
    //  the AB line segment.
    //
    if ( Dot < 0 )
    {
        int distSqr = PBx * PBx + PBy * PBy;
        between = false;
        return distSqr;
    }

    //
    //  Find the perpendicular distance between the point and the line.
    //
    double nom = (Py - Ay) * (Bx - Ax) - (Px - Ax) * (By - Ay);
    double denom = ABx * ABx + ABy * ABy;
    int d1sqr = (int)( nom * nom / denom );
    between = true;
    return d1sqr;
}

//
//  float pointSqrDistanceFromRay()
//
//  Find the closest distance between a point, P and a line defined by A and B.
//  Assuming the line is infinite.
//
//  Reference: Graphics Gems II, Chapter 1.3
//
//  Parameters:
//     const XbVec3f A, B : (in) defines the line, AB
//     const XbVec3f P    : (in) defines the point
//     XbVec3f& closestPt : (out) the closest on point AB
//     float& t           : (out) t value for the closest point
//
//  Returns:
//     float sqrDistance : the square distance between the point and the line.
//
inline float pointSqrDistanceFromRay(
                  const XbVec3f& A,
                  const XbVec3f& B,
                  const XbVec3f& P, 
                  XbVec3f& closestPt,
                  float& t )
{
  XbVec3f AP( P - A );
  XbVec3f AB( B - A );
  
  if (A==B) {
    t = 0.0f;
    closestPt = A;
    return AP.sqrlength();
  }
  //
  //  Find the perpendicular distance between the point and the line.
  //
  XbVec3f Cross = AP.cross( AB );
  float parallelogramSqrArea = Cross.sqrlength();
  float sqrDistance = parallelogramSqrArea / AB.sqrlength();
  
  float len = AB.normalize();
  t = AP.dot( AB );
  closestPt = A + t * AB;
  t = t / len;
  
  return sqrDistance;
}
//
//  Some math utility functions - probably should be put in some math library instead.
//  Reference: Graphics Gems I & II.
//

/* The unmatrix subroutine fills in a vector of floating point
 * values.  These symbols make it easier to get the data back out.
 */
enum unmatrix_indices {
 	U_SCALEX = 0,
 	U_SCALEY,
 	U_SCALEZ,
 	U_SHEARXY,
 	U_SHEARXZ,
 	U_SHEARYZ,
 	U_ROTATEX,
 	U_ROTATEY,
 	U_ROTATEZ,
 	U_TRANSX,
 	U_TRANSY,
 	U_TRANSZ,
 	U_PERSPX,
 	U_PERSPY,
 	U_PERSPZ,
 	U_PERSPW
};

/* multiply a hom. point by a matrix and return the transformed point */
inline void V4MulPointByMatrix( const float pin[4], const XbMatrix &m, float pout[4] )
{
	  pout[0] = pin[0] * m[0][0] + pin[1] * m[1][0] + pin[2] * m[2][0] + pin[3] * m[3][0];
	  pout[1] = pin[0] * m[0][1] + pin[1] * m[1][1] + pin[2] * m[2][1] + pin[3] * m[3][1];
	  pout[2] = pin[0] * m[0][2] + pin[1] * m[1][2] + pin[2] * m[2][2] + pin[3] * m[3][2];
	  pout[3] = pin[0] * m[0][3] + pin[1] * m[1][3] + pin[2] * m[2][3] + pin[3] * m[3][3];
}

/* unmatrix() - Decompose a non-degenerate 4x4 transformation matrix into
 * 	the sequence of transformations that produced it.
 * [Sx][Sy][Sz][Shearx/y][Sx/z][Sz/y][Rx][Ry][Rz][Tx][Ty][Tz][P(x,y,z,w)]
 *
 * The coefficient of each transformation is returned in the corresponding
 * element of the vector tran.
 *
 * Returns 1 upon success, 0 if the matrix is singular.
 */
inline int unmatrix( const XbMatrix &mat, float tran[16] )
{
 	  XbMatrix locmat = mat;
	  int i;
 	  for (i=0; i<4;i++ )
 		  for ( int j=0; j<4; j++ )
 			  locmat[i][j] /= locmat[3][3];

 	  /* pmat is used to solve for perspective, but it also provides
 	   * an easy way to test for singularity of the upper 3x3 component.
 	   */
 	  XbMatrix pmat = locmat;
 	  for ( i=0; i<3; i++ )
 		  pmat[i][3] = 0;
 	  pmat[3][3] = 1;

 	  if ( pmat.det4() == 0.0 )
 		  return 0;

 	  /* First, isolate perspective.  This is the messiest. */
 	  if ( locmat[0][3] != 0 || locmat[1][3] != 0 || locmat[2][3] != 0 )
    {
 		  /* prhs is the right hand side of the equation. */
      float prhs[4];
 		  prhs[0]= locmat[0][3];
 		  prhs[1] = locmat[1][3];
 		  prhs[2] = locmat[2][3];
 		  prhs[3] = locmat[3][3];

 		  /* Solve the equation by inverting pmat and multiplying
 		   * prhs by the inverse.  (This is the easiest way, not
 		   * necessarily the best.)
 		   * inverse function (and det4x4, above) from the Matrix
 		   * Inversion gem in the first volume.
 		   */
      XbMatrix invpmat = pmat.inverse();
      XbMatrix tinvpmat = invpmat.transpose();
 	    float psol[4];
 		  V4MulPointByMatrix( prhs, tinvpmat, psol );
 
 		  /* Stuff the answer away. */
 		  tran[U_PERSPX] = psol[0];
 		  tran[U_PERSPY] = psol[1];
 		  tran[U_PERSPZ] = psol[2];
 		  tran[U_PERSPW] = psol[3];

 		  /* Clear the perspective partition. */
 		  locmat[0][3] = locmat[1][3] = locmat[2][3] = 0;
 		  locmat[3][3] = 1;
 	  }
    else		/* No perspective. */
    {
 		  tran[U_PERSPX] = tran[U_PERSPY] = tran[U_PERSPZ] = tran[U_PERSPW] = 0;
    }

 	  /* Next take care of translation (easy). */
 	  for ( i=0; i<3; i++ )
    {
 		  tran[U_TRANSX + i] = locmat[3][i];
 		  locmat[3][i] = 0;
 	  }

 	  /* Now get scale and shear. */
    XbVec3f row[3];
 	  for ( i=0; i<3; i++ )
    {
 		  row[i][0] = locmat[i][0];
 		  row[i][1] = locmat[i][1];
 		  row[i][2] = locmat[i][2];
 	  }

 	  /* Compute X scale factor and normalize first row. */
    tran[U_SCALEX] = row[0].length();
    row[0].normalize();

 	  /* Compute XY shear factor and make 2nd row orthogonal to 1st. */
    tran[U_SHEARXY] = row[0].dot( row[1] );
    row[1] = row[1] + row[0] * -tran[U_SHEARXY];

 	  /* Now, compute Y scale and normalize 2nd row. */
 	  tran[U_SCALEY] = row[1].length();
    row[1].normalize();
 	  tran[U_SHEARXY] /= tran[U_SCALEY];

 	  /* Compute XZ and YZ shears, orthogonalize 3rd row. */
    tran[U_SHEARXZ] = row[0].dot( row[2] );
    row[2] = row[2] + row[0] * -tran[U_SHEARXZ];
    tran[U_SHEARYZ] = row[1].dot( row[2] );
    row[2] = row[2] + row[1] * -tran[U_SHEARYZ];

 	  /* Next, get Z scale and normalize 3rd row. */
 	  tran[U_SCALEZ] = row[2].length();
    row[2].normalize();
 	  tran[U_SHEARXZ] /= tran[U_SCALEZ];
 	  tran[U_SHEARYZ] /= tran[U_SCALEZ];
 
 	  /* At this point, the matrix (in rows[]) is orthonormal.
 	   * Check for a coordinate system flip.  If the determinant
 	   * is -1, then negate the matrix and the scaling factors.
 	   */
    if( row[0].dot( row[1].cross(row[2]) ) < 0 )
    {
 		  for ( i = 0; i < 3; i++ )
      {
 			  tran[U_SCALEX+i] *= -1;
 			  row[i][0] *= -1;
 			  row[i][1] *= -1;
 			  row[i][2] *= -1;
 		  }
    }
 
 	  /* Now, get the rotations out, as described in the gem. */
 	  tran[U_ROTATEY] = asinf(-row[0][2]);
 	  if ( cosf(tran[U_ROTATEY]) != 0 )
    {
 		  tran[U_ROTATEX] = atan2f(row[1][2], row[2][2]);
 		  tran[U_ROTATEZ] = atan2f(row[0][1], row[0][0]);
 	  }
    else
    {
 		  tran[U_ROTATEX] = atan2f(row[1][0], row[1][1]);
 		  tran[U_ROTATEZ] = 0;
 	  }

 	  /* All done! */
 	  return 1;
}

//
//  Decompose the matrix.
//  The angles are in radians.
//
void XbMatrix::getTransform(
        XbVec3f& translation,
        XbVec3f& angles,
        XbVec3f& scaleFactor ) const
{
#ifdef THIS_DOES_NOT_WORK
	XbMatrix m(
		mtx[0][0], mtx[0][1], mtx[0][2], 0,
		mtx[1][0], mtx[1][1], mtx[1][2], 0,
		mtx[2][0], mtx[2][1], mtx[2][2], 0,
		0, 0, 0, 1 );
	XbMatrix c, cinv, s, u, r, t;
	c.setTranslate( center );
	cinv.setTranslate( -center );
	( c * m * cinv ).factor( u, s, r, t );
	translation = getTranslation() + t.getTranslation();
	rotation.setValue( r );
	scaleFactor.setValue( s[0][0], s[1][1], s[2][2] );
	scaleOrientation.setValue( u.transpose() );
#else
    float result[16];
    (void)unmatrix( XbMatrix(mtx), result );

    translation[0] = result[U_TRANSX];
    translation[1] = result[U_TRANSY];
    translation[2] = result[U_TRANSZ];
    angles[0] = result[U_ROTATEX];
    angles[1] = result[U_ROTATEY];
    angles[2] = result[U_ROTATEZ];
    scaleFactor[0] = result[U_SCALEX];
    scaleFactor[1] = result[U_SCALEY];
    scaleFactor[2] = result[U_SCALEZ];
#endif
}

#endif // ifndef SWIG

#ifdef EA_PLATFORM_WINDOWS
#pragma warning( pop )
#endif

#endif // XbLinear_h
