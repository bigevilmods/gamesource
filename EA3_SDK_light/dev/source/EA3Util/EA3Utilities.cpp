// (c) 1998-2010 Electronic Arts Inc.

#include "EA3.h"
#include "XbLinear.h"
#include "EA3SkelUtil.h"
#include "EA3GeomUtil.h"
#include "EA3Utilities.h"
#include "EA3CoordUtil.h"
#include "EA3Skeleton.h"
#include "eastl/hash_map.h"
#include "eastl/hash_set.h"
#include "EA3Expr.h"
#include "sys/stat.h"
//#include "pk3unzip.h"
#include <stdarg.h>
#include "EAiostream.h"

using namespace EA3Expr;
using namespace eastl;

namespace EA3Util {
  bool g_useOldByNameBehaviour = false;
	
	static GlobalFlags g_flags[] = {
		{
			XbString("useOldByNameBehaviour"),
			&g_useOldByNameBehaviour,
			"When no links reference in skeleton, search by name to find associated items like animation channels."
		},
		{XbString(""), NULL,""}
	};
	EA3_Status getGlobalFlag(const XbString &flagname,bool &value) {
		GlobalFlags *gf = g_flags;
		while(gf->flag) {
			if (flagname == gf->name) {
				value = *(gf->flag);
				return EA3_OK;
			}
		  gf++;
		}
		return EA3_NOT_FOUND;
	}
	EA3_Status getGlobalFlags(const GlobalFlags *&flags) {
	  flags = g_flags;
		return EA3_OK;
	}

	EA3_Status setGlobalFlag(const XbString &flagname,bool value) {
		GlobalFlags *gf = g_flags;
		while(gf->flag) {
			if (flagname == gf->name) {
				*(gf->flag) = value;
				return EA3_OK;
			}
		  gf++;
		}
		return EA3_NOT_FOUND;
	}

//
// For debug output of text
//
#ifdef _DEBUGx
#define DEBUG_OUT(txt); \
cout << txt << endl;
#else
#define DEBUG_OUT(txt);
#endif

void getEA3UtilVersion(int &major,int &minor,int &interm) {
  major = EA3UTIL_MAJOR_VERSION;
  minor = EA3UTIL_MINOR_VERSION;
  interm = EA3UTIL_INTERM_VERSION;
}
const char *getEA3UtilLabel() {
  return EA3UTIL_LABEL;
}

class EA3Remover {
public:
  EA3Remover(const XbPArray<EA3Base *> &baseList) {
    int i;
    int num = baseList.getSize();
    // The select callback will select recursively and keep a count of how many times an item is selected
    // Items that are selected more than once must have had a parent that was in the input list.
    for(i=0;i<num;i++) {
      select(baseList[i]);
    }
    XbPArray<EA3Base *> masterRemoveList;
    XbPArray<EA3Block *> masterContainerList;
    for(i=0;i<num;i++) {
      EA3Base *b = baseList[i];
      HM_EA3BaseInt::iterator it = removeCount.find(b);
      int count = (*it).second;
      if (count == 1) {
        masterRemoveList.append(const_cast<EA3Base *>(b));
        masterContainerList.append(b->getContainer());
      }
    }
    for(i=masterRemoveList.getSize()-1;i>=0;i--) {
      EA3Base *b = masterRemoveList[i];
      EA3Block *con = masterContainerList[i];
      HM_EA3BaseInt::iterator it = containerCount.find(con);
      int cc = (*it).second;
      if (cc == 0) continue; // we have handled this containers members.
      //printf("name = %s %s %s %d\n",b->getType().getString(),b->getName().getString(),b->getClassType(),cc);
      if (con == NULL && b->getScene()==b) {
        b->toBlock()->removeAllMembers();
        continue;
      }
      if (cc == 1) {
        con->removeMember(b);
      } else {
        (*it).second = 0; // so we don't do this container again
        int conNum = con->getNumMembers();
        if (conNum == cc) {
          con->removeAllMembers();
        } else {
          XbPArray<EA3Base *> tmp;
          tmp.setSize(conNum);
          tmp.setSize(0);
          int j;
          for(j=0;j<conNum;j++) {
            EA3Base *ch = con->getByIndex(j);
            if (removeCount.find(ch)!=removeCount.end()) {
              tmp.append(ch);
            }
          } 
          con->removeMembers(tmp);
        }
      }
    }
  }
  ~EA3Remover() {
  }
  void select(const EA3Base *b) {
    HM_EA3BaseInt::iterator i;
    i = removeCount.find(b);
    if (i != removeCount.end()) {
      (*i).second++;
      return; // we have incremented the container count and recursed already.
    } else {
      removeCount[b] = 1;
    }
    EA3Base *con = b->getContainer();
    i = containerCount.find(con);
    if (i != containerCount.end()) {
      (*i).second++;
    } else {
      containerCount[con] = 1;
    }
    const EA3Block *bl = b->toBlock();
    if (!bl) return;
    for(int c=0;c<bl->getNumMembers();c++) {
      select(bl->getByIndex(c));
    }
  }
  // DATA

  struct EA3BaseEqual {
    bool operator()(const EA3Base *b1,const EA3Base *b2) const {
      return(b1==b2);
    }
  }; 
  struct EA3BaseHash : public unary_function<const EA3Base *,size_t>  {
    size_t operator()(const EA3Base *b) const {
      return((size_t)b);
    }
  };
  typedef eastl::hash_map<const EA3Base *,int,EA3BaseHash,EA3BaseEqual, StlAllocator > HM_EA3BaseInt;
  HM_EA3BaseInt removeCount, containerCount;
}; // class EA3Remover 

/*!
 * 
 * Remove each item in list from its container.
 * Items in list may not be valid after this call.
 */
void EA3Remove(const XbPArray<EA3Base *> &baseList) {
  EA3Remover r(baseList);
}
void EA3Remove(const XbPArray<EA3Block *> &baseList) {
  EA3Remover r(reinterpret_cast<const XbPArray<EA3Base *> &>(baseList));
}


bool doesMatch(EA3Base *a1,EA3Base *a2,XbIdentifier ignoreType) {
  if (a1->isField() && a2->isField()) {
    EA3Field *f1 = a1->toField();
    EA3Field *f2 = a2->toField();
    if (f1->getDataType() != f2->getDataType()) return false;
    int num=f1->getNum();
    const char *dt = f1->getDataType();
    for(int i=0;i<num;i++) {
      switch (dt[i]) {
      case 'f': if (f1->data[i].f != f2->data[i].f) return false; break;
      case 'i': if (f1->data[i].i != f2->data[i].i) return false; break;
      case 'c': if (f1->data[i].ui != f2->data[i].ui) return false; break;
      case 'd': 
      case 's': if (f1->data[i].str != f2->data[i].str) return false; break;
      }
    }
    return true;
  }
  if (a1->isReference() && a2->isReference()) {
    EA3Reference *r1 = a1->toReference();
    EA3Reference *r2 = a2->toReference();
    int num = r1->getNumReferences();
    if (r2->getNumReferences() != num) {
      return false;
    }
    for(int i=0;i<num;i++) {
      if (!doesMatch(r1->getReference(i),r2->getReference(i),ignoreType)) return false;
    }
    return true;
  }
  if (a1->isBlock() && a2->isBlock()) {
    EA3Block *b1 = a1->toBlock();
    EA3Block *b2 = a2->toBlock();
    if (b1->getNumMembers() != b2->getNumMembers()) return false;
    bool isArray = false;
    if (b1->isArray()) isArray = true;
    for(int i=0;i<b1->getNumMembers();i++) {
      EA3Base *cb1 = b1->getByIndex(i);
      if (b1->getType()==ignoreType) continue;
      EA3Base *cb2 = isArray? b2->getByIndex(i) : b2->getByType(cb1->getType());
      if (!doesMatch(cb1,cb2,ignoreType)) {
        return false;
      }
    }
  }
  return true;
}

static EA3Block *findGeomMatch(EA3Array *geoms, EA3Block *geom,bool allowVariations) {
  EA3Array *coord = geom->getByType(XBcoord);
  for(int i=0;i<geoms->getNumMembers();i++) {
    EA3Block *cand = geoms->getByIndex(i);
    if (cand==geom) continue;
    EA3Array *ccoord = cand->getByType(XBcoord);
    if (ccoord && ccoord->getNumMembers()==coord->getNumMembers()) {
      EA3Block *match = cand;
      XbIdentifier ignore;
      if (allowVariations) ignore = XBmaterial;
      if (doesMatch(cand,geom,ignore)) {
        return match;
      }
    }
  }
  return NULL;
}
void EA3InstanceSkeleton(EA3Block *skeleton,bool allowVariations) {
  EA3Block *geom = NULL;
  skelGetGeometry(skeleton,geom);
  if (geom) {
    EA3Block *model = skeleton;
    while(model->getType()!=XBmodel) model=model->getContainer();
    EA3Array *geoms = model->getByType(XBgeometry);
    EA3Block *match = findGeomMatch(geoms,geom,allowVariations);
    if (match) {
      EA3Block *otherSkel = NULL;
      geomGetSkeleton(match,otherSkel);
      skelSetGeom(skeleton,match);
      EA3Reference *instanceOf = skeleton->getByType(XBbone)->toBlock()->createReference(XBinstanceOf);
      instanceOf->addReference(otherSkel->getByType(XBbone));
      geom->getContainer()->removeMember(geom);
    }
  }
  EA3Array *skels = skeleton->getByType(XBskeleton);
  if (skels) {
    for(int i=0;i<skels->getNumMembers();i++) {
      EA3InstanceSkeleton(skels->getByIndex(i)->toBlock(),allowVariations);
    }
  }
}

void EA3UninstanceSkeleton( EA3Block *skeleton) {
  EA3Block *bone = skeleton->getByType("bone");
  EA3Block *iGeom = NULL;
  skelGetGeometry(skeleton,iGeom);
  if (iGeom && bone) {
    EA3Reference *inst = bone->getByType("instanceOf");
    if (inst) {
      EA3Block *mBone = inst->getReference(0);
      if (mBone) {
        EA3Block *mSkel = mBone->getContainer();
        EA3Block *mGeom = NULL;
        EA3Reference *links = bone->getByType("links");
        if (!links) {
          links=bone->createReference("links");
          links->addReference(iGeom);
        }
        skelGetGeometry(mSkel,mGeom);
        EA3Block *newGeom = mGeom->copy(mGeom->getContainer())->toBlock();
        newGeom->setName(skeleton->getName());
        links->replaceReference(iGeom,newGeom);  
        if (iGeom != mGeom) {
          EA3Array *geoPrimN = newGeom->getByType(XBlod)->toBlock()->findByType(XBgeoPrim);
          EA3Array *geoPrimI = iGeom->getByType(XBlod)->toBlock()->findByType(XBgeoPrim);
          for(int i=0;i<newGeom->getNumMembers();i++) {
            EA3Block *primN=geoPrimN->getByIndex(i);
            EA3Block *primI=geoPrimI->getByIndex(i);
            if (!primI) break;
            EA3Reference *matN = primN->getByType(XBmaterial);
            EA3Reference *matI = primI->getByType(XBmaterial);
            if (matN && matI) {
              matN->removeAllReferences();
              matN->addReference(matI->getReference(0));
            } else if (matN) {
              primN->removeMember(matN);
            }
          }
          iGeom->getContainer()->removeMember(iGeom);
        }
      }
      inst->getContainer()->removeMember(inst);
    }
  }
  EA3Array *chs = skeleton->getByType("skeleton");
  if (chs) {
    for(int i=0;i<chs->getNumMembers();i++) {
      EA3UninstanceSkeleton(chs->getByIndex(i));
    }
  }
}


int
EA3FreezeAnimation(EA3RecordedSkeleton &rs)
{
  
  //
  // Iterate through each child skeleton
  //
  for( int currchild=0; currchild<rs.children.getSize(); currchild++ ) {
    //
    // Iterate through each frame
    //
    for( int i=0; i<rs.numFrames; i++ ) {
      
      // Get the frame's transformation data
      XbMatrix Transformation;
      rs.getAnimLocal(Transformation, i);
      XbMatrix child_Transformation;
      rs.children[currchild]->getAnimLocal(child_Transformation, i);
      
      // Calculate the new
      XbMatrix new_child_Transformation = child_Transformation * Transformation;
      
      // Set the new value
      animSaveKey(rs.anim,rs.children[currchild]->channelList, new_child_Transformation, i);
      
    }
  }
  
  
  //
  // Freeze the animation of the current skeleton
  //
  setStaticAnimation(rs);
  
  
  ///////////////////
  //
  // Return an OK
  //
  return EA3_OK;
}

int
EA3FreezeHierarchy(EA3Block *skel, int recurse, EA3Block *anim)
{
  EA3RecordedSkeleton rs(skel, anim);
  return EA3FreezeHierarchy(rs, recurse);
}

int
EA3FreezeHierarchy(EA3RecordedSkeleton &rs, int recurse, bool updateAnimation, bool nonewchannels)
{
  //cerr << "Adjusting " << rs.skel->getName() << endl;  // DEBUG
  
  //
  // Some variables
  //
  int i;                                          // For loops
  int numchildren = rs.children.getSize();        // The number of children
  
  //
  // RECURSSION...
  // Apply the freeze onto each of the children
  //
  for( i=0; recurse && i<numchildren; i++) {
    //
    // Freeze the child now
    //
    EA3FreezeHierarchy(*rs.children[i],recurse,false,true);
  }

  //
  // Get the skeleton's local transform matrix
  //
  XbMatrix skelmatrix_local;
  rs.getInitLocal(skelmatrix_local);
  
  //
  // Transform the geometry corresponding to the current skeleton
  //
  EA3Block* geom = NULL;
  if( skelGetGeometry(rs.skel,geom) == EA3_OK ) {
    geomApplyTransform(geom, skelmatrix_local);
  }
  
  //
  // Reset the local matrix to the identity
  //
  for( i=0; i<rs.numFrames+2; i++ ) {
    rs.local[i] = skelmatrix_local.identity();
  }
  if (rs.par) {
    for(i=0;i<rs.numFrames+2;i++) {
      rs.global[i] = rs.local[i]*rs.par->global[i];
    }
  } else {
    for(i=0;i<rs.numFrames+2;i++) {
      rs.global[i] = rs.local[i];
    }
  }
  
  if( updateAnimation ) {
    //
    // Iterate through the chidren and update their local matricies
    //
    for( i=0; i<numchildren; i++) {
      rs.children[i]->updateLocal();
    }
    
    //
    // Apply the changes
    //
    EA3ApplyRecordedSkeleton(rs, false, false);
    for( i=0; i<rs.children.getSize(); i++ ) {
      EA3ApplyRecordedSkeleton(*rs.children[i],true,!nonewchannels);
    }
  }
   
  
  /////////////////////////////////
  //
  // Return true
  //
  return EA3_OK;
}


int
EA3ReParentSkeleton(EA3Block *newParent, EA3Block *child, EA3Block* anim)
{
  //
  // Generate 'proper' recorded skeletons pointers for child and its new parent
  //
  int e;
  EA3RecordedSkeleton *rs_child_structure=NULL, *rs_newparent_structure=NULL, *rs_child=NULL, *rs_newparent=NULL;
  e = getProperRecordedSkeleton(child, anim, rs_child_structure, rs_child);
  if( e != EA3_OK ) return e;
  e = getProperRecordedSkeleton(newParent, anim, rs_newparent_structure, rs_newparent);
  if( e != EA3_OK ) return e;
  
  //
  // ReParent the skeleton
  //
  int res = EA3ReParentSkeleton( rs_newparent, rs_child );
  delete rs_child_structure;
  delete rs_newparent_structure;
  return res;
}

int
EA3ReParentSkeleton(EA3RecordedSkeleton *newParent, EA3RecordedSkeleton *child, bool updateAnimation,bool nonewchannels)
{
  //
  // Check if child is already a child of newParent
  //
  if( child->par==newParent ) return EA3_OK;
  
  //
  // Some variables
  //
  XbPArray< XbMatrix >  matrixList;   // for resotring global animation
  
  //
  // Make sure newParent is not a descendent of child
  //
  EA3Block *parentcheck = newParent->skel;
  while(parentcheck) {
    if( parentcheck == child->skel ) {    // The child is a parent of the new parent
      //cerr << "ERR:  Tried to reParent with a newParent that was a descendent of the child\n";
      return EA3_NOT_FOUND;
    }
		parentcheck=parentcheck->getContainer();
		if (parentcheck) parentcheck=parentcheck->getContainer();
		// this will not give us the 'model' block.
    //skelGetParent(parentcheck,tmp);
    //parentcheck = tmp;
  }
  
  //
  // Remember the old parent
  //
  EA3RecordedSkeleton *old_parent = child->par;
  
  //
  // Remove the child from its old position and move it to the new
  // NOTICE THE CODE HAS BEEN EXPANDED HERE INSTEAD OF USING
  // THE BUILT IN COMMANDS.  THIS IS TO AVOID THE SKELETON
  // BEING REMOVED TWICE (WHICH TAKES AN UNRELATED SKELETON WITH IT)
  //
  //if(old_parent) old_parent->cutChild(child);
  if(old_parent) {
    for(int i=0;i<old_parent->children.getSize();i++) {
      if (old_parent->children[i] == child) {
        old_parent->children.remove(i);
        child->par = NULL;
        //child->skel->ref();
        if (updateAnimation) child->updateLocal();
        //EA3Array *childs = (EA3Array*)old_parent->skel->getByType("skeleton");
        //if (childs) {
        //  childs->removeMember(child->skel);
        //}
      }
    }
    
  }
  
  
  //newParent->addChild(child);
  newParent->children.append(child);
  child->par = newParent;
  if (updateAnimation) child->updateLocal(); 
  EA3Array *childs = (EA3Array *)(newParent->skel)->getByType("skeleton");
  if (!childs) childs = (newParent->skel)->createArray("skeleton","children");
  childs->appendMember(child->skel);    
  
  //
  // If the moved node was the only child of the old parent,
  // the children array can be removed
  //
  if(old_parent && old_parent->skel) {
    int removearraycheck=0;
    skelGetNumChildren(old_parent->skel,removearraycheck);
    if(removearraycheck==0) {
      EA3Base* childarray = old_parent->skel->getByType("skeleton");
      if(childarray) old_parent->skel->removeMember(childarray);
    }
  }
  
  
  //
  // Correct the local matrices so that the global matrix is still the same
  //
  if (updateAnimation) child->updateLocal();
  
  //
  // Apply the changes
  //
  if(updateAnimation) EA3ApplyRecordedSkeleton(*child,true,nonewchannels);
  
  /////////////////////////////////
  //
  // Return true
  //
  return EA3_OK;
}



// Flatten a skeleton into one node
static void flatten_recursivelyremovechildren(EA3RecordedSkeleton *rs) {
  int i;
  for( i=0; i<rs->children.getSize(); i++ ) {
    flatten_recursivelyremovechildren(rs->children[i]);
    
    // remove animation
    XbString stuff2go = rs->children[i]->skel->getName();
    EA3Remove(rs->children[i]->channelList);
    rs->children[i]->channelList.setSize(0);
    
    // find and remove envelope
    EA3Base *b = rs->anim; if(!b) b=rs->skel;
    while(b && b->getType()!=XBmodel) b = b->getContainer();
    if( b->toBlock() ) {
      EA3Block *envelope = b->toBlock()->getByType("envelope");
      if (envelope) {
        XbPArray<EA3Base*> clusters2go;
        find( envelope, clusters2go, HasContainer(IsArray() && HasType("cluster")) && (
          HasName(stuff2go) || HasMember(HasType("bone")&&IsReference()&&HasReference(HasName(stuff2go))) ));
        for( int delclust=0; delclust<clusters2go.getSize(); delclust++ ) {
          if( clusters2go[delclust] ) {
            EA3Block *cnt = clusters2go[delclust]->getContainer();
            cnt->removeMember(clusters2go[delclust]);
            while( cnt->getContainer() && !cnt->getNumMembers() ) {
              EA3Base *last = cnt;
              cnt = cnt->getContainer();
              cnt->removeMember(last);
            }
          }
        }
      }
    }
    
    // Delete the child rs
    delete rs->children[i];
  }
  rs->children.setSize(0);
  EA3Array* childarray = rs->skel->getByType(XBskeleton);
  if(childarray) rs->skel->removeMember(childarray);   
}
typedef struct {
  EA3RecordedSkeleton *rs;
  EA3Block *skel;
  EA3Block *geom;
} _RS_S_Gbind;
void
flatten(EA3RecordedSkeleton& rs)
{
  if( !rs.skel ) return;
  EA3UninstanceSkeleton(rs.skel);
  int i;
  bool hasgeometry = false;
  EA3Block* skel = rs.skel;
  XbPArray< _RS_S_Gbind > infoList;
  XbPArray< EA3Base* > skelList;
  /*
  1   Collect all geometry in children
  2   If geomlist.size > 0 && !skel.geom create skel.geom
  3   Merge all geom into skel.geom
  4   remove all children and their geometry
  */
  
  //
  // Collect all geometry
  //
  EA3Block *model = skel;
  while( model && model->getType()!=XBmodel ) model = model->getContainer();
  if( !model ) return;
  Expression e = HasType("skeleton")&&HasMember(HasType("bone")&&HasMember(HasType("type")&&HasValue("geom")));
  find(skel, skelList, e);
  for( i=0; i<skelList.getSize(); i++ ) {
    _RS_S_Gbind mydatabind;
    mydatabind.skel = skelList[i]->toBlock();
    mydatabind.rs   = findSkeleton(&rs,mydatabind.skel);
    mydatabind.geom = NULL;
    skelGetGeometry(mydatabind.skel,mydatabind.geom);
    if( mydatabind.geom ) hasgeometry = true;
    infoList.append(mydatabind);
  }
  
  if( hasgeometry ) {
    //
    // Create our geometry
    //
    EA3Block *geomMaster = NULL;
    skelGetGeometry(skel,geomMaster);
    if( !geomMaster ) {
      EA3Array* geos = model->getByType(XBgeometry);
      if(geos) geomMaster = geos->createBlock(XBgeometry, skel->getName());
      skelSetBoneType(skel,"geom");
      EA3Block *bone = skel->getByType("bone");
      bone->createReference("links")->addReference(geomMaster);
    }
    
    //
    // Merge geometry, and remove the old
    //
    for( i=0; i<infoList.getSize(); i++) {
      // if we created a geomMaster above, make sure we remember that in the infoList
      if (infoList[i].skel == skel) infoList[i].geom = geomMaster;
      if( infoList[i].geom==geomMaster ) continue;
      XbMatrix childMat;
      XbMatrix masterMat;
      XbMatrix mat;
      // compute new coordsys of geometry
      rs.getGlobal(masterMat);
      infoList[i].rs->getGlobal(childMat);

      if( infoList[i].geom )
      {
        mat = childMat * masterMat.inverse();
        geomApplyTransform(infoList[i].geom, mat);
        geomMerge(geomMaster,infoList[i].geom);
        EA3Block *geos = infoList[i].geom->getContainer();

        //
        //  Remove the geometry references from the skeleton bone block.
        //
        EA3Reference *geomRef = infoList[i].skel->getByType( XBlinks );
        if( geomRef )
        {
            int r;
            for( r = geomRef->getNumReferences() - 1; r >= 0; r-- )
            {
                EA3Base *base = geomRef->getReference( r );

                if( base == infoList[i].geom )
                {
                    (void)geomRef->removeReference( base );
                }
            }
        }

        geos->removeMember(infoList[i].geom);
      }

      skelSetBoneType(infoList[i].skel, "null");
    }
  }
  
  //
  // Remove the children
  //
  flatten_recursivelyremovechildren(&rs);
}



int
EA3FlattenHierarchy(EA3Block *skel, EA3Block *anim)
{
  EA3RecordedSkeleton rs(skel, anim);
  return EA3FlattenHierarchy(rs);
}

int
EA3FlattenHierarchy(EA3RecordedSkeleton &rs)
{
  
  //
  // Collapse the skeleton into one node
  //
  flatten(rs);
  
  return EA3_OK;
}


int
EA3MergeHierarchies(EA3Block *master, EA3Block *slave, EA3Block *anim)
{
  EA3RecordedSkeleton rs_master(master, anim);
  EA3RecordedSkeleton rs_slave(slave, anim);
  return EA3MergeHierarchies(&rs_master, &rs_slave);
}

int
EA3MergeHierarchies(EA3RecordedSkeleton *master, EA3RecordedSkeleton *slave, bool updateAnimation,bool nonewchannels)
{
  DEBUG_OUT("---> EA3MergeHierarchies(...)");
  int i;
  
  EA3Block *parentcheck = master->skel, *tmp;
  while(parentcheck) {
    if( parentcheck == slave->skel ) {    // The child is a parent of the new parent
      //cerr << "ERR:  Tried to merge hierarchies with a master that was a descendent of the slave\n";
      return EA3_NOT_FOUND;
    }
    skelGetParent(parentcheck,tmp);
    parentcheck = tmp;
  }
  
  /*
  The Plan...
  
   (1) create a dummy
   (2) reparent all child nodes onto the dummy
   (3) reparent the slave under the master
   (4) flatten the master
   (5) reparent all the child nodes onto the newly merged master
   (6) remove the dummy
  */
  
  //
  // Create the dummy
  //
  DEBUG_OUT("Create the dummy");
  EA3Block* skel_dummy = NULL;
  EA3Array* skels = NULL;
  EA3Scene *s = master->skel->getScene();
  for( i=0; i<s->getNumMembers(); i++) {
    EA3Block* b = (EA3Block*)s->getByIndex(i);
    if( b->getType() == "model") {
      skels = (EA3Array*)b->getByType("skeleton");
      skel_dummy = skels->createBlock("skeleton", "__dummy");
      //dummy->setBoneType("null");
      XbMatrix m;
      skelSetBoneType(skel_dummy,"geom");
      skelSetMatrix(skel_dummy,m.identity(), "init");
      skelSetMatrix(skel_dummy,m.identity(), "local");
      break;  // Should be exactly one desired block
    }
  }
  if(skels == NULL  ||  skel_dummy == NULL ) {
    //cerr << "ERR: Cannot generate dummy skeleton!!\n";
    return 0;
  }
  EA3RecordedSkeleton *dummy = new EA3RecordedSkeleton(skel_dummy, master->anim);
  
  
  //
  // Go through all of the children of slave and reparent them to our dummy
  // NOTE:  the slave goes first because the slave may be a descendent of its new master
  //
  DEBUG_OUT("reparent all child nodes onto the dummy");
  // slave
  while(slave->children.getSize()>0) {
    EA3ReParentSkeleton(dummy, slave->children[0],updateAnimation,nonewchannels);
  }
  // master
  while(master->children.getSize()>0) {
    EA3ReParentSkeleton(dummy, master->children[0],updateAnimation,nonewchannels);
  }
  
  //
  // Reparent the slave under the master
  //
  DEBUG_OUT("reparent the slave under the master");
  EA3ReParentSkeleton(master, slave,updateAnimation,nonewchannels);
  
  //
  // Flatten our new master structure
  // (note: the slave should now be lost!)
  //
  DEBUG_OUT("Flatten our new master structure");
  EA3FlattenHierarchy(*master);
  
  //
  // Reparent all the dummy's children back to
  // the master
  //
  DEBUG_OUT("Reparent all the dummy's children back to the master");
  while(dummy->children.getSize()>0) {
    //cerr << "dummy has " << dummy->children.getSize() << " children\n";
    EA3ReParentSkeleton(master, dummy->children[0],updateAnimation,nonewchannels);
  }
  
  //
  // Remove the dummy
  //
  DEBUG_OUT("Remove the dummy");
  skels->removeMember((EA3Base*)dummy->skel);
  delete dummy;
  
  /////////////////////////////////
  //
  // Return true
  //
  DEBUG_OUT("<---");
  return EA3_OK;
}

int
setStaticAnimation(EA3RecordedSkeleton &rs)
{
  XbMatrix mat; mat.makeIdentity();
  return setStaticAnimation(rs,mat);
}

int
setStaticAnimation(EA3RecordedSkeleton &rs, XbMatrix mat)
{
  //
  // Set the static animation
  //
  for( int i=0; i<rs.numFrames; i++ ) {
    int e = animSaveKey(rs.anim,rs.channelList, mat, i);
    if( e!= EA3_OK ) return e;
  }
  
  //
  // Recalculate the global matrix
  //
  rs.updateGlobal();
  
  ///////////////////
  //
  // Return an OK
  //
  return EA3_OK;
}


int
getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton &rs_head, EA3RecordedSkeleton *&rs_node)
{
  //
  // Check for errors
  //
  if( !skel_node ) {
    return EA3_WRONG_TYPE;
  }
  
  //
  // Find the head node of the skeleton
  //
  EA3Block* headnode = skel_node;
  for(;;) {
    EA3Block* tmp;
    skelGetParent(headnode,tmp);
    if(tmp) headnode=tmp;
    else break;
  }
  
  //
  // Generate our ever famous 'proper recorded skeleton'
  //
  EA3RecordedSkeleton temp(headnode,anim);
  rs_head = temp;
  
  
  //
  // Find the skel_node
  //
  rs_node = findSkeleton(&rs_head, skel_node);
  
  
  ////////////////////////////////////////////////////////////////
  //
  // Return an OK if rs_node is valid or EA3_NOT_FOUND otherwise
  //
  return (rs_node==NULL)?EA3_NOT_FOUND:EA3_OK;
}

int
getProperRecordedSkeleton(EA3Block *skel_node, EA3Block *anim, EA3RecordedSkeleton *&rs_head, EA3RecordedSkeleton *&rs_node)
{
  //
  // Check for errors
  //
  rs_head = NULL;
  if( !skel_node ) {
    return EA3_WRONG_TYPE;
  }
  
  //
  // Find the head node of the skeleton
  //
  EA3Block* headnode = skel_node;
  for (;;) {
    EA3Block* tmp;
    skelGetParent(headnode,tmp);
    if(tmp) headnode=tmp;
    else break;
  }
  
  //
  // Generate our ever famous 'proper recorded skeleton'
  //
  rs_head = new EA3RecordedSkeleton(headnode,anim);
  
  
  //
  // Find the skel_node
  //
  rs_node = findSkeleton(rs_head, skel_node);
  
  
  ////////////////////////////////////////////////////////////////
  //
  // Return an OK if rs_node is valid or EA3_NOT_FOUND otherwise
  //
  return (rs_head==NULL || rs_node==NULL)?EA3_NOT_FOUND:EA3_OK;
}


// These two functions change the behaviour two ways: first, a breadth search is done,
// so siblings are searched before children (less cache thrash) and each single child
// is checked within the parent function call, reducing the number of branch
// instructions executed
EA3RecordedSkeleton* findSkeletonChild(EA3RecordedSkeleton *rs, EA3Block *skel)
{
  int i, count = rs->children.getSize();
  for(i=0; i<count; i++)
  {
	  if(rs->children[i]->skel == skel)
		  return rs->children[i];
  }

  for(i=0; i<count; i++)
  {
    EA3RecordedSkeleton* foundit = findSkeletonChild(rs->children[i], skel);
    if( foundit != NULL ) return foundit;
  }

  return NULL;
}

EA3RecordedSkeleton* findSkeleton(EA3RecordedSkeleton *rs, EA3Block *skel)
{
  if( rs->skel == skel ) return rs;
  return findSkeletonChild(rs, skel);
}


EA3RecordedSkeleton*
findSkeleton(EA3RecordedSkeleton *rs, const XbString &name) {
  if( !rs || !rs->skel ) return NULL;
  if( rs->skel->getName()==name ) return rs;
  for( int i=0; i<rs->children.getSize(); i++ ) {
    EA3RecordedSkeleton *res = findSkeleton(rs->children[i],name);
    if( res ) return res;
  }
  return NULL;
}



int
EA3FreezeHierarchyWorld(EA3Block *skel, int recurse, EA3Block *anim)
{
  if( anim == NULL ) return EA3_NOT_FOUND;
  EA3RecordedSkeleton rs(skel, anim);
  return EA3FreezeHierarchyWorld(rs, recurse);
}


int
EA3FreezeHierarchyWorld(EA3RecordedSkeleton &rs, int recurse, bool updateAnimation, bool nonewchannels)
{
  //cerr << "EA3FreezeHierarchyWorld(" << recurse << ") " << rs.skel->getName().getString() << endl;  // DEBUG
  int i, e;
  int currchild;
  int numchildren = rs.children.getSize();        // The number of children
  
  /*
  How it works:
  (1) If recurssion is on, call EA3FreezeHierarchy on the children.
  (2) Do the same as EA3FreezeHierarchy
  (3) Set global animation to identity matrix
  (4) Fix the local animation and apply it to the skeleton
  (5) Do the recurssion
  */  
  
  //
  // RECURSSION...
  // Now, apply the freeze onto each of the children
  //
  for( currchild=0; recurse && currchild<numchildren; currchild++ ) {
    e = EA3FreezeHierarchyWorld(*rs.children[currchild],recurse,false,true);
    if( e != EA3_OK ) return e;
  }
  
  //
  // Get the skeleton's global transform matrix
  //
  XbMatrix skelmatrix_global;
  if( rs.numFrames>0 ) rs.getAnimGlobal(skelmatrix_global,0);
  else rs.getGlobal(skelmatrix_global);
  
  //
  // Transform the geometry corresponding to the current skeleton
  //
  EA3Block* geom = NULL;
  if( skelGetGeometry(rs.skel,geom) == EA3_OK ) {
    geomApplyTransform(geom, skelmatrix_global);
  }
  
  //
  // Reset the global matrix to the identity
  //
  for( i=0; i<rs.numFrames+2; i++ ) {
    rs.global[i].makeIdentity();
  }
  
  //
  // Apply the changes
  //
  if( updateAnimation ) {
    rs.updateLocal();
    EA3ApplyRecordedSkeleton(rs, false, false);
    for( i=0; i<rs.children.getSize(); i++ ) {
      EA3ApplyRecordedSkeleton(*rs.children[i],true,!nonewchannels);
    }
  }
  
  
  /////////////////////////////////
  //
  // Return OK
  //
  return EA3_OK;
}

int EA3RemoveAnimation(const XbString& anim2go, EA3Block *anim)
{
  //
  // Remove animation channel
  //
  XbPArray<EA3Base *> rlist;
  EA3Block *chlist=(EA3Block*)anim->getByType("channel");
  if( chlist ) {
    for(int i=0; i<chlist->getNumMembers(); i++) {
      EA3Base * currchannel = chlist->getByIndex(i);
      if( currchannel->getName()==anim2go ) {
        rlist.append(currchannel);
      }
    }
    chlist->removeMembers(rlist);
  }
  
  return(EA3_OK);
}


int
EA3RemoveCluster(XbString& cluster2go, EA3Block *cluster)
{
  //
  // Remove cluster
  //
  if( cluster ) {
    for(int i=0; i<cluster->getNumMembers(); i++) {
      EA3Base * currclust = cluster->getByIndex(i);
      if( currclust->getName()==cluster2go ) {
        cluster->removeMember(i);
        i--;
      }
    }
  }
  
  return(EA3_OK);
}


int
EA3ApplyRecordedSkeleton(EA3RecordedSkeleton &rs, bool recurse, bool createchans)
{
  int i, e;
  XbMatrix M;
  if( !rs.skel ) return EA3_NOT_FOUND;
  
  if( rs.skel->getType()==XBskeleton )
  {
    // Set local matrices
    EA3Block *skel = rs.skel;
    rs.getInitLocal(M);
    skelSetMatrix(skel,M, XBinit);

    rs.getLocal(M);
    skelSetMatrix(skel,M, XBlocal);
    int startFrame = 0;
    EA3Field *sf = rs.anim->getByType(XBstartFrame);
    if (sf) startFrame = sf->data[0].i;


	if(rs.numFrames)
	{
	ChannelsInfo eulChan, trnChan;

		if(createchans)
			animBuildChannels(rs.anim, rs.channelList, rs.skel->getName(), skel);

		EA3Field *fps = rs.anim->getByType(XBFPS);
		float FPS = fps ? fps->data[0].f : 30.0f;

		eulChan.StartFrame = startFrame;
		trnChan.StartFrame = startFrame;
		eulChan.fps = FPS;
		trnChan.fps = FPS;

		animBuildChannelInfo(rs.anim, rs.channelList, eulChan, trnChan);

		bool hasAnimClips = false;
		for(i=0;i<rs.channelList.getSize();i++) {
			EA3Block *clip = rs.channelList[i];
			if (clip->getType() == XBanimClip) {
				hasAnimClips = true;
				break;
			}
		}

		const XbMatrix *pMat = &rs.local[2];	// Do this to avoid copying memory

		if(trnChan.Count)
		{
			for(i=0; i<rs.numFrames; i++)
				channelListSaveKeyFast(trnChan, *(const XbVec3f *)pMat[i][3], i+startFrame);
		}

		if(eulChan.Count)
		{
			for(i=0; i<rs.numFrames; i++)
			{
				XbEuler eul(pMat[i], XbEuler::XYZ, XbEuler::STATIC);
				channelListSaveKeyFast(eulChan, eul, i+startFrame);
			}
		}
		if (hasAnimClips) {
			for(i=0; i<rs.numFrames; i++)
			{
				XbEuler eul(pMat[i], XbEuler::XYZ, XbEuler::STATIC);
				XbVec3f trn(pMat[i][3]);
				XbVec3f scale(1.0f,1.0f,1.0f);
				skelUpdateAnimClipValues(rs.skel,rs.channelList,i+startFrame,scale,eul,trn);
			}
		}
	}
  }
  
  // recurse children
  for( i=0; recurse&&i<rs.children.getSize(); i++ ) {
    e = EA3ApplyRecordedSkeleton(*rs.children[i],true,createchans);
    if(e!=EA3_OK) return e;
  }
  
  return EA3_OK;
}


int
EA3Collapse(EA3RecordedSkeleton &rs, bool updateAnimation, bool nonewchannels)
{
  DEBUG_OUT("---> EA3Collapse(...)");
  
  /*
  *    A                   B(merge A geom)   E
  *    |\                 / \
  *    B E         ----> C   D
  *   / \
  *  C   D
  How it works:
  (1)  If there are no children, remove the node
  (2)  Merge A's geometry into B
  (3)  Replace A's children array with B's
  (4)  Update children's parents
  (5)  Replace A's channel array with B's (delete A's)
  (6)  Copy B's global animation to A
  (7)  Replace A's skeleton with B's
  (8)  Fix up the skeletal block structure
  (9)  Update the A's children's parents
  (10) Deallocate the B recorded skeleton
  (11) Erase A's original animation
  (12) Correct local animation
  */
  
  //
  // If there are no children, do nothing
  //
  if( rs.children.getSize() <= 0 ) {
    if(!rs.par) {
      //cerr << "ERR: Can't remove only node\n";
      return EA3_NOT_FOUND;
    } else {
      EA3Remove(rs.channelList);
      rs.channelList.setSize(0);
      rs.par->cutChild(&rs);
    }
    return EA3_OK;
  }
  
  //
  // Something to remember
  //
  int i;
  EA3Block *oldskel = rs.skel;
  EA3RecordedSkeleton *B=NULL;
  B = rs.children[0];
  if( !B->skel ) return EA3_NOT_FOUND;
  
  //
  // Merge A's geometry into B
  //
  DEBUG_OUT("  Merge A's geometry into B");
  /*
  EA3Geometry *geom=NULL, *geomMaster=NULL;
  rs.skel->getGeometry(geom);
  B->skel->getGeometry(geomMaster);
  if( geom && geomMaster ) {
  Merge(geomMaster, geom);
  }
  */
  
  //
  // Replace A's children array with B's
  //
  XbPArray<EA3RecordedSkeleton *> children;
  children += &rs;
  DEBUG_OUT("  Replace A's children array with B's");
  if (rs.par) {
    for( i=1; rs.par&&i<rs.children.getSize(); i++ ) {
      rs.par->children += rs.children[i];
      children += rs.children[i];
    }
  } else {
    for(i=1;i<rs.children.getSize();i++) {
      rs.children[i]->par = NULL;
      children += rs.children[i];
    }
  }
  rs.children.clear();
  rs.children.setSize(0);
  for( i=0; i<B->children.getSize(); i++ ) {
    rs.children += B->children[i];
  }
  
  //
  // Update the children's parents
  //
  if( rs.par ) {
    for(i=0; i<rs.par->children.getSize(); i++) {
      rs.par->children[i]->par = rs.par;
    }
  }
  
  //
  // Replace A's channel array with B's
  //
  DEBUG_OUT("  Replace A's channel array with B's");
  for( i=0; i<rs.channelList.getSize(); i++ ) {
    EA3Block *ch = rs.channelList[i];
    if( ch&&ch->getContainer() ) ch->getContainer()->removeMember(ch);
  }
  rs.channelList.setSize(0);
  for( i=0; i<B->channelList.getSize(); i++ ) {
    rs.channelList += B->channelList[i];
  }
  
  //
  // Copy B's global animation to A
  //
  DEBUG_OUT("  Copy B's global animation to A");
  for( i=0; i<rs.numFrames+2; i++ ) {
    rs.global[i] = B->global[i];
  }
  
  //
  // Replace A's skeleton with B's
  //
  DEBUG_OUT("  Replace A's skeleton with B's");
  rs.skel = B->skel;
  
  //
  // Fix up the skeletal block structure
  //
  DEBUG_OUT("  Fix up the skeletal block structure");
  int numchilds=0;
  skelGetNumChildren(oldskel,numchilds);
  EA3Block *child=NULL;
  EA3Block *skels = oldskel->getContainer();
  while(numchilds>0) {
    skelGetChild(oldskel,0, child);
    skels->appendMember(child);
    skelGetNumChildren(oldskel,numchilds);
  }
  skels->removeMember(oldskel);
  
  //
  // Update the A's children's parents
  //
  for( i=0; i<rs.children.getSize(); i++ ) {
    rs.children[i]->par = &rs;
  }
  
  //
  // Deallocate the B recorded skeleton
  //
  DEBUG_OUT("  Deallocate the B recorded skeleton");
  B->children.setSize(0);
  B->channelList.setSize(0);
  delete B;
  
  //
  // Correct local animation
  //

  DEBUG_OUT("  Correct local animation");
  for(i=0; i<children.getSize(); i++) {
    children[i]->updateLocal();
    if( updateAnimation ) {
      EA3ApplyRecordedSkeleton(*children[i], true,!nonewchannels);
    }
  }   

  //
  // Return OK
  //
  DEBUG_OUT("<---");
  return EA3_OK;
}


static void expandbbox(Float3 &min_bbox, Float3 &max_bbox, Float3 &coord) {
  if( coord[0]<min_bbox[0] ) min_bbox[0] = coord[0];
  if( coord[1]<min_bbox[1] ) min_bbox[1] = coord[1];
  if( coord[2]<min_bbox[2] ) min_bbox[2] = coord[2];
  if( coord[0]>max_bbox[0] ) max_bbox[0] = coord[0];
  if( coord[1]>max_bbox[1] ) max_bbox[1] = coord[1];
  if( coord[2]>max_bbox[2] ) max_bbox[2] = coord[2];
}
static bool getAllGeomsFromRS(EA3RecordedSkeleton *rs, void *data) {
  XbPArray<EA3Block*> *geomList = (XbPArray<EA3Block*>*)data;
  if( rs->skel->getType()=="skeleton" ) {
    XbIdentifier btype;
    if( skelGetBoneType(rs->skel, btype)==EA3_OK && btype=="geom" ) {
      EA3Block *geom = NULL;
      skelGetGeometry(rs->skel, geom);
      if( geom ) geomList->append(geom);
    }
  }
  return true;
}


int
calculateBoundingBox(EA3Util::EA3RecordedSkeleton *rs, bool &bbox_first, Float3 &min_bbox, Float3 &max_bbox) {
  int i;
  XbPArray<EA3Block*> geomList;
  rsApplyFunc(rs, getAllGeomsFromRS, &geomList);
  for( i=0; i<geomList.getSize(); i++ ) {
    EA3RecordedGeometry rg(rs,geomList[i]);
    calculateBoundingBox(&rg, bbox_first, min_bbox, max_bbox);
  }
  return EA3_OK;
}

int
calculateBoundingBox(EA3Util::EA3RecordedGeometry *rg, bool &bbox_first, Float3 &min_bbox, Float3 &max_bbox)
{
  int i, num;
  if( rg && rg->isvalid()) {
    Float3 *coords = new Float3[rg->getNumCoords()];
    for( num=0; (num<rg->getNumFrames()) || (!num&&!rg->getNumFrames()); num++ ) {
      rg->EvaluateGlobalCoords(coords, num);
      if( bbox_first && rg->getNumCoords() ) {
        min_bbox[0] = max_bbox[0] = coords[0][0];
        min_bbox[1] = max_bbox[1] = coords[0][1];
        min_bbox[2] = max_bbox[2] = coords[0][2];
        bbox_first = false;
      }
      for( i=rg->getNumCoords()-1; i>=0; i-- ) {
        expandbbox(min_bbox, max_bbox, coords[i]);
      }
    }
    delete[] coords;
  }

  return EA3_OK;
}

struct EA3BaseEqual {
  bool operator()(const EA3Base *b1,const EA3Base *b2) const {
    return(b1==b2);
  }
};
struct EA3BaseHash : public unary_function<const EA3Base *,size_t> {
  size_t operator()(const EA3Base *b) const {
    return((size_t)b);
  }
};

EA3Util::EA3_Status
sceneReRef(EA3Block *root, EA3Base *from, EA3Base *to) {
  int i;
  bool state = true;
  for( i=root->getNumMembers()-1; i>=0; i-- ) {
    EA3Base *b = root->getByIndex(i);
    if( b->isBlock() ) state = state && sceneReRef(b->toBlock(), from, to)==EA3_OK;
    EA3Reference *r = b->toReference();
    if( r ) r->replaceReference(from, to);
  }
  return state?EA3_OK:EA3_ERROR;
}

int
count_bone_end( const EA3Block *skel ) {
  int count = 0;
  XbIdentifier btype;
  int num = 0;
  skelGetNumChildren(skel, num);
  
  for( int i=0; i<num; i++ ) {
    EA3Block *child = NULL;
    skelGetChild(skel, i, child);
    if( child && skelGetBoneType(child, btype)==EA3_OK ) {      
      if( btype=="bone" || btype=="end" ) {
        count ++;
      }
    }
  }
  return count;
}


} // namespace


