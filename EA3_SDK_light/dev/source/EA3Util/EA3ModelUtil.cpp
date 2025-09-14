// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3ModelUtil.cxx
 *
 *  Contains utility functions for operations on model blocks.
 */

#include "EA3ModelUtil.h"

namespace EA3Util {

/** @name MODEL FUNCTIONS
 *  Utility functions for working with models
 */
//@{

EA3_Status modelAddAnimation(EA3Block *model,const XbString &name,EA3Block *&anim) {
  EA3Array *a = model->createArray(XBanimation);
  anim = (EA3Block *)a->createBlock(XBanimation,name);
  return(EA3_OK);
};

EA3_Status modelGetAnimation(const EA3Block *model,const XbString &name,EA3Block *&anim) {
  EA3Array *a = model->getByType(XBanimation);
  if (!a) anim=NULL; else anim = a->getByName(name);
  if (!anim) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status modelGetAnimation(const EA3Block *model,int index,EA3Block *&anim) {
  EA3Array *a = model->getByType(XBanimation);
  if (!a) anim=NULL; else anim = (EA3Block *)a->getByIndex(index);
  if (!anim) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status modelGetNumAnimations(const EA3Block *model,int &num) {
  EA3Array *a = model->getByType(XBanimation);
  if(!a) num = 0; else num = a->getNumMembers();
  return(EA3_OK);
}

EA3_Status modelAddSkeleton(EA3Block *model,const XbString &name,EA3Block *&skel) {
  EA3Array *a = model->createArray(XBskeleton);
  skel = (EA3Block *)a->createBlock(XBskeleton,name);
  return(EA3_OK);
};

EA3_Status modelGetSkeleton(const EA3Block *model,const XbString &name,EA3Block *&skel) {
  EA3Array *a = model->getByType(XBskeleton);
  if (!a) skel=NULL; else skel = (EA3Block *)a->getByName(name);
  if (!skel) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status modelGetSkeleton(const EA3Block *model,int index,EA3Block *&skel) {
  EA3Array *a = model->getByType(XBskeleton);
  if (!a) skel=NULL; else skel = (EA3Block *)a->getByIndex(index);
  if (!skel) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status modelGetNumSkeletons(const EA3Block *model,int &num) {
  EA3Array *a = model->getByType(XBskeleton);
  if(!a) num = 0; else num = a->getNumMembers();
  return(EA3_OK);
}

//@}
} // namespace

