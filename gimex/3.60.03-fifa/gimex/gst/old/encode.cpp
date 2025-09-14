// (c) 2004-2007 Electronic Arts Inc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "helper.h"
#include "encode.h"

namespace Gst
{
    
void GSTEncoder::SetBlockVector(ARGB *pixel, InputVector *v)
{
    float imin = 255.0f;
    float imax = 0.0f;
    float amin = 255.0f;
    float amax = 0.0f;
    float sumcx = 0.0f;
    float sumcy = 0.0f;
    float sumw = 1e-30f;

    ARGB *p = pixel;
    for (int y = 0; y < mBlockHeight; ++y)
    {
        for (int x = 0; x < mBlockWidth; ++x)
        {
            float r = p->r;
            float g = p->g;
            float b = p->b;
            float a = p->a;

            float i = 0.3333f*(r+g+b);
            float cx = r - 0.5f*(g+b);
            float cy = 0.866f*(g-b);
            float w = (2.0f/(170.0f*85.0f))*i*(255.0f-i);

            if (i < imin) imin = i;
            if (i > imax) imax = i;
            if (a < amin) amin = a;
            if (a > amax) amax = a;
            sumcx += cx;
            sumcy += cy;
            sumw += w;

            ++p;
        }
        p += mWidth-mBlockWidth;
    }

    v->val[0] = static_cast<unsigned char>(IntRound(mChromaWeight*sumcx/sumw)+128);
    v->val[1] = static_cast<unsigned char>(IntRound(mChromaWeight*sumcy/sumw)+128);
    v->val[2] = static_cast<unsigned char>(IntRound(mLumaWeight*imin));
    v->val[3] = static_cast<unsigned char>(IntRound(mLumaWeight*imax));
    v->val[4] = static_cast<unsigned char>(IntRound(mAlphaWeight*amin));
    v->val[5] = static_cast<unsigned char>(IntRound(mAlphaWeight*amax));
}

int GSTEncoder::FindNearestCluster(unsigned char val[6], Cluster *clust)
{
    int minpos = 0;
    int minerr = 999999999;

    for (int i = 0; i < mClusters; ++i)
    {
        int d0 = val[0] - clust[i].val[0];
        int d1 = val[1] - clust[i].val[1];
        int d2 = val[2] - clust[i].val[2];
        int d3 = val[3] - clust[i].val[3];
        int d4 = val[4] - clust[i].val[4];
        int d5 = val[5] - clust[i].val[5];
        int err = d0*d0 + d1*d1 + d2*d2 + d3*d3 + d4*d4 + d5*d5;
        if (err < minerr)
        {
            minpos = i;
            minerr = err;
        }
    }

    return(minpos);
}

int GSTEncoder::FindBestSubpalette(ARGB *pixel)
{
    int bestpos = 0;
    int besterr = 999999999;

    for (int i = 0; i < mClusters; ++i)
    {
        int index = mDetailColours*i;
        int blockError = 0;

        ARGB *p = pixel;
        for (int y = 0; y < mBlockHeight; ++y)
        {
            for (int x = 0; x < mBlockWidth; ++x)
            {
                int minerr = 999999999;
                for (int j = 0; j < mDetailColours; ++j)
                {
                    int dr = p->r - mPalette[index+j][0];
                    int dg = p->g - mPalette[index+j][1];
                    int db = p->b - mPalette[index+j][2];
                    int da = p->a - mPalette[index+j][3];
                    int err = dr*dr + dg*dg + db*db + da*da;
                    if (err < minerr) minerr = err;
                }
                blockError += minerr;
                ++p;
            }
            p += mWidth-mBlockWidth;
        }

        if (blockError < besterr)
        {
            bestpos = i;
            besterr = blockError;
        }
    }

    return(bestpos);
}

void GSTEncoder::SetBlockCluster(BlockInfo *block, int cluster)
{
    --mClusterUsage[block->cluster];
    ++mClusterUsage[cluster];
    block->cluster = cluster;
}

void GSTEncoder::UpdateDetail(BlockInfo *block)
{
    int index = mDetailColours*block->cluster;
    int blockError = 0;

    ARGB *p = block->pixel;
    unsigned char *paletteIndex = block->index;
    for (int y = 0; y < mBlockHeight; ++y)
    {
        for (int x = 0; x < mBlockWidth; ++x)
        {
            int minpos = 0;
            int minerr = 999999999;
            for (int j = 0; j < mDetailColours; ++j)
            {
                int dr = p->r - mPalette[index+j][0];
                int dg = p->g - mPalette[index+j][1];
                int db = p->b - mPalette[index+j][2];
                int da = p->a - mPalette[index+j][3];
                int err = dr*dr + dg*dg + db*db + da*da;
                if (err < minerr)
                {
                    minpos = j;
                    minerr = err;
                }
            }

            *paletteIndex = static_cast<unsigned char>(index+minpos);
            blockError += minerr;

            ++p;
            ++paletteIndex;
        }
        p += mWidth-mBlockWidth;
        paletteIndex += mWidth-mBlockWidth;
    }

    block->error = blockError;
}


GSTEncoder::GSTEncoder(ARGB *image, int width, int height, int blockwidth, int blockheight, int detailbpp)
: mWidth(width),
  mHeight(height),
  mBlockWidth(blockwidth),
  mBlockHeight(blockheight),
  mDetailBpp(detailbpp),
  mImage(image),
  mClusters(0)
{
    mBaseWidth = mWidth/mBlockWidth;
    mBaseHeight = mHeight/mBlockHeight;
    mImageSize = mWidth*mHeight;
    mBaseSize = mBaseWidth*mBaseHeight;
    mBlockSize = mBlockWidth*mBlockHeight;
    mDetailColours = 1 << mDetailBpp;

    mBlock = new BlockInfo[mBaseSize];
    mPaletteIndex = new unsigned char[mImageSize];

    memset(mPaletteIndex, 0, mImageSize);
    memset(mPalette, 0, sizeof(mPalette));
    memset(mClusterUsage, 0, sizeof(mClusterUsage));
    mClusterUsage[0] = mBaseSize;
    for (int y = 0; y < mBaseHeight; ++y)
    {
        for (int x = 0; x < mBaseWidth; ++x)
        {
            int i = y*mBaseWidth+x;
            int i2 = y*mBlockHeight*mWidth+x*mBlockWidth;
            mBlock[i].cluster = 0;
            mBlock[i].error = 0;
            mBlock[i].pixel = &mImage[i2];
            mBlock[i].index = &mPaletteIndex[i2];
        }
    }
}

GSTEncoder::~GSTEncoder()
{
    delete[] mBlock;
    delete[] mPaletteIndex;
}

void GSTEncoder::EstimateBase(float chromaWeight, float lumaWeight, float alphaWeight, float outlierThreshold)
{
    mChromaWeight = chromaWeight;
    mLumaWeight = lumaWeight;
    mAlphaWeight = alphaWeight;

    InputVector *vect = new InputVector[mBaseSize];
    for (int i = 0; i < mBaseSize; ++i)
    {
        vect[i].id = i;
        SetBlockVector(mBlock[i].pixel, &vect[i]);
    }

    mClusters = 256/mDetailColours;
    Cluster *clust = new Cluster[mClusters];
    mClusters = Cluster6DVectors(vect, mBaseSize, clust, mClusters);

    memset(mClusterUsage, 0, sizeof(mClusterUsage));
    mClusterUsage[0] = mBaseSize;
    for (int i = 0; i < mBaseSize; ++i) mBlock[i].cluster = 0;

    if (mClusters > 1)
    {
        float sumError = 0.0f;
        for (int i = 0; i < mClusters; ++i) sumError += clust[i].error;
        float threshold = outlierThreshold*outlierThreshold * sumError/mBaseSize;

        for (int i = 0; i < mBaseSize; ++i)
        {
            InputVector *v = &vect[i];
            Cluster *c = &clust[v->cluster];

            int d0 = v->val[0] - c->val[0];
            int d1 = v->val[1] - c->val[1];
            int d2 = v->val[2] - c->val[2];
            int d3 = v->val[3] - c->val[3];
            int d4 = v->val[4] - c->val[4];
            int d5 = v->val[5] - c->val[5];
            if (float(d0*d0+d1*d1+d2*d2+d3*d3+d4*d4+d5*d5) < threshold)
            {
                SetBlockCluster(&mBlock[v->id], v->cluster);
            }
            else
            {
                int cluster = FindNearestCluster(v->val, clust);
                SetBlockCluster(&mBlock[v->id], cluster);
            }
        }
    }

    delete[] clust;
    delete[] vect;
}

void GSTEncoder::ComputePalette()
{
    int insertPoint[128];
    insertPoint[0] = 0;
    for (int i = 1; i < mClusters; ++i)
        insertPoint[i] = insertPoint[i-1] + mBlockSize*mClusterUsage[i-1];

    InputVector *vect = new InputVector[mImageSize];
    for (int i = 0; i < mBaseSize; ++i)
    {
        int cluster = mBlock[i].cluster;
        int index = insertPoint[cluster];
        ARGB *p = mBlock[i].pixel;
        for (int y = 0; y < mBlockHeight; ++y)
        {
            for (int x = 0; x < mBlockWidth; ++x)
            {
                vect[index].val[0] = p->r;
                vect[index].val[1] = p->g;
                vect[index].val[2] = p->b;
                vect[index].val[3] = p->a;
                ++index;
                ++p;
            }
            p += mWidth-mBlockWidth;
        }
        insertPoint[cluster] = index;
    }

    memset(mPalette, 0, sizeof(mPalette));
    Cluster *clust = new Cluster[mDetailColours];

    int clusterStart = 0;
    for (int i = 0; i < mClusters; ++i)
    {
        int clusterLength = mBlockSize*mClusterUsage[i];
        int colours = Cluster4DVectors(&vect[clusterStart], clusterLength, clust, mDetailColours);
        if (colours == 1)
        {
            int index = mDetailColours*i;
            mPalette[index][0] = vect[clusterStart].val[0];
            mPalette[index][1] = vect[clusterStart].val[1];
            mPalette[index][2] = vect[clusterStart].val[2];
            mPalette[index][3] = vect[clusterStart].val[3];
        }
        else
        {
            for (int j = 0; j < colours; ++j)
            {
                int index = mDetailColours*i+j;
                mPalette[index][0] = clust[j].val[0];
                mPalette[index][1] = clust[j].val[1];
                mPalette[index][2] = clust[j].val[2];
                mPalette[index][3] = clust[j].val[3];
            }
        }
        clusterStart += clusterLength;
    }

    delete[] clust;
    delete[] vect;
}

float GSTEncoder::ComputeDetail()
{
    double sumBlockError = 0.0;

    for (int i = 0; i < mBaseSize; ++i)
    {
        UpdateDetail(&mBlock[i]);
        sumBlockError += mBlock[i].error;
    }

    return(float(sumBlockError/mBaseSize));
}

void GSTEncoder::ImproveMapping(float errorThreshold)
{
    for (int i = 0; i < mBaseSize; ++i)
    {
        if (mBlock[i].error > errorThreshold)
        {
            int newCluster = FindBestSubpalette(mBlock[i].pixel);
            SetBlockCluster(&mBlock[i], newCluster);
            UpdateDetail(&mBlock[i]);
        }
    }
}
}
