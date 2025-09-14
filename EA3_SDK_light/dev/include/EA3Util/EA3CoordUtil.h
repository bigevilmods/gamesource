// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3CoordUtil.h
 *
 *  Contains utility functions for evaluating coordinates.
 */
#ifndef _EA3_Coord_Util_h_
#define _EA3_Coord_Util_h_
#include "EA3Util.h"
#include "EA3SkelUtil.h"
namespace EA3Util {


  /*!
  * The type for filtering morph shapes.  This type should have easy access to its bits and
  * have an accessable number of bits greater than the maximum number of shapes in any morph
  */
  typedef long int MorphShapeFilterType;
  /*!
  *   
  *   
  */
  //! A utility class that records geometry information for a skeleton.<br>Click <a href="EA3RecordedGeometry.html#TODO>here</a> for detailed information on EA3 recorded geometries.
  /*!
  *  
  * An EA3RecordedGeometry is a utility class that records the geometry of a
  * skeleton and evaluates the global coordinates based on this information.\n
  * Click <a href="EA3RecordedGeometry.html#TODO>here</a> for detailed information on EA3 recorded geometries.
  *
  * \sa EA3Model EA3Skeleton EA3Animation EA3Geometry EA3RecordedSkeleton
  */
  class EA3UTIL_EXPORT EA3RecordedGeometry {
  public:
    //! Construct from scratch
    /*!
    *  
    * Construct and initialize a recorded geometry from a given skeleton, animation, and geometry.\n
    * NOTE: The skeleton must contain all required geometric and skeletal information within its branches.
    * To be sure all this information exists, it is recommended that the skeleton is actually the model block.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param s is the skeletons root node (or a model block)
    * \param a is the animation block
    * \param g is the geometry block
    */
    EA3RecordedGeometry(EA3Block *s, EA3Block *a, EA3Block *g);
    //! Construct from a recorded skeleton
    /*!
    *  
    * Construct and initialize a recorded geometry from a given recorded skeleton and geometry.\n
    * The recorded skeleton must exist for the entire scope of the recorded geometry, and is
    * not deallocated by the recorded geometry.
    * NOTE: The recorded skeleton must contain all required geometric and skeletal information within its branches.
    * To be sure all this information exists, it is recommended that the recorded skeleton is build from the
    * model block.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param _rs is the recorded skeleton
    * \param g is the geometry block
    */
    EA3RecordedGeometry(EA3Util::EA3RecordedSkeleton *_rs,EA3Block *g);
    /*
    * Deconstructor
    */
    ~EA3RecordedGeometry();
    //! Initialize
    /*!
    *  
    * Initialize the recorded geometry using the recorded skeleton and geometry
    * stored in the class.  All geometry, envelope, etc. information is found
    * and cached in this process.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    */
    void Initialize();

    //! Get the number of coordinates for the current recorded geometry
    /*!
    *  
    * Get the number of coordinates for the current recorded geometry
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    */
    int getNumCoords() const { return coords->getNumMembers(); }

    //! Get the number of frames in the current recorded geometry
    /*!
    *  
    * Get the number of frames in the current recorded geometry
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    */
    int getNumFrames() const { if(rs) return rs->numFrames; else return 0; }

    //! Get the master bone
    /*!
    *  
    * Get the master bone
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    */
    EA3Util::EA3RecordedSkeleton* getMasterBone() { return masterbone; }

    //! Gets the local coords
    /*!
    *  
    * Get the local coords and place them in the provided array.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param mycrds (out) is an already allocated array of at least getNumCoords()
    * coordinates that will filled with the local coords of the geometry.
    */
    void getCoords( Float3 *mycrds );
    
    //! Apply morphing for the given frame
    /*!
    *  
    * Modify the given local coordinate array to solve for morphing at the given frame.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param mycrds (in/out) is an already allocated array of at least getNumCoords()
    * coordinates that is set to the local coords for the current geometry.  It will be
    * modified to be the local coords with morphing at the given frame.
    * \param frame (in) is the frame to evaluate at.
    */
    void applymorphing(Float3 *mycrds, const int frame);
    
    //! Transform the coords to the world for the given frame
    /*!
    *  
    * Modify the given morphed local coordinate array by transforming
    * to world coordinates at the given frame.
    *  \par Synopsis
    *  \#include "EA3SkelUtil.h"
    * \param mycrds (in/out) is an already allocated array of at least getNumCoords()
    * coordinates that is set to the morphed local coords for the current geometry.  They will be
    * transformed to become the world coords at the given frame.
    * \param frame (in) is the frame to evaluate at.
    */
    void trnToWorld(Float3 *mycrds, const int frame);
    
    //! Apply skinning onto mycrds for the given frame
    /*!
    *  
    * Apply skinning onto the given world coordinate array for the given frame.
    *  \par Synopsis
    *  \#include "EA3SkelUtil.h"
    * \param mycrds (in/out) is an already allocated array of at least getNumCoords()
    * coordinates that is set to the world coords for the current geometry.  They will
    * have skinning applied to them at the given frame.
    * \param frame (in) is the frame to evaluate at.
    */
    void applySkin(Float3 *mycrds, const int frame);
    
    //! Evaluates global coords and fills the given coordinate array
    /*!
    *  
    * Takes a coordinate array of at least getNumCoords() and evaluates the global
    * coords of the current recorded geometry for the given frame.<br>
    * Use -1 for the current frame and -2 for the base pose.
    * Click <a href="EA3RecordedGeometry.html">here</a> for detailed information on EA3 recorded geometries.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \par Example
    * \code
    *   // Initialize and set up OpenGL
    *   // Load a scene and get a model, animation, and geometry block
    *   EA3RecordedGeometry rg(model, anim, geom);
    *   Float3 *mycrds = new Float3[rg.getNumCoords()];  // for storing coords
    *   int i;                                           // for our display loop
    *   int frame = rg.getNumFrames() / 2;               // choose some frame
    *   rg.EvaluateGlobalCoords(mycrds, frame);          // evaluate the global coords at the wanted frame
    *   for( i=0; i<rg.getNumCoords(); i++ ) {           // draw each coord in OpenGL
    *     glVertex3f( mycrds[i][0], mycrds[i][1], mycrds[i][2] );
    *   }
    *   delete[] mycrds;
    * \endcode
    * \param mycrds (out) is an already allocated array of at least getNumCoords()
    * coordinates that will be filled with the morphed-transformed-skinned global
    * coordinates of the current recorded geometry at the given frame.
    * \param frame (in) is the frame to evaluate at.
    */
    void EvaluateGlobalCoords( Float3 *mycrds, const int frame );
    //! Get the global matrix of a coordinate at a given frame
    /*!
    *  
    * Gets the global matrix of a coordinate at a given frame.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param index (in) is the index of the coordinate to evaluate.
    * \param frame (in) is the frame that the coordinate should be evaluated at.
    * \return Returns the global matrix of the wanted coordinate at the given frame.
    */
    XbMatrix getCoordMatrix(const int index, const int frame);
    bool isvalid() const {
      return rs && geom && coords && masterbone;
    }
    //! Get a list of geometries that are use as morph targets
    /*!
    *  
    * Get a list of geometries that are use as morph targets.
    * \par Synopsis
    * \#include "EA3SkelUtil.h"
    * \param mtgeoms (out) will have all morph target geometries added to it
    */
    void getMorphTargetGeometries(XbPArray<EA3Block*> &mtgeoms);

    struct EA3WeightedBone {
      EA3Util::EA3RecordedSkeleton *bone;
      float weight;
      EA3WeightedBone *next;
    };
    EA3WeightedBone* getBoneList(int vertex);

    /* DATA */
    EA3Util::EA3RecordedSkeleton *rs;
    EA3Block *geom;

  protected:
    /*
    *Do we own the recorded skeleton??? (Deallocates only if we do)
    */
    bool doweownrs;

    /*
    * Point to the coordinates
    */
    EA3Block *coords;

    /*
    * The master bone
    */
    EA3Util::EA3RecordedSkeleton *masterbone;


    /* SKINNING */

    //! The weighted bones
    /*!
    * Contains an EA3RecordedSkeleton and weight
    * in a linked list
    */
    XbPArray< EA3WeightedBone* > weightedbonelist; // Weighted bone list per vertex

    /* MORPHING */
    EA3Block *coordDeltas;
                   EA3Array *morphTargets;
    XbPArray<EA3Base *> morphChannels;
  };

} //Namespace

#endif //_EA3_Coord_Util_h_
