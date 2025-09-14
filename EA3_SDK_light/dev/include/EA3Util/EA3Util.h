// (c) 1998-2010 Electronic Arts Inc.

//!  Namespace for EA3 utility functions.
/*!
 *  \file EA3Util.h
 *
 *  Contains utility function declarations.
 */
/*!
 *  \namespace EA3Util EA3 utility functions
 *
 */
/*!
 *   
 *   
 */
/*!
 *  \page ea3utilitiesOverview EA3 Utilities Overview
 *   
 *  Utilities allow complex operations to be carried out on an 
 *  EA3 Scene without needing to worry about the underlying EA3
 *  data structure.
 */
/*!
 *   
 *   
 */
/*!
 *   
 *   
 */
/*!
 *   
 *   
 */
/*!
 *   
 *   
 */
/*!
 *   
 *   
 */


#ifndef EA3Util_H
#define EA3Util_H
#include "EA3.h"
#include "EA3_Status.h"
#include "XbLinear.h"
#define D2R ((float)M_PI/180.0f)
#define R2D (180.0f/(float)M_PI)

#if defined(EA_PLATFORM_WINDOWS) && defined(EA_DLL) 
  #ifdef EA3UTIL_EXPORTS
    #define EA3UTIL_EXPORT __declspec(dllexport)
  #else
    #define EA3UTIL_EXPORT __declspec(dllimport)
  #endif
#else
  #define EA3UTIL_EXPORT
#endif

#include "EA3names.h"

namespace EA3Util {

#define EA3UTIL_MAJOR_VERSION 2
#define EA3UTIL_MINOR_VERSION 1
#define EA3UTIL_INTERM_VERSION 1
// Version labels are based on names of 3D things...
// Comment out one line and add a new one to change.
//#define EA3UTIL_LABEL "Nurbs " __DATE__ " " __TIME__ 
#define EA3UTIL_LABEL "Cone " __DATE__ " " __TIME__ 

void EA3UTIL_EXPORT getEA3UtilVersion(int &major,int &minor,int &interm);
bool EA3UTIL_EXPORT InitGimex();

//! Call the EA3Scene::progressCallback if the user has set one.
/*  
*/
inline void EA3progress(const XbString &str,int numDone,int total) {
  if (EA3Scene::progressCallback) 
    (*EA3Scene::progressCallback)( (intptr_t)str.getString(), numDone, total);
}

EA3UTIL_EXPORT const char *getEA3UtilLabel();

template <class T>
int getValues(const EA3Array *a, int num, T *vals) {
  int i;
  for(i=0;i<a->getNumMembers() && i<num;i++) {
    EA3Field *f = (EA3Field *)a->getByIndex(i);
    assert(f->isField());
    f->CopyTo(&vals[i], sizeof(T));
  }
  return(i);
}

template <class T>
int getValuesViaRef(const EA3Block *b,const XbIdentifier &what, int num, T *vals) {
  EA3Reference *r = (EA3Reference*)b->getByType(what);
  if (!r) return(0);
  int i;
  for(i=0;i<r->getNumReferences() && i<num;i++) {
    EA3Field *f = (EA3Field *)r->getReference(i);
    assert(f->isField());
    if (!f) return(i);
    f->CopyTo(&vals[i], sizeof(T));
  }
  return(num);
}

template <class T> 
int setValuesViaRef(EA3Block *b,EA3Array *array,const XbIdentifier &what, int num,T *vals) {
  EA3Reference *r = (EA3Reference*)b->getByType(what);
  int hadRef = 1;
  if (!r) {
    r=b->createReference(what,"");
    hadRef = 0;
  }
  int i;
  for(i=0;(!hadRef || i<r->getNumReferences()) && i<num;i++) {
    EA3Field *f = (EA3Field *)r->getReference(i);
    if (!f) {
      f = (EA3Field *)array->createField(what,"");
      r->addReference(f);
    }
    assert(f->isField());
    f->CopyFrom(&vals[i], sizeof(T));
  }
  return(num);
}

template <class T> 
int setValues(EA3Array *array, int num,T *vals) {
  int i;
  for(i=0;i<num;i++) {
    EA3Field *f = (EA3Field *)array->getByIndex(i);
    if (!f) {
      f = (EA3Field *)array->createField(array->getType(),"");
    }
    assert(f->isField());
    f->CopyFrom(&vals[i], sizeof(T));
  }
  array->setNum(num);
  return(num);
}

template <class T> 
int setValues(EA3Array *array, int num,T *vals,bool add) {
  int i;
	int base = (add?array->getNumMembers():0);
	EA3Field* f = NULL;
  for(i=0;i<num;i++) {
    f = (EA3Field *)array->getByIndex(i+base);
		if (!f) {
      f = (EA3Field *)array->createField(array->getType(),"");
    }; /* if */
    assert(f->isField());
    f->CopyFrom(&vals[i], sizeof(T));
  }
  //array->setNum(num);
	array->setNum(num+base);
  return( base );
}

template <class T> 
int addValues( EA3Array *array, int num, T *vals )
{
  int i;
  int origNumMembers = array->getNumMembers();
  for(i=0;i<num;i++) {
    EA3Field *f = (EA3Field *)array->getByIndex( origNumMembers + i );
    if (!f) {
      f = (EA3Field *)array->createField(array->getType(),"");
    }
    assert(f->isField());
    f->CopyFrom(&vals[i], sizeof(T));
  }
  array->setNum(origNumMembers+num);
  return(num);
}

}

#endif

