// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3GeomUtil.h
 *
 *  Contains utility function declarations for operations on geometry blocks.
 */

#ifndef _EA3_Geom_Util_h_
#define _EA3_Geom_Util_h_
#include "EA3Util.h"
namespace EA3Util {

//! A list of all the fields that relate to a given vertex.
/*!
 *  
 * A EA3VertexData contains a list of all other fields that are related to
 * a given vertex.  There is no EA3 block that contains this information,
 * it is implied by the parallel reference lists in a Geo prim.  The class
 * does not inherit from any other EA3 base class.  It should be used by
 * EA3Geometry::getVertexData
 *
 * A VertexData has a pointer to a coordinate field, and a list of layers.
 * Each layer has an array of pointers to data that is related to this
 * vertex for the given layer.  It also has a list of pointers for adding
 * other data that is related to a vertex but not via layers (for example 
 * skin weights, material, geo prim it belongs to, etc.)
 *
 * \sa EA3Geometry EA3GeoPrim
 */

class EA3UTIL_EXPORT EA3VertexData : public EA3Mem {
public:
  EA3Field *coord;
  XbPArray<XbPArray<EA3Base*> *> layerData;
  XbPArray<EA3Base*> data;
  ~EA3VertexData();
};



EA3UTIL_EXPORT EA3_Status geomGetMorphChannels(const EA3Block *anim, EA3Block *geom, XbPArray<EA3Block*> &chanlist);
EA3UTIL_EXPORT EA3_Status geomGetMorphCoords(const EA3Block *geom, XbPArray<EA3Block*> &chanlist,XbVec3f *coords);
//! Set material block for the geomtry primitive.
/*!
 *  
 * Set material block for the geomtry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetMaterial( EA3Block *geoPrim, EA3Block *mat );
//! Get the material block from the geometry primitive.
/*!
 *  
 * Get the material block from the geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the material block doesn't exist.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetMaterial( const EA3Block *geoPrim, EA3Block *&mat );
//! Set the number of coords in the geometry primitive.
/*!
 *  
 * Set the number of coords in the geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the primitive does not have a coord array.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNumCoords( const EA3Block *geoPrim, int &num );
//! Get the coordinates of the geometry primitive.
/*!
 *  
 * Get the coordinates of the geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the primitive does not have a coord array.
 * \retval EA3_BAD_NUM if there are not enough coord in the primitive.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetCoords( const EA3Block *geoPrim, int num, Float3 *coords );
//! Get the coordinate indices of the geometry primitive.
/*!
 *  
 * Get the coordinate indices of the geometry primitive.
 * The caller is responsible for allocating memory for the index array
 * (num_coords * sizeof(int)).
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_BAD_NUM if there aren't enough indices in the primitive.
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetCoordIndices( const EA3Block *geoPrim, int num,int *ix );
//! Append an array of coordinates to a geometry primitive.
/*!
 *  
 * Append an array of coordinates to a geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * Creates a coord array if one does not exist.
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetCoords( EA3Block *geoPrim, int num,const Float3 *coords );
//! Set coordinate indices for a primitive.
/*!
 *  
 * Creates references to the array of coords in the geometry and sets
 * up the references to these values based on the indices.  
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of coordinate indices to set
 * \param coords (in) coordinate index array
 * \param baseIndex (in) offset by this index when referencing the coordinate array
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetCoordIndices( EA3Block *geoPrim, int num,const int *coords, int baseIndex=0 );

//! Get the number of colours in the geometry primitive.
/*!
 *  
 * Get the number of colours in the geometry primitive
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (out) # colours
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNumColours( const EA3Block *geoPrim, int &num, int layer=0);
//! Get colour attribute from a geometry primitive layer.
/*!
 *  
 * Get colour attribute from a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param index (in) which attribute
 * \param col (out) colour
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer is not found, attribute array in that layer
 * is not found, or the index is out of range.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetColour( const EA3Block *geoPrim, int index, U32 &col,int layer=0);
//! Get an array of colours from a geometry primitive layer.
/*!
 *  
 * Get an array of colours from a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of colours to get
 * \param colours (out) colour array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetColours( const EA3Block *geoPrim, int num, U32 *colours, int layer=0 );
//! Get the colour indices of a geometry primitive layer.
/*!
 *  
 * Get the colour indices of a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of colour indices to get
 * \param ix (out) colour index array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetColourIndices( const EA3Block *geoPrim, int num, int *ix, int layer=0);
//! Append an array of colours to a geometry primitive layer.
/*!
 *  
 * Append an array of colours to a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of colour to set
 * \param val (out) colour array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetColours( EA3Block *geoPrim, int num,const U32 *val, int layer=0);
//! Set colour indices to a layer of a geometry primitive.
/*!
 *  
 * Set colour indices to a layer of a geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of colour indices to set
 * \param val (in) colour index array
 * \param layer (in) which layer
 * \param baseIndex (in) offset by this index when referencing the colour array
 *
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_BAD_NUM if an index is out of range, but any indices that
 * were in range before it will be set.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetColourIndices( EA3Block *geoPrim, int num,const int *val, int layer=0, int baseIndex=0);
//! Get the number of UVs in the geometry primitive.
/*!
 *  
 * Get the number of UVs in the geometry primitive
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (out) # uvs
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNumUVs( const EA3Block *geoPrim, int &num, int layer=0 );
//! Get uv from a geometry primitive layer.
/*!
 *  
 * Get uv from a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param index (in) which uv
 * \param uv (out) uv
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer is not found, attribute array in that layer
 * is not found, or the index is out of range.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetUV( const EA3Block *geoPrim, int index, float *&uv, int layer=0 );
//! Get an array of uvs from a geometry primitive layer.
/*!
 *  
 * Get an array of uvs from a geometry primitive layer.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of uvs to get
 * \param uvs (out) uv array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetUVs( const EA3Block *geoPrim, int num, Float3 *uvs, int layer=0 );
//! Get the uv indices of a geometry primitive layer.
/*!
 *  
 * Get the uv indices of a geometry primitive layer.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of uv indices to get
 * \param ix (out) uv index array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetUVIndices( const EA3Block *geoPrim, int num, int *ix, int layer=0 );
//! Append an array of uvs to a geometry primitive layer.
/*!
 *  
 * Append an array of uvs to a geometry primitive layer.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of uv to set
 * \param val (in) uv array
 * \param layer (in) which layer
 *
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetUVs( EA3Block *geoPrim, int num,const Float3 *val, int layer=0 );
//! Set uv indices to a layer of a geometry primitive.
/*!
 *  
 * Set uv indices to a layer of a geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of uv indices to set
 * \param val (in) uv index array
 * \param layer (in) which layer
 * \param baseIndex (in) offset by this index when referencing the UV array
 *
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_BAD_NUM if an index is out of range, but any indices that
 * were in range before it will be set.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetUVIndices( EA3Block *geoPrim, int num,const int *val, int layer=0, int baseIndex=0 );
//! Get the number of normals in the geometry primitive.
/*!
 *  
 * Get the number of normals in the geometry primitive
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (out) # normals
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNumNormals( const EA3Block *geoPrim, int &num, int layer=0 );
//! Get normal from a geometry primitive layer.
/*!
 *  
 * Get normal from a geometry primitive layer.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param index (in) which normal
 * \param nrm (out) normal
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer is not found, attribute array in that layer
 * is not found, or the index is out of range.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNormal( const EA3Block *geoPrim, int index, float *&nrm, int layer=0 );
//! Get an array of normals from a geometry primitive layer.
/*!
 *  
 * Get an array of normals from a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of normals to get
 * \param normals (out) normal array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNormals( const EA3Block *geoPrim, int num, Float3 *normals, int layer=0 );
//! Get the normal indices of a geometry primitive layer.
/*!
 *  
 * Get the normal indices of a geometry primitive layer
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of normal indices to get
 * \param ix (out) normal index array
 * \param layer (in) which layer
 * \retval EA3_NOT_FOUND if layer or attribute array in that layer is not found
 * \retval EA3_BAD_NUM if num is greater than actual number of values
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetNormalIndices( const EA3Block *geoPrim, int num, int *ix, int layer=0 );
//! Append an array of normals to a geometry primitive layer.
/*!
 *  
 * Append an array of normals to a geometry primitive layer.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of normals to set
 * \param val (in) normal array
 * \param layer (in) which layer
 *
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetNormals( EA3Block *geoPrim, int num,const Float3 *val, int layer=0 );
//! Set normal indices to a layer of a geometry primitive.
/*!
 *  
 * Set normal indices to a layer of a geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) geometry primitive
 * \param num (in) # of normal indices to set
 * \param val (in) normal index array
 * \param layer (in) which layer
 * \param baseIndex (in) offset by this index when referencing the normal array
 *
 * \retval EA3_NOT_FOUND if layer is not found
 * \retval EA3_BAD_NUM if an index is out of range, but any indices that
 * were in range before it will be set.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geoPrimSetNormalIndices( EA3Block *geoPrim, int num,const int *val, int layer=0, int baseIndex=0 );
//! Get an array of EA3VertexData's.
/*!
 *  
 * Get an array of EA3VertexData's.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * The EA3VertexData's in this primitive are appended to <I>vs</I>.<br>
 * One EA3VertexData is appended for each coordinate.<br>
 * The vertex will have the same number of layers as the EA3Block.<br>
 * For each layer, all fields related to the vertex
 * will be added to the EA3VertexData's coresponding layerData.<br>
 * For example, assume a primitive with 3 coordinates and two layers.
 * 3 EA3VertexData will be appended to <I>vs</I>.<br>
 * Other data like the geoPrim and the material will be added
 * to the EA3VertexData's data list.
 *
 * \param geoPrim (in) geometry primitve
 * \param vs (mod) array of EA3VertexData's
 * 
 * \retval EA3_BAD_NUM if an index is out of range, but any indices that
 * were in range before it will be set.
 */
EA3UTIL_EXPORT EA3_Status geoPrimGetVertexData(EA3Block *geoPrim, XbPArray<EA3VertexData *> &vs );
//! Compute the normal of the polygonal plane.
/*!
 *  
 * Compute the normal of the polygonal plane (assuming it's a polygon).
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) source polygon geometry primitive
 * \param normal (out) normal
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geoPrimComputeNormal( const EA3Block *geoPrim, XbVec3f &normal );
//! Perform a ray intersection test on the surface.
/*!
 *  
 * Perform a ray intersection test on the surface (assuming it's a polygon).
 * \par Reference: Graphics Gems I: An Efficient Ray-Polygon Intersection
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geoPrim (in) source polygon geometry primitive
 * \param rayOrig (in) ray origin
 * \param rayDir (in) ray direction
 * \param intersect (out) boolean: intersection or not
 * \param t (out) distance from the ray origin to intersection (if intersect==true)
 * \param intersectPoint (out) intersect point (if intersect==true)
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geoPrimRayIntersect( const EA3Block *geoPrim, const XbVec3f &rayOrig, const XbVec3f &rayDir, bool &intersect, float &t, XbVec3f &intersectPoint );

EA3UTIL_EXPORT EA3_Status geoPrimGetGeometry( const EA3Block *geoPrim, EA3Block *&geom );
EA3UTIL_EXPORT EA3_Status geoPrimGetLayer( EA3Block *geoPrim, int index, int create, EA3Block *&layer );
EA3UTIL_EXPORT EA3_Status geoPrimGetLayer( const EA3Block *geoPrim, int index, EA3Block *&layer);

//! Get the # weights field of the curve primitive.
/*!
 *  
 * Get the # weights field of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the # weights field doesn't exist.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status curveGetNumWeights( const EA3Block *curve, int &num );
//! Get the weights of the curve primitive.
/*!
 *  
 * Get the weights of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * The caller is responsible for allocating memory for this array:
 * (num_weights * sizeof(float)).
 * \param curve (in) curve primitive
 * \param num (in) size of weight array
 * \param weights (in) weight array
 *
 * \retval EA3_NOT_FOUND if the weight array doesn't exist.
 * \retval EA3_BAD_NUM if there aren't enough weight values in the primitive
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status curveGetWeights( const EA3Block *curve, int num, float *weights );
//! Set the weights for the curve primitive.
/*!
 *  
 * Set the weights for the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * Create the weights array if one does not exist.
 *
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status curveSetWeights( EA3Block *curve, int num,const float *weights );
//! Get the # knots field of the curve primitive.
/*!
 *  
 * Get the # knots field of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the # knots field doesn't exist.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status curveGetNumKnots( const EA3Block *curve, int &num );
//! Get the knots of the curve primitive.
/*!
 *  
 * Get the knots of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * The caller is responsible for allocating memory for this array:
 * (num_weights * sizeof(float)).
 * \param curve (in) curve primitive
 * \param num (in) size of weight array
 * \param knots (in) weight array
 *
 * \retval EA3_NOT_FOUND if the weight array doesn't exist.
 * \retval EA3_BAD_NUM if there aren't enough knots values in the primitive
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status curveGetKnots( const EA3Block *curve, int num, float *knots );
//! Set the knots for the curve primitive.
/*!
 *  
 * Set the knots for the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * Create the knots array if one does not exist.
 *
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status curveSetKnots( EA3Block *curve, int num,const float *knots );
//! Get the degree of the curve primitive.
/*!
 *  
 * Get the degree of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \retval EA3_NOT_FOUND if the degree field doesn't exist.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status curveGetDegree( const EA3Block *curve, int & degree);
//! Set the degree of the curve primitive.
/*!
 *  
 * Set the degree of the curve primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * Creates degree field if one does not exist.
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status curveSetDegree( EA3Block *curve, int degree);

//! Get the # weights of the surface geometry primitive.
/*!
 *  
 * Get the # weights of the surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (out) # weights
 *
 * \retval EA3_NOT_FOUND if the # weight field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetNumWeights( const EA3Block *surf, int &num );
//! Get the weights of the surface geometry primitive.
/*!
 *  
 * Get the weights of the surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * The caller is responsible for allocating memory for this array:
 * (num * sizeof(float)).
 * \param surf (in) surface geometry primitive
 * \param num (in) size of weight array
 * \param weights (out) weight array
 *
 * \retval EA3_NOT_FOUND if the weight array is not found.
 * \retval EA3_BAD_NUM if there aren't enough weight values in the primitive.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetWeights( const EA3Block *surf, int num, float *weights );
//! Set the weights of a surface geometry primitive.
/*!
 *  
 * Set the weights of a surface geometry primitive.
 * Create the weight array if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) is the number of weights to set
 * \param weights (in) is the weight array to get values from
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetWeights( EA3Block *surf, int num,const float *weights );
//! Get the # weights of the surface geometry primitive.
/*!
 *  
 * Get the # weights of the surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (out) # U knots
 *
 * \retval EA3_NOT_FOUND if the # V kntos field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetNumUKnots( const EA3Block *surf, int &num );
//! Get the # V knots of the surface geometry primitive.
/*!
 *  
 * Get the # V knots of the surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (out) # V knots
 *
 * \retval EA3_NOT_FOUND if the # V knots field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetNumVKnots( const EA3Block *surf, int &num );
//! Get the U knot values of the surface geometry primitive.
/*!
 *  
 * Get the U knot values of the surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * The caller is responsible for allocating memory for this array:
 * (num * sizeof(float)).
 * \param surf (in) surface geometry primitive
 * \param num (in) size of weight array
 * \param knots (out) knot array
 *
 * \retval EA3_NOT_FOUND if the knot array is not found.
 * \retval EA3_BAD_NUM if there aren't enough knot values in the primitive.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetUKnots( const EA3Block *surf, int num, float *knots );
//! Get the V knot values of the surface geometry primitive.
/*!
 *  
 * Get the V knot values of the surface geometry primitive.
 * The caller is responsible for allocating memory for this array:
 * (num * sizeof(float)).
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) size of weight array
 * \param knots (out) knot array
 *
 * \retval EA3_NOT_FOUND if the knot array is not found.
 * \retval EA3_BAD_NUM if there aren't enough knot values in the primitive.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetVKnots( const EA3Block *surf, int num, float *knots );
//! Set the U knot vector of a surface geometry primitive.
/*!
 *  
 * Set the U knot vector of a surface geometry primitive.
 * Create the knots array if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) is the number of weights to set
 * \param knots (in) is the knot array to get values from
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetUKnots( EA3Block *surf, int num,const float *knots );
//! Set the V knot vector of a surface geometry primitive.
/*!
 *  
 * Set the V knot vector of a surface geometry primitive.
 * Create the weight array if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) is the number of weights to set
 * \param knots (in) is the knot array to get values from
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetVKnots( EA3Block *surf, int num,const float *knots );
//! Get U degree of a surface geometry primitive.
/*!
 *  
 * Get U degree of a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param deg (out) will be the U degree
 * \retval EA3_NOT_FOUND if the U degree field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetUDegree( const EA3Block *surf, int &deg );
//! Get V degree of a surface geometry primitive.
/*!
 *  
 * Get V degree of a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param deg (out) will be the U degree
 * \retval EA3_NOT_FOUND if the V degree field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetVDegree( const EA3Block *surf, int &deg );
//! Set U degree of a surface geometry primitive.
/*!
 *  
 * Set U degree of a surface geometry primitive.
 * Create a # U degree field if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param deg (in) is the U degree
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetUDegree( EA3Block *surf, int deg );
//! Set V degree of a surface geometry primitive.
/*!
 *  
 * Set V degree of a surface geometry primitive.
 * Create a V degree field if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param deg (in) is the V degree
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetVDegree( EA3Block *surf, int deg );
//! Get the # of U points from a surface geometry primitive.
/*!
 *  
 * Get the # of U points from a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (out) will be the number of U points
 * \retval EA3_NOT_FOUND if the # U points field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetNumUPoints( const EA3Block *surf, int &num );
//! Get the # of V points from a surface geometry primitive.
/*!
 *  
 * Get the # of V points from a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (out) will be the number of V points
 * \retval EA3_NOT_FOUND if the # V points field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetNumVPoints( const EA3Block *surf, int &num );
//! Set the # of U points of a surface geometry primitive.
/*!
 *  
 * Set the # of U points of a surface geometry primitive.
 * Create a # U point field if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) is the new number of U points
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetNumUPoints( EA3Block *surf, int num );
//! Set the # of V points of a surface geometry primitive.
/*!
 *  
 * Set the # of V points of a surface geometry primitive.
 * Create a # V point field if one doesn't exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param num (in) is the new number of V points
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status surfaceSetNumVPoints( EA3Block *surf, int num );
//! Get the U stride value of a surface geometry primitive.
/*!
 *  
 * Get the U stride value of a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param stride (out) will be the U stride
 * \retval EA3_NOT_FOUND if the U stride field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetUStride( const EA3Block *surf, int &stride );
//! Get the V stride value of a surface geometry primitive.
/*!
 *  
 * Get the V stride value of a surface geometry primitive.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param stride (out) will be the V stride
 * \retval EA3_NOT_FOUND if the V stride field is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status surfaceGetVStride( const EA3Block *surf, int &stride );
//! Set the U stride value of a surface geometry primitive.
/*!
 *  
 * Set the U stride value of a surface geometry primitive.
 * Creates an U stride field if one does not exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param stride (in) is the new U stride
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT EA3_Status surfaceSetUStride( EA3Block *surf, int stride );
//! Set the V stride value of a surface geometry primitive.
/*!
 *  
 * Set the V stride value of a surface geometry primitive.
 * Creates an V stride field if one does not exist.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param surf (in) surface geometry primitive
 * \param stride (in) is the new U stride
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT EA3_Status surfaceSetVStride( EA3Block *surf, int stride );
EA3UTIL_EXPORT EA3_Status lodGetNumGeoPrim( EA3Block *lof, int &num );
EA3UTIL_EXPORT EA3_Status lodGetGeoPrim(const EA3Block *lod, int index , EA3Block *&prim);
EA3UTIL_EXPORT EA3_Status lodAddGeoPrim( EA3Block *lod, const XbIdentifier &tp , EA3Block *&prim);
//! Get the # coords in the geometry.
/*!
 *  
 * Get the # coords in the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (out) # coords
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geomGetNumCoords(const EA3Block *geom, int &num );
//! Get coordinates of the geometry.
/*!
 *  
 * Get coordinates of the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (in) size of vals array
 * \param vals (out) array to store the coordinates (caller must allocate).
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if there is no coord array in geometry
 * \retval EA3_BAD_NUM if # coordinates in the geometry is less than <I>num</I>
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetCoords(const EA3Block *geom, int num, Float3 *vals );
//! Set geometry coordinates.
/*!
 *  
 * Set geometry coordinates.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the coordinates array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # coordinates
 * \param vals (in) coordinates array
 * \param add (in) add to end
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomSetCoords( EA3Block *geom, int num,const Float3 *vals, bool add=false );
//! Append coordinates to the geometry.
/*!
 *  
 * Append coordinates to the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the coordinates array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # coordinates
 * \param vals (in) coordinates
 * 
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geomAddCoords( EA3Block *geom, int num,const Float3 *vals );
//! Get the # colours in the geometry.
/*!
 *  
 * Get the # colours in the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (out) # coords
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geomGetNumColours(const EA3Block *geom, int &num );
//! Get colours of the geometry.
/*!
 *  
 * Get colours of the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (in) size of vals array
 * \param vals (out) array to store the colours (caller must allocate).
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if there is no colour array in geometry
 * \retval EA3_BAD_NUM if # colours in the geometry is less than <I>num</I>
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetColours(const EA3Block *geom, int num, U32 *vals );
//! Set geometry colours.
/*!
 *  
 * Set geometry colours.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the colours array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # colours
 * \param vals (in) colours array
 * \param add (in) add to end 
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomSetColours( EA3Block *geom, int num,const U32 *vals, bool add=false );
//! Get the # uvs in the geometry.
/*!
 *  
 * Get the # uvs in the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (out) # uvs
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geomGetNumUVs(const EA3Block *geom, int &num );
//! Get uvs of the geometry.
/*!
 *  
 * Get uvs of the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (in) size of vals array
 * \param vals (out) array to store the uvs (caller must allocate).
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if there is no uv array in geometry
 * \retval EA3_BAD_NUM if # uvs in the geometry is less than <I>num</I>
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetUVs(const EA3Block *geom, int num, Float3 *vals );
//! Set geometry uvs.
/*!
 *  
 * Set geometry uvs.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the uvs array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # uvs
 * \param vals (in) uvs array
 * \param add (in) add to end of array
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomSetUVs( EA3Block *geom, int num,const Float3 *vals, bool add=false );
//! Append uvs to the geometry.
/*!
 *  
 * Append uvs to the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the uvs array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # uvs
 * \param vals (in) uvs
 * 
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geomAddUVs( EA3Block *geom, int num, Float3 *vals );
//! Get the # normals in the geometry.
/*!
 *  
 * Get the # normals in the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (out) # normals
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geomGetNumNormals(const EA3Block *geom, int &num);
//! Get normals of the geometry.
/*!
 *  
 * Get normals of the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (in) size of vals array
 * \param vals (out) array to store the normals (caller must allocate).
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if there is no normal array in geometry
 * \retval EA3_BAD_NUM if # normals in the geometry is less than <I>num</I>
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetNormals(const EA3Block *geom, int num, Float3 *vals );
//! Set geometry normals.
/*!
 *  
 * Set geometry normals.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the normals array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # normals
 * \param vals (in) normals array
 * \param add (in) add to end
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomSetNormals( EA3Block *geom, int num,const Float3 *vals, bool add=false );
//! Append normals to the geometry.
/*!
 *  
 * Append normals to the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the normals array, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # normals
 * \param vals (in) normals
 * 
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geomAddNormals( EA3Block *geom, int num, Float3 *vals );
//! Get the # lods in the geometry.
/*!
 *  
 * Get the # lods in the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (out) # lods
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status geomGetNumLODs(const EA3Block *geom, int &num );
//! Get the # primitives in the lod.
/*!
 *  
 * Get the # primitives in the lod.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param lod (in) which lod
 * \param num (out) # primitives
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry.
 * \retval EA3_NOT_FOUND if the lod is not found.
 * \retval EA3_OK.
 */
EA3UTIL_EXPORT EA3_Status geomGetNumGeoPrims(const EA3Block *geom, int lod, int &num );
//! Add a primitive to the geometry.
/*!
 *  
 * Add a primitive to the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param lod (in) which lod to create the new primitive
 * \param type (in) new primitive type
 * \param geoPrim (out) new primitive created
 *
 * LOD's will be created until index (lod) is valid.
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry.
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomAddGeoPrim( EA3Block *geom, int lod, const XbIdentifier &type, EA3Block *&geoPrim);
//! Get a primitive from geometry.
/*!
 *  
 * Get a primitive from geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param lod (in) which lod
 * \param index (in) which primitive in the lod
 * \param geoPrim (out) primitive
 *
 * Returns the geometry primitive in the given lod at the given index.
 * 
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if the lod or primitive is not found
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetGeoPrim(const EA3Block *geom, int lod, int index, EA3Block *&geoPrim );
EA3UTIL_EXPORT EA3_Status geomGetVertexData( EA3Block *geom, int lod,XbPArray<EA3VertexData *> &vd );
//! Gets the skeletal node for the given geometry
/*!
 *  
 * Gets the skeletal node for the given geometry
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry block
 * \param skel (out) will be the skeletal node
 * \param lookin (in) is an optional block to start looking for the skeleton in (speeds it up)
 */
EA3UTIL_EXPORT EA3_Status geomGetSkeleton(EA3Block *geom, EA3Block *&skel, EA3Block *lookin=NULL);
//! Applies a transformation to a given geometry
/*!
 *  
 * Applies a transformation to a given geometry
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 */
EA3UTIL_EXPORT EA3_Status geomApplyTransform(EA3Block *geom, const XbMatrix& matrix);
//! Merge geometry in geom into geomMaster
/*!
 *  
 * Merge geometry in geom into geomMaster
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 */
EA3UTIL_EXPORT EA3_Status geomMerge(EA3Block *geomMaster, EA3Block *geom);
//! Find the surface point on a geometry by projecting another point onto it.
/*!
 *  
 * Find the surface point on the source geometry where the target vertex is
 * projected on by its normal.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param srcGeom (in) source polygon geometry primitive
 * \param tgtPoint (in) target vertex coordinate
 * \param tgtNormal (in) target vertex normal
 * \param index (out) which polygon facet it intersects
 * \param intersectPoint (out) intersect point
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status geomGetPolygonByProjection( const EA3Block *srcGeom, XbVec3f tgtPoint, XbVec3f tgtNormal, int &index, XbVec3f &intersectPoint );
//! Find the closest surface point on a geometry.
/*!
 *  
 * Find the surface point on the source geometry where the target vertex is
 * closest to.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param srcGeom (in) source polygon geometry primitive
 * \param tgtPoint (in) target vertex coordinate
 * \param polyIndex (out) which polygon facet is the closest
 * \param intersectPoint (out) intersect point
 * \retval EA3_NOT_FOUND if there is no closest vertex or closest line.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status geomGetClosestPolygon( const EA3Block *srcGeom, const XbVec3f &tgtPoint, int& polyIndex, XbVec3f &intersectPoint );
//! Get collapse map of the geometry.
/*!
 *  
 * Get collapse map of the geometry.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * \param geom (in) geometry
 * \param num (in) size of vals array
 * \param vals (out) array to store the collapse map (caller must allocate).
 * 
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_NOT_FOUND if there is no collapse map in geometry
 * \retval EA3_BAD_NUM if # collapse map entries in the geometry is less than <I>num</I>
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomGetCollapseMap( const EA3Block *geom, int num, int *vals );
//! Set geometry collapse map.
/*!
 *  
 * Set geometry collapse map.
 * \par Synopsis
 * \#include "EA3GeomUtil.h"
 * If the geometry does not have the collapse map, one will be created.
 *
 * \param geom (in) geometry
 * \param num (in) # collapse map entries
 * \param vals (in) collapse map array
 * \param add (in) add to existing map
 *
 * \retval EA3_WRONG_TYPE if this is not a geometry
 * \retval EA3_OK
 */
EA3UTIL_EXPORT EA3_Status geomSetCollapseMap( EA3Block *geom, int num,const int *vals, bool add=false );

EA3UTIL_EXPORT EA3_Status geomGetLOD( const EA3Block *geom, int , EA3Block *&LOD);
EA3UTIL_EXPORT EA3_Status geomAddLOD( EA3Block *geom , EA3Block *&LOD);
EA3UTIL_EXPORT EA3_Status geoMinimizeNumVertices( EA3Block *geom );
EA3UTIL_EXPORT EA3_Status geoMinimizeNumNormals( EA3Block *geom );
EA3UTIL_EXPORT EA3_Status geoMinimizeNumUVs( EA3Block *geom );
EA3UTIL_EXPORT EA3_Status geoMinimizeNumColours( EA3Block *geom );
class EA3UTIL_EXPORT  Quad {
  public:
  XbVec3f coord[4];
  XbVec3f uv[4];
  XbVec3f normal[4];
  Quad &operator=(const Quad &other) {
    memcpy(this,&other,sizeof(other));
    return *this;
  };
};
//! Tesselate a BSpline or Bezier patch.
/*!
 *  
 * Tesselate a BSpline or Bezier patch.
 * divisions is the number of divisions in U and V.
 * Returns EA3_OK and fills in the Quad array if successful.
 */
EA3UTIL_EXPORT EA3_Status geoPrimTesselate(EA3Block *geoPrim,int division,XbPArray<Quad> &quads);
}
#endif

