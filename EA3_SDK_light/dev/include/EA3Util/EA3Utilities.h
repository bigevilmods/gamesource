// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3Utilities.h
 *
 *  Holds utility functions for operating on recorded skeletons.
 */
#ifndef _EA3UTILS_H
#define _EA3UTILS_H

//
// Holds headers for recorded skeleton utilities
//
#include "EA3.h"
#include "XbLinear.h"
#include "EA3ModelUtil.h"
#include "EA3SkelUtil.h"
#include "EA3AnimUtil.h"
#include "EA3CoordUtil.h"
//#include "EA3Tools.h"
#include "EA3Expr.h"

#ifndef EA_PLATFORM_WINDOWS
#define _stricmp _stricmp
#endif 
class EAistream;
namespace EA3Util {
//
// Checks if strings are equal
//
#define streq( s1, s2 ) \
   ( strcmp( s1, s2 ) == 0 )

	struct EA3UTIL_EXPORT GlobalFlags {
		XbString name;
		bool *flag;
		const char *desc;
	};

	EA3UTIL_EXPORT EA3_Status getGlobalFlag(const XbString &flagname,bool &value);

	EA3UTIL_EXPORT EA3_Status setGlobalFlag(const XbString &flag,bool value);

	EA3UTIL_EXPORT EA3_Status getGlobalFlags(const GlobalFlags *&flags);

	EA3UTIL_EXPORT void EA3Remove(const XbPArray<EA3Base *> &baseList);

//!Uninstance a skeleton
/*!
 *  
 * Replace instances with copies of the instanced skeleton.
 * \param skeleton (in) is the target skeleton
 */
EA3UTIL_EXPORT void EA3UninstanceSkeleton(EA3Block *skeleton);

//!Automatically create instances.
/*!
 *  
 * Searches for geometries that are identical (or if allowVariations then allow for different materials per polygon)
 * and make instances (or a variation) of the geometry.
 * \param skeleton (in) is the skeleton you are searching in
 * \param allowVariations (in) determines whether to allow for different materials per polygon
 */
EA3UTIL_EXPORT void EA3InstanceSkeleton(EA3Block *skeleton,bool allowVariations=false);

//! Freeze the animation
/*!
 *  
 * Freeze the animation\n
 * Alters the recorded skeleton so that the given node's transform is always the
 * identity matrix.  Alters each of its children so that their animation remains the same
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \par How it works:
 * - Iterate through children and alter transform in by the position of the corresponding motion in the parent\n
 * - Iterate through the parent's frames and reset to the identity
 * \par NOTE:
 * This command was designed to work in conjunction with EA3FreezeHierarchy()
 *
 * \param rs (in) is a valid recorded skeleton containing skeletal and animation information
 * \retval EA3_OK if all went well
 */
EA3UTIL_EXPORT int EA3FreezeAnimation(EA3RecordedSkeleton &rs);

//! Freeze the Hierarchy (stub)
/*!
 *  
 * Freeze the Hierarchy (stub)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel (in) is a valid EA3 skeleton
 * \param recurse (in) see \ref EA3FreezeHierarchy
 * \param anim (in) a valid EA3 animation block corresponding to skel
 * \sa EA3FreezeHierarchy
 */
EA3UTIL_EXPORT int EA3FreezeHierarchy(EA3Block *skel, int recurse, EA3Block *anim = NULL);

//! Freeze the hierarchy
/*!
 *  
 * \anchor EA3FreezeHierarchy
 * Freezes the heirarchy, by resetting specified transforms to the identity matrix.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is the target recorded skeleton
 * \param recurse (in)\n
 * - if 0, takes a node in the hierarchy and sets it's transform
 *      to the identity, without changing the world transforms of it's children
 *      (i.e. the children's local transforms will need to compensate)
 *      If the node is a geometry node, then the geometry should be transformed
 *      to keep it in the same world coordinates. (use EA3GeometryApplyTransform)
 * - if 1, all nodes in the hierarchy will have their transforms set to the
 *      identity and all nodes that are geometry nodes should keep the same world coordinates.
 * \param updateAnimation (in) is true if the animation information is to be applied to the channel
 *      list, and false otherwise.
 * \param nonewchannels (in) if true, then no new animation channels will be created.
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT int EA3FreezeHierarchy(EA3RecordedSkeleton &rs, int recurse, bool updateAnimation=true, bool nonewchannels=false);

//! Re-Parent a skeleton (stub)
/*!
 *  
 * Re-Parent a skeleton (stub)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param newParent (in) is a valid EA3 skeleton
 * \param child (in) see \ref EA3ReParentSkeleton
 * \param anim (in) a valid EA3 animation block corresponding to skel
 * \sa EA3ReParentSkeleton
 */
EA3UTIL_EXPORT int EA3ReParentSkeleton(EA3Block *newParent, EA3Block *child, EA3Block* anim=NULL);

//! Re-Parent a skeleton
/*!
 * \anchor EA3ReParentSkeleton
 *  
 * Re-Parent a skeleton\n
 *  Changes the parent of of the given child.  World transforms and
 *  world animation transforms are preserved.  If anim is true, the
 *  world animation transforms are also preserved.
 *  An error occurs if newParent is a descendent of child.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param newParent (in) is a valid EA3RecordedSkeleton to become the parent of child
 * \param child (in) is a valid EA3RecordedSkeleton whose parent is to be changed
 * \param updateAnimation (in) is true if the animation information is to be applied to the channel
 *      list, and false otherwise.
 * \param nonewchannels (in) if true, then no new animation channels will be created.
 * \retval EA3_NOT_FOUND if the given parent was a descendent of the child
 * \retval EA3_OK all is well
 */
EA3UTIL_EXPORT int EA3ReParentSkeleton(EA3RecordedSkeleton *newParent, EA3RecordedSkeleton *child, bool updateAnimation = true,bool nonewchannels=false);

//! Merge geometry in geom into geomMaster
EA3UTIL_EXPORT void Merge(EA3Block *geomMaster, EA3Block *geom);

//! Flatten a Hierarchy (stub)
/*!
 *  
 * Flatten a Hierarchy (stub)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel (in) is a valid EA3 skeleton
 * \param anim (in) a valid EA3 animation block corresponding to skel
 * \sa EA3FlattenHierarchy
*/
EA3UTIL_EXPORT int EA3FlattenHierarchy(EA3Block *skel, EA3Block *anim=NULL);

//! Flatten a Hierarchy
/*!
 * \anchor EA3FlattenHierarchy
 *  
 * Flatten a Hierarchy\n
 *  Takes a node in the hierarchy and recursively merges all it's children into it.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is the recorded skeleton to flatten
 * \retval EA3_OK always
 * 
 */
EA3UTIL_EXPORT int EA3FlattenHierarchy(EA3RecordedSkeleton &rs);

//! Merge hierarchies (stub)
/*!
 *  
 * Merge hierarchies (stub)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param master (in) see \ref EA3MergeHierarchies
 * \param slave (in) see \ref EA3MergeHierarchies
 * \param anim (in) see \ref EA3MergeHierarchies
 * \sa EA3MergeHierarchies
*/
EA3UTIL_EXPORT int EA3MergeHierarchies(EA3Block *master, EA3Block *slave, EA3Block *anim);

//! Merge hierarchies
/*!
 * \anchor EA3MergeHierarchies
 *  
 * Merge hierarchies\n
 *  After the merge, the master skeleton now holds the geometry of both itself and
 *  of the slave.  The master's children will be those that it used to have plus
 *  those of the slave.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param master (in) is a valid EA3RecordedSkeleton that will retain its transforms
 * \param slave (in) is a valid EA3RecordedSkeleton to merge with 'master' (it will loose its transforms)
 * \param updateAnimation (in) is true if the animation information is to be applied to the channel
 *      list, and false otherwise.
 * \param nonewchannels (in) if true, then no new animation channels will be created.
 * \retval EA3_NOT_FOUND if the slave is a parent of the new master node
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int EA3MergeHierarchies(EA3RecordedSkeleton *master, EA3RecordedSkeleton *slave, bool updateAnimation=true,bool nonewchannels=false);

//! Sets a static animation (using matrix).
/*!
 *  
 * The animation is set to the specified value for all of its frames.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) a valid recorded skeleton
 * \param mat (in) is the matrix value to set the animation at
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int setStaticAnimation(EA3RecordedSkeleton &rs, XbMatrix mat);

//! Sets a static animation (using a matrix)
/*!
 *  
 * The animation is set to the identity matrix for all of its frames
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid recorded skeleton
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int setStaticAnimation(EA3RecordedSkeleton &rs);

//! Gets a 'proper' recorded skeleton (by reference)
/*!
 *  
 * Gets a 'proper' recorded skeleton (by reference)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel_node (in) see \ref getProperRecordedSkeleton
 * \param anim (in) see \ref getProperRecordedSkeleton
 * \param rs_head (out) see \ref getProperRecordedSkeleton
 * \param rs_node (out) see \ref getProperRecordedSkeleton
 * \sa getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton *rs_head, EA3RecordedSkeleton *&rs_node)
*/
EA3UTIL_EXPORT int getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton &rs_head, EA3RecordedSkeleton *&rs_node);

//! Gets a 'proper' recorded skeleton
/*!
 * \anchor getProperRecordedSkeleton
 *  
 * Gets a 'proper' recorded skeleton\n
 * Normally, if the command <code> EA3RecordedSkeleton rs(skel, anim) </code> is used, a recorded skeleton
 * is generated with the passed skeleton as the head node (it has no parent).  This causes
 * problems with getting it's global matrix.  By getting a proper recorded skeleton, the rs_head
 * is actually a head node that doesn't have a parent.  An rs_node is also returned as to operate
 * 'properly' with the wanted recorded skeleton
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel_node (in) is the node we want to generate a recorded skeleton from
 * \param anim (in) is the associated animation block
 * \param rs_head (out) is the head node (must be in memory as rs_node is a pointer into it)
 * \param rs_node (out) is a pointer to the recorded skeleton node with the same data as skel_node
 * \sa getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton &rs_head, EA3RecordedSkeleton *&rs_node)
 * \retval EA3_WRONG_TYPE if a valid skel_node is not given
 * \retval EA3_NOT_FOUND if the specified skel_node cannot be found in the generated recorded skeleton
 * \retval EA3_OK if all is well
*/
EA3UTIL_EXPORT int getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton *&rs_head, EA3RecordedSkeleton *&rs_node);

//! Find a skeleton within a recorded skeleton (by matching blocks)
/*!
 *  
 * Find a skeleton within a recorded skeleton (by matching blocks)
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid recorded skeleton
 * \param skel (in) is the skeleton block to find 
 * \return a pointer to the recorded skeleton that is found, or NULL if one cannot be.
 * \sa findSkeleton(EA3RecordedSkeleton *rs, const XbString &name)
 */
EA3UTIL_EXPORT EA3RecordedSkeleton* findSkeleton(EA3RecordedSkeleton *rs, EA3Block *skel);

//! Find a skeleton within a recorded skeleton (by name)
/*!
 *  
 * Find a skeleton within a recorded skeleton (by matching names)
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid recorded skeleton
 * \param name (in) is name of the skeleton to find 
 * \return a pointer to the recorded skeleton that is found, or NULL if one cannot be.
 * \sa findSkeleton(EA3RecordedSkeleton *rs, EA3Block *skel)
 */
EA3UTIL_EXPORT EA3RecordedSkeleton* findSkeleton(EA3RecordedSkeleton *rs, const XbString &name);

//! Freezes the hierarchy in the world (stub)
/*!
 *  
 * Freezes the hierarchy in the world (stub)
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel (in) a skeleton block
 * \param recurse (in) see \ref EA3FreezeHierarchyWorld
 * \param anim (in) the animation block
 * \sa EA3FreezeHierarchyWorld
*/
EA3UTIL_EXPORT int EA3FreezeHierarchyWorld(EA3Block *skel, int recurse, EA3Block *anim);

//! Freezes the hierarchy in the world
/*!
 * \anchor EA3FreezeHierarchyWorld
 *  
 * Freezes the hierarchy in the world\n
 *  Freezes the heirarchy, by resetting specified transforms to the identity matrix
 *  Resets the animation so that it is not the identity, but is the reverse of its global
 *  This results in it staying stationary children dispite parental movements.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid EA3RecordedSkeleton to freeze
 * \param recurse (in) is true if all the children should also be frozen in the world, otherwise false.
 * \param updateAnimation (in) is true if the animation information is to be applied to the channel
 *      list, and false otherwise.
 * \param nonewchannels (in) if true, then no new animation channels will be created.
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int EA3FreezeHierarchyWorld(EA3RecordedSkeleton &rs, int recurse, bool updateAnimation=true, bool nonewchannels=false);

//! Remove animation of a specific skeleton
/*!
 *  
 * Removes the animation channels with the given name
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param anim2go (in) is the name of the animation to go
 * \param anim (in) is the animation block
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT int EA3RemoveAnimation(const XbString& anim2go, EA3Block *anim);

//! Remove cluster for a specific skeleton
/*!
 *  
 * Remove cluster for a specific skeleton
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param cluster2go (in) the name of the cluster to go
 * \param cluster (in) the cluster to remove from
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT int EA3RemoveCluster(XbString& cluster2go, EA3Block *cluster);

//! Apply changes to the recorded skeleton
/*!
 *  
 * Resets the EA3 skeleton's (and its childrens') local matrices
 * to those in the recorded skeleton.
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid recorded skeleton
 * \param recurse (in) if true, changes should be applied to children as well.
 * \param createchans (in) if true, the procedure is allowed to create new channels as needed.
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int EA3ApplyRecordedSkeleton(EA3RecordedSkeleton &rs, bool recurse, bool createchans);

//! Collapse recorded skeleton
/*!
 *  
 * Removes the given node and shifts all of its children up.
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid EA3RecordedSkeleton to collapse
 * \param updateAnimation (in) is true if the animation information is to be applied to the channel
 *      list, and false otherwise.
 * \param nonewchannels (in) if true, then no new animation channels will be created.
 * \retval EA3_OK if all goes well
 */
EA3UTIL_EXPORT int EA3Collapse(EA3RecordedSkeleton &rs, bool updateAnimation = true, bool nonewchannels = false);

//! Calculate the bounding box of a recorded skeleton
/*!
 *  
 * Calculate the bounding box of a recorded skeleton.  Every coordinate of every geometry
 * is evaluated at every frame and the bounding box is a rectangular box (x,y,z axis aligned)
 * that contains every evaluated coordinate.
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rs (in) is a valid EA3RecordedSkeleton to find the bounding box of.
 * \param bbox_first (in/out) is a flag that is true if the coord being evaluate is the first.
 *      If it is, the bounding box is automatically intialized to surround only that point.
 *      This flag allows for the current bounding box to be extended across several recorded geometries.
 * \param min_bbox (in/out) will be extended to contain the minimum x,y,z values in the recorded geometry.
 * \param max_bbox (in/out) will be extended to contain the maximum x,y,z values in the recorded geometry.
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT int calculateBoundingBox(EA3Util::EA3RecordedSkeleton *rs, bool &bbox_first, Float3 &min_bbox, Float3 &max_bbox);

//! Calculate the bounding box of a recorded geometry
/*!
 *  
 * Calculate the bounding box of a geometry.  Every coordinate of the geometry
 * is evaluated at every frame and the bounding box is a rectangular box (x,y,z axis aligned)
 * that contains every evaluated coordinate.
 *
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param rg (in) is a valid EA3RecordedGeometry to find the bounding box of
 * \param bbox_first (in/out) is a flag that is true if the coord being evaluate is the first.
 *      If it is, the bounding box is automatically intialized to surround only that point.
 *      This flag allows for the current bounding box to be extended across several recorded geometries.
 * \param min_bbox (in/out) will be extended to contain the minimum x,y,z values in the recorded geometry.
 * \param max_bbox (in/out) will be extended to contain the maximum x,y,z values in the recorded geometry.
 * \retval EA3_OK always
 */
EA3UTIL_EXPORT int calculateBoundingBox(EA3Util::EA3RecordedGeometry *rg, bool &bbox_first, Float3 &min_bbox, Float3 &max_bbox);

//! Rereference a scene (change all references who point to from to point to to)
/*!
 *  
 * Rereference a scene (change all references who point to from to point to to).
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param root (in) is a starting point
 * \param from (in) is a base that was being referenced
 * \param to (in) will be the new base referenced
 * \retval EA3_OK if all is well
 * \retval EA3_ERROR on error
 * \sa EA3Util::cleanupRefs
 */
EA3UTIL_EXPORT EA3Util::EA3_Status sceneReRef(EA3Block *root, EA3Base *from, EA3Base *to);


//! Count the number of bones of type 'bone' or 'end'
/*!
 *  
 * Count the number of child bones of type 'bone' or 'end'.
 * Normally, this number should be 1.
 * \par Synopsis
 * \#include "EA3Utilities.h"
 * \param skel (in) is the skeleton to count the bones of
 * \return Returns the number of bones of type bone or end
 */
EA3UTIL_EXPORT int count_bone_end( const EA3Block *skel );

inline void EA3TransferMembers(EA3Block *from,EA3Block *to) {
  XbPArray<EA3Base *> alist;
  int i;
  for(i=0;i<from->getNumMembers();i++) {
    EA3Base *b = from->getByIndex(i);
    b->ref();
    alist.append(b);
  }
  from->removeAllMembers();
  for(i=0;i<alist.getSize();i++) {
    to->appendMember(alist[i]);
    alist[i]->unref();
  }
}
//! load an image
EA3UTIL_EXPORT EA3_Status loadImage(const char *dname,int &width,int &height,unsigned char *&framebuffer,bool &hasAlpha);
EA3UTIL_EXPORT EA3_Status loadImage(const char *dname,int &width,int &height,unsigned char *&framebuffer,bool &hasAlpha,unsigned int frame);
//! warp an image by specifying a source and destination 2D tri mesh.
EA3UTIL_EXPORT EA3_Status EA3WarpImageByUVtrimesh(Float2 *srcuvs,Float2 *dstuvs,int numTris,
    const unsigned char *inImage,int width,int height,int depth,
    int newWidth,int newHeight,unsigned char *outImage);

EA3UTIL_EXPORT EA3_Status imageFileGetNumFrames(const char* dname, int& NumFrames);
EA3UTIL_EXPORT EA3_Status imageFileGetFrameNames(const char* dname, XbPArray<const char*>& names);

EA3UTIL_EXPORT EA3_Status smoothNormals(XbPArray<EA3Base *> &prims,const EA3Expr::Expression &expr);

//! create a field and set the first float value
/*!
 * \brief create a field in a container and set it's values
 *  
 * Create a field in a container and set its values
 * Like printf, make sure your arguments match the types for the created field.
 * You can only use the following types: char *, int and double.  Do
 * not use XbIdentifiers or XbStrings etc.  Floats will be 
 * promoted to doubles, and char and short will be promoted to int.
 * \param container (in) is the container in which to create your field.
 * \param type (in) is the type of field.
 * \param f (in) is a float value.
 * Returns EA3_WRONG_TYPE if field can not be created.
 * Returns EA3_ERROR if container is NULL.
 */
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, float f) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].f = f;
  return EA3_OK;
}
//! create a field and set the first 2 float values
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, float f1,float f2) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].f = f1;
  fld->data[1].f = f2;
  return EA3_OK;
}
//! create a field and set the first 3 float values
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, float f1,float f2,float f3) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].f = f1;
  fld->data[1].f = f2;
  fld->data[2].f = f3;
  return EA3_OK;
}
//! create a field and set the first 4 float values
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, float f1,float f2,float f3,float f4) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].f = f1;
  fld->data[1].f = f2;
  fld->data[2].f = f3;
  fld->data[3].f = f4;
  return EA3_OK;
}
//! create a field and set the all the float values from an array of floats
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, const float *fa) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  unsigned int num = fld->getNum();
  for(unsigned int i=0;i<num;i++) {
    fld->data[i].f = *fa++;
  }
  return EA3_OK;
}
//! create a field and set the first 3 floats using an XbVec3f
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, const XbVec3f &fa) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].f = fa[0];
  fld->data[1].f = fa[1];
  fld->data[2].f = fa[2];
  return EA3_OK;
}
//! create a field and set the first string value
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, XbIdentifier &val) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].str = val;
  return EA3_OK;
}
//! create a field and set the first string value
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, XbString &val) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].str = val;
  return EA3_OK;
}
//! create a field and set the first string value
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type, const char *val) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].str = val;
  return EA3_OK;
}
//! create a field and set the first int value
inline EA3_Status createFieldWithValues(EA3Block *container, const XbIdentifier &type,int val) {
  if(!container) return EA3_ERROR;
  EA3Field *fld = container->createField(type);
  if (!fld) return EA3_WRONG_TYPE;
  fld->data[0].i = val;
  return EA3_OK;
}

//EA3UTIL_EXPORT EA3_Status readTextFile(const char *filename,EAistream &is);

}
#endif  // _EA3UTILS_H


