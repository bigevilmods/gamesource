// (c) 1998-2010 Electronic Arts Inc.

/**
 * \file EA3GeomUtil.cxx
 *
 *  Contains utility functions for operations on geometry blocks and geometry
 *  primitives.
 */

#include "EA3Expr.h"
#include "EA3GeomUtil.h"
#include "EA3Utilities.h"
#include "XbArray.h"

namespace EA3Util {
using namespace EA3Expr;

/** @name GEOMETRY FUNCTIONS
 *  Utility functions for working with geometries
 */
//@{

// Get indices via a reference

static int getIndicesViaRef(const EA3Block *b,const XbIdentifier &what, int num,int *ix) {
  EA3Reference *r = (EA3Reference*)b->getByType(what);
  if (!r) return(0);
  int i;
  for(i=0;i<r->getNumReferences() && i<num;i++) {
    EA3Base *f = r->getReference(i);
    if (!f) return(i);
    ix[i] = f->getIndex();
  }
  return(num);
}

//Set indices via a reference

static int setIndicesViaRef(EA3Block *b,EA3Array *array,const XbIdentifier &what, int num,const int *ix) {
  EA3Reference *r = (EA3Reference*)b->getByType(what);
  if (!r) r=b->createReference(what,"");
  r->removeAllReferences();
  int i;
  for(i=0;i<num;i++) {
    EA3Base *base = array->getByIndex(ix[i]);
    if (!base) return(i);
    r->addReference(base);
  }
  return(num);
}

static int setIndicesViaRef(EA3Block *b,EA3Array *array,const XbIdentifier &what, int num,const int *ix, int base) {
  EA3Reference *r = (EA3Reference*)b->getByType(what);
  if (!r) r=b->createReference(what,"");
  r->removeAllReferences();
  int i;
  for(i=0;i<num;i++) {
    EA3Base *eBase = array->getByIndex(ix[i] + base);
    if (!eBase) {
			eBase = array->createField( what, "" );
		} /* if */
    r->addReference(eBase);
  }
  return(num);
}

EA3VertexData::~EA3VertexData() {
  for(int i=0;i<layerData.getSize();i++) {
    delete layerData[i];
  }
}
EA3_Status geoPrimGetNumColours(const EA3Block *prim,int &num,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBcolour);
  if (!r) return(EA3_NOT_FOUND);
  num = r->getNumReferences();
  return(EA3_OK);
}

EA3_Status geoPrimGetNumNormals(const EA3Block *prim,int &num,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBnormal);
  if (!r) return(EA3_NOT_FOUND);
  num = r->getNumReferences();
  return(EA3_OK);
}

EA3_Status geoPrimGetNumUVs(const EA3Block *prim,int &num,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBuv);
  if (!r) return(EA3_NOT_FOUND);
  num = r->getNumReferences();
  return(EA3_OK);
}

EA3_Status geoPrimGetColour(const EA3Block *prim,int index, U32 &col,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBcolour);
  if (!r) return(EA3_NOT_FOUND);
  EA3Field *f = (EA3Field *)r->getReference(index);
  if (!f) return(EA3_NOT_FOUND);
  col = f->data[0].ui;
  return(EA3_OK);
} 


EA3_Status geoPrimGetUV(const EA3Block *prim,int index,float *&uv,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBuv);
  if (!r) return(EA3_NOT_FOUND);
  EA3Field *f = (EA3Field *)r->getReference(index);
  if (!f) return(EA3_NOT_FOUND);
  uv = (float*)f->data;
  return(EA3_OK);
}

EA3_Status geoPrimGetNormal(const EA3Block *prim,int index,float *&nrm,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Reference *r = (EA3Reference*)l->getByType(XBnormal);
  if (!r) return(EA3_NOT_FOUND);
  EA3Field *f = (EA3Field *)r->getReference(index);
  if (!f) return(EA3_NOT_FOUND);
  nrm = (float*)f->data;
  return(EA3_OK);
}

EA3_Status geoPrimGetColours(const EA3Block *prim,int num, U32 *colours,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getValuesViaRef(l,XBcolour,num,colours)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
} 

EA3_Status geoPrimGetUVs(const EA3Block *prim,int num, Float3 *uvs,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getValuesViaRef(l,XBuv,num,uvs)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
} 

EA3_Status geoPrimGetNormals(const EA3Block *prim,int num, Float3 *normals,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getValuesViaRef(l,XBnormal,num,normals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geoPrimGetColourIndices(const EA3Block *prim,int num, int *ix,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getIndicesViaRef(l,XBcolour,num,ix)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geoPrimGetNormalIndices(const EA3Block *prim,int num, int *ix,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getIndicesViaRef(l,XBnormal,num,ix)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geoPrimGetUVIndices(const EA3Block *prim,int num, int *ix,int layer)  {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,l);
  if (!l) return(EA3_NOT_FOUND);
  if (getIndicesViaRef(l,XBuv,num,ix)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geoPrimSetColours(EA3Block *prim,int num,const U32 *val,int layer) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,true,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBcolour);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }
  setValuesViaRef(l,a,what,num,val);
  return(EA3_OK);
}

EA3_Status geoPrimSetNormals(EA3Block *prim,int num,const Float3 *val,int layer) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,true,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBnormal);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }
  setValuesViaRef(l,a,what,num,val);
  return(EA3_OK);
}

EA3_Status geoPrimSetUVs(EA3Block *prim,int num,const Float3 *val,int layer) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,true,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBuv);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }
  setValuesViaRef(l,a,what,num,val);
  return(EA3_OK);
}

EA3_Status geoPrimSetColourIndices(EA3Block *prim,int num,const int *val,int layer, int baseIndex) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,true,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBcolour);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }

  if( baseIndex )
  {
    if(setIndicesViaRef(l,a,what,num,val,baseIndex)!=num)
      return(EA3_BAD_NUM);
  }
  else
  {
    if(setIndicesViaRef(l,a,what,num,val)!=num)
      return(EA3_BAD_NUM);
  }

  return(EA3_OK);
}

EA3_Status geoPrimSetNormalIndices(EA3Block *prim,int num,const int *val,int layer, int baseIndex) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,1,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBnormal);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }

  if( baseIndex )
  {
    if(setIndicesViaRef(l,a,what,num,val,baseIndex)!=num)
      return(EA3_BAD_NUM);
  }
  else
  {
    if(setIndicesViaRef(l,a,what,num,val)!=num)
      return(EA3_BAD_NUM);
  }

  return(EA3_OK);
}

EA3_Status geoPrimSetUVIndices(EA3Block *prim,int num,const int *val,int layer, int baseIndex) {
  EA3Block *l = NULL;
  geoPrimGetLayer(prim,layer,true,l);
  if (!l) return(EA3_NOT_FOUND);
  XbIdentifier what(XBuv);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }

  if( baseIndex )
  {
    if(setIndicesViaRef(l,a,what,num,val,baseIndex)!=num)
      return(EA3_BAD_NUM);
  }
  else
  {
    if(setIndicesViaRef(l,a,what,num,val)!=num)
      return(EA3_BAD_NUM);
  }

  return(EA3_OK);
}

EA3_Status geoPrimGetGeometry(const EA3Block *prim, EA3Block *&geom) {
  geom = prim->getContainer();
  while (geom && geom->getType() != XBgeometry) geom=geom->getContainer();
  if(geom) return (EA3_OK);
  return EA3_WRONG_TYPE;
}


EA3_Status geoPrimSetMaterial(EA3Block *prim,EA3Block *mat) {
  EA3Reference *m = prim->getByType(XBmaterial);
  if (!m) {
    m = prim->createReference(XBmaterial,"");
  }
  m->setReference(0,mat);
  return(EA3_OK);
}


EA3_Status geoPrimGetMaterial(const EA3Block *prim,EA3Block *&mat)  {
  EA3Reference *m = prim->getByType(XBmaterial);
  if (!m) return(EA3_NOT_FOUND);
  mat = (EA3Block *)m->getReference(0);
  if (!mat) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status geoPrimGetLayer(EA3Block *prim,int index,int create, EA3Block *&layer) {
  layer = NULL;
  EA3Array *l = prim->getByType(XBlayer);
  if (!l) {
    if(!create) return(EA3_NOT_FOUND);
    l = prim->createArray(XBlayer);
  }
  while(index >= l->getNumMembers() && create) {
    l->createBlock(XBlayer);
  }
  layer = l->getByIndex(index);
  return EA3_OK;
}

EA3_Status geoPrimGetLayer(const EA3Block *prim,int index, EA3Block *&layer)  {
  EA3Array *l = prim->getByType(XBlayer);
  if (!l) {
    layer = NULL;
    return EA3_NOT_FOUND;
  } else {
    layer = l->getByIndex(index);
  }
  return EA3_OK;
}


/*
EA3Layer *addLayer() {
  EA3Array *l = (EA3Array *)getByType(XBlayer);
  if (!l) {
    l = createArray(XBlayer,"");
  }
  return((EA3Layer *)l->createBlock(XBlayer,""));
}
int removeLayer(int index) {
  EA3Array *l = (EA3Array *)getByType(XBlayer);
  if (!l) return(0);
  return(l->removeMember(index));
}
*/


EA3_Status geoPrimGetNumCoords(const EA3Block *prim,int &num)  {
  EA3Reference *r = (EA3Reference*)prim->getByType(XBcoord);
  if (!r) return(EA3_NOT_FOUND);
  num = r->getNumReferences();
  return(EA3_OK);
}



EA3_Status geoPrimGetCoords(const EA3Block *prim,int num, Float3 *val)  {
  if (getValuesViaRef(prim,XBcoord,num,val)!=num)return(EA3_BAD_NUM);
  return(EA3_OK);
}



EA3_Status geoPrimGetCoordIndices(const EA3Block *prim,int num,int *ix)  {
  if(getIndicesViaRef(prim,XBcoord,num,ix)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geoPrimSetCoords(EA3Block *prim,int num,const Float3 *val)  {
  XbIdentifier what(XBcoord);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }
  setValuesViaRef(prim,a,what,num,val);
  return(EA3_OK);
}


EA3_Status geoPrimSetCoordIndices(EA3Block *prim,int num,const int *val, int baseIndex) {
  XbIdentifier what(XBcoord);
  EA3Block *g = NULL;
  geoPrimGetGeometry(prim,g);
  if (!g) return(EA3_NOT_FOUND);
  EA3Array *a = (EA3Array *)g->getByType(what);
  if (!a) {
    a = g->createArray(what,"");
  }

  if( baseIndex )
  {
    setIndicesViaRef(prim,a,what,num,val,baseIndex);
  }
  else
  {
    setIndicesViaRef(prim,a,what,num,val);
  }

  return(EA3_OK);
}

EA3_Status geoPrimGetVertexData(EA3Block *prim,XbPArray<EA3VertexData *> &vs) {
//  int num;
  int i,j;
  EA3Reference *coords = prim->getByType(XBcoord);

  //Keiths addition to code as of April 26 2002 - If an "instance of" is used then no coordinates exist
  if (!coords)
    return(EA3_OK);

  EA3Array *layers = prim->getByType(XBlayer);
  for(i=0;i<coords->getNumReferences();i++) {
    EA3VertexData *v = new EA3VertexData;
    vs += v;
    if (layers) {
      v->layerData.setSize(layers->getNumMembers());
    }
    v->coord = (EA3Field *)coords->getReference(i);
    EA3Base *mat = prim->getByType(XBmaterial);
    v->data.append(prim);
    if (mat) v->data.append(mat);
    for(j=0;layers && j<layers->getNumMembers(); j++) {
      EA3Block *layer = (EA3Block *)layers->getByIndex(j);
      XbPArray<EA3Base*> *bl = new XbPArray<EA3Base*>;
      v->layerData[j] = bl;
      for(int k=0;k<layer->getNumMembers();k++) {
        EA3Base *b = layer->getByIndex(k);
        if (b->isReference()) {
          EA3Reference *r = b->toReference();
          bl->append(r->getReference(i));
        } else if (b->isArray()) {
          EA3Array *a = b->toArray();
          bl->append(a->getByIndex(i));
        }
      }
    }
  }
  return(EA3_OK);
}

EA3_Status geomGetGeoPrim(const EA3Block *geom,int index, EA3Block *&prim) {
  EA3Array *a = (EA3Array *)geom->getByType(XBgeoPrim);
  prim = a->getByIndex(index);
  if (prim) return EA3_OK;
  return EA3_NOT_FOUND;
}

EA3_Status geomAddGeoPrim(EA3Block *geom,const XbIdentifier &tp, EA3Block *&prim) {
  EA3Array *a = (EA3Array *)geom->getByType(XBgeoPrim);
  if (!a) a = geom->createArray(XBgeoPrim,"");
  prim = a->createBlock(tp,"");
  if (prim) return EA3_OK;
  return EA3_WRONG_TYPE;
}

EA3_Status geomGetLOD(const EA3Block *geom,int index, EA3Block *&lod) {
  lod = NULL;
  EA3Array *a = (EA3Array *)geom->getByType(XBlod);
  if (!a) return(EA3_NOT_FOUND);
  lod = a->getByIndex(index);
  if (lod) return EA3_OK;
  return EA3_NOT_FOUND;
}
EA3_Status geomAddLOD(EA3Block *geom, EA3Block *&lod) {
  EA3Array *a = (EA3Array *)geom->getByType(XBlod);
  if (!a) a = geom->createArray(XBlod,"");
  lod = a->createBlock(XBlod,"");
  if (lod) return EA3_OK;
  return EA3_WRONG_TYPE;
}

EA3_Status lodGetNumGeoPrim( EA3Block *lod, int &num ) {
  EA3Array *a = lod->getByType(XBgeoPrim);
  if( a )
    num = a->getNumMembers();
  else
    num = 0;
  return EA3_NOT_FOUND;
}

EA3_Status lodGetGeoPrim(const EA3Block *lod,int index, EA3Block *&prim) {
  EA3Array *a = lod->getByType(XBgeoPrim);
  prim =  a->getByIndex(index);
  if (prim) return EA3_OK;
  return EA3_NOT_FOUND;
}

EA3_Status lodAddGeoPrim(EA3Block *lod,const XbIdentifier &tp, EA3Block *&prim) {
  EA3Array *a = lod->getByType(XBgeoPrim);
  if (!a) a = lod->createArray(XBgeoPrim,"");
  prim = a->createBlock(tp,"");
  if (prim) return EA3_OK;
  return EA3_WRONG_TYPE;
}

/*
EA3_Status minimizeNumVertices() {
  return(EA3_ERROR);
}
EA3_Status minimizeNumNormals() {
  return(EA3_ERROR);
}
EA3_Status minimizeNumUVs() {
  return(EA3_ERROR);
}
EA3_Status minimizeNumColours() {
  return(EA3_ERROR);
}
*/

EA3_Status geomGetNumGeoPrims(const EA3Block *geom,int lod,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Block *l = NULL;
  geomGetLOD(geom,lod,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Array *a = (EA3Array *)l->getByType(XBgeoPrim);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetNumCoords(const EA3Block *geom,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBcoord);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetNumLODs(const EA3Block *geom,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBlod);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetNumNormals(const EA3Block *geom,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBnormal);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetNumUVs(const EA3Block *geom,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBuv);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetNumColours(const EA3Block *geom,int &num) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBcolour);
  num = (a? a->getNumMembers() : 0);
  return(EA3_OK);
}

EA3_Status geomGetCoords(const EA3Block *geom,int num,Float3 *vals) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBcoord);
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,vals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geomGetNormals(const EA3Block *geom,int num,Float3 *vals) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBnormal);
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,vals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geomGetUVs(const EA3Block *geom,int num,Float3 *vals) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBuv);
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,vals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geomGetColours(const EA3Block *geom,int num,U32 *vals) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType(XBcolour);
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,vals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geomGetCollapseMap( const EA3Block *geom, int num, int *vals )
{
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Array *a = geom->getByType( "collapseMap" );
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,vals)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status geomSetCoords(EA3Block *geom,int num,const Float3 *vals,bool add) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  XbIdentifier what(XBcoord);
  EA3Array *a = geom->getByType(what);
  if (!a) a = geom->createArray(what,"");

  if( add )
    setValues( a, num, vals, true );
  else
    setValues( a, num, vals );

  return(EA3_OK);
}

EA3_Status geomSetCollapseMap( EA3Block *geom, int num,const int *vals, bool add )
{
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);

  EA3Array *a = geom->getByType( "collapseMap" );
  if (!a) a = geom->createArray( "collapseMap", "" );

  if( add )
    setValues( a, num, vals, true );
  else
    setValues( a, num, vals );

  return(EA3_OK);
}

EA3_Status geomSetNormals(EA3Block *geom,int num,const Float3 *vals,bool add) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  XbIdentifier what(XBnormal);
  EA3Array *a = geom->getByType(what);
  if (!a) a = geom->createArray(what,"");

  if( add )
    setValues( a, num, vals, true );
  else
    setValues( a, num, vals );

  return(EA3_OK);
}

EA3_Status geomSetUVs(EA3Block *geom,int num,const Float3 *vals, bool add) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  XbIdentifier what(XBuv);
  EA3Array *a = geom->getByType(what);
  if (!a) a = geom->createArray(what,"");

  if( add )
    setValues( a, num, vals, true );
  else
    setValues( a, num, vals );

  return(EA3_OK);
}

EA3_Status geomSetColours(EA3Block *geom,int num,const U32 *vals,bool add) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  XbIdentifier what(XBcolour);
  EA3Array *a = geom->getByType(what);
  if (!a) a = geom->createArray(what,"");

  if( add )
    setValues( a, num, vals, true );
  else
    setValues( a, num, vals );

  return(EA3_OK);
}

EA3_Status geomAddCoords(EA3Block *geom, int num,const Float3 *vals )
{
    if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);

    XbIdentifier what(XBcoord);
    EA3Array *a = geom->getByType(what);
    if (!a) a = geom->createArray(what,"");

    addValues(a,num,vals);
    return(EA3_OK);
}

EA3_Status geomAddNormals(EA3Block *geom, int num, Float3 *vals )
{
    if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);

    XbIdentifier what(XBnormal);
    EA3Array *a = geom->getByType(what);
    if (!a) a = geom->createArray(what,"");

    addValues(a,num,vals);
    return(EA3_OK);
}

EA3_Status geomAddUVs(EA3Block *geom, int num, Float3 *vals )
{
    if ( geom->getType() != XBgeometry ) return( EA3_WRONG_TYPE );

    XbIdentifier what( XBuv );
    EA3Array *uvArray = geom->getByType( what );
    if ( !uvArray ) uvArray = geom->createArray( what, "" );

    addValues( uvArray, num, vals );
    return(EA3_OK);
}

EA3_Status geomAddGeoPrim(EA3Block *geom,int lod,const XbIdentifier &type, EA3Block *&prim) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Block *l = NULL;
  geomGetLOD(geom,lod,l);
  while (!l) {
    EA3Array *lods = geom->createArray(XBlod);
    lods->createBlock(XBlod);
    l= NULL;
    geomGetLOD(geom,lod,l);
  }
  prim = NULL;
  lodAddGeoPrim(l,type,prim);
  return(EA3_OK);
}

EA3_Status geomGetGeoPrim(const EA3Block *geom,int lod,int index,EA3Block *&prim) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Block *l = NULL;
  geomGetLOD(geom,lod,l);
  if (!l) return(EA3_NOT_FOUND);
  prim = NULL;
  lodGetGeoPrim(l,index,prim);
  if (!prim) return(EA3_NOT_FOUND);
  return(EA3_OK);
}

EA3_Status geomGetVertexData(const EA3Block *geom,int lod,XbPArray<EA3VertexData *> &vs) {
  if (geom->getType() != XBgeometry) return(EA3_WRONG_TYPE);
  EA3Block *l = NULL;
  geomGetLOD(geom,lod,l);
  if (!l) return(EA3_NOT_FOUND);
  EA3Array *a = (EA3Array *)l->getByType(XBgeoPrim);
  int num = (a? a->getNumMembers() : 0);
  for(int i=0;i<num;i++) {
    EA3Block *prim = NULL;
    lodGetGeoPrim(l,i,prim);
    geoPrimGetVertexData(prim,vs);
  }
  return(EA3_OK);
}

EA3_Status geomGetSkeleton(EA3Block *geom, EA3Block *&skel, EA3Block *lookin)
{ 
  if( !lookin ) {
    lookin = geom;
    
    while( lookin && lookin->getType()!=XBmodel ) {lookin=lookin->getContainer();}
  }
  if (lookin && lookin->getType()!=XBskeleton) {
    EA3Block *tmp = lookin->getByType(XBskeleton);
    if (tmp) lookin = tmp;
  }
  if( lookin )  skel = findFirst(lookin, !IsArray() && HasType("skeleton") && HasMember(HasType("bone")&&HasMember(HasType("type")&&HasValue("geom"))&&HasMember(HasType("links")&&IsReference()&&HasReference(This()==geom))), HasType("skeleton"));
  if( skel )    return(EA3_OK);
  if( lookin )  skel = findFirst(lookin, HasName(geom->getName()) && !IsArray() && HasType("skeleton") && HasMember(HasType("bone")&&HasMember(HasType("type")&&HasValue("geom"))), HasType("skeleton"));
  if( skel )    return(EA3_OK);
  return(EA3_NOT_FOUND);
}

EA3_Status geomApplyTransform(EA3Block *geom, const XbMatrix& matrix)
{
  //
  // some variables
  //
  XbVec3f oldvect, newvect;   // Holds vectors
  int num = 0;                // Holds number of coords/normals/etc
  int cnt;                    // A counter
  
  
  ////////////////////////////////////////////
  //
  // Modify coordinates
  //
  
  //
  // Get the coordinates
  //
  geomGetNumCoords(geom,num);
  Float3* mycoordinates = new Float3[num];
  geomGetCoords(geom,num, mycoordinates);
  
  //
  // Apply the transformation onto each coordinate
  //
  for( cnt=0; cnt<num; cnt++ ) {
    // get the coords into a vector
    oldvect[0] = mycoordinates[cnt][0];
    oldvect[1] = mycoordinates[cnt][1];
    oldvect[2] = mycoordinates[cnt][2];
    // transform the vector
    matrix.multVecMatrix( oldvect, newvect );
    // reset the coordinates
    mycoordinates[cnt][0] = newvect[0];
    mycoordinates[cnt][1] = newvect[1];
    mycoordinates[cnt][2] = newvect[2];
  }
  
  //
  // Set the coordinates to their new value
  //
  if( geomSetCoords(geom,num, mycoordinates) != EA3_OK ) {
    //cerr << "ERR: (geomApplyTransform) Cannot reset coordinates\n";
    return EA3_ERROR;
  }
  
  //
  // Deallocate the coordinate array
  //
  delete[] mycoordinates;
  
  
  
  ////////////////////////////////////////////
  //
  // Modify normals
  //
  
  //
  // Get the coordinates
  //
  geomGetNumNormals(geom,num);
  Float3* mynormals = new Float3[num];
  geomGetNormals(geom,num, mynormals);
  
  //
  // Apply the transformation onto each coordinate
  // and then normalize the normals :)
  //
  for( cnt=0; cnt<num; cnt++ ) {
    // get the coords into a vector
    oldvect[0] = mynormals[cnt][0];
    oldvect[1] = mynormals[cnt][1];
    oldvect[2] = mynormals[cnt][2];
    // transform the vector (normals are not transposable)
    matrix.multDirMatrix( oldvect, newvect);
    // normalize
    newvect.normalize();
    // reset the coordinates
    mynormals[cnt][0] = newvect[0];
    mynormals[cnt][1] = newvect[1];
    mynormals[cnt][2] = newvect[2];
  }
  
  //
  // Set the coordinates to the new value
  //
  if( geomSetNormals(geom,num, mynormals) != EA3_OK ) {
    //cerr << "ERR: (geomApplyTransform) Cannot reset normals\n";
    return EA3_ERROR;
  }
  
  //
  // Deallocate the normal array
  //
  delete[] mynormals;
  
  
  ////////////////////////////////////////////
  //
  // Modify coordinate deltas
  //
  
  EA3Array* morphTargets = (EA3Array*)geom->getByType(XBmorphTarget);
  int cnt2;
  if( morphTargets ) {
    
    for( cnt=0; cnt<morphTargets->getNumMembers(); cnt++ ) {
      EA3Block *mt = morphTargets->getByIndex(cnt);
      // Starting coord delta code
      EA3Array* corddeltas = (EA3Array*)mt->getByType(XBcoordDelta);
      if( corddeltas ) {
        num = corddeltas->getNumMembers();
        
        for( cnt2=0; cnt2<num; cnt2++) {
          EA3Field* mycorddelta = (EA3Field*)corddeltas->getByIndex(cnt2);
          
          // Get the old delta values
          oldvect[0] = mycorddelta->data[0].f;
          oldvect[1] = mycorddelta->data[1].f;
          oldvect[2] = mycorddelta->data[2].f;
          
          // Transform them
          matrix.multDirMatrix( oldvect, newvect );
          
          // Reset the delta values
          mycorddelta->data[0].f = newvect[0];
          mycorddelta->data[1].f = newvect[1];
          mycorddelta->data[2].f = newvect[2];
        }
      }
      
      
      
      ////////////////////////////////////////////
      //
      // Modify normal deltas
      //
      
      EA3Array* normaldeltas = (EA3Array*)mt->getByType(XBnormalDelta);
      if( normaldeltas ) {
        num = normaldeltas->getNumMembers();
        
        for( cnt2=0; cnt2<num; cnt2++) {
          EA3Field* mynormaldelta = (EA3Field*)normaldeltas->getByIndex(cnt2);
          
          
          // Get the old delta values
          oldvect[0] = mynormaldelta->data[0].f;
          oldvect[1] = mynormaldelta->data[1].f;
          oldvect[2] = mynormaldelta->data[2].f;
          
          // Transform them
          matrix.multDirMatrix( oldvect, newvect );
          
          // Reset the delta values
          mynormaldelta->data[0].f = newvect[0];
          mynormaldelta->data[1].f = newvect[1];
          mynormaldelta->data[2].f = newvect[2];
        }
      }
    }
  }
  
  
  /////////////////////////////////
  //
  // Return true
  //
  return EA3_OK;
}

EA3_Status geomMerge(EA3Block *geomMaster, EA3Block *geom)
{
  int i,j,k;
  EA3Block *model = geom->getContainer()->getContainer();
  EA3Array *mts = geom->getByType(XBmorphTarget);
  EA3Array *mtsMaster = geomMaster->getByType(XBmorphTarget);
  if (mts) {
    if (!mtsMaster) {
      mtsMaster = geomMaster->createArray(XBmorphTarget);
    }
    EA3Array *animation = model->getByType(XBanimation);
    if (animation) {
      XbPArray<EA3Base *> chanList,chanListMaster;
      find(animation,chanList,(HasName(geom->getName())||(HasName(mts->getName())&&HasMember(HasValue("morphWeight")))));
      
      for(i=0;i<chanList.getSize();i++) {
        EA3Block *chan = chanList[i]->toBlock();
        chan->setName(geomMaster->getName());
        EA3Field *index = chan->getByType(XBindex);
        if(!index) continue;
        int ix = index->data[0].i;
        EA3Block *mt = mts->getByIndex(ix);
        find(animation,chanListMaster,(
          HasName(geomMaster->getName())||(HasName(mtsMaster->getName())&&
          HasMember(
            HasValue("morphWeight"))&&
            HasMember(
              HasType("index")&&(GetValue(0)==ix)
            )
          ))
        );
        if (mt) {
          if (chanListMaster.getSize()==0) {
            mtsMaster->appendMember(mt);
            index->data[0].i = mt->getIndex();
          } else {
            EA3Block *mtMaster = mtsMaster->getByIndex(ix);
            EA3Block *chanMaster = chanListMaster[0]->toBlock();
            EA3Array *a1 = chan->getByType(XBdata);
            if (!a1) a1=chan->getByType(XBkey);
            EA3Array *a2 = chanMaster->getByType(XBdata);
            if (!a2) a2=chanMaster->getByType(XBkey);
            bool same = true;
            if (!mtMaster || !a1 || !a2 || a1->getNumMembers()!=a2->getNumMembers()) {
              same = false;
            }
            if (mt->getByType(XBcoordDelta)->toArray()==NULL) same = false;
            if (same) {
              for(j=0;j<a1->getNumMembers();j++) {
                EA3Field *f1 = a1->getByIndex(j);
                EA3Field *f2 = a2->getByIndex(j);
                for(k=0;k<(int)f1->getNum();k++) {
                  if (f1->data[k].ui != f2->data[k].ui) {
                    same = false;
                    break;
                  }
                }
              }
            }
            if (same) {
              // merge the morph targets
             
              for(j=0;j<mt->getNumMembers();j++) {
                a1 = mt->getByIndex(j);
                if (a1) {
                  a2 = mtMaster->getByType(a1->getType());
                  if (!a2) a2=mtMaster->createArray(a1->getType());
                  EA3TransferMembers(a1,a2);
                } else {
                  EA3Reference *r1 = mt->getByIndex(j);
                  if (r1) {
                    EA3Reference *r2 = mtMaster->getByType(r1->getType());
                    if (!r2) r2 = mtMaster->createReference(r1->getType());
                    for(k=0;k<r1->getNumReferences();k++) {
                      r2->addReference(r1->getReference(k));
                    }
                  }
                }
              }
              mts->removeMember(mt);
              chan->getContainer()->removeMember(chan);
            } else {
              mtsMaster->appendMember(mt);
              index->data[0].i = mt->getIndex();
            }
          }
        }
      }
    }
  }
  for(i=0;i<geom->getNumMembers();i++) {
    EA3Base *b = geom->getByIndex(i);
    if (b->isArray() && b->getType() != "lod") {
      EA3Array *a = b->toArray();
      EA3Array *ma = (EA3Array *)geomMaster->getByType(a->getType());
      if (!ma) ma = geomMaster->createArray(a->getType(),a->getName());
      EA3TransferMembers(a,ma);
    }
    if (b->getType() == XBlod) {
      EA3Array *a = b->toArray();
      EA3Array *ma = (EA3Array *)geomMaster->getByType(a->getType());
      if (!ma) ma = geomMaster->createArray(a->getType(),a->getName());
      for(int l=0;l<a->getNumMembers();l++) {
        EA3Block *la = (EA3Block *)a->getByIndex(l);
        if(!la) continue;
        EA3Block *mla = (EA3Block *)ma->getByIndex(l);
        if (!mla) mla = ma->createBlock(XBlod);
        EA3Array *gp = (EA3Array *)la->getByType(XBgeoPrim);
        if(!gp) continue;
        EA3Array *mgp = (EA3Array *)mla->getByType(XBgeoPrim);
        //if (!mgp) geomMaster->createArray(gp->getType(),gp->getName());
        if (!mgp) mgp = mla->createArray(gp->getType(),gp->getName());
        EA3TransferMembers(gp,mgp);
      }
    }
  }
  return EA3_OK;
}

EA3_Status curveGetNumWeights(const EA3Block *curve,int &num) 
{
    EA3Array *r = curve->getByType(XBweight);
    if (!r) return(EA3_NOT_FOUND);
    num = r->getNumMembers();
    return(EA3_OK);
}

EA3_Status curveGetNumKnots(const EA3Block *curve, int &num ) 
{
    EA3Array *r = curve->getByType(XBknot);
    if (!r) return(EA3_NOT_FOUND);
    num = r->getNumMembers();
    return(EA3_OK);
}

EA3_Status curveGetWeights(const EA3Block *curve, int num, float *weights )  {
  EA3Array *a = (EA3Array *)curve->getByType(XBweight);
  if (!a) return(EA3_NOT_FOUND);
  if(getValues(a,num,weights)!=num) return(EA3_BAD_NUM);
  return(EA3_OK);
}

EA3_Status curveGetKnots(const EA3Block *curve, int num, float *knots ) 
{
    EA3Array *a = (EA3Array *)curve->getByType(XBknot);
    if (!a) return(EA3_NOT_FOUND);
    if( getValues( a,num,knots)!=num ) return( EA3_BAD_NUM );
    return(EA3_OK);
}

EA3_Status curveSetWeights(EA3Block *curve, int num, const float *weights )
{
    XbIdentifier what(XBweight);
    EA3Array *a = curve->getByType(what);
    if (!a) a = curve->createArray(what,"");
    setValues( a, num, weights );
    return(EA3_OK);
}

EA3_Status curveSetKnots(EA3Block *curve, int num,const float *knots )
{
    EA3Array *a = curve->getByType(XBknot);
    if (!a) a = curve->createArray(XBknot,"");
    setValues( a, num, knots );
    return(EA3_OK);
}

EA3_Status curveGetDegree(const EA3Block *curve, int &deg ) 
{
    EA3Field *degField = curve->getByType( "degree" );
    if( !degField ) return( EA3_NOT_FOUND );

    deg = degField->data->i;
    return(EA3_OK);
}

EA3_Status curveSetDegree(EA3Block *curve, int deg )
{
    EA3Field *degField = curve->getByType( "degree" );
    if( !degField ) degField = curve->createField( "degree", "" );

    degField->data->i = deg;
    return(EA3_OK);
}

EA3_Status surfaceGetNumWeights(const EA3Block *surf,int &num)
{
    EA3Reference *r = surf->getByType(XBweight);
    if (!r) return(EA3_NOT_FOUND);
    num = r->getNumReferences();
    return(EA3_OK);
}

EA3_Status surfaceGetNumUKnots(const EA3Block *surf, int &num ) 
{
    EA3Reference *r = surf->getByType( XBuknot );
    if (!r) return(EA3_NOT_FOUND);
    num = r->getNumReferences();
    return(EA3_OK);
}

EA3_Status surfaceGetNumVKnots(const EA3Block *surf, int &num ) 
{
    EA3Reference *r = surf->getByType( XBvknot );
    if (!r) return(EA3_NOT_FOUND);
    num = r->getNumReferences();
    return(EA3_OK);
}

EA3_Status surfaceGetWeights(const EA3Block *surf, int num, float *weights ) 
{
    EA3Array *a = surf->getByType(XBweight);
    if (!a) return(EA3_NOT_FOUND);
    if(getValues(a,num,weights)!=num) return(EA3_BAD_NUM);
    return(EA3_OK);
}

EA3_Status surfaceGetUKnots(const EA3Block *surf, int num, float *knots ) 
{
    EA3Array *a = surf->getByType( XBuknot );
    if (!a) return(EA3_NOT_FOUND);
    if( getValues( a,num,knots)!=num ) return( EA3_BAD_NUM );
    return(EA3_OK);
}

EA3_Status surfaceGetVKnots(const EA3Block *surf, int num, float *knots ) 
{
    EA3Array *a = surf->getByType( XBvknot );
    if (!a) return(EA3_NOT_FOUND);
    if( getValues( a,num,knots)!=num ) return( EA3_BAD_NUM );
    return(EA3_OK);
}

EA3_Status surfaceSetWeights(EA3Block *surf, int num,const float *weights )
{
    XbIdentifier what(XBweight);
    EA3Array *a = surf->getByType(what);
    if (!a) a = surf->createArray(what,"");
    setValues( a, num, weights );
    return(EA3_OK);
}

EA3_Status surfaceSetUKnots(EA3Block *surf, int num,const float *knots )
{
    EA3Array *a = surf->getByType( XBuknot );
    if (!a) a = surf->createArray( XBuknot, "" );
    setValues( a, num, knots );
    return(EA3_OK);
}

EA3_Status surfaceSetVKnots(EA3Block *surf, int num,const float *knots )
{
    EA3Array *a = surf->getByType( XBvknot );
    if (!a) a = surf->createArray( XBvknot, "" );
    setValues( a, num, knots );
    return(EA3_OK);
}

EA3_Status surfaceGetUDegree(const EA3Block *surf, int &deg ) 
{
    EA3Field *degField = surf->getByType( XBuDegree );
    if( !degField ) return( EA3_NOT_FOUND );

    deg = degField->data->i;
    return(EA3_OK);
}

EA3_Status surfaceGetVDegree(const EA3Block *surf, int &deg ) 
{
    EA3Field *degField = surf->getByType( XBvDegree );
    if( !degField ) return( EA3_NOT_FOUND );

    deg = degField->data->i;
    return(EA3_OK);
}

EA3_Status surfaceSetUDegree(EA3Block *surf, int deg )
{
    EA3Field *degField = surf->getByType( XBuDegree );
    if( !degField ) degField = surf->createField( XBuDegree, "" );

    degField->data->i = deg;
    return(EA3_OK);
}

EA3_Status surfaceSetVDegree(EA3Block *surf, int deg )
{
    EA3Field *degField = surf->getByType( XBvDegree );
    if( !degField ) degField = surf->createField( XBvDegree, "" );

    degField->data->i = deg;
    return(EA3_OK);
}

EA3_Status surfaceGetNumUPoints(const EA3Block *surf, int &pt ) 
{
    EA3Field *pField = surf->getByType( XBnumUpoints );
    if( !pField ) return( EA3_NOT_FOUND );

    pt = pField->data->i;
    return(EA3_OK);
}

EA3_Status surfaceGetNumVPoints(const EA3Block *surf, int &pt ) 
{
    EA3Field *pField = surf->getByType( XBnumVpoints );
    if( !pField ) return( EA3_NOT_FOUND );

    pt = pField->data->i;
    return(EA3_OK);
}

EA3_Status surfaceSetNumUPoints(EA3Block *surf, int pt )
{
    EA3Field *pField = surf->getByType( XBnumUpoints );
    if( !pField ) pField = surf->createField( XBnumUpoints, "" );

    pField->data->i = pt;
    return(EA3_OK);
}

EA3_Status surfaceSetNumVPoints(EA3Block *surf, int pt )
{
    EA3Field *pField = surf->getByType( XBnumVpoints );
    if( !pField ) pField = surf->createField( XBnumVpoints, "" );

    pField->data->i = pt;
    return(EA3_OK);
}

EA3_Status surfaceGetUStride(const EA3Block *surf, int &stride ) 
{
    EA3Field *sField = surf->getByType( XBuStride );
    if( !sField ) return( EA3_NOT_FOUND );

    stride = sField->data->i;
    return(EA3_OK);
}

EA3_Status surfaceGetVStride(const EA3Block *surf, int &stride ) 
{
    EA3Field *sField = surf->getByType( XBvStride );
    if( !sField ) return( EA3_NOT_FOUND );

    stride = sField->data->i;
    return(EA3_OK);
}


EA3_Status surfaceSetUStride(EA3Block *surf, int stride )
{
    EA3Field *sField = surf->getByType( XBuStride );
    if( !sField ) sField = surf->createField( XBuStride, "" );

    sField->data->i = stride;
    return(EA3_OK);
}

EA3_Status surfaceSetVStride(EA3Block *surf, int stride )
{
    EA3Field *sField = surf->getByType( XBvStride );
    if( !sField ) sField = surf->createField( XBvStride, "" );

    sField->data->i = stride;
    return(EA3_OK);
}

class LINE
{
public:
    LINE( int i1, int i2, XbVec3f v1, XbVec3f v2, int idx, EA3Block &geoPrm )
    {
        m_index1 = i1;
        m_index2 = i2;
        m_vertex1 = v1;
        m_vertex2 = v2;
        m_primIndex = idx;
        m_geoPrim = &geoPrm;
    };

    int index1() const        { return m_index1; }
    int index2() const        { return m_index2; }
    XbVec3f vertex1() const   { return m_vertex1; }
    XbVec3f vertex2() const   { return m_vertex2; }
    int primIndex() const     { return m_primIndex; }
    EA3Block *geoPrim() const { return m_geoPrim; }

private:
    int       m_index1;     // index of the first vertex
    int       m_index2;     // index of the second vertex
    XbVec3f   m_vertex1;    // first vertex
    XbVec3f   m_vertex2;    // second vertex
    int       m_primIndex;  // polygon primitive index
    EA3Block* m_geoPrim;    // geometry primitive that contains this line
};

class POLYGON
{
public:
    POLYGON( int idx, EA3Block &geoPrm )
    {
        m_index = idx;
        m_geoPrim = &geoPrm;
    }

    int primIndex() const       { return m_index; }
    EA3Block *geoPrim() const   { return m_geoPrim; }

private:
    int         m_index;        // index of the polygon primitive in the geometry
    EA3Block*   m_geoPrim;      // geometry primitive
};

#if 0
//
//  static bool findClosestVertex()
//
//  Find the closest vertex in a geometry to certain point, <tgtPoint>.
//
//  Parameters:
//    const EA3Block &srcGeom : (in) source polygon geometry primitive
//    XbVec3f tgtPoint        : (in) coordinates to find the closest vertex
//    int& vtxIdx             : (out) vertex index
//    XbVec3f& closestVtx     : (out) closest vertex to <tgtPoint>
//
static bool findClosestVertex(
                const EA3Block &srcGeom,
                XbVec3f tgtPoint,
                int& vtxIdx,
                XbVec3f& closestVtx,
                float& distance )
{
    //
    //  Get the coordinates.
    //
    int numCoords;
    (void)geomGetNumCoords( &srcGeom, numCoords );

    if( numCoords <= 0 ) return false;

    Float3 *coords = new Float3[numCoords];
    (void)geomGetCoords( &srcGeom, numCoords, coords );

    float minDist = FLT_MAX;
    int c;

    for( c = 0; c < numCoords; c++ )
    {
        XbVec3f pt( coords[c][0], coords[c][1], coords[c][2] );
        float dist = XbVec3f( tgtPoint - pt ).sqrlength();

        if( dist < minDist )
        {
            minDist = dist;
            vtxIdx = c;
            closestVtx = pt;
            distance = dist;
        }
    }

    delete []coords;
    return true;
}
#endif

#if 0
//
//  static bool lineExists()
//
//  Does the line, defined by vertex indices <i1> and <i2>, already exist?
//
//  Parameters:
//    int i1                           : (in) vertex index 1
//    int i2                           : (in) vertex index 2
//    const XbPArray<LINE *>& lineList : (in) array of lines
//
static bool lineExists( int i1, int i2, const XbPArray<LINE *>& lineList )
{
    int l;
    for( l = 0; l < lineList.getSize(); l++ )
    {
        if( (i1 == lineList[l]->index1() &&
             i2 == lineList[l]->index2()) ||
            (i1 == lineList[l]->index2() &&
             i2 == lineList[l]->index1()) )
        {
            return true;
        }
    }

    return false;
}
#endif

#if 0
//
//  static void findVertexLines()
//
//  Find all the lines that share a common vertex.
//
//  Parameters:
//    const EA3Block &srcGeom    : (in) source polygon geometry
//    int vtxIdx                 : (in) vertex index
//    XbPArray<LINE *>& lineList : (out) array of lines that share the vertex.
//
static void findVertexLines(
                const EA3Block& srcGeom,
                int vtxIdx,
                XbPArray<LINE *>& lineList )
{
    lineList.clear();

    int numGeoPrims;
    (void)geomGetNumGeoPrims( &srcGeom, 0, numGeoPrims );

    int numCoords;
    (void)geomGetNumCoords( &srcGeom, numCoords );

    Float3 *coords = new Float3[numCoords];
    (void)geomGetCoords( &srcGeom, numCoords, coords );

    int p;
    for( p = 0; p < numGeoPrims; p++ )
    {
        //
        //  Make sure this is a polygon geometry primitive.
        //
        EA3Block *srcPolyPrim;
        (void)geomGetGeoPrim( &srcGeom, 0, p, srcPolyPrim );

        if( srcPolyPrim->getType() != XBpolygon ) continue;

        int nCoords;
        (void)geoPrimGetNumCoords( srcPolyPrim, nCoords );

        int *coordIndices = new int[nCoords];
        (void)geoPrimGetCoordIndices( srcPolyPrim, nCoords, coordIndices );

        int c;
        for( c = 0; c < nCoords; c++ )
        {
            if( coordIndices[c] == vtxIdx )
            {
                int i1 = (c > 0) ? coordIndices[c-1] : coordIndices[nCoords-1];
                int i2 = coordIndices[c];

                if( !lineExists( i1, i2, lineList ) )
                {
                    XbVec3f v1( coords[i1][0], coords[i1][1], coords[i1][2] );
                    XbVec3f v2( coords[i2][0], coords[i2][1], coords[i2][2] );

                    LINE *line = new LINE( i1, i2, v1, v2, p, *srcPolyPrim );
                    lineList.append( line );
                }

                i1 = coordIndices[c];
                i2 = coordIndices[(c+1) % nCoords];

                if( !lineExists( i1, i2, lineList ) )
                {
                    XbVec3f v1( coords[i1][0], coords[i1][1], coords[i1][2] );
                    XbVec3f v2( coords[i2][0], coords[i2][1], coords[i2][2] );

                    LINE *line = new LINE( i1, i2, v1, v2, p, *srcPolyPrim );
                    lineList.append( line );
                }
            }
        }

        delete []coordIndices;
    }

    delete []coords;
}
#endif

#if 0
//
//  static LINE* findClosestVertexLine()
//
//  Find the line that is closest to the point, <tgtPoint>.
//
//  Parameters:
//    const XbPArray<LINE *>& lineList : (in) array of lines
//    XbVec3f tgtPoint                 : (in) closest distance to this position
//    XbVec3f& closestLinePoint        : (out) closest point on the line
//    float& closestLineDist           : (out) distance to the closest line
//
static LINE *findClosestVertexLine(
                 const XbPArray<LINE *>& lineList,
                 XbVec3f tgtPoint,
                 XbVec3f& closestLinePoint,
                 float& closestLineDist )
{
    LINE *closestLine = NULL;
    closestLineDist = FLT_MAX;
    int l;
    for( l = 0; l < lineList.getSize(); l++ )
    {
        XbVec3f pointOnLine;
        float distance;
        bool between;

        (void)getClosestPointOnLine(
                  lineList[l]->vertex1(),
                  lineList[l]->vertex2(),
                  tgtPoint, 
                  pointOnLine,
                  distance,
                  between );

        if( distance < closestLineDist )
        {
            closestLine = lineList[l];
            closestLinePoint = pointOnLine;
            closestLineDist = distance;
        }
    }

    return closestLine;
}
#endif

#if 0
//
//  static void findPolygons()
//
//  Find all the polygons in the geometry that share this line.
//
//  Parameters:
//    const EA3Block &srcGeom        : (in) source polygon geometry
//    const LINE& line               : (in) line
//    XbPArray<POLYGON *>& polyList  : (in) array of polygons
//
static void findPolygons(
                const EA3Block &srcGeom,
                const LINE& line,
                XbPArray<POLYGON *>& polyList )
{
    int numGeoPrims;
    (void)geomGetNumGeoPrims( &srcGeom, 0, numGeoPrims );

    int p;
    for( p = 0; p < numGeoPrims; p++ )
    {
        //
        //  Make sure this is a polygon geometry primitive.
        //
        EA3Block *srcPolyPrim;
        (void)geomGetGeoPrim( &srcGeom, 0, p, srcPolyPrim );

        if( srcPolyPrim->getType() != XBpolygon ) continue;

        //
        //  Get the polygon's coordinate indices.
        //
        int nCoords;
        (void)geoPrimGetNumCoords( srcPolyPrim, nCoords );

        int *coordIndices = new int[nCoords];
        (void)geoPrimGetCoordIndices( srcPolyPrim, nCoords, coordIndices );

        int c;
        for( c = 0; c < nCoords; c++ )
        {
            int i1 = c;
            int i2 = (c + 1) % nCoords;

            if( (i1 == line.index1() && i2 == line.index2()) ||
                (i1 == line.index2() && i2 == line.index1()) )
            {
                POLYGON *poly = new POLYGON( p, *srcPolyPrim );
                polyList.append( poly );
            }
        }

        delete []coordIndices;
    }
}
#endif

//
//  static POLYGON* findClosestPolygon()
//
//  Find the polygon that is closest to the point, <tgtPoint>.
//
//  Parameters:
//    const XbPArray<LINE *>& lineList : (in) array of lines
//    XbVec3f tgtPoint                 : (in) closest distance to this position
//    XbVec3f& closestPolyPoint        : (out) closest point on the polygon
//    float& closestPolyDist           : (out) distance to the closest polygon
//
static POLYGON *findClosestPolygon(
                 const XbPArray<POLYGON *>& polyList,
                 XbVec3f tgtPoint,
                 XbVec3f& closestPolyPoint,
                 float& closestPolyDist )
{
    POLYGON *closestPoly = NULL;
    closestPolyDist = FLT_MAX;
    int p;
    for( p = 0; p < polyList.getSize(); p++ )
    {
        //
        //  Find the polygon normal.
        //
        XbVec3f normal;
        (void)geoPrimComputeNormal( polyList[p]->geoPrim(), normal );

        //
        //  See if the point can be projected onto the polygon.
        //  The point is projected in the reverse direction of the normal.
        //
        bool intersect = false;
        float t;
        XbVec3f intersectPoint;

        (void)geoPrimRayIntersect(
                 polyList[p]->geoPrim(),
                 tgtPoint,
                 -normal,
                 intersect,
                 t,
                 intersectPoint );

        if( intersect )
        {
            t = fabsf( t );

            if( t < closestPolyDist )
            {
                closestPoly = polyList[p];
                closestPolyPoint = intersectPoint;
                closestPolyDist = t;
            }
        }
    }

    return closestPoly;
}

EA3_Status geoPrimComputeNormal( const EA3Block *srcPolyPrim, XbVec3f &normal )
{
    int numCoords = 0;
    (void)geoPrimGetNumCoords( srcPolyPrim, numCoords );

    Float3 coords[100];
    (void)geoPrimGetCoords( srcPolyPrim, numCoords, coords );

    XbVec3f l1, l2;
    l1[0] = coords[1][0] - coords[0][0];
    l1[1] = coords[1][1] - coords[0][1];
    l1[2] = coords[1][2] - coords[0][2];
    l2[0] = coords[2][0] - coords[0][0];
    l2[1] = coords[2][1] - coords[0][1];
    l2[2] = coords[2][2] - coords[0][2];

    l1.normalize();
    l2.normalize();
    normal = l1.cross( l2 );
    normal.normalize();
    return EA3_OK;
}


EA3_Status geoPrimRayIntersect(
                const EA3Block *srcPolyPrim,
                const XbVec3f &rayOrig,
                const XbVec3f &rayDir,
                bool &intersect,
                float &t,
                XbVec3f &intersectPoint )
{
    intersect = false;

//  Compute the intersection point and its distance t from the ray origin.
  
    //Computations for the plane equation (plane normal * point on plane = a constant).
    int numCoords = 0;
    (void)geoPrimGetNumCoords( srcPolyPrim, numCoords );

    Float3 *origcoords = new Float3[numCoords];
    int numPasses = 1;
    int numSubCoords = numCoords;
    (void)geoPrimGetCoords( srcPolyPrim, numCoords, origcoords );
    if (srcPolyPrim->getType()==XBtriMesh) {
      numPasses = numCoords/3;
      numSubCoords = 3;
    }
    for(int pass=0;pass<numPasses;pass++) {
      XbVec3f l1,l2;
      int offset = pass*numSubCoords;
      Float3 *coords = &origcoords[offset];
      l1[0] = coords[1][0] - coords[0][0];
      l1[1] = coords[1][1] - coords[0][1];
      l1[2] = coords[1][2] - coords[0][2];
      l2[0] = coords[2][0] - coords[0][0];
      l2[1] = coords[2][1] - coords[0][1];
      l2[2] = coords[2][2] - coords[0][2];
      
      l1.normalize();
      l2.normalize();
      XbVec3f planeNormal = l1.cross( l2 );
      planeNormal.normalize();
      
      XbVec3f firstPnt;
      firstPnt[0] = coords[0][0];
      firstPnt[1] = coords[0][1];
      firstPnt[2] = coords[0][2];   //a point on polygonal plane
      
      float d = -planeNormal.dot( firstPnt );  //From equation: N dot P + d = zero.
      
      //Find t and save it in a temporary variable - we only want to return a new value for t if intersect==true.
      float t_temp = -(d + planeNormal.dot( rayOrig )) / planeNormal.dot( rayDir );
      
      //Intersection Point - like t, save it in a temporary variable.
      XbVec3f intersectPoint_temp = rayOrig + rayDir * t_temp;
      
//  Find the dominant axis of the plane normal verctor.

      int i1, i2;
      if( planeNormal[0] >= planeNormal[1] && planeNormal[0] >= planeNormal[2] )
      {
        i1 = 1;
        i2 = 2;
      }
      else if( planeNormal[1] >= planeNormal[0] && planeNormal[1] >= planeNormal[2] )
      {
        i1 = 0;
        i2 = 2;
      }
      else
      {
        i1 = 0;
        i2 = 1;
      }
      
//  Determine whether the intersection is within the polygon.
      
      float u0 = intersectPoint_temp[i1] - coords[0][i1];
      float v0 = intersectPoint_temp[i2] - coords[0][i2];
            
      bool didIntersect = false;
      
      int idx = 2;     
      do
      {
        float u1 = coords[idx-1][i1] - coords[0][i1];
        float v1 = coords[idx-1][i2] - coords[0][i2];
        float u2 = coords[idx  ][i1] - coords[0][i1];
        float v2 = coords[idx  ][i2] - coords[0][i2];
        float alpha, beta;
        
        if (u1 == 0)
        {
          beta = u0 / u2;
          if ((beta >= -0.0001f)&&(beta <= 1.0001f))
          {
            alpha = (v0 - beta*v2)/v1;
            didIntersect = ((alpha >= -0.0001f) && ((alpha+beta) <= 1.0001f));
          }
        }
        else
        {
          beta = (v0*u1 - u0*v1)/(v2*u1 - u2*v1);
          if ((beta >= -0.0001f)&&(beta <= 1.0001f))
          {
            alpha = (u0 - beta*u2)/u1;
            didIntersect = ((alpha >= -0.0001f) && ((alpha+beta) <= 1.0001f));
          }
        }
        
      }
      while( !didIntersect && ++idx < numSubCoords  );
      
      if ((didIntersect) && (t_temp >= 0.0f)) { //We want to ignore cases of negative t (line of ray intersects poly but not ray itself)
        if (!intersect) { //our first intersection (there may be multiple intersections)
          t = t_temp;
          intersect = true; //only have to assign it this once, when we find our first intersection
          intersectPoint = intersectPoint_temp;
        }
        if (t_temp < t) {
          t = t_temp; //we want the closest intersection - it represents the ray collision with the polygon
          intersectPoint = intersectPoint_temp;
        }
      }
    } //end for-loop
    delete [] origcoords;
    return EA3_OK;
}

EA3_Status geomGetPolygonByProjection(
               const EA3Block *srcGeom,
               XbVec3f tgtPoint,
               XbVec3f tgtNormal,
               int &index,
               XbVec3f &intersectPoint )
{
    float minT = FLT_MAX;

    int numGeoPrims = 0;
    (void)geomGetNumGeoPrims( srcGeom, 0, numGeoPrims );

    int p;
    for( p = 0; p < numGeoPrims; p++ )
    {
        //
        //  Make sure this is a polygon geometry primitive.
        //
        EA3Block *srcPolyPrim = 0;
        (void)geomGetGeoPrim( srcGeom, 0, p, srcPolyPrim );

        if( srcPolyPrim->getType() != XBpolygon ) continue;

        XbVec3f thisIntersect;
        float t;

        //
        //  Find the closest polygon (in absolute distance) that intersects.
        //
        bool intersect = false;

        (void)geoPrimRayIntersect(
                      srcPolyPrim,
                      tgtPoint,
                      tgtNormal,
                      intersect,
                      t,
                      thisIntersect );

        if( intersect )
        {
            t = fabsf( t );

            if( t < minT )
            {
                index = p;
                minT = t;
                intersectPoint = thisIntersect;
            }
        }
    }

    return EA3_OK;
}


EA3_Status geomGetClosestPolygon(
               const EA3Block *srcGeom,
               const XbVec3f &tgtPoint,
	           int& polyIndex,
               XbVec3f &intersectPoint )
{
    //
    //  Find all the polygons in this geometry.
    //
    XbPArray<POLYGON *> polyList;

    int numPolygons = 0;
    (void)geomGetNumGeoPrims( srcGeom, 0, numPolygons );

    int p;
    for( p = 0; p < numPolygons; p++ )
    {
        EA3Block *geoPrim = 0;
        (void)geomGetGeoPrim( srcGeom, 0, p, geoPrim );

        if( geoPrim->getType() == XBpolygon )
        {
            POLYGON *poly = new POLYGON( p, *geoPrim );
            polyList.append( poly );
        }
    }

    //
    //  Find the closest point on the polygons.
    //
    XbVec3f closestPolyPoint(0.0f, 0.0f, 0.0f);
    float closestPolyDist;
    POLYGON *closestPoly = findClosestPolygon(
                             polyList, tgtPoint, closestPolyPoint, closestPolyDist );

    polyIndex = closestPoly->primIndex();
    intersectPoint = closestPolyPoint;

    //
    //  Clean up.
    //
    int l;
    for( l = 0; l < polyList.getSize(); l++ ) delete polyList[l];

    return EA3_OK;
}   /* geomGetClosestPolygon */

class COORD4 {
public:
  float x,y,z,w;
};

static COORD4 Eval(COORD4 *CVS, COORD4 & t)
{
	COORD4 res;
	res.w = 1.0f;
	res.x = t.x * CVS[0].x + t.y * CVS[1].x + t.z * CVS[2].x + t.w * CVS[3].x;
	res.y = t.x * CVS[0].y + t.y * CVS[1].y + t.z * CVS[2].y + t.w * CVS[3].y;
	res.z = t.x * CVS[0].z + t.y * CVS[1].z + t.z * CVS[2].z + t.w * CVS[3].z;
	return res;
}
COORD4 VecMatMul(const COORD4 &v, XbMatrix &mat) {
	const float *f = reinterpret_cast<const float *>(&v);
	COORD4 res;
	float *r = reinterpret_cast<float *>(& res);
	for(int i=0;i<4;i++) {
		r[i] = 0.0f;
		for(int j=0;j<4;j++) 
			r[i] += f[j] * mat[j][i];
	}
	return res;
}
// Same order as matrices...
typedef enum {
  SP_BSPLINE=0,
  SP_BEZIER=1,
} SplineType;
XbMatrix Basis[] = {
	XbMatrix( -1.0f/6.0f,  3.0f/6.0f, -3.0f/6.0f,  1.0f/6.0f,
             3.0f/6.0f, -6.0f/6.0f,  3.0f/6.0f,  0.0f/6.0f,
            -3.0f/6.0f,  0.0f/6.0f,  3.0f/6.0f,  0.0f/6.0f,
             1.0f/6.0f,  4.0f/6.0f,  1.0f/6.0f,  0.0f/6.0f
  ),
  XbMatrix( -1.0f,  3.0f, -3.0f,  1.0f,
             3.0f, -6.0f,  3.0f,  0.0f,
            -3.0f,  3.0f,  0.0f,  0.0f,
             1.0f,  0.0f,  0.0f,  0.0f
  )
};

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#endif // unreferenced formal parameter

void tesselate(XbMatrix &UBasis, XbMatrix &VBasis,int nextU, int nextV,  
                 int uStride, int vStride,
                 int divisions,int numU,int numV,XbVec3f *coord,int *form,
                 XbArray<XbVec3f> &coords,
                 XbArray<XbVec3f> &normals,
                 XbArray<XbVec3f> &uvs) {
  divisions++;
  if (divisions > 100) divisions = 100;
  int i;
  int origNum = numU*numV;
  int origNumV = numV;
  int origNumU = numU;
  numU += form[0];
  numV += form[1];
  // Compute the T vector
  float tstep = 1.0f/(float)(divisions-1);
	
  // build the T vector
  float step = 0.0f;
  COORD4 mpTVals;
  COORD4 mpProdU[100],mpProdV[100];
  COORD4 mpProdUD[100],mpProdVD[100];
  for(i=0;i<divisions;i++) {
    mpTVals.w = 1.0f;  // const
    mpTVals.z = step;  // t
    mpTVals.y = mpTVals.z * step; //t*t
    mpTVals.x = mpTVals.y * step; //t*t*t
    mpProdU[i] = VecMatMul(mpTVals, UBasis);
    mpProdV[i] = VecMatMul(mpTVals, VBasis);
    //derivative for normals
    mpTVals.w = 0.0f;  // const
    mpTVals.z = 1.0f;  // const
    mpTVals.y = 2.0f * step; //2*t
    mpTVals.x = 3.0f * step * step; //3*t*t
    mpProdUD[i] = VecMatMul(mpTVals, UBasis);
    mpProdVD[i] = VecMatMul(mpTVals, VBasis);
    step += tstep;
  }
  COORD4 curveU[4],curveV[4],du,dv;
  COORD4 point[100][100];
  
  float ustep = tstep / (float(numU-(4-nextU))/float(nextU));
  float vstep = tstep / (float(numV-(4-nextV))/float(nextV));
  float uv[2] = {0.0f,0.0f};
  for(int m = 0; m < numU - 3; m+=nextU) {
    uv[0] = (float) m / (float)(numU-(4-nextU));
    for(int n = 0; n < numV - 3; n+=nextV) {
      uv[1] = (float)n / (float)(numV-(4-nextV));
      
      // We now have 16 CVS to play with.
      
      int l,j;
      XbVec3f xdu[100][100];
      XbVec3f xdv[100][100];
      for(l = 0; l < divisions; ++l) {
        int k;
        for(k = 0; k < 4; ++k) {
          COORD4 vert[4];
          for(int kk=0;kk<4;kk++) {
            int ix = ((m+kk)%origNumU)*origNumV+((n+k)%origNumV);
            vert[kk].x = coord[ix][0];
            vert[kk].y = coord[ix][1];
            vert[kk].z = coord[ix][2];
            vert[kk].w = 1.0f;
          }
          curveU[k] = Eval(vert,mpProdU[l]);
        }
        
        for(k = 0; k < 4; ++k) {
          COORD4 vert[4];
          for(int kk=0;kk<4;kk++) {
            int ix = ((m+k)%origNumU)*origNumV+((n+kk)%origNumV);
            vert[kk].x = coord[ix%origNum][0];
            vert[kk].y = coord[ix%origNum][1];
            vert[kk].z = coord[ix%origNum][2];
            vert[kk].w = 1.0f;
          }
          curveV[k] = Eval(vert, mpProdV[l]);
        }
        
        for(j=0;j < divisions; ++j) {
          du = Eval(curveU,mpProdUD[j]);
          dv = Eval(curveV,mpProdVD[j]);
          xdu[l][j] = XbVec3f((float*)&du);
          xdv[j][l] = XbVec3f((float*)&dv);
          point[l][j] = Eval(curveU, mpProdV[j]);
        }
      }
      for(l=0;l<divisions-1;l+=1) {
        //glBegin(GL_QUAD_STRIP);
        for(j=0;j < divisions; j++) {
          int ix = normals.getSize();
          if (j>1) {
            normals.append(normals[ix-2]);
            normals.append(normals[ix-1]);
            coords.append(coords[ix-2]);
            coords.append(coords[ix-1]);
            uvs.append(uvs[ix-2]);
            uvs.append(uvs[ix-1]);
          }
          //xdu[l][j].normalize();
          //xdv[l][j].normalize();
          XbVec3f cr1 = xdu[l][j].cross(xdv[l][j]);
          cr1.normalize();
          XbVec3f cr2 = xdu[l+1][j].cross(xdv[l+1][j]);
          cr2.normalize();
          XbVec3f p1((float*)&point[l][j]);
          XbVec3f p2((float*)&point[l+1][j]);
          XbVec3f norm1 = cr1;
          XbVec3f norm2 = cr2;
          float tmp[3];
          tmp[2] = 0.0f;
          tmp[0] = uv[0]+(l+1)*ustep;
          tmp[1] = uv[1]+j*vstep;
          normals.append(norm2);
          //glNormal3fv(norm1.getValue());
          uvs.append(XbVec3f(tmp));
          //glTexCoord2fv(tmp);
          coords.append(p2);
          //glVertex3fv(p1.getValue());
          normals.append(norm1);
          //glNormal3fv(norm2.getValue());
          tmp[0]-=ustep;
          uvs.append(XbVec3f(tmp));
          //glTexCoord2fv(tmp);
          coords.append(p1);
          //glVertex3fv(p2.getValue());
          
        }
        //glEnd();
      }
    }
  }
}

void LERP(float f1,float f2,float t,float &res) {
  res = f1+t*(f2-f1);
}
void quadInterpolate(XbVec3f *quad,XbVec3f *corners) {
  for(int i=0;i<4;i++) {
    XbVec3f &uv = quad[i];
    float u1,u2,ures;
    float v1,v2,vres;
    LERP(corners[0][0],corners[3][0],uv[0],u1);
    LERP(corners[1][0],corners[2][0],uv[0],u2);
    LERP(u1,u2,uv[1],ures);
    LERP(corners[0][1],corners[1][1],uv[1],v1);
    LERP(corners[3][1],corners[2][1],uv[1],v2);
    LERP(v1,v2,uv[0],vres);
    uv[0] = ures;
    uv[1] = vres;
  }
}
EA3_Status geoPrimTesselate(EA3Block *geoPrim,int divisions,XbPArray<Quad> &quads) {
  EA3Field *f1 = geoPrim->getByType(XBuType);
  EA3Field *f2 = geoPrim->getByType(XBvType);
  if (!f1 || !f2) return(EA3_ERROR);
  SplineType stu=SP_BSPLINE,stv=SP_BSPLINE;
  int nextU=0,nextV=0;
  if (strcmp(f1->data[0].str,"BSPLINE")==0) {stu = SP_BSPLINE; nextU=1;}
  else if (strcmp(f1->data[0].str,"BEZIER")==0) {stu = SP_BEZIER; nextU=3;}
  else return(EA3_ERROR);
  if (strcmp(f2->data[0].str,"BSPLINE")==0) {stv = SP_BSPLINE;nextV=1;}
  else if (strcmp(f2->data[0].str,"BEZIER")==0) {stv = SP_BEZIER;nextV=3;}
  else return(EA3_ERROR);
  EA3Field *form1 = geoPrim->getByType(XBuForm);
  EA3Field *form2 = geoPrim->getByType(XBvForm);
  int form[2] = {0,0};
  if (form1 && form1->data[0].str == XbIdentifier("PERIODIC")) form[0] = 3;
  if (form2 && form2->data[0].str == XbIdentifier("PERIODIC")) form[1] = 3;
  int numU=0,numV=0,uStride=0,vStride=0;
  EA3Field *f = geoPrim->getByType(XBnumUpoints);
  if (f) numU = f->data[0].i; else return(EA3_ERROR);
  f = geoPrim->getByType(XBnumVpoints); 
  if (f) numV = f->data[0].i; else return(EA3_ERROR);
  f= geoPrim->getByType(XBuStride);
  if(f) uStride = f->data[0].i; else return(EA3_ERROR);
  f= geoPrim->getByType(XBvStride);
  if(f) vStride = f->data[0].i; else return(EA3_ERROR);
  EA3Reference *cref = geoPrim->getByType(XBcoord);
  EA3Reference *uvRef = geoPrim->findByType(XBuv);
  XbVec3f corners[4];
  corners[0] = XbVec3f(0,0,0);
  corners[1] = XbVec3f(0,1,0);
  corners[2] = XbVec3f(1,1,0);
  corners[3] = XbVec3f(1,0,0);
  
  if (uvRef) {
    f = uvRef->getReference(0);
    corners[0] = XbVec3f(f->data[0].f,f->data[1].f,0);
    f = uvRef->getReference((numV-1)*vStride);
    corners[1] = XbVec3f(f->data[0].f,f->data[1].f,0);
    f = uvRef->getReference((numV-1)*vStride+(numU-1)*uStride);
    corners[2] = XbVec3f(f->data[0].f,f->data[1].f,0);
    f = uvRef->getReference((numU-1)*uStride);
    corners[3] = XbVec3f(f->data[0].f,f->data[1].f,0);
  }
  if (!cref) return(EA3_ERROR);
  int i;
  //for(i=0;i<4;i++) {
  //  printf("corner[%d] =%f %f\n",i,corners[i][0],corners[i][1]);
  //}
  XbVec3f *crd = new XbVec3f[cref->getNumReferences()];
  for(i=0;i<cref->getNumReferences();i++) {
    EA3FieldData *data = cref->getReference(i)->toField()->data;
    crd[i][0] = data[0].f;
    crd[i][1] = data[1].f;
    crd[i][2] = data[2].f;
  }
  int num = numU*numV;
  XbArray<XbVec3f> coords,normals;
  XbArray<XbVec3f> uvs;
  tesselate(Basis[stu],Basis[stv],nextU,nextV,0,0,divisions,
    numU,numV,crd,form,
    coords,normals,uvs
    );
  num = coords.getSize();
  quads.setSize(num/4);
  for(i=0;i<num;i+=4) {
    Quad &quad = quads[i/4];
    for(int j=0;j<3;j++) {
      quad.coord[2][j] = coords[i][j];
      quad.coord[3][j] = coords[i+1][j];
      quad.coord[1][j] = coords[i+2][j];
      quad.coord[0][j] = coords[i+3][j];

      quad.normal[2][j] = normals[i][j];
      quad.normal[3][j] = normals[i+1][j];
      quad.normal[1][j] = normals[i+2][j];
      quad.normal[0][j] = normals[i+3][j];

      quad.uv[2][j] = uvs[i][j];
      quad.uv[3][j] = uvs[i+1][j];
      quad.uv[1][j] = uvs[i+2][j];
      quad.uv[0][j] = uvs[i+3][j];
    }
    quadInterpolate(quad.uv,corners);  
  }
  return(EA3_OK);
}

//@}
} // namespace

