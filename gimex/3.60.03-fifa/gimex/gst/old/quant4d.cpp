// (c) 2003-2007 Electronic Arts Inc.

#include <math.h>

#include "helper.h"
#include "quantize.h"

namespace Gst
{
    
static const int BUCKETCOUNT = 500;

struct ClusterInfo
{
    int min[4];
    int max[4];
    float cov[4][4];
};

struct BucketInfo
{
    int count;
    double sum[4];
    double sum2;
};


static bool GatherClusterInfo(InputVector v[], int count, ClusterInfo *ci)
{
    int min0 = v[0].val[0];
    int min1 = v[0].val[1];
    int min2 = v[0].val[2];
    int min3 = v[0].val[3];
    int max0 = v[0].val[0];
    int max1 = v[0].val[1];
    int max2 = v[0].val[2];
    int max3 = v[0].val[3];

    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    double s3 = 0.0;
    double s00 = 0.0;
    double s01 = 0.0;
    double s02 = 0.0;
    double s03 = 0.0;
    double s11 = 0.0;
    double s12 = 0.0;
    double s13 = 0.0;
    double s22 = 0.0;
    double s23 = 0.0;
    double s33 = 0.0;

    for (int i = 0; i < count; ++i)
    {
        int i0 = v[i].val[0];
        int i1 = v[i].val[1];
        int i2 = v[i].val[2];
        int i3 = v[i].val[3];
        min0 = IntMin(min0, i0);
        min1 = IntMin(min1, i1);
        min2 = IntMin(min2, i2);
        min3 = IntMin(min3, i3);
        max0 = IntMax(max0, i0);
        max1 = IntMax(max1, i1);
        max2 = IntMax(max2, i2);
        max3 = IntMax(max3, i3);

        double x0 = i0;
        double x1 = i1;
        double x2 = i2;
        double x3 = i3;
        s0 += x0;
        s1 += x1;
        s2 += x2;
        s3 += x3;
        s00 += x0*x0;
        s01 += x0*x1;
        s02 += x0*x2;
        s03 += x0*x3;
        s11 += x1*x1;
        s12 += x1*x2;
        s13 += x1*x3;
        s22 += x2*x2;
        s23 += x2*x3;
        s33 += x3*x3;
    }

    ci->min[0] = min0;
    ci->min[1] = min1;
    ci->min[2] = min2;
    ci->min[3] = min3;
    ci->max[0] = max0;
    ci->max[1] = max1;
    ci->max[2] = max2;
    ci->max[3] = max3;

    double n = count;
    double rn2 = 1.0/(n*n);
    ci->cov[0][0] = float((n*s00 - s0*s0)*rn2);
    ci->cov[0][1] = float((n*s01 - s0*s1)*rn2);
    ci->cov[0][2] = float((n*s02 - s0*s2)*rn2);
    ci->cov[0][3] = float((n*s03 - s0*s3)*rn2);
    ci->cov[1][1] = float((n*s11 - s1*s1)*rn2);
    ci->cov[1][2] = float((n*s12 - s1*s2)*rn2);
    ci->cov[1][3] = float((n*s13 - s1*s3)*rn2);
    ci->cov[2][2] = float((n*s22 - s2*s2)*rn2);
    ci->cov[2][3] = float((n*s23 - s2*s3)*rn2);
    ci->cov[3][3] = float((n*s33 - s3*s3)*rn2);
    ci->cov[1][0] = ci->cov[0][1];
    ci->cov[2][0] = ci->cov[0][2];
    ci->cov[3][0] = ci->cov[0][3];
    ci->cov[2][1] = ci->cov[1][2];
    ci->cov[3][1] = ci->cov[1][3];
    ci->cov[3][2] = ci->cov[2][3];

    return(ci->cov[0][0]+ci->cov[1][1]+ci->cov[2][2]+ci->cov[3][3] > 0.0f);
}

static void ComputePrincipalAxis(ClusterInfo &ci, float axis[4])
{
    const int ITERATIONS = 20;

    int maxpos = 0;
    int maxval = ci.max[0]-ci.min[0];
    for (int i = 1; i < 4; ++i)
    {
        int val = ci.max[i]-ci.min[i];
        if (val > maxval)
        {
            maxpos = i;
            maxval = val;
        }
    }
    for (int i = 0; i < 4; ++i) axis[i] = 0.0f;
    axis[maxpos] = 1.0f;

    for (int i = 0; i < ITERATIONS; ++i)
    {
        float t0 = ci.cov[0][0]*axis[0] + ci.cov[0][1]*axis[1] + ci.cov[0][2]*axis[2] + ci.cov[0][3]*axis[3];
        float t1 = ci.cov[1][0]*axis[0] + ci.cov[1][1]*axis[1] + ci.cov[1][2]*axis[2] + ci.cov[1][3]*axis[3];
        float t2 = ci.cov[2][0]*axis[0] + ci.cov[2][1]*axis[1] + ci.cov[2][2]*axis[2] + ci.cov[2][3]*axis[3];
        float t3 = ci.cov[3][0]*axis[0] + ci.cov[3][1]*axis[1] + ci.cov[3][2]*axis[2] + ci.cov[3][3]*axis[3];
        float scale = (float)(1.0f/sqrt(t0*t0+t1*t1+t2*t2+t3*t3));
        axis[0] = t0*scale;
        axis[1] = t1*scale;
        axis[2] = t2*scale;
        axis[3] = t3*scale;
    }
}

static void FillBuckets(InputVector v[], int count, ClusterInfo &ci, BucketInfo bucket[BUCKETCOUNT])
{
    float axis[4];
    ComputePrincipalAxis(ci, axis);

    float min = 0.0f;
    float max = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        float x = ci.min[i] * axis[i];
        float y = ci.max[i] * axis[i];
        min += FloatMin(x,y);
        max += FloatMax(x,y);
    }
    float scale = (BUCKETCOUNT-1)/(max-min);
    float offset = -min*scale;
    for (int i = 0; i < 4; ++i) axis[i] *= scale;

    for (int i = 0; i < BUCKETCOUNT; ++i)
    {
        bucket[i].count = 0;
        bucket[i].sum[0] = 0.0;
        bucket[i].sum[1] = 0.0;
        bucket[i].sum[2] = 0.0;
        bucket[i].sum[3] = 0.0;
        bucket[i].sum2 = 0.0;
    }

    for (int i = 0; i < count; ++i)
    {
        float x0 = v[i].val[0];
        float x1 = v[i].val[1];
        float x2 = v[i].val[2];
        float x3 = v[i].val[3];

        int n = IntRound(axis[0]*x0+axis[1]*x1+axis[2]*x2+axis[3]*x3+offset);
        v[i].cluster = n;
        bucket[n].count += 1;
        bucket[n].sum[0] += x0;
        bucket[n].sum[1] += x1;
        bucket[n].sum[2] += x2;
        bucket[n].sum[3] += x3;
        bucket[n].sum2 += x0*x0+x1*x1+x2*x2+x3*x3;
    }
}

static int FindSplitPoint(BucketInfo bucket[BUCKETCOUNT])
{
    const double EPSILON = 1e-30;

    int count1 = 0;
    int count2 = 0;
    double sum1[4] = { 0.0, 0.0, 0.0, 0.0 };
    double sum2[4] = { 0.0, 0.0, 0.0, 0.0 };
    for (int i = 0; i < BUCKETCOUNT; ++i)
    {
        count2 += bucket[i].count;
        sum2[0] += bucket[i].sum[0];
        sum2[1] += bucket[i].sum[1];
        sum2[2] += bucket[i].sum[2];
        sum2[3] += bucket[i].sum[3];
    }

    int maxpos = 0;
    double maxval = (sum2[0]*sum2[0]+sum2[1]*sum2[1]+sum2[2]*sum2[2]+sum2[3]*sum2[3])/(count2+EPSILON);

    for (int i = 1; i < BUCKETCOUNT; ++i)
    {
        int count = bucket[i-1].count;
        double *sum = bucket[i-1].sum;
        count1 += count;
        count2 -= count;
        sum1[0] += sum[0];
        sum1[1] += sum[1];
        sum1[2] += sum[2];
        sum1[3] += sum[3];
        sum2[0] -= sum[0];
        sum2[1] -= sum[1];
        sum2[2] -= sum[2];
        sum2[3] -= sum[3];

        double val = (sum1[0]*sum1[0]+sum1[1]*sum1[1]+sum1[2]*sum1[2]+sum1[3]*sum1[3])/(count1+EPSILON) +
                     (sum2[0]*sum2[0]+sum2[1]*sum2[1]+sum2[2]*sum2[2]+sum2[3]*sum2[3])/(count2+EPSILON);
        if (val > maxval)
        {
            maxpos = i;
            maxval = val;
        }
    }

    return(maxpos);
}

static void UpdateClusterStats(BucketInfo bucket[BUCKETCOUNT], int splitpoint, Cluster *c1, Cluster *c2)
{
    int count = 0;
    double sum[4] = { 0.0, 0.0, 0.0, 0.0 };
    double sum2 = 0.0;
    for (int i = 0; i < splitpoint; ++i)
    {
        count += bucket[i].count;
        sum[0] += bucket[i].sum[0];
        sum[1] += bucket[i].sum[1];
        sum[2] += bucket[i].sum[2];
        sum[3] += bucket[i].sum[3];
        sum2 += bucket[i].sum2;
    }
    c1->error = float(sum2 - (sum[0]*sum[0]+sum[1]*sum[1]+sum[2]*sum[2]+sum[3]*sum[3])/(count));
    c1->val[0] = static_cast<unsigned char>(IntRound(float(sum[0]/count)));
    c1->val[1] = static_cast<unsigned char>(IntRound(float(sum[1]/count)));
    c1->val[2] = static_cast<unsigned char>(IntRound(float(sum[2]/count)));
    c1->val[3] = static_cast<unsigned char>(IntRound(float(sum[3]/count)));

    count = 0;
    sum[0] = 0.0;
    sum[1] = 0.0;
    sum[2] = 0.0;
    sum[3] = 0.0;
    sum2 = 0.0;
    for (int i = splitpoint; i < BUCKETCOUNT; ++i)
    {
        count += bucket[i].count;
        sum[0] += bucket[i].sum[0];
        sum[1] += bucket[i].sum[1];
        sum[2] += bucket[i].sum[2];
        sum[3] += bucket[i].sum[3];
        sum2 += bucket[i].sum2;
    }
    c2->error = float(sum2 - (sum[0]*sum[0]+sum[1]*sum[1]+sum[2]*sum[2]+sum[3]*sum[3])/(count));
    c2->val[0] = static_cast<unsigned char>(IntRound(float(sum[0]/count)));
    c2->val[1] = static_cast<unsigned char>(IntRound(float(sum[1]/count)));
    c2->val[2] = static_cast<unsigned char>(IntRound(float(sum[2]/count)));
    c2->val[3] = static_cast<unsigned char>(IntRound(float(sum[3]/count)));
}

static int SortVectors(InputVector v[], int count, int splitpoint, int cluster1, int cluster2)
{
    int i1 = 0;
    int i2 = count-1;
    while (i1 <= i2)
    {
        if (v[i1].cluster < splitpoint)
        {
            v[i1].cluster = cluster1;
            ++i1;
        }
        else
        {
            InputVector temp = v[i1];
            v[i1] = v[i2];
            v[i2] = temp;
            v[i2].cluster = cluster2;
            --i2;
        }
    }

    return(i1);
}

static bool SplitCluster(InputVector v[], Cluster c[], int cluster1, int cluster2)
{
    int startIndex = c[cluster1].startIndex;
    int endIndex = c[cluster1].endIndex;
    int count = endIndex-startIndex+1;

    ClusterInfo ci;
    if (!GatherClusterInfo(&v[startIndex], count, &ci)) return(false);

    BucketInfo bucket[BUCKETCOUNT];
    FillBuckets(&v[startIndex], count, ci, bucket);

    int splitpoint = FindSplitPoint(bucket);

    int count1 = SortVectors(&v[startIndex], count, splitpoint, cluster1, cluster2);
    c[cluster1].endIndex = startIndex+count1-1;
    c[cluster2].startIndex = startIndex+count1;
    c[cluster2].endIndex = endIndex;
    UpdateClusterStats(bucket, splitpoint, &c[cluster1], &c[cluster2]);

    return(true);
}


int Cluster4DVectors(InputVector v[], int count, Cluster c[], int maxclusters)
{
    if (maxclusters < 2) return(1);

    c[0].startIndex = 0;
    c[0].endIndex = count-1;
    if (!SplitCluster(v, c, 0, 1)) return(1);

    int clusters = 2;
    while (clusters < maxclusters)
    {
        int maxpos = 0;
        float maxval = c[0].error;
        for (int i = 1; i < clusters; ++i)
        {
            float val = c[i].error;
            if (val > maxval)
            {
                maxpos = i;
                maxval = val;
            }
        }
        if (maxval == 0.0f) break;

        SplitCluster(v, c, maxpos, clusters);
        ++clusters;
    }

    return(clusters);
}
}
