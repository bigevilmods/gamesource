// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3RayTrace.h
 *
 *  Contains utility functions for ray tracing.
 */

// Source is in EA3CoordUtil.cxx

#ifndef _EA3_RayTrace_h_
#define _EA3_RayTrace_h_
#include "EA3Util.h"
#include <eastl/vector.h>
#include "EA3CoordUtil.h"
namespace EA3Util {
  namespace RayTrace {

    const float LIMIT_TRIANGLE_P = .999999f;
    const float VERY_SMALL = 0.0000001f;
    const float MIN_DISTANCE = 0.01f;


    struct EA3UTIL_EXPORT Plane {
      XbVec3f norm;
      float d;
    };
    struct EA3UTIL_EXPORT Layer3 {
      XbVec3f normal[3],uv[3];
    };
    struct EA3UTIL_EXPORT Layer1 {
      XbVec3f normal,uv;
    };
    struct EA3UTIL_EXPORT Ray {
      XbVec3f origin;
      XbVec3f dir;
      bool shadow;
    };
    class TriangleCache;
    struct EA3UTIL_EXPORT HitData {
      float distance;
      TriangleCache *tri;
      XbVec3f intersect;
      eastl::vector<Layer1, StlAllocator> layer;
    };
    class EA3UTIL_EXPORT TriangleCache {
    public:
      TriangleCache(EA3RecordedGeometry *_rg,EA3Block *gp) {
        rg = _rg;
        geoPrim = gp;
      }
      int update();
      void rayCast(Ray *ray, HitData *hit);
      // Member data
      EA3Block *geoPrim;
      float transform[4];
      XbVec3f vert[3];
      Plane v1,v2,flat;
	  eastl::vector<Layer3, StlAllocator> layer;
      bool changingNormal;
      EA3RecordedGeometry *rg;
    };
    int EA3UTIL_EXPORT getTriangleCaches (EA3RecordedGeometry *rg,int frame,EA3Block *geoPrim,XbPArray<TriangleCache*> &tlist);
  } // RayTrace namespace
} // EA3Util Namespace

#endif 
