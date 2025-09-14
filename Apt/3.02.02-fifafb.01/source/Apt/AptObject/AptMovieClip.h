/*H************************************************************************************************/
/*!

    \File
        AptMovieClip.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTMOVIECLIP_H__
#define __APTMOVIECLIP_H__

class AptMovieClip : public AptObject
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptMovieClip() : AptObject(AptVFT_MovieClip)
    {
    }

protected:
    APT_INLINE
    virtual
    ~AptMovieClip() {}
};

#endif  //  __APTMOVIECLIP_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

