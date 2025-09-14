// (c) 2003-2007 Electronic Arts Inc.

#ifndef QUANTIZE_H
#define QUANTIZE_H

namespace Gst
{
    
struct InputVector
{
    int id;
    int cluster;
    unsigned char val[8];
};

struct Cluster
{
    int startIndex;
    int endIndex;
    float error;
    unsigned char val[8];
};

int Cluster3DVectors(InputVector v[], int count, Cluster c[], int maxclusters);
int Cluster4DVectors(InputVector v[], int count, Cluster c[], int maxclusters);
int Cluster6DVectors(InputVector v[], int count, Cluster c[], int maxclusters);
 
}

#endif
