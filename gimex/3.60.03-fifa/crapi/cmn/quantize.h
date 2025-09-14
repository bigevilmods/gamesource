// (c) 2003-2007 Electronic Arts Inc.

#ifndef QUANTIZE_H
#define QUANTIZE_H

struct InputVector
{
    unsigned char val[4];
    int id;
    int cluster;
};

struct Cluster
{
    unsigned char val[4];
    int startIndex;
    int endIndex;
    float error;
};

int Cluster3DVectors(InputVector v[], int count, Cluster c[], int maxclusters);
int Cluster4DVectors(InputVector v[], int count, Cluster c[], int maxclusters);
int Cluster6DVectors(InputVector v[], int count, Cluster c[], int maxclusters);

#endif
