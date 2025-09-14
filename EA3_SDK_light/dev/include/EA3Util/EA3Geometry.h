// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Geometry_h
#define EA3Geometry_h

#include "EA3.h"
#include "EA3_Status.h"
#include "EA3GeomUtil.h"
using namespace EA3Util;
#ifdef EA_PLATFORM_WINDOWS
#   ifdef EA3_3D_EXPORTS
#       define EA3_3D_EXPORT __declspec(dllexport)
#   else
#       define EA3_3D_EXPORT __declspec(dllimport)
#   endif
#else
#   define EA3_3D_EXPORT
#endif

class EA3Geometry;

class EA3GeoPrim : public EA3Block {
protected:
  EA3GeoPrim(
    EA3Block *con,
    EA3Grammar *g,
    int ix,
    const XbIdentifier &t,
    const XbString &nm) : EA3Block(con,g,ix,t,nm) {}
  
public:
  
  EA3_Status setMaterial(EA3Block *mat)
  {
    return geoPrimSetMaterial( this, mat );
  }
  
  EA3_Status getMaterial(EA3Block *&mat) const
  {
    return geoPrimGetMaterial( this, mat );
  }
  
  EA3_Status getNumCoords(int &num) const
  {
    return geoPrimGetNumCoords( this, num );
  }
  
  EA3_Status getCoords(int num, Float3 *coords) const
  {
    return geoPrimGetCoords( this, num, coords );
  }
  
  EA3_Status getCoordIndices(int num,int *ix) const
  {
    return geoPrimGetCoordIndices( this, num, ix );
  }
  
  EA3_Status setCoords(int num, Float3 *coords)
  {
    return geoPrimSetCoords( this, num, coords );
  }
  
  EA3_Status setCoordIndices(int num, int *coords)
  {
    return geoPrimSetCoordIndices( this, num, coords );
  }
  
  EA3_Status getNumColours(int &num,int layer=0) const
  {
    return geoPrimGetNumColours( this, num, layer );
  }
  
  EA3_Status getColour(int ix, U32 &col,int layer=0) const
  {
    return geoPrimGetColour( this, ix, col, layer );
  }
  
  EA3_Status getColours(int num, U32 *colours,int layer=0) const
  {
    return geoPrimGetColours( this, num, colours, layer );
  }
  
  EA3_Status getColourIndices(int num, int *ix,int layer=0) const
  {
    return geoPrimGetColourIndices( this, num, ix, layer );
  }
  
  EA3_Status setColours(int num,U32 *val,int layer=0)
  {
    return geoPrimSetColours( this, num, val, layer );
  }
  
  EA3_Status setColourIndices(int num,int *val,int layer=0)
  {
    return geoPrimSetColourIndices( this, num, val, layer );
  }
  
  EA3_Status getNumUVs(int &num,int layer=0) const
  {
    return geoPrimGetNumUVs( this, num, layer );
  }
  
  EA3_Status getUV(int ix, float *&uv,int layer=0) const
  {
    return geoPrimGetUV( this, ix, uv, layer );
  }

  EA3_Status getUVs(int num, Float3 *uvs,int layer=0) const
  {
    return geoPrimGetUVs( this, num, uvs, layer );
  }

  EA3_Status getUVIndices(int num, int *ix,int layer=0) const
  {
    return geoPrimGetUVIndices( this, num, ix, layer );
  }
  
  EA3_Status setUVs(int num,Float3 *val,int layer=0)
  {
    return geoPrimSetUVs( this, num, val, layer );
  }
  
  EA3_Status setUVIndices(int num,int *val,int layer=0)
  {
    return geoPrimSetUVIndices( this, num, val, layer );
  }
  
  EA3_Status getNumNormals(int &num,int layer=0) const
  {
    return geoPrimGetNumNormals( this, num, layer );
  }
  
  EA3_Status getNormal(int ix,float *&normal,int layer=0) const
  {
    return geoPrimGetNormal( this, ix, normal, layer );
  }
  
  EA3_Status getNormals(int num, Float3 *normals,int layer=0) const
  {
    return geoPrimGetNormals( this, num, normals, layer );
  }

  EA3_Status getNormalIndices(int num, int *ix,int layer=0) const
  {
    return geoPrimGetNormalIndices( this, num, ix, layer );
  }
  
  EA3_Status setNormals(int num, Float3 *val,int layer=0)
  {
    return geoPrimSetNormals( this, num, val, layer );
  }
  
  EA3_Status setNormalIndices(int num,int *val,int layer=0)
  {
    return geoPrimSetNormalIndices( this, num, val, layer );
  }
  
  EA3_Status getVertexData(XbPArray<EA3VertexData *> &vs)
  {
    return geoPrimGetVertexData( this, vs );
  }

protected:
  
  EA3Geometry *getGeometry() const
  {
    EA3Block *geom = NULL;
    geoPrimGetGeometry( this,geom);
    return (EA3Geometry*) geom;
  }
  
private:

  EA3Block *getLayer(int ix,int create)
  {
    EA3Block *layer = NULL;
    geoPrimGetLayer( this, ix, create,layer );
    return layer;
  }

  EA3Block *getLayer(int ix) const
  {
    EA3Block *layer = NULL;
    geoPrimGetLayer( this, ix , layer);
    return layer;
  }
};

class  EA3GeoNurbsCurve : public EA3GeoPrim {
protected:
  EA3GeoNurbsCurve(
    EA3Block *con,
    EA3Grammar *g,
    int ix,
    const XbIdentifier &t,
    const XbString &nm) : EA3GeoPrim(con,g,ix,t,nm)
  {};
  
public:
  
  EA3_Status getNumWeights( int &num ) const
  {
    return curveGetNumWeights( this, num );
  }
  
  EA3_Status getWeights( int num, float *weights ) const
  {
    return curveGetWeights( this, num, weights );
  }
  
  EA3_Status setWeights( int num, float *weights )
  {
    return curveSetWeights( this, num, weights );
  }
  
  EA3_Status getNumKnots( int &num ) const
  {
    return curveGetNumKnots( this, num );
  }
  
  EA3_Status getKnots( int num, float *knots ) const
  {
    return curveGetKnots( this, num, knots );
  }
  
  EA3_Status setKnots( int num, float *knots )
  {
    return curveSetKnots( this, num, knots );
  }
  
  EA3_Status getDegree( int &deg ) const
  {
    return curveGetDegree( this, deg );
  }
  
  EA3_Status setDegree( int deg )
  {
    return curveSetDegree( this, deg );
  }
};


class  EA3GeoNurbsSurface : public EA3GeoPrim {
protected:
  EA3GeoNurbsSurface(
    EA3Block *con,
    EA3Grammar *g,
    int ix,
    const XbIdentifier &t,
    const XbString &nm) : EA3GeoPrim(con,g,ix,t,nm)
  {};
  
public:
  
  EA3_Status getNumWeights( int &num ) const
  {
    return surfaceGetNumWeights( this, num );
  }
  
  EA3_Status getWeights( int num, float *weights ) const
  {
    return surfaceGetWeights( this, num, weights );
  }
  
  //! Set the weights
  EA3_Status setWeights( int num, float *weights )
  {
    return surfaceSetWeights( this, num, weights );
  }
  
  
  EA3_Status getNumUKnots( int &num ) const
  {
    return surfaceGetNumUKnots( this, num );
  }
  
  EA3_Status getNumVKnots( int &num ) const
  {
    return surfaceGetNumVKnots( this, num );
  }
  
  EA3_Status getUKnots( int num, float *knots ) const
  {
    return surfaceGetUKnots( this, num, knots );
  }
  
  EA3_Status getVKnots( int num, float *knots ) const
  {
    return surfaceGetVKnots( this, num, knots );
  }
  
  EA3_Status setUKnots( int num, float *knots )
  {
    return surfaceSetUKnots( this, num, knots );
  }
  
  EA3_Status setVKnots( int num, float *knots )
  {
    return surfaceSetVKnots( this, num, knots );
  }
  

  EA3_Status getUDegree( int &deg ) const
  {
    return surfaceGetUDegree( this, deg );
  }
  
  EA3_Status getVDegree( int &deg ) const
  {
    return surfaceGetVDegree( this, deg );
  }
  
  EA3_Status setUDegree( int deg )
  {
    return surfaceSetUDegree( this, deg );
  }
  
  EA3_Status setVDegree( int deg )
  {
    return surfaceSetVDegree( this, deg );
  }
  
  
  EA3_Status getNumUPoints( int &num ) const
  {
    return surfaceGetNumUPoints( this, num );
  }
  
  EA3_Status getNumVPoints( int &num ) const
  {
    return surfaceGetNumVPoints( this, num );
  }
  
  EA3_Status setNumUPoints( int num )
  {
    return surfaceSetNumUPoints( this, num );
  }
  
  EA3_Status setNumVPoints( int num )
  {
    return surfaceSetNumVPoints( this, num );
  }
  
  
  EA3_Status getUStride( int &num ) const
  {
    return surfaceGetUStride( this, num );
  }
  
  EA3_Status getVStride( int &num ) const
  {
    return surfaceGetVStride( this, num );
  }
  
  EA3_Status setUStride( int num )
  {
    return surfaceSetUStride( this, num );
  }
  
  EA3_Status setVStride( int num )
  {
    return surfaceSetVStride( this, num );
  }
  
};


class  EA3LOD : public EA3Block {
protected:
  EA3LOD(
    EA3Block *con,
    EA3Grammar *g,
    int ix,
    const XbIdentifier &t,
    const XbString &nm) : EA3Block(con,g,ix,t,nm)
  {};
public:

  int numGeoPrim()
  {
    int num;
    return (lodGetNumGeoPrim( this ,num),num);
  }
  
  EA3GeoPrim *getGeoPrim(int ix)
  {
    EA3Block *prim = NULL;
    return (EA3GeoPrim*)(lodGetGeoPrim( this, ix,prim ),prim);
  }
  
  EA3GeoPrim *addGeoPrim(const XbIdentifier &tp)
  {
    EA3Block *prim = NULL;
    return (EA3GeoPrim*)(lodAddGeoPrim( this, tp,prim ),prim);
  }
};


class  EA3Geometry : public EA3Block {
protected:
  EA3Geometry(
    EA3Block *con,
    EA3Grammar *g,
    int ix,
    const XbIdentifier &t,
    const XbString &nm) : EA3Block(con,g,ix,t,nm)
  {};
  
public:

  EA3_Status getNumCoords(int &num)
  {
    return geomGetNumCoords( this, num );
  }
  
  EA3_Status getCoords(int num,Float3 *vals)
  {
    return geomGetCoords( this, num, vals );
  }
  
  EA3_Status setCoords(int num,Float3 *vals)
  {
    return geomSetCoords( this, num, vals );
  }
  
  EA3_Status addCoords( int num, Float3 *vals )
  {
    return geomAddCoords( this, num, vals );
  }
  

  EA3_Status getNumColours(int &num)
  {
    return geomGetNumColours( this, num );
  }
  
  EA3_Status getColours(int num,U32 *vals)
  {
    return geomGetColours( this, num, vals );
  }
  
  EA3_Status setColours(int num,U32 *vals)
  {
    return geomSetColours( this, num, vals );
  }
  

  EA3_Status getNumUVs(int &num)
  {
    return geomGetNumUVs( this, num );
  }
  
  EA3_Status getUVs(int num,Float3 *vals)
  {
    return geomGetUVs( this, num, vals );
  }
  
  EA3_Status setUVs(int num,Float3 *vals)
  {
    return geomSetUVs( this, num, vals );
  }
  
  EA3_Status addUVs( int num, Float3 *vals )
  {
    return geomAddUVs( this, num, vals );
  }
  

  EA3_Status getNumNormals(int &num)
  {
    return geomGetNumNormals( this, num );
  }
  
  EA3_Status getNormals(int num,Float3 *vals)
  {
    return geomGetNormals( this, num, vals );
  }
  
  EA3_Status setNormals(int num,Float3 *vals)
  {
    return geomSetNormals( this, num, vals );
  }

  EA3_Status addNormals( int num, Float3 *vals )
  {
    return geomAddNormals( this, num, vals );
  }
  

  EA3_Status getNumLODs(int &num)
  {
    return geomGetNumLODs( this, num );
  }
  

  EA3_Status getNumGeoPrims(int lod,int &num)
  {
    return geomGetNumGeoPrims( this, lod, num );
  }
  
  EA3_Status addGeoPrim(int lod,const XbIdentifier &type, EA3GeoPrim *&prim)
  {
    return geomAddGeoPrim( this, lod, type, (EA3Block *&)prim );
  }

  EA3_Status getGeoPrim(int lod,int ix, EA3GeoPrim *&prim)
  {
    return geomGetGeoPrim( this, lod, ix, (EA3Block *&)prim );
  }
  

  EA3_Status getVertexData(int lod,XbPArray<EA3VertexData *> &vs)
  {
    return geomGetVertexData( this, lod, vs );
  }
  
  EA3LOD *getLOD(int ix)		{ EA3Block *lod;return (EA3LOD*)(geomGetLOD( this, ix,lod ),lod); }

  EA3LOD *addLOD()			{ EA3Block *lod;return (EA3LOD*)(geomAddLOD( this, lod),lod); }
  
  private:
}; 

#endif // EA3Geometry_h
