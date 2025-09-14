// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3SkelUtil.h
 *
 *  Contains utility function declarations for operations on skeletons.
 */
#ifndef EA3SkelUtil_H
#define EA3SkelUtil_H
#include "EA3Util.h"
#include "EA3AnimUtil.h"

#ifdef EA_PLATFORM_WINDOWS
#  ifdef EA3_3D_EXPORTS
#    define EA3_3D_EXPORT __declspec(dllexport)
#  else
#    define EA3_3D_EXPORT __declspec(dllimport)
#  endif
#else
#  define EA3_3D_EXPORT
#endif

namespace EA3Util {

//! Get the hierarchy level of a skeleton
/*!
 *   
 *  Get the hierarchy level of a skeleton.  Level is defined as parent level + 1 and if no parent, level is 0.
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 *  \param skel input skeleton block
 *  \param hlevel : (out) hierarchy level
 *  \retval EA3_WRONG_TYPE if this is not a skeleton
 *  \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status skelGetHierLevel(const EA3Block *skel,int &hlevel);

//! Get the number of children in a skeleton
/*!
 *   
 *  Get the number of children in a skeleton
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 *  \param skel input skeleton block
 *  \param num : (out) # children
 *  \retval EA3_WRONG_TYPE if this is not a skeleton
 *  \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status skelGetNumChildren(const EA3Block *skel, int &num ); 
//! Gets the "index"th child if it exists.
/*!
 *  
 * Get a child from a skeleton
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param index (in) the index of the child to get
 * \param child (out) will be a pointer to the "index"th child if it exists
 * \retval EA3_NOT_FOUND if the child is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetChild(const EA3Block *skel, int index, EA3Block *&child );
//! Creates a child and sets the name
/*!
 *  
 * Create a child and sets its name\n
 * Also creates a block named "bone" in the child and sets its name to name.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param name (in) is the name of the new child
 * \param child (out) will be a pointer to the newly created child
 * \retval EA3_WRONG_TYPE if this is not a skeleton
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status skelAddChild( EA3Block *skel, const XbString &name, EA3Block *&child );
//! Gets the parent if it exists.
/*!
 *  
 * Get the parent of a skeleton
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param par (out) will be a pointer to the parent of the given skeleton or NULL if it doesn't have one
 * \retval EA3_NOT_FOUND if there is no parent.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetParent(const EA3Block *skel, EA3Block *&par ); 
//! Get a local matrix for a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init")\n
 * If it's not found, the matrix returned will be an identity matrix.\n
 * The scales, euler angles, and translations are composed to get the matrix.
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param mat (out) will be the skeleton's local matrix of the given type
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetMatrix(const EA3Block *skel, XbMatrix &mat, const XbIdentifier &type );
//! Set bone's transformation matrix
/*!
 *  
 * Set bone's transformation matrix
 * Searches for an xform of the given type (useally "local" or "init") .
 * If not found, the xform will be created.<br>
 * Set the euler, translation, and scale fields of the transformation block.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param mat (is) is the matrix to the xform will be set at
 * \param type (in) is the xform type
 * \retval EA3_OK if all goes well.
 * \sa skelGetBone skelGetXform xformSetMatrix
 */
EA3UTIL_EXPORT EA3_Status skelSetMatrix( EA3Block *skel, const XbMatrix &mat, const XbIdentifier &type );
//! Get the bone type of a skeleton.
/*!
 *  
 * Get the bone type of a skeleton
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param type (out) is the bone type from the "type" field in the "bone" block.
 * If not found, type "null" is returned.
 * \retval EA3_OK if all goes well.
 */
EA3UTIL_EXPORT EA3_Status skelGetBoneType( EA3Block *skel,XbIdentifier &type );
//! Set the bone type for a skeleton.
/*!
 *  
 * Set the bone type for a skeleton.  If the bone block is not found, it will be created.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param type (in) will be the new bone type.
 * If not found, type "null" is returned.
 * \retval EA3_OK if all goes well.
 */
EA3UTIL_EXPORT EA3_Status skelSetBoneType( EA3Block *skel, const XbIdentifier &type );
//! Get the euler rotation of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and gets its euler rotation\n
 * If the field is not found (0,0,0) is used
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param eul (out) will be the euler rotation of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetEuler(const EA3Block *skel, XbEuler &eul, const XbIdentifier &type );
//! Get the rotate pivot of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and gets its rotate pivot\n
 * If the field is not found (0,0,0) is used
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param rp (out) will be the rotate pivot of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetRotatePivot(const EA3Block *skel,XbVec3f &rp, const XbIdentifier &type);
//! Set the euler rotation of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and sets its euler rotation.\n
 * If not found, the xform will be created.
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param eul (in) is the new euler rotation of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelSetEuler(EA3Block *skel, const XbEuler &eul, const XbIdentifier &type );
//! Get the translation of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and gets its translation\n
 * If the field is not found (0,0,0) is used
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param trn (out) will be the translation of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetTranslation(const EA3Block *skel, XbVec3f &trn, const XbIdentifier &type );
//! Set the translation of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and sets its translation.\n
 * If not found, the xform will be created.
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param trn (in) is the new translation of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelSetTranslation(EA3Block *skel, const XbVec3f &trn, const XbIdentifier &type );
//! Get the scale of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and gets its scale\n
 * If the field is not found (0,0,0) is used
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param scl (out) will be the scale of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetScale(const EA3Block *skel, XbVec3f &scl, const XbIdentifier &type);
//! Set the scale field of a skeleton
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init") and sets its scale field.\n
 * If not found, the xform will be created.
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param scl (in) is the new scale of the skeletal node
 * \param type (in) is the xform type
 * \retval EA3_NOT_FOUND if not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelSetScale( EA3Block *skel, const XbVec3f &scl, const XbIdentifier &type );
//! Gets the geometry associated with this node in the hierarchy.
/*!
 *  
 * Get the geometry associated with this node in the hierarchy.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param geom (out) will be the geometry if it is found
 * \retval EA3_NO_GEOMETRY if geometry is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetGeometry(const EA3Block *skel, EA3Block *&geom );
//! Gets the light associated with this node in the hierarchy.
/*!
 *  
 * Get the light associated with this node in the hierarchy.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel input skeleton block
 * \param light (out) will be the light if it is found
 * \retval EA3_NO_GEOMETRY if light is not found.
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelGetLight(const EA3Block *skel,EA3Block *&light);
//! Searches for an xform of the given type (usually "local" or "init")
/*!
 *  
 * Searches for an xform of the given type (usually "local" or "init")
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel (in) skeleton block
 * \param xfm (out) will be the xform of the given type
 * \param type (in) is the xform type
 * \param create (in) is true if an xform should be created if not found, otherwise false.
 * \retval EA3_WRONG_TYPE if this is not a skeleton
 * \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status skelGetXform(const EA3Block *skel, EA3Block *&xfm, const XbIdentifier &type, int create);
//! Get the animation channels for this skeleton
/*!
 *  
 * Appends to the list, all the channels that go with this skeleton.
 * Will use the links reference in the bone block if it contains 
 * animation channels.  Otherwise it will search for channels by name.
 * \param skel input skeleton
 * \param anim an animation block
 * \param channelList list of channels found 
 */
EA3UTIL_EXPORT EA3_Status skelGetChannelList(const EA3Block *skel,const EA3Block *anim,XbPArray<EA3Block *> &channelList);
//! Get the animation clips for this skeleton
/*!
 *  
 * Appends to the list all the animation clips that animate fields in the xform of this skel
 * Results are sorted by animation clip start time.
 * \param skel input skeleton
 * \param animClipArray the array of animation clips in a model block
 * \param animClips list of animClips found
 */
EA3UTIL_EXPORT EA3_Status skelGetAnimClipList(const EA3Block *skel,const EA3Array *animClipArray,XbPArray<EA3Block *> &animClips);

//! compute the SRT values for a skeleton using the animClips.
/*!
 *  mask will be | of 1=scaleX, 2=scaleY, 4=scaleZ, 8=eulX,... 256=trnZ
 *  if the animClips animate those components.  Only the DOFs of mask will be set in the SRT values.
 */
EA3UTIL_EXPORT EA3_Status skelEvalAnimClips(const EA3Block *skel,const XbPArray<EA3Block *> &animClips,int frame,XbVec3f &scale,XbEuler &eul,XbVec3f &trans,unsigned int &mask);

//! update the SRT values in the animClips.
/*!
 *  If an animClip for this skeleton covers (in time) this frame, then update its values.
 */
EA3UTIL_EXPORT EA3_Status skelUpdateAnimClipValues(const EA3Block *skel,const XbPArray<EA3Block *> &animClips,int frame,XbVec3f &scale,XbEuler &eul,XbVec3f &trans);
  
//! compute the matrix for a skeleton at a frame
/*!
 *  
 */
EA3UTIL_EXPORT EA3_Status skelGetMatrixAtFrame(const EA3Block *skel,XbPArray<EA3Block *> channelList,int frame,XbMatrix &mat);
//! Get a bone from a skeleton.
/*!
 *  
 * Get the geometry associated with this node in the hierarchy.
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \return Returns the bone for this skeleton (or NULL if none exists)
 */
EA3UTIL_EXPORT EA3Block *skelGetBone(const EA3Block *skel );
//! Creates a links pointer from the skeleton block to the geometry block
/*!
 *  
 * Searches for a links pointer and if found replaces it with the geometry block pointer, if
 * no links pointer is found one is created.  If no bone is found, one is created.
 * Bone type is set to "geom".
 *
 * \par Synopsis
 * \#include "EA3SkelUtil.h"
 * \param skel (in) is the skeleton block that needs a links pointer created
 * \param geom (in) is the geometry block to create a links pointer to
 * \retval EA3_ERROR if geom is NULL
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status skelSetGeom(const EA3Block *skel,EA3Block *geom);
//!Get a matrix from an xform
/*!
 *  
 * Get a matrix from an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param mat (out) will be the matrix of the given xform (or the identity matrix if the xform is NULL)
 */
EA3UTIL_EXPORT void xformGetMatrix(const EA3Block *xform, XbMatrix &mat );
//!Set the matrix of an xform
/*!
 *  
 * Set transformations of an xform by using a matrix
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param mat (in) is matrix for the given xform
 */
EA3UTIL_EXPORT void xformSetMatrix( EA3Block *xform, const XbMatrix &mat );
//!Get the euler rotation from an xform
/*!
 *  
 * Get the euler rotation from an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param eul (out) will be the euler rotation of the given xform
 */
EA3UTIL_EXPORT void xformGetEuler(const EA3Block *xform, XbEuler &eul );
//!Set the euler rotation of an xform
/*!
 *  
 * Set the euler rotation of an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param eul (in) is the euler rotation for the given xform
 */
EA3UTIL_EXPORT void xformSetEuler( EA3Block *xform, const XbEuler &eul );
//!Get the translation of an xform
/*!
 *  
 * Get the translation of an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param trn (out) will be the translation of the given xform
 */
EA3UTIL_EXPORT void xformGetTranslation(const EA3Block *xform, XbVec3f &trn );
//!Set the translation of an xform
/*!
 *  
 * Set the translation of an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param trn (in) is the new translation for the given xform
 */
EA3UTIL_EXPORT void xformSetTranslation( EA3Block *xform, const XbVec3f &trn );
//!Get the scale of an xform
/*!
 *  
 * Get the scale of an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param scl (out) will be the scale of the given xform
 */
EA3UTIL_EXPORT void xformGetScale(const EA3Block *xform, XbVec3f &scl );
//!Set the scale of an xform
/*!
 *  
 * Set the scale of an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param scl (in) is the new scale for the given xform
 */
EA3UTIL_EXPORT void xformSetScale( EA3Block *xform, const XbVec3f &scl );
//!Get the rotate pivot from an xform
/*!
 *  
 * Get the rotate pivot from an xform
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param xform (in) is an xform
 * \param rp (out) will be the rotate pivot
 */
EA3UTIL_EXPORT void xformGetRotatePivot(const EA3Block *xform,XbVec3f &rp);

//! Get the scale, rotation, and translation from an xform
/*!
 * Get the scale, rotation, and translation from an xform (an SRT decomposition)
 *   
 *  \param xform : (in) xform block
 *  \param scl : (out) scale
 *  \param eul : (out) rotation
 *  \param trn : (out) translation
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 *  \retval EA3_NOT_FOUND if the SRT data is missing
 *  \retval EA3_OK otherwise
 */
EA3UTIL_EXPORT EA3_Status xformGetSRT( const EA3Block &xform, XbVec3f &scl, XbEuler &eul, XbVec3f &trn );
//!Get the xform from a bone (with create option)
/*!
 *  
 * Get the xform from a bone (with create option)
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a bone
 * \param type (in) is the type of xform
 * \param create (in) is true if the xform should be created if it is not found
 * \return Returns the xform from a bone, or NULL if one does not exist and will not be created
 */
EA3UTIL_EXPORT EA3Block *boneGetXform(EA3Block *bone, const XbIdentifier &type, int create);
//!Get the xform from a bone
/*!
 *  
 * Get the xform from a bone
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a bone
 * \param type (in) is the type of xform (usually "local" or "init")
 * \return Returns the xform from a bone, or NULL if one does not exist and will not be created
 */
EA3UTIL_EXPORT EA3Block *boneGetXform(const EA3Block *bone, const XbIdentifier &type);
//!Get the local transform matrix from a bone
/*!
 *  
 * Get the local transform matrix from a bone
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param mat (out) will be the local matrix
 */
EA3UTIL_EXPORT void boneGetLocalMatrix(const EA3Block *bone, XbMatrix &mat );
//!Set the local transform matrix from a bone
/*!
 *  
 * Set the local transform matrix from a bone
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param mat (in) is the new local matrix
 */
EA3UTIL_EXPORT void boneSetLocalMatrix( EA3Block *bone, const XbMatrix &mat);
//!Get the current global transformation matrix from a bone
/*!
 *  
 * Get the current global transformation matrix from a bone.
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) bone block
 * \param mat (in/out) the current global matrix (should be set to 
 *                     identity initially
 */
EA3UTIL_EXPORT void boneCalcGlobalMatrix( const EA3Block *bone, XbMatrix &mat );
//!Get the local init matrix from a bone
/*!
 *  
 * Get the local init matrix from a bone
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param mat (out) will be the local init matrix
 */
EA3UTIL_EXPORT void boneGetInitMatrix( EA3Block *bone, XbMatrix &mat);
//!Set the local init matrix from a bone
/*!
 *  
 * Set the local init matrix from a bone
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param mat (in) is the new local init matrix
 */
EA3UTIL_EXPORT void boneSetInitMatrix( EA3Block *bone, const XbMatrix &mat);
//!Get the current global init transformation matrix from a bone
/*!
 *  
 * Get the current global init transformation matrix from a bone.
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) bone block
 * \param mat (in/out) the current global matrix (should be set to 
 *                     identity initially
 */
EA3UTIL_EXPORT void boneCalcGlobalInitMatrix( const EA3Block *bone, XbMatrix &mat );
//!Set the bone type
/*!
 *  
 * Set the bone type
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param type (in) is the new bone type
 */
EA3UTIL_EXPORT void boneSetType( EA3Block *bone, const XbIdentifier &type);
//!Get the bone type
/*!
 *  
 * Get the bone type
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param bone (in) is a valid bone
 * \param type (out) will be the bone type
 */
EA3UTIL_EXPORT void boneGetType(const EA3Block *bone, XbIdentifier &type);
//!Get transforms from a matrix
/*!
 *  
 * Get transforms from a matrix
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param mat (in) ia a matrix
 * \param trn (out) will be the translation
 * \param eul (out) will be the euler rotation
 * \param scl (out) will be the scale
 * \par NOTE
 *  The function calls abort() if there it contains scale that causes a loss of
 *  precision greater than 0.0001
 */
EA3UTIL_EXPORT void getTransform(const XbMatrix &mat,XbVec3f &trn,XbEuler &eul,XbVec3f &scl );

/*!
 *   
 *   
 */

//! A utility class that records the motion of a skeleton.
/*!
 *  
 * An EA3RecordedSkeleton is a utility class that records the motion of a
 * skeleton and stores the motion in arrays of global and local matrices
 * for every node in the skeleton hierarchy.
 *
 * \sa EA3Model EA3Skeleton EA3Animation
 */

class EA3UTIL_EXPORT EA3RecordedSkeleton {
public:
  //! Initialization constructor
/*!
 *   
 * Construct and initialize a recorded skeleton for a given skeleton and animation
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param s is the skeletons root node (or a model block)
 * \param a is the animation block
 */
  EA3RecordedSkeleton(EA3Block *s, EA3Block *a);
  EA3RecordedSkeleton(
    EA3Block *s, EA3RecordedSkeleton *par,EA3Block *a,int numFrames);
    EA3RecordedSkeleton(EA3RecordedSkeleton &Src,EA3RecordedSkeleton *thePar=NULL);	// Copy constructor (faster to build one & copy than to build two)

  ~EA3RecordedSkeleton();
  //! Get the init global matrix
/*!
 *   
 * Get the init global matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param glb (out) is the init global matrix
 */
  void getInitGlobal(XbMatrix &glb);
  /*!
 *   
 * Set the init global matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param glb (in) is the init global matrix
 */
  void setInitGlobal(const XbMatrix &glb);
  //! Get the init global inverse matrix
/*!
 *   
 * Get the init global inverse matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param glb (out) is the init global inverse matrix
 */
  void getInitGlobalInverse(XbMatrix &glb);
  //! Get the global matrix
/*!
 *   
 * Get the global matrix
 * \param glb (out) is the global matrix
 */
  void getGlobal(XbMatrix &glb);
  /*!
 *   
 * Set the global matrix
 * \param glb is the global matrix
 */
  void setGlobal(const XbMatrix &glb);
  //! Get the init local matrix
/*!
 *   
 * Get the init local matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc (out) is the local matrix
 */
  void getInitLocal(XbMatrix &loc);
  /*!
 *   
 * Set the init local matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc is the local matrix
 */
  void setInitLocal(const XbMatrix &loc);
  //! Get the local matrix
/*!
 *   
 * Get the local matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc (out) is the local matrix
 */
  void getLocal(XbMatrix &loc);
  /*!
 *   
 * Set the local matrix
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc  is the local matrix
 */
  void setLocal(const XbMatrix &loc);
  //! Get the global matrix at a particular animation frame
/*!
 *   
 * Get the global matrix at a particular animation frame
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param glb (out) is the global matrix for the given frame
 * \param frame (in) is a frame index (between -2 and numFrames-1)
 * (frame -2 is init pose, -1 is current pose)
 */
  void getAnimGlobal(XbMatrix &glb,int frame);
  /*!
 *   
 * Set the global matrix at a particular animation frame
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param glb (in) is the global matrix for the given frame
 * \param frame (in) is a frame index (between -2 and numFrames-1)
 * (frame -2 is init pose, -1 is current pose)
 */
  void setAnimGlobal(const XbMatrix &glb,int frame);
  //! Get the local matrix at a particular animation frame
/*!
 *   
 * Get the local matrix at a particular animation frame
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc (out) is the local matrix for the given frame
 * \param frame (in) is a frame index (between -2 and numFrames-1)
 * (frame -2 is init pose, -1 is current pose)
 */
  void getAnimLocal(XbMatrix &loc,int frame);
  //! Get the local matrix at a particular animation frame
/*!
 *   
 * Get the local matrix at a particular animation frame
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param loc (in) is the local matrix for the given frame
 * \param frame (in) is a frame index (between -2 and numFrames-1)
 * (frame -2 is init pose, -1 is current pose)
 */
  void setAnimLocal(const XbMatrix &loc,int frame);
  //! Record the animation from a given animation block
/*!
 *   
 * Record the skeletal information (local/global matrices, etc.) for each frame
 * based on a given animation block
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param a is the animation block
 */
  void record(EA3Block *a);
  //! Update the local matrices for this node and its children
/*!
 *   
 * Update the local matrices for this node and its children
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param lockchildglobal (in) is true if you want the the childrens'
 * global matrices to stay in tact, while the local matrices are updated.
 * If it is false, the childrens' local matrices stay in tact while their
 * global matrices are updated.
 */
  void updateLocal(bool lockchildglobal = true);
	// Called by updateLocal when there is more than one child, lockchildglobal == true,
	// and the parent global matrix inverses have been precomputed
	void updateLocalFast(void);

  //! Update the global matrices for this node and its children
/*!
 *   
 * Update the global matrices for this node and its children
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 * \param lockchildlocal (in) is true if you want the the childrens'
 * local matrices to stay in tact, while the global matrices are updated.
 * If it is false, the childrens' global matrices stay in tact while their
 * local matrices are updated.
 */
  void updateGlobal(bool lockchildlocal = true);
  void postMultGlobalBy(XbMatrix &post);
  void preMultGlobalBy(XbMatrix &post);
  void postMultLocalBy(XbMatrix &post);
  void preMultLocalBy(XbMatrix &post);
  //! Cut a child node from the recorded skeleton
/*!
 *   
 *  Cut the given child from this recorded skeleton
 *  cutChild() will cut from the EA3 skeleton hierarchy also.  
 *  The EA3RecordedSkeleton passed in is deleted.
 *  \par Synopsis
 *  \#include "EA3SkelUtil.h"
 *  \param ch (in) is the child skeleton
 */

  void cutChild(EA3RecordedSkeleton *ch);
  //! Add a child node to the recorded skeleton - does no matrix updates.  
  void addChild(EA3RecordedSkeleton *ch);
  
  EA3_Status geometryApplyTransform(EA3Block *geom, const XbMatrix& matrix);
  EA3_Status freezeHierarchy(int recurse);
  EA3_Status freezeHierarchyWorld(int recurse);
  /*!
  *   
  *  Add the given child to this recorded skeleton
  *  addChild() will add to the EA3 skeleton hierarchy also.  
  *  Local matrices of child are updated.
  *  \par Synopsis
  *  \#include "EA3SkelUtil.h"
  *  \param ch (in) is the child skeleton
  */
  EA3_Status reParentSkeleton(EA3RecordedSkeleton *child);
  /*!
    merge every descendant into this node.  
    Merges geometries when encountered after applying the transformation to the geometry.
  */
  EA3_Status flattenHierarchy();
  EA3_Status mergeHierarchies(EA3RecordedSkeleton *slave);
  /*!
    Collapse the hierarchy by:<br>
    1) removing this node if no children, or
    2) this node is merged with first child (this node's pointer is still valid, first child is deleted).  Then, this node's children are moved to this node's parent.
  */
  EA3_Status collapse();
  
  EA3Block *skel;
  EA3Block *anim;
  EA3RecordedSkeleton *par;
  XbPArray<EA3RecordedSkeleton *> children;
  int numFrames;    // does not include the first two matrices
  XbMatrix *global; // 0 is init, 1 is pose, 2... is anim
  XbMatrix *local;  // 0 is init, 1 is pose, 2... is anim
  XbPArray<EA3Block *> channelList;
  XbMatrix initGlobalInverse;

};
typedef bool RecordedSkeletonFunc(EA3RecordedSkeleton *rs, void *data);
//! Apply a function to every node of an EA3RecordedSkeleton
/*!
 *  
 * Apply a function to every node of an EA3RecordedSkeleton.\n
 * If any of the functions return false, the recurssion is terminated.
 * The function is first applied to the current node, and then in order
 * to each of each children.
 * \par synopsis
 *  #include "EA3SkelUtil.h"
 * \param rs (in) is a valid recorded skeleton
 * \param f (in) is a pointer to a RecordedSkeletonFunc
 * \param data (in) is used when executing each f
 * \par Example:
 *  To count all skeletal nodes, try making a function like this:
 * \code
 *  static bool countskels(EA3RecordedSkeleton *rs, void *data) {
 *    int *num = (int*)data;
 *    if( rs->skel->getType()=="skeleton" ) (*num)++;
 *    return true;
 *  }
 * \endcode
 *  To execute this function, try this:
 * \code
 *  // First load a file and find the appropriate model and anim blocks
 *  int num=0;
 *  EA3RecordedSkeleton rs(model,anim);
 *  rsApplyFunc(&rs,countskels,&num);
 *  printf("There are %i skeletal nodes\n", num);
 * \endcode
 * \retval true if every recurssive function call returns true
 * \retval false if one of the function calls returns false
 */
EA3UTIL_EXPORT bool rsApplyFunc(EA3RecordedSkeleton *rs, RecordedSkeletonFunc *f, void *data);
}


#endif

