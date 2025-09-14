// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3CoordUtil.cxx
 *
 *  Contains utility functions for operations on coordinates.
 */

#include "EA3Utilities.h"
#include "EA3RayTrace.h"
#include "EA3CoordUtil.h"
#include "EA3GeomUtil.h"
#include "EA3AnimUtil.h"
#include "EA3SkelUtil.h"
#include "EA3Expr.h"
#include <eastl/vector.h>

using namespace EA3Expr;

// The largest weighting that will produce any effect
#define WEIGHT_THRESH 0.005

namespace EA3Util {

/*** EA3 RECORDED GEOMETRY *********************************************************/



EA3RecordedGeometry::EA3RecordedGeometry(EA3Block *s,EA3Block *a,EA3Block *g) {
  rs    = new EA3RecordedSkeleton(s,a);
  doweownrs = true;
  geom  = g;
  Initialize();
}


EA3RecordedGeometry::EA3RecordedGeometry(EA3RecordedSkeleton *_rs,EA3Block *g) {
  rs        = _rs;
  doweownrs = false;
  geom      = g;
  Initialize();
}


EA3RecordedGeometry::~EA3RecordedGeometry() {
  int i;
  if( rs && doweownrs ) delete rs;
  for( i=0; i<weightedbonelist.getSize(); i++ ) {
    EA3WeightedBone *wb = weightedbonelist[i];
    EA3WeightedBone *tmp;
    while( wb ) {
      tmp = wb;
      wb = wb->next;
      delete tmp;
    }
  }
  weightedbonelist.setSize(0);
}

void EA3RecordedGeometry::Initialize() {
  EA3Block *model = geom;
  while( model && model->getType()!=XBmodel ) model = model->getContainer();
  if( model ) {
    // COORDS
    coords = geom->getByType(XBcoord);
    // WORLD TRANSFORMS
    EA3Block *skel = NULL;
    geomGetSkeleton(geom,skel,rs->skel);
    masterbone = findSkeleton(rs, skel);
    if( !masterbone ) {
      masterbone = rs;
#ifdef DEBUG
      printf("Using root skeleton as geometric node!!");
#endif
    }
    // MORPHING
    morphTargets = geom->getByType(XBmorphTarget);
    EA3Array *anim = model->getByType(XBanimation);
    if (anim && morphTargets) {
      find(anim,morphChannels,
        HasMember(
          HasType("what") && 
          HasValue("morphWeight")
        ) && 
        (HasName(morphTargets->getName()) || HasName(geom->getName()))
      );
    }
    // SKINNING
    EA3Block *envelopes = model->getByType(XBenvelope);
    int cnt1,cnt2;
    if( envelopes ) {
      EA3Block *clusters = envelopes->getByName(geom->getName())->toBlock()->getByType(XBcluster);
      if( clusters ) {
        weightedbonelist.setSize(coords->getNumMembers());
        for( cnt1=0; cnt1<weightedbonelist.getSize(); cnt1++ ) weightedbonelist[cnt1] = NULL;
        EA3Block *cl;
        for( cnt1=0; cnt1<clusters->getNumMembers(); cnt1++ ) {
          cl = clusters->getByIndex(cnt1);
          EA3Reference *coordref = cl->getByType(XBcoord);
          if( !coordref ) continue;
          EA3Array *weights = cl->getByType(XBweight);
          EA3Reference *boneref = cl->getByType(XBbone);
          EA3RecordedSkeleton *bone = NULL;
          if( boneref ) {
            bone = findSkeleton(rs,boneref->getReference(0)->getContainer());
          } else {
            XbString bonename = boneref?(boneref->getReference(0)->getName()):(cl->getName());
            bone = findSkeleton(rs,bonename);
          }
          if( !bone ) {
            // ERROR!!!
            continue;
          }
          EA3WeightedBone *wb;
          for( cnt2=0; cnt2<coordref->getNumReferences(); cnt2++ ) {
            if( coordref->getReference(cnt2)->getContainer()==coords ) {
              // This coord is one of ours!!
              int index = coordref->getReference(cnt2)->getIndex();
              EA3Field *weight = weights->getByIndex(cnt2);
              if( !weight ) continue;
              wb = new EA3WeightedBone;
              wb->bone = bone;
              wb->weight = weight->data[0].f;
              wb->next = weightedbonelist[index];
              weightedbonelist[index] = wb;
            }
          }
        }
      }
    }
  }
}

void EA3RecordedGeometry::getCoords( Float3 *mycrds ) {
  int i;
  EA3Field *f;
  for( i=getNumCoords()-1; i>=0; i-- ) {
    f = coords->getByIndex(i);
    mycrds[i][0] = f->data[0].f;
    mycrds[i][1] = f->data[1].f;
    mycrds[i][2] = f->data[2].f;
  }
}

void EA3RecordedGeometry::applymorphing(Float3 *mycrds, const int frame) {
  if( !morphTargets || !mycrds || !rs ) return;
  
  int i,mc_cnt;
  EA3Field *whatfield;
  EA3Block *morphTarget;
  for( mc_cnt=0; mc_cnt<morphChannels.getSize(); mc_cnt++ ) {
    EA3Block *chan = morphChannels[mc_cnt]->toBlock();
    if (!chan) continue;
    whatfield = chan->getByType(XBwhat);
    if( !whatfield || strcmp(whatfield->data[0].str,"morphWeight")!=0 ) continue;
    int index = chan->getByType(XBindex)->toField()->data[0].i;
    morphTarget = morphTargets->getByIndex(index);
    EA3Array *data = chan->getByType(XBdata);     
    float w = 0.0f;
    if( data ) {
      if (frame >= 0 && frame < data->getNumMembers() ) {
        w = data->getByIndex(frame)->toField()->data[0].f;
      }
    } else {
      float FPS = 30.0f;
      animGetFPS(rs->anim,FPS);
      float time = float(frame)/FPS;
      fcrvEval(chan,time,w);
    }
    if( w<WEIGHT_THRESH ) continue;
    EA3Field *morphCoord;
    
    
    EA3Block *crdDeltas = morphTarget->getByType(XBcoordDelta);
    EA3Reference *morphCoordRef = morphTarget->getByType(XBcoord);
    XbPArray< int > crdfinder;
    EA3Field *coordSysField = morphTarget->getByType(XBcoordSys);
    if( crdDeltas ) {
      // Delta
      for(i=0;i<morphCoordRef->getNumReferences();i++) {
        EA3Field *coord = morphCoordRef->getReference(i);
        if (!coord) continue;
        morphCoord = crdDeltas->getByIndex(i);
        int ix = coord->getIndex();
        mycrds[ix][0] += morphCoord->data[0].f * w;
        mycrds[ix][1] += morphCoord->data[1].f * w;
        mycrds[ix][2] += morphCoord->data[2].f * w;
      }
    } else {
      // Target
      EA3Array *morphCoordArray = morphCoordRef->getReference(0);
      if( coordSysField && coordSysField->data[0].str == "world" ) {
        EA3Block *tgeom = morphCoordArray->getContainer();
        EA3RecordedGeometry *trg = new EA3RecordedGeometry(rs,tgeom);
        Float3 *tcoords = new Float3[trg->getNumCoords()];
        trg->EvaluateGlobalCoords(tcoords,frame);
        //EA3RecordedSkeleton *mb = trg->getMasterBone();
        //const char *same1 = mb->skel->getName().getString();
        //const char *same2 = tgeom->getName().getString();
        int minnumcoords = getNumCoords();
        if( trg->getNumCoords()<minnumcoords ) minnumcoords=trg->getNumCoords();
        for( i=minnumcoords-1; i>=0; i-- ) {
            EA3Field *origCoord = coords->getByIndex(i);
            mycrds[i][0] += (tcoords[i][0]-origCoord->data[0].f)*w;
            mycrds[i][1] += (tcoords[i][1]-origCoord->data[1].f)*w;
            mycrds[i][2] += (tcoords[i][2]-origCoord->data[2].f)*w;
        }
        delete[] tcoords;
        delete trg;
      } else {
        if (morphCoordArray && morphCoordArray->getNumMembers() == getNumCoords()) {
          for( i=getNumCoords()-1; i>=0; i-- ) {
            morphCoord = morphCoordArray->getByIndex(i);
            EA3Field *origCoord = coords->getByIndex(i);
            mycrds[i][0] += (morphCoord->data[0].f-origCoord->data[0].f)*w;
            mycrds[i][1] += (morphCoord->data[1].f-origCoord->data[1].f)*w;
            mycrds[i][2] += (morphCoord->data[2].f-origCoord->data[2].f)*w;
          }
        }
      }
    }
  }
}

void EA3RecordedGeometry::trnToWorld(Float3 *mycrds, const int frame) {
  if( !masterbone || !mycrds || !rs ) return;

  //transform to world
  int i;
  XbMatrix M; M.makeIdentity();
  XbVec3f result;
  if( rs->anim && rs->numFrames ) {
    masterbone->getAnimGlobal(M, frame);
  } else {
    masterbone->getGlobal(M);
  }
  for( i=getNumCoords()-1; i>=0; i-- ) {
    XbVec3f vect(mycrds[i]);
    M.multVecMatrix(vect, result);
    mycrds[i][0] = result[0];
    mycrds[i][1] = result[1];
    mycrds[i][2] = result[2];
  }
}

void EA3RecordedGeometry::applySkin(Float3 *mycrds, const int frame) {
  if( (weightedbonelist.getSize()<getNumCoords()) || !mycrds || !rs ) return;

  //
  // VG  = skinned global vertex
  // vg  = geometry global vertex
  // Igi = init global matrix for weighted bone i
  // Mgi = animation global matrix for weighted bone i
  // Mi  = inverse(Igi)*Mgi = delta matrix for weighted bone i
  // wi  = weighting for bone i
  // MD  = delta matrix from all weighting info
  // I   = identity matrix
  //
  // VG = vg[ sum(wi(Mi-I)) + I ] = vg*MD
  //    = vg*sum(Mi*wi) + (1-sum(wi))*vg
  //
  /*
  * \par FORMULA:
  * \f[
  *   M_i = Ig_{i}^{-1} Mg_{i}
  * \f]
  * \f[
  *   V_{g} = v_{g}
  *        \left\[
  *          \sum_{i=0}^{# weighted bones}
  *            \left\(
  *              w_{i} \left\( M_{i} - I_4 \right\)
  *            \right\) + I_4
  *        \right\]
  * \f]
  */
  
  // apply skinning
  int i;
  XbMatrix M;
  XbVec3f result;
  EA3WeightedBone *wb;
  for( i=getNumCoords()-1; i>=0; i-- ) {
    wb = weightedbonelist[i];
    
    XbMatrix MD;
    MD.makeIdentity();
    XbMatrix Igi, Mgi;
    while( wb ) {
      
      wb->bone->getInitGlobalInverse(Igi);
      if( rs->anim && rs->numFrames ) {
        wb->bone->getAnimGlobal(Mgi,frame);
      } else {
        wb->bone->getGlobal(Mgi);
      }
      XbMatrix Mi = Igi * Mgi;
      
      Mi[0][0] -= 1.0f;
      Mi[1][1] -= 1.0f;
      Mi[2][2] -= 1.0f;
      Mi[3][3] -= 1.0f;
      
      MD[0][0] += Mi[0][0] * wb->weight;
      MD[0][1] += Mi[0][1] * wb->weight;
      MD[0][2] += Mi[0][2] * wb->weight;
      MD[0][3] += Mi[0][3] * wb->weight;
      MD[1][0] += Mi[1][0] * wb->weight;
      MD[1][1] += Mi[1][1] * wb->weight;
      MD[1][2] += Mi[1][2] * wb->weight;
      MD[1][3] += Mi[3][3] * wb->weight;
      MD[2][0] += Mi[2][0] * wb->weight;
      MD[2][1] += Mi[2][1] * wb->weight;
      MD[2][2] += Mi[2][2] * wb->weight;
      MD[2][3] += Mi[2][3] * wb->weight;
      MD[3][0] += Mi[3][0] * wb->weight;
      MD[3][1] += Mi[3][1] * wb->weight;
      MD[3][2] += Mi[3][2] * wb->weight;
      MD[3][3] += Mi[3][3] * wb->weight;
      
      wb = wb->next;
    }
    // apply the coordinate matrix to the result
    XbVec3f vect(mycrds[i]);
    MD.multVecMatrix(vect, result);
    mycrds[i][0] = result[0];
    mycrds[i][1] = result[1];
    mycrds[i][2] = result[2];
  }
}

void EA3RecordedGeometry::EvaluateGlobalCoords( Float3 *mycrds, const int frame ) {
  int realframe = frame;
  if (frame < -2 || frame > rs->numFrames-1) realframe = -1;
  getCoords(mycrds);
  applymorphing(mycrds,realframe);
  trnToWorld(mycrds,realframe);
  applySkin(mycrds,realframe);
}

XbMatrix EA3RecordedGeometry::getCoordMatrix(const int index, const int frame)
{
  XbMatrix Mvi;
  int realframe = rs->numFrames?abs(frame % rs->numFrames):-1; // a negative 1 frame will force the rs to return the "local" matrix
  // Get the initial skeleton matrix
  masterbone->getAnimGlobal(Mvi, realframe);
  // Apply the skinned matrix
  if( index < weightedbonelist.getSize() ) {
    EA3WeightedBone *wb = weightedbonelist[index];  
    XbMatrix MD;
    MD.makeIdentity();
    XbMatrix Igi, Mgi;
    while( wb ) {    
      wb->bone->getInitGlobalInverse(Igi);
      if( rs->anim && rs->numFrames ) {
        wb->bone->getAnimGlobal(Mgi,realframe);
      } else {
        wb->bone->getGlobal(Mgi);
      }
      XbMatrix Mi = Igi * Mgi;
      
      Mi[0][0] -= 1.0f;
      Mi[1][1] -= 1.0f;
      Mi[2][2] -= 1.0f;
      Mi[3][3] -= 1.0f;
      
      MD[0][0] += Mi[0][0] * wb->weight;
      MD[0][1] += Mi[0][1] * wb->weight;
      MD[0][2] += Mi[0][2] * wb->weight;
      MD[0][3] += Mi[0][3] * wb->weight;
      MD[1][0] += Mi[1][0] * wb->weight;
      MD[1][1] += Mi[1][1] * wb->weight;
      MD[1][2] += Mi[1][2] * wb->weight;
      MD[1][3] += Mi[3][3] * wb->weight;
      MD[2][0] += Mi[2][0] * wb->weight;
      MD[2][1] += Mi[2][1] * wb->weight;
      MD[2][2] += Mi[2][2] * wb->weight;
      MD[2][3] += Mi[2][3] * wb->weight;
      MD[3][0] += Mi[3][0] * wb->weight;
      MD[3][1] += Mi[3][1] * wb->weight;
      MD[3][2] += Mi[3][2] * wb->weight;
      MD[3][3] += Mi[3][3] * wb->weight;
      
      wb = wb->next;
    }
    Mvi.multLeft(MD);
  }
  
  return Mvi;
}

void EA3RecordedGeometry::getMorphTargetGeometries(XbPArray<EA3Block*> &mtgeoms) {
  if( !morphTargets ) return;
  int i;
  for( i=0; i<morphTargets->getNumMembers(); i++ ) {
    EA3Block *mt = morphTargets->getByIndex(i);
    if( mt->getByType(XBcoordDelta) ) continue;
    EA3Reference *crds = mt->getByType(XBcoord);
    if( crds ) {
      EA3Array *coordarray = crds->getReference(0)->toArray();
      if( coordarray && coordarray->getContainer()->getType()==XBgeometry ) {
        mtgeoms.append(coordarray->getContainer());
      }
    }
  }
}

EA3RecordedGeometry::EA3WeightedBone* EA3RecordedGeometry::getBoneList(int vertex){
  if (weightedbonelist.getSize() <= vertex) return 0;
  return weightedbonelist[vertex];
}

namespace RayTrace {

inline float Unit(XbVec3f *v) {
  float len = v->length();
  if (len < 0.00001) return(-1.0f);
  *v *= (1.0f/len);
  return (len);
}

void testRayTrace(EA3Scene *s) {
  EA3Block *model = s->getByType(XBmodel);
  if (!model) return;
  XbPArray<TriangleCache *> trilist;
  XbPArray<EA3Base *> glist;
  int i;
  find(model,glist,HasType("geometry")&&!IsArray()&&IsBlock());
  EA3Block *anim = model->getByType(XBanimation)->toArray()->getByIndex(0);
  printf("anim = %x\n",(unsigned int)(uintptr_t)anim);
  for(int g=0;g<glist.getSize();g++) {
    EA3Block *geom = glist[g]->toBlock();
    EA3RecordedGeometry rg(model,anim,geom);
    XbPArray<EA3Base *> geoPrims;
    find(geom,geoPrims,HasType("triMesh"));
    for(i=0;i<geoPrims.getSize();i++) {
      getTriangleCaches(&rg,3,geoPrims[i]->toBlock(),trilist);
    }
  }
  Ray r;
  r.shadow = false;
  const float REALLY_FAR = 100000.0f;
  r.origin = XbVec3f(0,0,-30);
  FILE *f = fopen("d:/temp/i.txt","w");
  fprintf(f,"{\n");
  XbEuler eul(XbVec3f(0,1.5,0),XbEuler::XYZ,XbEuler::STATIC);
  XbMatrix mat(eul);
  int hits = 0;
  for(float y=38.0f;y>-38.0f;y-=0.2f) {
    fprintf(f,"{");
    fprintf(stderr,"\r%f %d        ",y,hits);
    for(float x=-38.0f;x<38.0f;x+=0.2f) {
      HitData ht; 
      ht.distance = REALLY_FAR;
      ht.layer.resize(1);
      XbVec3f Dir(1,0,0);
      float SCALE = 0.2f;
      XbVec3f Orig(-1000.0f,y*SCALE+7.0f,x*SCALE+6.0f);
      mat.multVecMatrix(Orig,r.origin);
      mat.multDirMatrix(Dir,r.dir);
      r.dir.normalize();
      for(i=0;i<trilist.getSize();i++) {
        trilist[i]->rayCast(&r,&ht);
      }
      float R=1.0f,G=1.0f,B=1.0f;
      float intens = 0.0f;
      if (ht.distance < REALLY_FAR) {
        hits++;
        intens = ht.layer[0].normal.dot(r.dir);
        EA3Reference *matr = ht.tri->geoPrim->getByType(XBmaterial);
        if (matr) {
          EA3Block *mtl = matr->getReference(0);
          if (mtl) {
            EA3Field *dif = mtl->getByType(XBdiffuse);
            if (dif) {
              R = dif->data[0].f;
              G = dif->data[1].f;
              B = dif->data[2].f;
            }
          }
        }
        int u = int(8.0f*ht.layer[0].uv[0]) % 2;
        int v = int(8.0f*ht.layer[0].uv[1]) % 2;
        if ((u && v) || (!u && !v)) {
          intens = intens*0.3f;
        }
      }
      unsigned int colR = (unsigned int)fabs(intens*R*256.0);
      unsigned int colG = (unsigned int)fabs(intens*G*256.0);
      unsigned int colB = (unsigned int)fabs(intens*B*256.0);
      if (colR > 255) colR = 255;
      if (colG > 255) colG = 255;
      if (colB > 255) colB = 255;
      fprintf(f,"#%.2x%.2x%.2x ",colR,colG,colB);
    }
    fprintf(f,"}\n");
  }
  fprintf(f,"}\n");
  fclose(f);
};

int getTriangleCaches (EA3RecordedGeometry *rg,int frame,EA3Block *geoPrim,XbPArray<TriangleCache*> &tlist) {
	/* 
	 * Get three vectors that are orthogonal, the first two are on 
	 * the triangle plane.  V1 lies on an edge of the triangle while
	 * V2 is perpendicular to V1, and the origin lies on vertex 0.
	 */
  EA3Reference *crd = geoPrim->getByType(XBcoord);
  EA3Reference *nrm = geoPrim->findByType(XBnormal);
  EA3Reference *uvRef = geoPrim->findByType(XBuv);
  if (!crd || !nrm) return(0);
  // convert to tris...
  // call update
  XbIdentifier type = geoPrim->getType();
  XbMatrix gmat;
  if (type == XBtriMesh) {
    for(int i = 0;i<crd->getNumReferences();i += 3) {
      TriangleCache *tri = new TriangleCache(rg,geoPrim);
      for (int j=0;j<3;j++) {
        EA3Field *c = crd->getReference(i+j);
        EA3Field *n = nrm->getReference(i+j);
        EA3Field *uv = uvRef? uvRef->getReference(i+j)->toField() : 0;
        int ix = c->getIndex();
        gmat = rg->getCoordMatrix(ix,frame);
        XbVec3f coord(c->data[0].f,c->data[1].f,c->data[2].f);
        gmat.multVecMatrix(coord,tri->vert[j]);
        XbVec3f normal(n->data[0].f,n->data[1].f,n->data[2].f);
        tri->layer.resize(1);
        if (uv) {
          tri->layer[0].uv[j][0] = uv->data[0].f;
          tri->layer[0].uv[j][1] = uv->data[1].f;
          tri->layer[0].uv[j][2] = uv->data[2].f;
        } else {
          tri->layer[0].uv[j][0] = 0.0f;
          tri->layer[0].uv[j][1] = 0.0f;
          tri->layer[0].uv[j][2] = 0.0f;
        }
        gmat.multDirMatrix(normal,tri->layer[0].normal[j]);
      }
      tri->update();
      tlist.append(tri);
    }
  }
  return(1);
}
int TriangleCache::update() {
  float a, b, c;
	float colinear;
	int i;
  TriangleCache *tri = this;
	tri->v1.norm = tri->vert[1]-tri->vert[0];
	if(Unit(&tri->v1.norm) < 0.)
		{
    delete tri;
		return(0);
		}

	tri->v2.norm = tri->vert[2] -tri->vert[0];
	if(Unit(&tri->v2.norm) < 0.)
		{
    delete tri;
		return(0);
		}

	colinear= tri->v1.norm.dot(tri->v2.norm);
	if(colinear > LIMIT_TRIANGLE_P || colinear < -LIMIT_TRIANGLE_P)
		{
    delete tri;
		return(0);
		}

	/* Surface normal */
	tri->flat.norm= tri->v1.norm.cross(tri->v2.norm);
	Unit(&tri->flat.norm);

	tri->v2.norm= tri->flat.norm.cross(tri->v1.norm);
	Unit(&tri->v2.norm);
	tri->v1.d= -tri->vert[0].dot(tri->v1.norm);
	tri->v2.d= -tri->vert[0].dot(tri->v2.norm);
	tri->flat.d= -tri->vert[0].dot(tri->flat.norm);

	/* 
	 * Generate 2x2 matrix to convert from orth 
	 * space to interp space; see diagram in tech doc. 
	 */
	a= tri->v1.norm.dot(tri->vert[1]) + tri->v1.d;
	b= tri->v1.norm.dot(tri->vert[2]) + tri->v1.d;
	c= tri->v2.norm.dot(tri->vert[2]) + tri->v2.d;
	tri->transform[0]= 1.f/a;
	tri->transform[1]= -b/(a*c);
	tri->transform[2]= 0.f;
	tri->transform[3]= 1.f/c;

	/* normals cannot be specified when using stainedglass windows */
	if(tri->layer.size())
		{
		tri->changingNormal= true;
		for(i=0;i<3;i++)
			{
			if(Unit(&tri->layer[0].normal[i]) < 0.)
				{
				printf("Sorry, couldn't normalize normals.\n");
				delete tri;
				return(false);
				}
			}
		}
	else	tri->changingNormal= false;


	
	return(true);
	}


/*
 * Intersect the ray with the object in np.  If there is an intersection and
 * it is closer than the intersection described in hit then update hit else
 * return.	Hit needs information about
 *	The object				np
 *	The distance along the ray.		distance
 *	The intersection point			intersect
 *	The texture				texture
 *	The normal				normal
 *	The tangent				tangent
 */

void TriangleCache::rayCast(Ray *ray, HitData *hit)
	{
  TriangleCache *tri = this;
	float u, v;		/* u/v = distance from viewer to plane */
	float t;		/* hit distance to the plane equation of tri */
	float s1, s2;		/* orth space coordinates */
	float t1, t2;		/* interp space coordinates */
	float sum;		/* vertex 0 contribution to interp normal */
	Plane Flat;		/* plane equation for triangle */
	XbVec3f intersect;	/* intersection point in world coord */
		
	Flat= tri->flat;
	u= Flat.norm.dot(ray->origin) + Flat.d;
	v= Flat.norm.dot(ray->dir);

	if ((u > 0. && v < -VERY_SMALL) || (u < 0. && v > VERY_SMALL))
		{
		/* Check if intersection with plane is behind viewer */
		t= -u/v;
		if(t > MIN_DISTANCE && t < hit->distance)
			{
			intersect = ray->origin + t*ray->dir;

			/* Transform into interp space */
			s1= intersect.dot(tri->v1.norm) + tri->v1.d;
			s2= intersect.dot(tri->v2.norm) + tri->v2.d;
			t1= s1*tri->transform[0] + s2*tri->transform[1];
			t2= s1*tri->transform[2] + s2*tri->transform[3];

			if(t1 >= 0. && t2 >= 0. && t1 + t2 <= 1.)
				{
				hit->distance= t;
				hit->tri= tri;
				hit->intersect= intersect;
				if(!ray->shadow)
					{
            //if (1) { // has texture
						sum= 1 - t1 - t2;
						
						hit->layer[0].uv[0]= 
							sum*tri->layer[0].uv[0][0] + 
							t1*tri->layer[0].uv[1][0] + 
							t2*tri->layer[0].uv[2][0];
						hit->layer[0].uv[1]= 
							sum*tri->layer[0].uv[0][1] + 
							t1*tri->layer[0].uv[1][1] + 
							t2*tri->layer[0].uv[2][1];
            hit->layer[0].uv[2]= 
							sum*tri->layer[0].uv[0][2] + 
							t1*tri->layer[0].uv[1][2] + 
							t2*tri->layer[0].uv[2][2];
						//}
					

					if(tri->changingNormal)
						{
						sum= 1.f - t1 - t2;
						hit->layer[0].normal[0]= 
							sum*tri->layer[0].normal[0][0] + 
							t1*tri->layer[0].normal[1][0] + 
							t2*tri->layer[0].normal[2][0];
						hit->layer[0].normal[1]= 
							sum*tri->layer[0].normal[0][1] + 
							t1*tri->layer[0].normal[1][1] + 
							t2*tri->layer[0].normal[2][1];
						hit->layer[0].normal[2]= 
							sum*tri->layer[0].normal[0][2] + 
							t1*tri->layer[0].normal[1][2] + 
							t2*tri->layer[0].normal[2][2];
						}
					else	hit->layer[0].normal = Flat.norm;
					}
				}
			}
		}
	}
} // namespace




} // namespace

