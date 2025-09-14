// (c) 1998-2010 Electronic Arts Inc.

#ifndef XbMathUtil_H
#define XbMathUtil_H

#include "XbLinear.h"
inline bool BETWEEN(float f,float f1,float f2) {
  if (f1 < f2) {
    if (f >= f1 && f <= f2) return 1;
  } else {
    if (f <= f1 && f >= f2) return 1;
  }
  return 0;
}
#define DOT3(a,b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])
#define NORM3(a) {float len = (a[0]*a[0]+a[1]*a[1]+a[2]*a[2]); a[0]/=len; a[1]/=len; a[2]/=len;}
#ifndef ZERO_EPSILON
#define ZERO_EPSILON	(0.001)
#endif

#ifndef M_PI
#define M_PI             3.14159265358979323846
#endif
#ifndef M_PIf
#define M_PIf            3.14159265358979323846f
#endif
#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880f
#define M_SQRT1_2       0.70710678118654752440f
#endif

#ifndef FZERO
#define FZERO( x )	(( fabs(x) < ZERO_EPSILON ) ? true : false)
#endif

#define FLESS( x )	( (x) - ZERO_EPSILON )
#define FMORE( x )	( (x) + ZERO_EPSILON )

class BoundingBox {
public:
  XbVec3f min;
  XbVec3f max;
  void expand(const XbVec3f &tmp) {
    if (first) {
      first = 0; 
      min[0] = max[0] = tmp[0];
      min[1] = max[1] = tmp[1];
      min[2] = max[2] = tmp[2];
    }
    if (tmp[0] < min[0]) {min[0] = tmp[0];}
    if (tmp[1] < min[1]) {min[1] = tmp[1];}
    if (tmp[2] < min[2]) {min[2] = tmp[2];}
    if (tmp[0] > max[0]) {max[0] = tmp[0];}
    if (tmp[1] > max[1]) {max[1] = tmp[1];}
    if (tmp[2] > max[2]) {max[2] = tmp[2];}
  }
  void empty() {
    first = 1;
    min[0] = max[0] = 0.0f;
    min[1] = max[1] = 0.0f;
    min[2] = max[2] = 0.0f;
  }
private:
  int first;
};

inline void rotateFromTo(XbVec3f &from,XbVec3f &to,XbMatrix &m) {
  from.normalize();
  to.normalize();
  XbRotation rFix;
  XbVec3f Cross = from.cross(to);
  //printf("cross len = %f dot = %f\n",cross.length(),from.dot(to));
  if (Cross.length() < 1e-4) {
    if (from.dot(to) > 0) {
      //printf("parallel\n");
      rFix.setValue(XbVec3f(1,0,0),0);
    } else {
      //printf("parallel but opposite dirs\n");
      rFix.setValue(XbVec3f(1,0,0),3.14159f);
    }
  } else {
    rFix.setValue(from,to);
  }
  //float angle;
  //XbVec3f axis;
  //rFix.getValue(axis,angle);
  //printf("axis = %f %f %f angle = %f\n",axis[0],axis[1],axis[2],angle);
  m.setRotate(rFix);
}

inline XbVec3f normalizeScale(XbMatrix &A) {
  XbVec3f xvec(A[0]),yvec(A[1]),zvec(A[2]);
  XbVec3f scl;
  scl[0] = xvec.normalize();
  scl[1] = yvec.normalize();
  scl[2] = zvec.normalize();
  A[0][0] = xvec[0];  A[0][1] = xvec[1];  A[0][2] = xvec[2];
  A[1][0] = yvec[0];  A[1][1] = yvec[1];  A[1][2] = yvec[2];
  A[2][0] = zvec[0];  A[2][1] = zvec[1];  A[2][2] = zvec[2];
  return scl;
}
// ASSUMES POSITIVE UNIFORM SCALE - NO SHEARING
inline void interpolateMatricesWithScale(XbMatrix A, XbMatrix B, float t, XbMatrix &res) {
  XbVec3f scl1 = normalizeScale(A);
  XbVec3f scl2 = normalizeScale(B);
  XbRotation rot1(A);
  XbRotation rot2(B);
  XbRotation rot3;
  float q1[4];
  float q2[4];
  float q3[4];
  rot1.getValue(q1[0],q1[1],q1[2],q1[3]);
  rot2.getValue(q2[0],q2[1],q2[2],q2[3]);
  q3[0] = q1[0]-q2[0];
  q3[1] = q1[1]-q2[1];
  q3[2] = q1[2]-q2[2];
  q3[3] = q1[3]-q2[3];
  float dist1 = (q3[0]*q3[0])+(q3[1]*q3[1])+(q3[2]*q3[2])+(q3[3]*q3[3]);
  q3[0] = q1[0]+q2[0];
  q3[1] = q1[1]+q2[1];
  q3[2] = q1[2]+q2[2];
  q3[3] = q1[3]+q2[3];
  float dist2 = (q3[0]*q3[0])+(q3[1]*q3[1])+(q3[2]*q3[2])+(q3[3]*q3[3]);
  if (dist2 < dist1) {
    rot2.setValue(-q2[0],-q2[1],-q2[2],-q2[3]);
  }
  
  rot3 = XbRotation::slerp(rot1,rot2,t);
  XbVec3f v1 = A[3];
  XbVec3f v2 = B[3];
  XbVec3f v3 = ((1.0f-t) * v1) + (t * v2);
  XbVec3f scl3 = ((1.0f-t)*scl1) + (t*scl2);
  if ((fabs(scl3[0] - 1.0f) > 0.0001f) || (fabs(scl3[1] - 1.0f) > 0.0001f) || (fabs(scl3[2] - 1.0f) > 0.0001f)) {
    res.setTransform(v3,rot3,scl3);
  } else {
    res.setRotate(rot3);
  
  }
  res[3][0] = v3[0];
  res[3][1] = v3[1];
  res[3][2] = v3[2];
}

// ASSUMES NO SCALING
inline void interpolateMatrices(XbMatrix &A, XbMatrix &B, float t, XbMatrix &res) {
  XbRotation rot1(A);
  XbRotation rot2(B);
  XbRotation rot3;
  float q1[4];
  float q2[4];
  float q3[4];
  rot1.getValue(q1[0],q1[1],q1[2],q1[3]);
  rot2.getValue(q2[0],q2[1],q2[2],q2[3]);
  q3[0] = q1[0]-q2[0];
  q3[1] = q1[1]-q2[1];
  q3[2] = q1[2]-q2[2];
  q3[3] = q1[3]-q2[3];
  float dist1 = (q3[0]*q3[0])+(q3[1]*q3[1])+(q3[2]*q3[2])+(q3[3]*q3[3]);
  q3[0] = q1[0]+q2[0];
  q3[1] = q1[1]+q2[1];
  q3[2] = q1[2]+q2[2];
  q3[3] = q1[3]+q2[3];
  float dist2 = (q3[0]*q3[0])+(q3[1]*q3[1])+(q3[2]*q3[2])+(q3[3]*q3[3]);
  if (dist2 < dist1) {
    rot2.setValue(-q2[0],-q2[1],-q2[2],-q2[3]);
  }
  
  rot3 = XbRotation::slerp(rot1,rot2,t);
  XbVec3f v1 = A[3];
  XbVec3f v2 = B[3];
  XbVec3f v3 = ((1.0f-t) * v1) + (t * v2);
  res.setRotate(rot3);
  res[3][0] = v3[0];
  res[3][1] = v3[1];
  res[3][2] = v3[2];
}


inline XbRotation XbWeightQuaternionSum(XbRotation *quats, float *weights, int num) {
  XbRotation res(XbVec3f(1,0,0),0);
  int i;
  float w = 0;
  int num2 = 0;;
  for(i=0;i<num;i++) {
    w+=weights[i];
    if (weights[i]!=0.0f) num2++;
  }
  if (num2 == 0) return res;
  XbRotation *q = new XbRotation[num2];
  float *wts = new float[num2];
  int j = 0;
  for(i=0;i<num;i++) {
    if (weights[i]!=0.0f) {
      wts[j] = weights[i];
      q[j] = quats[i];
      j++;
    }
  }
  res = q[0];
  if (num2 == 1) {
    delete [] q;
    delete [] wts;
    return res;
  }
  float wtmp,lwtmp;
  float w1,w2;
  wtmp = wts[0]+wts[1];
  w1 = wts[0]/wtmp;
  w2 = 1-w1;
  res = XbRotation::slerp(q[0],q[1],w2);
  for(i=2;i<num;i++) {
    lwtmp = wtmp;
    wtmp = lwtmp+wts[i];
    w1 = lwtmp/wtmp;
    w2 = 1-w1;
    res = XbRotation::slerp(res,q[i],w2);
  }
  delete [] wts;
  delete [] q;
  return res;
}

inline XbRotation XbShortestWeightQuaternionSum(
	XbRotation *quats, float *weights, int num)
{
    if( num <= 0 ) return( XbRotation( XbVec3f( 1, 0, 0 ), 0 ) );

    XbRotation result = quats[0];
    float totalWeight = weights[0];
    float neg[4];
    XbRotation negQ;

    for( int idx = 1; idx < num; idx++ )
    {
	if( FZERO( weights[idx] ) ) continue;
        if(weights[idx]==1.0f) return(quats[idx]);
	totalWeight += weights[idx];
	float weightNormalized = weights[idx] / totalWeight;

	quats[idx].getValue( neg[0], neg[1], neg[2], neg[3] );
	negQ.setValue( -neg[0], -neg[1], -neg[2], -neg[3] );
	float _dist    = XbRotation::distance( result, quats[idx] );
	float distNeg = XbRotation::distance( result, negQ );

	if( _dist < distNeg )
	    result = XbRotation::slerp( result, quats[idx], weightNormalized );
	else
	    result = XbRotation::slerp( result, negQ, weightNormalized );
    }

    return result;
}

class XbRay {
public:
  XbVec3f dir;
  XbVec3f orig;
};
inline float distanceRayPoint(XbRay &ray, float pt[3]) {
  XbVec3f v1(ray.dir);
  XbVec3f v2(pt[0]-ray.orig[0],pt[1]-ray.orig[1],pt[2]-ray.orig[2]);
  float d = (v1.cross(v2)).length()/v2.length();
  return(d);
}

inline float project_to_sphere(float r, float x, float y)
/*
* Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
* if we are away from the center of the sphere.
*/
{
  float d, t, z;
  
  d = (float)sqrt(x*x + y*y);
  if (d < r*M_SQRT1_2)    /* Inside sphere */
    z = (float)sqrt(r*r - d*d);
  else
  {                       /* On hyperbola */
    t = r / M_SQRT2;
    z = t*t / d;
  }
  return z;
}

inline float closestPointOnLine(const XbRay &ray,const XbVec3f &pt, XbVec3f &res, float &t)
{
  XbVec3f Q(ray.orig);
  XbVec3f P(pt);
  XbVec3f v2(ray.dir);
  XbVec3f R=Q+v2;
  XbVec3f v1 = P-Q;
  v2.normalize();
  t = v1.dot(v2);
  res = Q+t*v2;
  t = t/ray.dir.length();
  return(v1.cross(v2).length());
}

inline void intersectLineWithPlane(float p[3], float v[3], float q[3], float w[3], XbVec3f &ptn) {
  // p -> point on plane
  // v -> normal to plane
  // q -> point on line
  // w -> direction of line
  float pq[3];  
  pq[0] = p[0]-q[0];
  pq[1] = p[1]-q[1];
  pq[2] = p[2]-q[2];
  float dot1 = pq[0]*v[0] + pq[1]*v[1] + pq[2]*v[2];
  float dot2 = w[0]*v[0] + w[1]*v[1] + w[2]*v[2];
  float t = dot1/dot2;
  ptn[0] = q[0] + t * w[0];
  ptn[1] = q[1] + t * w[1];
  ptn[2] = q[2] + t * w[2];
}
/*
* intersect a parameterized ray with a plane,
* and return the parameter at the intersection
*/

inline bool rayPlane(XbRay &ray, float p[4], float w[3], float &t)
{
  float Dot = DOT3(ray.dir, p);
  if (fabs(Dot)<0.0001) return(0);
  t = -(DOT3(ray.orig,p)+p[3])/Dot;
  w[0] = ray.orig[0]+ray.dir[0]*t;
  w[1] = ray.orig[1]+ray.dir[1]*t;
  w[2] = ray.orig[2]+ray.dir[2]*t;
  return 1;
}

/*
* calculate the plane equation for a 3D planar polygon
* given as a list of nb points, let the plane equation be
* Ax + By + Cz + D = 0,
* then, A, B, C is the area of projection of the polygon
* onto YOZ, ZOX, and XOY plane respectively.
*/

inline void planeEquation(int nb, Float3 *v, float eq[4]) {
  float  Sxy, Syz, Szx;
  float  cx, cy, cz;
  int   i, j, j_1;
  Sxy = Syz = Szx = 0.0;
  cx = cy = cz = 0.0;
  for (i=0; i<nb; i++) {
    j = i;
    j_1 = (i+1)%nb;
    cx += v[j][0];
    cy += v[j][1];
    cz += v[j][2];
    Sxy += (v[j_1][1]*v[j][0]-v[j][1]*v[j_1][0]);
    Syz += (v[j_1][2]*v[j][1]-v[j][2]*v[j_1][1]);
    Szx += (v[j_1][0]*v[j][2]-v[j][0]*v[j_1][2]);
  }
  eq[0] = Syz/2.0f;
  eq[1] = Szx/2.0f;
  eq[2] = Sxy/2.0f;
  eq[3] = -(eq[0]*cx+eq[1]*cy+eq[2]*cz)/nb;
}

//
//	Double version of the above
//
inline void planeEquation(int nb, Float3 *v, double eq[4]) {
  float  Sxy, Syz, Szx;
  float  cx, cy, cz;
  int   i, j, j_1;
  Sxy = Syz = Szx = 0.0;
  cx = cy = cz = 0.0;
  for (i=0; i<nb; i++) {
    j = i;
    j_1 = (i+1)%nb;
    cx += v[j][0];
    cy += v[j][1];
    cz += v[j][2];
    Sxy += (v[j_1][1]*v[j][0]-v[j][1]*v[j_1][0]);
    Syz += (v[j_1][2]*v[j][1]-v[j][2]*v[j_1][1]);
    Szx += (v[j_1][0]*v[j][2]-v[j][0]*v[j_1][2]);
  }
  eq[0] = Syz/2.0f;
  eq[1] = Szx/2.0f;
  eq[2] = Sxy/2.0f;
  eq[3] = -(eq[0]*cx+eq[1]*cy+eq[2]*cz)/nb;
}

inline void cross3(float u[3],float v[3],float w[3]) {
  w[0] = u[1]*v[2]-u[2]*v[1];
  w[1] = u[2]*v[0]-u[0]*v[2];
  w[2] = u[0]*v[1]-u[1]*v[0];
}

inline float clockwise(float v0[3],float v1[3],float w0[3],float w1[3]) {
  float A[3],B[3],C[3],T[3];
  A[0] = v1[0]-v0[0];
  A[1] = v1[1]-v0[1];
  A[2] = v1[2]-v0[2];
  B[0] = w1[0]-v0[0];
  B[1] = w1[1]-v0[1];
  B[2] = w1[2]-v0[2];
  C[0] = w1[0]-w0[0];
  C[1] = w1[1]-w0[1];
  C[2] = w1[2]-w0[2];
  cross3(A,B,T);
  return DOT3(T,C);
}

inline int rayPoly(int nb, Float3 *v, XbRay &ray, float intx[3], float &t, int& vertex, bool triStrip=false) {
  if( !nb ) return(0);
  bool cw = true;
  int i;
  float tmp1[3],tmp2[3];

  //////
  float eq[4];
  planeEquation(nb,v,eq);
  if (!rayPlane(ray,eq,intx,t)) {
    return(0);
  }
  tmp1[0] = intx[0] - ray.dir[0];
  tmp1[1] = intx[1] - ray.dir[1];
  tmp1[2] = intx[2] - ray.dir[2];
  tmp2[0] = intx[0] + ray.dir[0];
  tmp2[1] = intx[1] + ray.dir[1];
  tmp2[2] = intx[2] + ray.dir[2];
  vertex = 0;
  float sqrdist = (v[0][0]-intx[0])*(v[0][0]-intx[0])+(v[0][1]-intx[1])*(v[0][1]-intx[1])+(v[0][2]-intx[2])*(v[0][2]-intx[2]);
  bool first = true;
  for (i=0; i<nb; i++) {
    float mysqrdist = (v[i][0]-intx[0])*(v[i][0]-intx[0])+(v[i][1]-intx[1])*(v[i][1]-intx[1])+(v[i][2]-intx[2])*(v[i][2]-intx[2]);
    if( mysqrdist<sqrdist ) { vertex=i; sqrdist=mysqrdist; }
    int next = cw?((i+1)%nb):(i-1);
    if( next<0 ) next = nb-1;
    if (clockwise(tmp1, tmp2, v[i], v[next]) > 0.0) {
      if( first && triStrip ) {
        cw=!cw;
        i--;
      } else return(0);
    }
    first = false;
  }
  return(1);
}

/* lineIntersects:  AUTHOR: Mukesh Prasad
 *
 *   This function computes whether two line segments,
 *   respectively joining the input points (x1,Y1) -- (x2,y2)
 *   and the input points (x3,y3) -- (x4,y4) intersect.
 *   If the lines intersect, the output variables x, y are
 *   set to coordinates of the point of intersection.
 *
 *   All values are in integers.  The returned value is rounded
 *   to the nearest integer point.
 *
 *   If non-integral grid points are relevant, the function
 *   can easily be transformed by substituting floating point
 *   calculations instead of integer calculations.
 *
 *   Entry
 *        x1, Y1,  x2, y2   Coordinates of endpoints of one segment.
 *        x3, y3,  x4, y4   Coordinates of endpoints of other segment.
 *
 *   Exit
 *        x, y              Coordinates of intersection point.
 *
 *   The value returned by the function is one of:
 *
 *        DONT_INTERSECT    0
 *        DO_INTERSECT      1
 *        COLLINEAR         2
 *
 * Error conditions:
 *
 *     Depending upon the possible ranges, and particularly on 16-bit
 *     computers, care should be taken to protect from overflow.
 *
 *     In the following code, 'long' values have been used for this
 *     purpose, instead of 'int'.
 */

typedef enum
{
    DONT_INTERSECT,
    DO_INTERSECT,
    COLINEAR
} LineIntersectStatus;

/**************************************************************
 *                                                            *
 *    NOTE:  The following macro to determine if two numbers  *
 *    have the same sign, is for 2's complement number        *
 *    representation.  It will need to be modified for other  *
 *    number systems.                                         *
 *                                                            *
 **************************************************************/

#define SAME_SIGNS( a, b )	\
		(((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )

inline LineIntersectStatus lineIntersects2D(
                int x1, int Y1,
                int x2, int y2,     // defines the first line
                int x3, int y3,
                int x4, int y4,     // defines the second line
                int &x, int &y )
{
    long a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
    long r1, r2, r3, r4;         /* 'Sign' values */
    long denom, offset, num;     /* Intermediate values */

    /* Compute a1, b1, c1, where line joining points 1 and 2
     * is "a1 x  +  b1 y  +  c1  =  0".
     */

    a1 = y2 - Y1;
    b1 = x1 - x2;
    c1 = x2 * Y1 - x1 * y2;

    /* Compute r3 and r4.
     */

    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;

    /* Check signs of r3 and r4.  If both point 3 and point 4 lie on
     * same side of line 1, the line segments do not intersect.
     */

    if ( r3 != 0 &&
         r4 != 0 &&
         SAME_SIGNS( r3, r4 ))
        return ( DONT_INTERSECT );

    /* Compute a2, b2, c2 */

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    /* Compute r1 and r2 */

    r1 = a2 * x1 + b2 * Y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;

    /* Check signs of r1 and r2.  If both point 1 and point 2 lie
     * on same side of second line segment, the line segments do
     * not intersect.
     */

    if ( r1 != 0 &&
         r2 != 0 &&
         SAME_SIGNS( r1, r2 ))
        return ( DONT_INTERSECT );

    /* Line segments intersect: compute intersection point. 
     */

    denom = a1 * b2 - a2 * b1;
    if ( denom == 0 )
        return ( COLINEAR );
    offset = denom < 0 ? - denom / 2 : denom / 2;

    /* The denom/2 is to get rounding instead of truncating.  It
     * is added or subtracted to the numerator, depending upon the
     * sign of the numerator.
     */

    num = b1 * c2 - b2 * c1;
    x = (int)(( num < 0 ? num - offset : num + offset ) / denom);

    num = a2 * c1 - a1 * c2;
    y = (int)(( num < 0 ? num - offset : num + offset ) / denom);

    return ( DO_INTERSECT );
}

//! Intersect two lines (P1 to V1) with (P2 to V2) and return the closest points on each line (R1 and R2)
inline void lineIntersectLine(XbVec3f P1,XbVec3f V1, XbVec3f P2, XbVec3f V2, XbVec3f &R1, XbVec3f &R2,float &t,float &s) {
  XbVec3f P12,V1x2;
  XbMatrix M;
  float l;
  V1 = V1-P1;
  V1 = V1/V1.length();
  V2 = V2-P2;
  V2 = V2/V2.length();
  P12 = P2-P1;
  V1x2 = V1.cross(V2);
  l = V1x2.length();
  l = l*l;
  M[0][0] = P12[0];
  M[0][1] = P12[1];
  M[0][2] = P12[2];
  M[1][0] = V2[0];
  M[1][1] = V2[1];
  M[1][2] = V2[2];
  M[2][0] = V1x2[0];
  M[2][1] = V1x2[1];
  M[2][2] = V1x2[2];
  t = M.det3()/l;
  M[1][0] = V1[0];
  M[1][1] = V1[1];
  M[1][2] = V1[2];
  s = M.det3()/l;
  R1 = P1+V1*t;
  R2 = P2+V2*s;
}

// finds the number of intersections between a ray and a sphere, also calculates the intersection points if any
inline bool findSphereRayIntersection (const XbRay& rkRay, const XbVec3f center, const float radius, 
																				int& riQuantity, XbVec3f akPoint[2])
{
    // set up quadratic Q(t) = a*t^2 + 2*b*t + c
    XbVec3f kDiff = rkRay.orig - center;
    float fA = rkRay.dir.sqrlength();
    float fB = kDiff.dot(rkRay.dir);
    float fC = kDiff.sqrlength() - radius*radius;

    float afT[2];
    float fDiscr = fB*fB - fA*fC;
    if ( fDiscr < 0.0 )
    {
        riQuantity = 0;
        return false;
    }
    else if ( fDiscr > 0.0 )
    {
        float fRoot = sqrtf(fDiscr);
        float fInvA = 1.0f/fA;
        afT[0] = (-fB - fRoot)*fInvA;
        afT[1] = (-fB + fRoot)*fInvA;

        if ( afT[0] >= 0.0 )
        {
            riQuantity = 2;
            akPoint[0] = rkRay.orig + afT[0]*rkRay.dir;
            akPoint[1] = rkRay.orig + afT[1]*rkRay.dir;
            return true;
        }
        else if ( afT[1] >= 0.0 )
        {
            riQuantity = 1;
            akPoint[0] = rkRay.orig + afT[1]*rkRay.dir;
            return true;
        }
        else
        {
            riQuantity = 0;
            return false;
        }
    }
    else
    {
        afT[0] = -fB/fA;
        if ( afT[0] >= 0.0 )
        {
            riQuantity = 1;
            akPoint[0] = rkRay.orig + afT[0]*rkRay.dir;
            return true;
        }
        else
        {
            riQuantity = 0;
            return false;
        }
    }
}

//  This function finds a point on a 3D line given the two end points (or any points on
//  the line) and a k value normally between 0 and 1, which represents the place on the 
//  line. ie. 0.5 will give the point half way in between the two given points
inline void findPointOn3DLine(XbVec3f &P1, XbVec3f &P2, float k, XbVec3f &R)
{
  R = P1 + k * (P2 - P1);
}

//
//  This function finds the distance to the closest point on a plane from any arbitrary point
//
inline float ptToPlane(XbVec3f &pt,float eq[4])
{
  return pt[0]*eq[0] + pt[1]*eq[1] + pt[2]*eq[2] + eq[3];
}

#define NUMDIM	3
#define C_RIGHT	0
#define C_LEFT	1
#define C_MIDDLE	2

//
//	This function does a ray-box intersection. It returns true if the ray intersects the box and false if not.
//	(in)	minB	= the minimum of the box's bounding box
//	(in)	maxB	= the maximum of the box's bounding box
//	(in)	ray		= the ray to intersect with the box
//	(out) coord = the intersection point with the box if there is one
//	return			= true if intersection, false if none
//
inline bool rayCubeIntersect(const XbVec3f &minB,const XbVec3f &maxB,const XbRay &ray,XbVec3f &coord)
{
	bool inside = true;
	char quadrant[NUMDIM];
	register int i;
	int whichPlane;
	float maxT[NUMDIM];
	float candidatePlane[NUMDIM];

	/* Find candidate planes; this loop can be avoided if
   	rays cast all from the eye(assume perpsective view) */
	for (i=0; i<NUMDIM; i++)
		if(ray.orig[i] < minB[i]) {
			quadrant[i] = C_LEFT;
			candidatePlane[i] = minB[i];
			inside = false;
		}else if (ray.orig[i] > maxB[i]) {
			quadrant[i] = C_RIGHT;
			candidatePlane[i] = maxB[i];
			inside = false;
		}else	{
			quadrant[i] = C_MIDDLE;
		}

	/* Ray origin inside bounding box */
	if(inside)	{
		coord = ray.orig;
		return (true);
	}

	/* Calculate T distances to candidate planes */
	for (i = 0; i < NUMDIM; i++)
		if (quadrant[i] != C_MIDDLE && ray.dir[i] !=0.)
			maxT[i] = (candidatePlane[i]-ray.orig[i]) / ray.dir[i];
		else
			maxT[i] = -1.;

	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	for (i = 1; i < NUMDIM; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	/* Check final candidate actually inside box */
	if (maxT[whichPlane] < 0.) return (false);
	for (i = 0; i < NUMDIM; i++) {
		if (whichPlane != i) {
			coord[i] = ray.orig[i] + maxT[whichPlane] * ray.dir[i];
			if (coord[i] < minB[i] || coord[i] > maxB[i])
				return (false);
		} else {
			coord[i] = candidatePlane[i];
		}
	}
	return (true);				/* ray hits box */
}

#endif

