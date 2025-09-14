// (c) 2003-2007 Electronic Arts Inc.

#ifndef ENCODE_H
#define ENCODE_H

#include "gimex.h"
#include "quantize.h"

namespace Gst
{
    
struct BlockInfo
{
    int cluster;
    int error;
    ARGB *pixel;
    unsigned char *index;
};

class GSTEncoder
{
public:
    int mWidth;
    int mHeight;
    int mBlockWidth;
    int mBlockHeight;
    int mDetailBpp;
    int mBaseWidth;
    int mBaseHeight;
    int mImageSize;
    int mBaseSize;
    int mBlockSize;
    int mDetailColours;

    ARGB *mImage;
    BlockInfo *mBlock;
    unsigned char *mPaletteIndex;
    unsigned char mPalette[256][4];
    int mClusterUsage[128];

    int mClusters;

    GSTEncoder(ARGB *image, int width, int height, int blockwidth, int blockheight, int detailbpp);
    ~GSTEncoder();

    void  EstimateBase(float chromaWeight, float lumaWeight, float alphaWeight, float outlierThreshold);
    void  ComputePalette();
    float ComputeDetail();
    void  ImproveMapping(float errorThreshold);

private:
    GSTEncoder() {};

    float mChromaWeight;
    float mLumaWeight;
    float mAlphaWeight;

    void SetBlockVector(ARGB *pixel, InputVector *v);
    int  FindNearestCluster(unsigned char val[4], Cluster *clust);
    int  FindBestSubpalette(ARGB *pixel);
    void SetBlockCluster(BlockInfo *block, int cluster);
    void UpdateDetail(BlockInfo *block);
};
 
}

#endif
