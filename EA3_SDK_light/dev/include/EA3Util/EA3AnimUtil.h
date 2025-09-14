// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3AnimUtil.h
 *
 *  Contains utility function declarations for operations on animation
 *  blocks, channel blocks, function curve blocks, and key fields.
 */

#ifndef EA3AnimUtil_h
#define EA3AnimUtil_h
#include "EA3Util.h"
namespace EA3Util {

typedef enum {
      EULER=1,
      TRANSLATION=2,
      QUATERNION=4,
      SCALE=8,
      MATRIX=16,
} ChannelTypes;
//! Creates DiscreteF3Channels given a skeleton and name.
/*!
*  
* \param anim input animation
* \param name will be used as the name of each channel created.
* \param skel is the skeleton used to determine which channels will be created.
* \param outList output list of channels
* \param types an | of the channel types created - TRANSLATION, EULER.
* If the skeleton has a bone block with an xform array that has
* an xform with a field "type" and that field value is "local", then
* that xform block will determine what types of channels to create.<br>
* If it has an euler field, then a channel with the field "what" 
* set to "eul" will be created.<br> Similarily a "translation" field 
* generates a "trn" channel.  
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if xform can not be found.
* \retval EA3_OK otherwise.
*/

EA3UTIL_EXPORT EA3_Status animCreateDiscreteF3Channels(
  EA3Block *anim,
      const XbString &name,
  EA3Block *skel,
  XbPArray<EA3Block *> &outList,
  unsigned int &types );

//! Creates DiscreteF3Channels given a name and a bit union of types.
/*!
*  
* \param anim input animation block
* \param name will be used as the name of each channel created.
* \param what is an | of the types of channels to create: TRANSLATION, EULER.
* \param outList output list of channels
* \param types output types of channels created
* \param skel skeleton block to create/add links reference
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status animCreateDiscreteF3Channels(
  EA3Block *anim,
      const XbString &name,
  unsigned int what,
  XbPArray<EA3Block *> &outList,
  unsigned int &types,
  EA3Block *skel=NULL);

//! Creates a continuous or discrete F1Channel
/*!
*  
* \param anim input animation block
* \param name will be that name of the channel
* \param what is usually: "eul", "trn", "scl", "morphTarget" or "user"
* \param index index for channel.  Example index=0 and what="trn" would create the x translation channel.
* \param discrete if true creates a discreteF1Channel, else a continuousF1Channel
* \param skel skeleton block to create/add links reference
* \param outList output list of channels
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status animCreateF1Channel(EA3Block *anim,const XbString &name, XbIdentifier what,unsigned int index,bool discrete,EA3Block *skel, XbPArray<EA3Block *> &outlist);

//! Saves a key of animation by decomposing a matrix into euler angles and
//! translation.
/*!
*  
* If the list of channels has an euler channel, it will have a key saved.<br>
* If the list of channels has a translation channel, it will have a key
* saved.<br>
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
* \sa channelSaveKey()
*/

EA3UTIL_EXPORT EA3_Status animSaveKey(
  EA3Block *anim,
      XbPArray<EA3Block *> &channelList,
  XbMatrix &mat,
  int frame );
//! Saves a key of animation by decomposing a matrix into euler angles and 
//! translation.
/*!
*  
* Create the euler and translation channels if they do not already exist.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
* \sa channelSaveKey()
*/
EA3UTIL_EXPORT EA3_Status animSaveKey(
  EA3Block *anim,
      XbPArray<EA3Block *> &channelList,
  XbMatrix &mat,
  int frame,
  const XbString &name,
  EA3Block *skel = NULL);

//! Uses a list of channels to compose a matrix given a frame index
//! (from 0) into the channels.
/*!
*  
* The matrix is composed of a translation vector and
* euler angles.<br>
* If the channel type is not found 
* then default values will be used to compose the matrix: (0,0,0) for 
* translation and euler angles, (1,1,1) for scale.
* If frame < 0 then frame 0 will be used.
* If frame > last frame then last frame will be used.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status animGetMatrix(
  const EA3Block *anim, XbPArray<EA3Block *> &channelList, int, XbMatrix &mat);
//! Uses a list of channels to compose a matrix given a frame index
//! (from 0) into the channels.
/*!
*  
* The matrix is composed of a translation vector and euler angles.<br>
* 
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status animGetMatrix(
  const EA3Block *anim,
  XbPArray<EA3Block *> &channelList,
  int frame,
  XbMatrix &mat,
  XbVec3f &trn,
  XbEuler &eul,
  XbVec3f &scl );
/*!
*  
* Appends channels to list that have the same name as "name".
* list is not reset to empty before appending.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status animGetChannels(
  const EA3Block *anim, const XbString &name, XbPArray<EA3Block *> &channelList );

//! get the Frames Per Second
/*! get the Frames Per Second
 *  
 */
EA3UTIL_EXPORT EA3_Status animGetFPS(EA3Block *anim,float &fps);
//! Set the Frames Per Second
/*! Set the Frames Per Second
 *  
 * If timeWarp is not set, then no adjustment is made to the continuousF1Channels time values.
 * Otherwise, the continuousF1Channels will be timeWarped by oldFPS/newFPS.
 * This will keep them at the same frame, but not at the same time.
 */
EA3UTIL_EXPORT EA3_Status animSetFPS(EA3Block *anim,float fps,bool timeWarp);


//! Saves a value into a channel at a given frame index (from 0).
/*!
*  
* If frame is greater than the number of frames currently in the channel,
* then the channel will be padded with the value given.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status channelSaveKey( EA3Block *anim, const XbVec3f &val, int frame );

//! Saves a value into a channel at a given frame index (from 0).
/*!
*  
* If frame is greater than the number of frames currently in the channel,
* then the channel will be padded with the value given.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/

EA3UTIL_EXPORT EA3_Status channelSaveKey( EA3Block *channel, const XbEuler &val, int frame );
EA3UTIL_EXPORT EA3_Status channelSaveKey( EA3Block *channel, float val, int frame );
EA3UTIL_EXPORT EA3_Status channelListSaveKey( XbPArray<EA3Block *> &channelList, const XbVec3f &val, int frame );
EA3UTIL_EXPORT EA3_Status channelListSaveKey( XbPArray<EA3Block *> &channelList, const XbEuler &val, int frame );
/*!
*  
* \brief Fill in the SRT (scale, rotate, translate) values based on a list of channels
* If frame < startframe then the startframe will be used.
* If frame > last frame then last frame will be used.
* Values in scl, eul and trn will only change if a channel is associated with it.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status channelListGetSRT(XbPArray<EA3Block *> &channelList,int frame,XbVec3f &scl,XbEuler &eul, XbVec3f &trn);

typedef enum
{
	CSRT_None = 0,
	CSRT_F3,
	CSRT_F1Discrete,
	CSRT_F1Continuous,
} ChanSRTType;

struct ChanRec
{
	ChanSRTType	Type;		// Channel type
	EA3Block	*pChan;		// Channel pointer
	int			Index;		// Field index (if F1 channel)
	EA3Array	*pData;		// Pointer to channel data (not always gathered)
};

struct ChannelInfo
{
	int				Count;			// Number of array members used (0-3)
	ChanRec			Chan[3];		// Individual channel records
};

struct ChannelListSRTInfo
{
	int				StartFrame;		// Start frame of the animation
	float			fps;			// used only by continuous animations
	ChannelInfo		S, R, T;		// Info for each of the three channels
};

struct ChannelsInfo : public ChannelInfo
{
	int				StartFrame;		// Start frame of the container object
	float			fps;			// For continuous channels only
};


EA3UTIL_EXPORT EA3_Status channelListGetR(XbPArray<EA3Block *> &channelList, int frame, XbEuler &eul);
EA3UTIL_EXPORT EA3_Status channelListGetSRTInfo(XbPArray<EA3Block *> &channelList, ChannelListSRTInfo &Info);
EA3UTIL_EXPORT EA3_Status channelListGetRFast(const ChannelInfo &Info, int frame, XbEuler &eul, int StartFrame, float fps);
EA3UTIL_EXPORT EA3_Status channelListGetSRTFast(const ChannelListSRTInfo &Info, int frame, XbVec3f &scl, XbEuler &eul, XbVec3f &trn);

EA3UTIL_EXPORT EA3_Status channelSaveKeyFast(const ChanRec &Chan, const XbVec3f &val, int frame, int startFrame);
EA3UTIL_EXPORT EA3_Status channelSaveKeyFast(const ChanRec &Chan, const XbEuler &eul, int frame, int startFrame);
EA3UTIL_EXPORT EA3_Status channelSaveKeyFast(const ChannelsInfo &Info, int ChanIndex, float val, int frame);
EA3UTIL_EXPORT EA3_Status channelListSaveKeyFast(const ChannelsInfo &Info, const XbVec3f &val, int frame);
EA3UTIL_EXPORT EA3_Status channelListSaveKeyFast(const ChannelsInfo &Info, const XbEuler &eul, int frame);


// Build channel info (eulChan and trnChan arrays) once, then call animSaveKeyFast repeatedly

// Create eul and trn channels if they don't already exist
EA3UTIL_EXPORT EA3_Status animBuildChannels(EA3Block *anim, XbPArray<EA3Block *> &clist, const XbString &name, EA3Block *skel);

// Build channel info from existing channels
EA3UTIL_EXPORT EA3_Status animBuildChannelInfo(EA3Block *anim, XbPArray<EA3Block *> &clist, ChannelInfo &eulChan, ChannelInfo &trnChan);

//! Get the number of frames in a channel.
/*!
*  
* \param channel (in) channel block
* \param num (out) # frames
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always.
*/
EA3UTIL_EXPORT EA3_Status channelGetNumFrames(const EA3Block *channel, int &num );
//! Set key time.
/*!
*  
* \param key (in) function key
* \param t (in) key time
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keySetTime( EA3Field *key, float t );
//! Set key value.
/*!
*  
* \param key (in) function key
* \param v (in) key value
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/

EA3UTIL_EXPORT EA3_Status keySetValue( EA3Field *key, float v );
//! Get key time.
/*!
*  
* \param key (in) function key
* \param t (out) key time
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keyGetTime( const EA3Field *key, float &t );
//! Set key value.
/*!
*  
* \param key (in) function key
* \param v (out) key value
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keyGetValue( const EA3Field *key, float &v );
//! Set key left slope.
/*!
*  
* \param key (in) function key
* \param slope (in) key slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keySetLSlope( EA3Field *key, float slope );

//! Set key right slope.
/*!
*  
* \param key (in) function key
* \param slope (in) key slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keySetRSlope( EA3Field *key, float slope );
//! Set key slopes.
/*!
*  
* \param key (in) function key
* \param left (in) left slope
* \param right (in) right slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keySetSlope( EA3Field *key, float left, float right );
//! Get key left slope.
/*!
*  
* \param key (in) function key
* \param ls (out) left slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keyGetLSlope( const EA3Field *key, float &ls );
//! Get key right slope.
/*!
*  
* \param key (in) function key
* \param rs (out) right slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status keyGetRSlope( const EA3Field *key, float &rs );
//! Get key slope.
/*!
*  *  
\param key (in) function key
* \param left (out) left slope
* \param right (out) left slope
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/

EA3UTIL_EXPORT EA3_Status keyGetSlope( const EA3Field *key, float &left, float &right );

//! Get the # of keys in the function curve.
/*!
*  
* \param fcrv (in) function curve
* \param num (out) # keys
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there are no key array in the function curve.
* \retval EA3_OK otherwise.
*/
EA3UTIL_EXPORT EA3_Status fcrvGetNumKeys( const EA3Block *fcrv, int &num );
//! Create in the function curve a constant key.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param value (in) key value
* \param key (out) the new key
* \param sortkeys (in) should I sort the keys after creation?
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvCreateConstantKey(EA3Block *fcrv, float time, float value, EA3Field *&key, bool sortkeys=false );
//! Create in the function curve a linear key.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param value (in) key value
* \param key (out) new key
* \param sortkeys (in) should I sort the keys after creation?
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvCreateLinearKey(EA3Block *fcrv, float time, float value, EA3Field *&key,bool sortkeys=false );
//! Create in the function curve a hermite key.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param value (in) key value
* \param leftSlope (in) key left slope
* \param rightSlope (in) key right slope
* \param key (out) new key
* \param sortkeys (in) should I sort the keys after creation?
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/

EA3UTIL_EXPORT EA3_Status fcrvCreateHermite1DKey( EA3Block *fcrv, float time, float value, 
                          float leftSlope, float rightSlope, EA3Field *&key,bool sortkeys=false );
//! Create in the function curve a Softimage hermite key.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param value (in) key value
* \param leftSlope (in) key left slope
* \param rightSlope (in) key right slope
* \param key (out) new key
* \param sortkeys (in) should I sort the keys after creation?
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvCreateSoftHermiteKey(
	EA3Block *fcrv,
	float time, float value,
	float leftSlope, float rightSlope,
  EA3Field *&key,
	bool sortkeys=false );
//! Remove a key.
/*!
*  
* \param fcrv (in) function curve
* \param which (in) index of the key to be removed
* 
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found.
* \retval EA3_OK otherwise.
*/
EA3UTIL_EXPORT EA3_Status fcrvRemoveKey( EA3Block *fcrv, int which );
//! Remove a key.
/*!
*  
* \param fcrv (in) function curve
* \param k (in) key to be removed
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found.
* \retval EA3_OK otherwise.
*/
EA3UTIL_EXPORT EA3_Status fcrvRemoveKey( EA3Block *fcrv, EA3Field *k );
//! Remove all keys from a function curve.
/*!
*  
* \param fcrv (in) function curve
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there are no keys in the function curve.
* \retval EA3_OK otherwise.
*/

EA3UTIL_EXPORT EA3_Status fcrvRemoveAllKeys( EA3Block *fcrv );

//! Move all key times by a given amount.
/*!
*  
* \param fcrv (in) function curve
* \param delta (in) the amount of time to move the keys
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there is no key array
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvTranslateTime( EA3Block *fcrv, float delta );
//! Scale all key times by a given amount.
/*!
*  
* \param fcrv (in) function curve
* \param scale (in) amount to scale the key times
* \param origin (in) origin point to scale the keys
* \param doSlope (in) scale the slopes too?
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there is no key array
* \retval EA3_OK otherwise
*/

EA3UTIL_EXPORT EA3_Status fcrvScaleTime( EA3Block *fcrv, float scale, float origin=0.0, bool doSlope=true );
//! Sort function curve keys
/*!
*  
* \param fcrv (in) function curve
* \param keys (in) key array to be sorted.
* If this is null, the key array in the function curve will be used.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there is no key array
* \retval EA3_OK otherwise
*/

EA3UTIL_EXPORT EA3_Status fcrvSortKeys( EA3Block *fcrv, EA3Array *keys = NULL );
//! Find a key.
/*!
*  
* \param fcrv (in) function curve
* \param index (in) which key to find
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not there
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvGetKey( const EA3Block *fcrv, int index , EA3Field *&key);
//! Find the first key in the function curve.
/*!
*  
* \param fcrv (in) function curve
* \param key (out) first key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvFirstKey( const EA3Block *fcrv, EA3Field *&key);
//! Find the last key in the function curve.
/*!
*  
* \param fcrv (in) function curve
* \param key (out) last key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvLastKey( const EA3Block *fcrv, EA3Field *&key);
//! Find the key index, given a pointer to the key.
/*!
*  
* \param fcrv (in) function curve
* \param keySeek (in) key
* \param ix (out) key index
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvGetKeyIndex( const EA3Block *fcrv, EA3Field &keySeek, int &ix );
//! Find a key closest to the given key time.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not there
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvFindClosestKey(const EA3Block *fcrv, float time, EA3Field *&key);
//! Find a key closest to the given key time and value.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param y (in) key value
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not there
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvFindClosestKey(const EA3Block *fcrv, float time, float y, EA3Field *&key);
//! Find a key and which slope that is closest to the given key time and value.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param y (in) key value
* \param rightSlope (in) is the right slope closer?
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not there
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvFindClosestSlope(const EA3Block *fcrv, float time, float y, bool &rightSlope, EA3Field *&key );
//! Automatic adjust all the key slopes given the tension.
/*!
*  
* \param fcrv (in) function curve
* \param tension (in) tension
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/

EA3UTIL_EXPORT EA3_Status fcrvAutoSlopes( EA3Block *fcrv, float tension );
//! Find the last key in the function curve whose time value is less than
//! the given time.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) time
* \param which (out) which key in the curve?
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvFindPreviousKey( const EA3Block *fcrv, float time, int *which, EA3Field *&key);
//! Find the first key in the function curve whose time value is more than
//! the given time.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) time
* \param which (out) which key in the curve?
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvFindNextKey( const EA3Block *fcrv, float time, int *which, EA3Field *&key);
//! Change the key to the next type.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) time
* A constant key will become linear.
* Linear will become hermite.
* Hermite will become Softimage hermite.
* Softimage hermite will become constant.
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/

EA3UTIL_EXPORT EA3_Status fcrvCycleCurveType( EA3Block *fcrv, float time );
//! Change the key type to a new type.
/*!
*  
* \param fcrv (in) function curve
* \param index (in) which key to change
* \param newType (in) new key type
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/

EA3UTIL_EXPORT EA3_Status fcrvChangeKeyType( EA3Block *fcrv, int index, XbIdentifier newType, EA3Field *&key);
//! Change the key type to a new type.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) key time
* \param newType (in) new key type
* \param key (out) key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvChangeKeyType(EA3Block *fcrv, float time, XbIdentifier newType , EA3Field *&key);
//! Change the key type to a new type.
/*!
*  
* \param fcrv (in) function curve
* \param key (in) key
* \param newType (in) new key type
* \param newKey (out) the new key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if the key is not found
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvChangeKeyType(EA3Block *fcrv, EA3Field &key, XbIdentifier newType, EA3Field *&newKey );
//! Change all keys to a new type.
/*!
*  
* \param fcrv (in) function curve
* \param newType (in) new key type
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvChangeAllKeysType( EA3Block *fcrv, XbIdentifier newType );
//! Evaluate the function curve at a given time.
/*!
*  
* \param fcrv (in) function curve
* \param time (in) time
* \param res (out) function curve value
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_NOT_FOUND if there are no keys in the function curve
* \retval EA3_OK otherwise
*/
EA3UTIL_EXPORT EA3_Status fcrvEval( const EA3Block *fcrv, float time, float &res );
//! Evaluate the function curve at a given time, using the hermite method.
/*!
*  
* \param startKey (in) starting key for evaluation
* \param endKey (in) ending key for evaluation
* \param time (in) time (must be between startKey and endKey)
* \param res (out) function curve value
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/
EA3UTIL_EXPORT EA3_Status fcrvEvalHermite1D( const EA3Field &startKey, EA3Field &endKey, float time, float &res );
//! Evaluate the function curve at a given time, using the Softimage hermite
//! method.
/*!
*  
* \param startKey (in) starting key for evaluation
* \param endKey (in) ending key for evaluation
* \param time (in) time (must be between startKey and endKey)
* \param res (out) function curve value
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval EA3_OK always
*/

EA3UTIL_EXPORT EA3_Status fcrvEvalSoftHermite( const EA3Field &startKey, EA3Field &endKey, float time , float &res);

//! Compare the two key times.
/*!
*  
* \param k1 (in) first key
* \param k2 (in) second key
* \par Synopsis
* \#include "EA3AnimUtil.h"
* \retval -1 if first key time is less than second
* \retval 0 if first key time is equal to second
* \retval 1 if first key time is more than second
*/
EA3UTIL_EXPORT int fcrvKeyCompare( EA3Base **k1, EA3Base **k2 );

} // namespace
#endif

