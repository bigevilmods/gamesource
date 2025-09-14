// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3ModelUtil.h
 *
 *  Contains utility function declarations for operations on model blocks.
 */
#ifndef EA3ModelUtil_H
#define EA3ModelUtil_H
#include "EA3Util.h"
namespace EA3Util {
//! Add an animation array to a model block.
/*!
 *  
 * Creates a new EA3Array (if it does not already exist) inside the
 * model block.
 * It then creates an EA3Block of type "animation" inside this array.
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are adding to
 * \param name (in) is the name of the new animation
 * \param anim (out) will be a pointer to the newly created animation
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status modelAddAnimation( EA3Block *model, const XbString &name, EA3Block *&anim );

//! Get an animation block from within a model block.
/*!
 *  
 * Searches and returns the first animation that matches name.
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are retrieving from
 * \param name (in) is the name of the animation to get
 * \param anim (out) will be a pointer to the wanted animation
 * \retval EA3_NOT_FOUND if not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status modelGetAnimation(const EA3Block *model, const XbString &name, EA3Block *&anim );

//! Get an animation block from within a model block.
/*!
 *  
 * Searches and returns the first animation that matches the index.
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are retrieving from
 * \param index (in) is the index of the animation to get
 * \param anim (out) will be a pointer to the wanted animation
 * \retval EA3_NOT_FOUND if not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status modelGetAnimation(const EA3Block *model, int index, EA3Block *&anim );

//! Returns the number of animations in a model.
/*!
 *  
 * Returns the number of animations in a model
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are retrieving from
 * \param num (out) will be the number of animations in this model block
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status modelGetNumAnimations(const EA3Block *model, int &num );

//! Add a skeleton array to a model block.
/*!
 *  
 * Creates a new EA3Array (if it does not already exist) inside the model
 * block.  It then creates an EA3Block of type "skeleton" inside this array.
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are adding to
 * \param name (in) is the name of the new skeleton
 * \param skel (out) will be pointer to the newly created skeleton
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status modelAddSkeleton( EA3Block *model, const XbString &name, EA3Block *&skel );

//! Searches and returns the first skeleton that matches name.
/*!
 *  
 * Searches and returns the first skeleton that matches name
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are searching
 * \param name (in) is the name of the wanted skeleton
 * \param skel (out) will be a pointer to the wanted skeleton
 * \retval EA3_NOT_FOUND if not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status modelGetSkeleton(const EA3Block *model, const XbString &name, EA3Block *&skel );

//! Searches and returns the first skeleton that matches the index.
/*!
 *  
 * Searches and returns the first skeleton that matches the index
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is the model block you are searching
 * \param index (in) is the index of the wanted skeleton
 * \param skel (out) will be a pointer to the wanted skeleton
 * \retval EA3_NOT_FOUND if not found
 * \retval EA3_OK otherwise.
 */
EA3UTIL_EXPORT EA3_Status modelGetSkeleton(const EA3Block *model, int index, EA3Block *&skel );

//! Returns the number of skeletons (roots of skeletons) in a model.
/*!
 *  
 * Returns the number of skeletons (roots of skeletons) in a model.
 * \par Synopsis
 * \#include "EA3ModelUtil.h"
 * \param model (in) is a model block
 * \param num (out) will be the number of skeletons in this model block
 * \retval EA3_OK always.
 */
EA3UTIL_EXPORT EA3_Status modelGetNumSkeletons(const EA3Block *model, int &num );
}
#endif

