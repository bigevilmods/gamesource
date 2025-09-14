// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3SkelUtil.cxx
*
*  Contains utility functions for operations on bone blocks, xform blocks,
*  skeleton blocks, and recorded skeletons.
*/

#include "EA3.h"
#include "EA3SkelUtil.h"
#include "EA3GeomUtil.h"
#include "EA3Utilities.h"

#include "math.h"
#include "float.h"
#ifdef EA_PLATFORM_WINDOWS 
#define isnan _isnan
#define finite _finite
#else 
#endif

#include <eastl/set.h>
using eastl::set;

#include "XbArray.h"

namespace EA3Util {

	extern bool g_useOldByNameBehaviour;
	/** @name SKELETON FUNCTIONS
	*  Utility functions for working with skeletons
	*/
	//@{

	void boneGetLocalMatrix( EA3Block *bone, XbMatrix &mat )
	{
		EA3Block *xfm = boneGetXform( bone, XBlocal, 1 );
		xformGetMatrix( xfm, mat );
	}


	void boneSetLocalMatrix( EA3Block *bone, const XbMatrix &mat)
	{
		EA3Block *xfm = boneGetXform( bone, XBlocal, 1 );
		xformSetMatrix( xfm, mat );
	}


	void boneCalcGlobalMatrix( const EA3Block *boneBlock, XbMatrix &outMat )
	{
		EA3Block *xfmBlock = boneGetXform( const_cast<EA3Block *>(boneBlock), XBlocal, 0 );
		XbMatrix localMat;

		if( xfmBlock )
			xformGetMatrix( xfmBlock, localMat );
		else
			localMat.makeIdentity();

		outMat = outMat * localMat;

		EA3Block *parentBlock = boneBlock->getContainer()->getContainer()->getContainer();
		if( parentBlock && parentBlock->getType() == XBskeleton )
		{
			EA3Block *parentBoneBlock = parentBlock->getByType(XBbone);
			if( parentBoneBlock ) boneCalcGlobalMatrix( parentBoneBlock, outMat );
		}
	}


	void boneGetInitMatrix( EA3Block *bone, XbMatrix &mat )
	{
		EA3Block *xfm = boneGetXform( bone, XBinit, 1 );
		xformGetMatrix( xfm, mat );
	}


	void boneSetInitMatrix( EA3Block *bone, const XbMatrix &mat)
	{
		EA3Block *xfm = boneGetXform( bone, XBinit, 1 );
		xformSetMatrix( xfm, mat );
	}


	void boneCalcGlobalInitMatrix( const EA3Block *boneBlock, XbMatrix &outMat )
	{
		EA3Block *xfmBlock = boneGetXform( const_cast<EA3Block *>(boneBlock), XBinit, 0 );
		XbMatrix localMat;

		if( xfmBlock )
		{
			xformGetMatrix( xfmBlock, localMat );
		}
		else
		{
			xfmBlock = boneGetXform( const_cast<EA3Block *>(boneBlock), XBlocal, 0 );
			if( xfmBlock )
				xformGetMatrix( xfmBlock, localMat );
			else
				localMat.makeIdentity();
		}

		outMat = outMat * localMat;

		EA3Block *parentBlock = boneBlock->getContainer()->getContainer()->getContainer();
		if( parentBlock && parentBlock->getType() == XBskeleton )
		{
			EA3Block *parentBoneBlock = parentBlock->getByType(XBbone);
			if( parentBoneBlock ) boneCalcGlobalInitMatrix( parentBoneBlock, outMat );
		}
	}


	void boneSetType( EA3Block *bone, const XbIdentifier &t)
	{
		EA3Field *f = bone->createField( XBtype, "" );
		f->data[0].str = const_cast<char *>(t.getString());
	}


	void boneGetType( EA3Block *bone, XbIdentifier &t)
	{
		EA3Field *f = (EA3Field *)bone->getByType( XBtype );
		if (f) t = f->data[0].str;
		else t = "null";
	}

	float matrixDifference(const XbMatrix &m1,const XbMatrix &m2) {
		int i,j;
		float dist = 0.0f;
		for(i=0;i<4;i++) {
			for(j=0;j<4;j++) {
				float f = fabs(m1[i][j]-m2[i][j]);
				if (f > dist) {dist = f;}
			}
		}
		return dist;
	}
#if 0
	class TestGetTransform {
	public:
		TestGetTransform() {
			XbEuler eul(XbVec3f(0,0,0),XbEuler::XYZ,XbEuler::STATIC);
			XbMatrix mat;
			mat.setTransform(XbVec3f(0,0,0),eul,XbVec3f(-2,1,1));
			XbVec3f trn;
			XbVec3f scl;
			getTransform(mat,trn,eul,scl);
			XbMatrix mat2;
			mat2.setTransform(trn,eul,scl);
			float dist = matrixDifference(mat,mat2);
		}
	};
	static TestGetTransform tg;
#endif

	void matrixGetScale(const XbMatrix &mat,XbVec3f &scl) {
		scl = XbVec3f(
			mat[0][0]*mat[0][0]+mat[0][1]*mat[0][1]+mat[0][2]*mat[0][2],
			mat[1][0]*mat[1][0]+mat[1][1]*mat[1][1]+mat[1][2]*mat[1][2],
			mat[2][0]*mat[2][0]+mat[2][1]*mat[2][1]+mat[2][2]*mat[2][2] );
		scl[0] = (float)sqrt(scl[0]);
		scl[1] = (float)sqrt(scl[1]);
		scl[2] = (float)sqrt(scl[2]);
	}


	void getTransform(const XbMatrix &mat,XbVec3f &trn,XbEuler &eul,XbVec3f &scl) {
		int i,j,k;
		trn.setValue(mat[3][0],mat[3][1],mat[3][2]);
		XbRotation rot,srot;
		matrixGetScale(mat,scl);
		bool hasScale = false;
		for(i=0;i<3;i++) {
			if( fabs(fabs(scl[i])-1.0f) > 0.002f) {
				hasScale = true;
			}
		}
		if (!hasScale) {
			eul = XbEuler(mat,XbEuler::XYZ,XbEuler::STATIC);
			return;
		}
		float dist = 0.0f;
		float mindist = 100000.0f;
		if (hasScale) {
			XbMatrix sclmat;
			XbVec3f sclO = scl;
			for(i=1;i>-2;i-=2) {
				for(j=1;j>-2;j-=2) {
					for(k=1;k>-2;k-=2) {
						scl[0] = sclO[0] * i;
						scl[1] = sclO[1] * j;
						scl[2] = sclO[2] * k;
						XbVec3f scaleI(1.0f/scl[0],1.0f/scl[1],1.0f/scl[2]);
						sclmat.setScale(scaleI);
						XbMatrix M = sclmat * mat;
						XbMatrix mat2;
						eul = XbEuler(M,XbEuler::XYZ,XbEuler::STATIC);
						mat2.setTransform(trn,eul,scl);
						dist = matrixDifference(mat,mat2);\
							if (dist < mindist) mindist = dist;
						if (dist < 0.002) return;
					} 
				} 
			}
		}
		fprintf(stderr,"Could not find a good SRT for matrix.  Possibly because of shearing.  Error = %f\n",mindist);
		//abort();
		return;
	}


	EA3Block *boneGetXform(EA3Block *bone,const XbIdentifier &tname, int create) {
		EA3Array *xforms = bone->getByType(XBxform);
		if (!xforms && !create) return(0);
		if (!xforms) xforms = bone->createArray(XBxform,"");
		int i;
		for(i=0;i<xforms->getNumMembers();i++) {
			EA3Block *xform = xforms->getByIndex(i);
			EA3Field *f = xform->getByType(XBtype);
			if (XbString(f->data[0].str) == tname) {
				return(xform);
			}
		}
		if (!create) return(0);
		EA3Block *xform = (EA3Block *)xforms->createBlock(XBxform,"");
		EA3Field *f = xform->createField(XBtype,"");
		f->data[0].str = tname.getString();
		return(xform);
	}


	EA3Block *boneGetXform(const EA3Block *bone,const XbIdentifier &tname) {
		return boneGetXform(const_cast<EA3Block*>(bone),tname,0);
	}

	void xformGetMatrix(const EA3Block *xform,XbMatrix &mat) {
		mat.makeIdentity();
		if (!xform) return;
		XbVec3f trn(0,0,0);
		XbEuler eul(XbVec3f(0,0,0),XbEuler::XYZ,XbEuler::STATIC);
		XbVec3f scl(1,1,1); 
		EA3Field *fm = xform->getByType(XBmatrix);
		if (fm) {
			int c = 0;
			for(int i=0;i<4;i++) {
				for(int j=0;j<4;j++) {
					mat[i][j] = fm->data[c].f;
					c++;
				}
			}
			return;
		}
		EA3Field *f = xform->getByType(XBtranslation);
		if (f) {trn = XbVec3f(f->data[0].f,f->data[1].f,f->data[2].f);}
		f = xform->getByType(XBeuler);
		if (f) {eul.setAngles(XbVec3f(f->data[0].f*D2R,f->data[1].f*D2R,f->data[2].f*D2R));}
		f = xform->getByType(XBscale);
		if (f) {scl =XbVec3f(f->data[0].f,f->data[1].f,f->data[2].f);}
		f = xform->getByType(XBrotatePivot);
		if (f) {
			XbVec3f rp(f->data[0].f,f->data[1].f,f->data[2].f);
			XbMatrix T,R,S,P,PI;
			T.setTranslate(trn);
			R.setRotate(eul);
			S.setScale(scl);
			P.setTranslate(rp);
			PI.setTranslate(-rp);
			mat = S*PI*R*P*T;
		} else {
			mat.setTransform(trn,eul,scl);
		}
	}

	void xformGetRotatePivot(const EA3Block *xform,XbVec3f &rp) {
		rp.setValue(0,0,0);
		EA3Field *f = xform->getByType(XBrotatePivot);
		if (f) {rp.setValue(f->data[0].f,f->data[1].f,f->data[2].f);}
	}

	void xformGetEuler(const EA3Block *xform,XbEuler &eul) {
		eul.setValue(XbVec3f(0,0,0),XbEuler::XYZ,XbEuler::STATIC);
		if (!xform) return;
		EA3Field *f = xform->getByType(XBeuler);
		if (f) {eul.setAngles(XbVec3f(f->data[0].f*D2R,f->data[1].f*D2R,f->data[2].f*D2R));}
	}

	void xformSetEuler(EA3Block *xform,const XbEuler &eul) {
		if (!xform) return;
		EA3Field *f = xform->createField(XBeuler,"");
		eul.getAngles().getValue(f->data[0].f,f->data[1].f,f->data[2].f);
		f->data[0].f *= (float)R2D;
		f->data[1].f *= (float)R2D;
		f->data[2].f *= (float)R2D;
	}

	void xformGetTranslation(const EA3Block *xform,XbVec3f &vec) {
		if (!xform) return;
		EA3Field *f = xform->getByType(XBtranslation);
		if (f) {
			vec.setValue((float *)f->data);
		} else {
			vec.setValue(0,0,0);
		}
	}

	void xformSetTranslation(EA3Block *xform,const XbVec3f &vec) {
		if (!xform) return;
		EA3Field *f = xform->createField(XBtranslation,"");
		vec.getValue(f->data[0].f,f->data[1].f,f->data[2].f);
	}

	void xformGetScale(const EA3Block *xform,XbVec3f &vec) {
		if (!xform) return;
		EA3Field *f = xform->getByType(XBscale);
		if (f) {
			vec.setValue((float *)f->data);
		} else {
			vec.setValue(1,1,1);
		}
	}

	void xformSetScale(EA3Block *xform,const XbVec3f &vec) {
		if (!xform) return;
		EA3Field *f = xform->createField(XBscale,"");
		vec.getValue(f->data[0].f,f->data[1].f,f->data[2].f);
	}



	void xformSetMatrix(EA3Block *xform,const XbMatrix &mat) {
		if (!xform) return;
		XbVec3f trn;
		XbEuler eul;
		XbVec3f scl;
		EA3Field *f = xform->getByType(XBmatrix);
		if (f) {
			int c = 0;
			for(int i=0;i<4;i++) {
				for(int j=0;j<4;j++) {
					f->data[c].f = mat[i][j];
					c++;
				}
			}
			return;
		}
		getTransform(mat,trn,eul,scl);
		f = xform->getByType(XBtranslation);
		if (!f) f = xform->createField(XBtranslation,"");
		trn.getValue(f->data[0].f,f->data[1].f,f->data[2].f);
		f = xform->getByType(XBeuler);
		if (!f) f = xform->createField(XBeuler,"");
		eul.getAngles().getValue(f->data[0].f,f->data[1].f,f->data[2].f);
		f->data[0].f *= (float)R2D;
		f->data[1].f *= (float)R2D;
		f->data[2].f *= (float)R2D;
		f = xform->getByType(XBscale);
		if (!f) f = xform->createField(XBscale,"");
		scl.getValue(f->data[0].f,f->data[1].f,f->data[2].f);
		f = xform->getByType(XBrotatePivot);
		if (f) f->getContainer()->removeMember(f);
	}


	EA3_Status xformGetSRT(
		const EA3Block &xform, XbVec3f &scl, XbEuler &eul, XbVec3f &trn )
	{
		EA3Field *matrixField = xform.getByType( XBmatrix );

		if( matrixField )
		{
			//
			// Get transformation matrix and decompose it to get the SRT values.
			//
			XbMatrix matrix(
				matrixField->data[0].f,
				matrixField->data[1].f,
				matrixField->data[2].f,
				matrixField->data[3].f,
				matrixField->data[4].f,
				matrixField->data[5].f,
				matrixField->data[6].f,
				matrixField->data[7].f,
				matrixField->data[8].f,
				matrixField->data[9].f,
				matrixField->data[10].f,
				matrixField->data[11].f,
				matrixField->data[12].f,
				matrixField->data[13].f,
				matrixField->data[14].f,
				matrixField->data[15].f );

			getTransform( matrix, trn, eul, scl );
			return EA3_OK;
		}

		EA3Field *sclField = xform.getByType( XBscale );
		EA3Field *eulField = xform.getByType( XBeuler );
		EA3Field *quatField = xform.getByType( XBquaternion );
		EA3Field *trnField = xform.getByType( XBtranslation );

		if( !( sclField && (eulField || quatField) && trnField ) )
		{
			//
			//  Missing S/R/T field(s)
			//
			return EA3_NOT_FOUND;
		}

		//
		//  Set the individual SRT values
		//
		scl.setValue( (float *)sclField->data );
		trn.setValue( (float *)trnField->data );

		if( eulField )
		{
			eul.setAngles(
				XbVec3f(
				eulField->data[0].f * D2R,
				eulField->data[1].f * D2R,
				eulField->data[2].f * D2R ) );
		}
		else
		{
			XbRotation quat;
			quat.setValue( (float *)quatField->data );
			eul.setValue( quat, XbEuler::XYZ, XbEuler::STATIC );
		}

		return(EA3_OK);
	}


	EA3_Status skelGetNumChildren(const EA3Block *skel,int &num) {
		//if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Array *chld = skel->getByType(XBskeleton);
		num = 0;
		if (chld) num = chld->getNumMembers();
		return(EA3_OK);
	}

	EA3_Status skelGetHierLevel(const EA3Block *skel,int &hlevel) {
		//if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Block *parent = NULL;
		skelGetParent(skel, parent);
		if( !parent ) {
			hlevel = 0;
			return EA3_OK;
		}
		int phlevel = 0;
		skelGetHierLevel(parent, phlevel);
		hlevel = phlevel + 1;
		return EA3_OK;
	}



	EA3_Status skelGetChild(const EA3Block *skel,int index, EA3Block *&child) {
		//if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Array *chld = skel->getByType(XBskeleton);
		if (chld) {
			if(index < chld->getNumMembers()) {
				child = chld->getByIndex(index);
				return(EA3_OK);
			}
		}
		return(EA3_NOT_FOUND);
	}


	EA3_Status skelGetParent(const EA3Block *skel,EA3Block *&par) {
		par = (EA3Block*)skel->getContainer()->getContainer();
		if (par&&par->getType()!=XBskeleton) {
			par = NULL;
			return(EA3_NOT_FOUND);
		}
		return(EA3_OK);
	}



	EA3_Status skelAddChild(EA3Block *skel,const XbString &name,EA3Block *&child) {
		//if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Array *chld;
		if(!skel->isArray()) {
			chld = skel->getByType(XBskeleton);
			if (!chld) chld = skel->createArray(XBskeleton);
		} else {
			chld = (EA3Array *)skel;
		}
		child = (EA3Block *)chld->createBlock(XBskeleton,name);
		child->createBlock(XBbone,name);
		return(EA3_OK);
	}


	EA3_Status skelGetXform(const EA3Block *skel,EA3Block *&xfm,const XbIdentifier &type, int create) {
		if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Block *bone = skelGetBone(skel);
		if (!bone) {
			return(EA3_NO_XFORM);
		}
		xfm = boneGetXform(bone,type,create);
		if (!xfm) {
			return(EA3_NO_XFORM);
		}
		return(EA3_OK);
	}

	EA3_Status skelGetChannelList(const EA3Block *skel,const EA3Block *anim,XbPArray<EA3Block *> &channelList) {
		EA3Reference *links = skel->getByType(XBbone)->toBlock()->getByType(XBlinks);
		if (links) {
			bool foundChannels = false;
			for(int i=0;i<links->getNumReferences();i++) {
				EA3Block *bl = links->getReference(i);
				if (!bl) continue;
				EA3Block *con = bl->getContainer();
				if (con && con->getType()==XBchannel) {
					channelList.append(bl);
					foundChannels = true;
				}
			}
			return(EA3_OK);
		}
		if (g_useOldByNameBehaviour) {
			XbString morphName = skel->getName();
			EA3Block *geom = NULL;
			skelGetGeometry(skel,geom);
			if (geom) {
				EA3Array *mts = geom->getByType(XBmorphTarget);
				if (mts && mts->getName()!="") {
					morphName = mts->getName();
				}
			}
			if (skel->getType()==XBskeleton && anim) {
				EA3Array *chans = (EA3Array *)anim->getByType(XBchannel);
				if (chans) {
					for(int i=0;i<chans->getNumMembers();i++) {
						EA3Block *chan = (EA3Block *)chans->getByIndex(i);
						if(chan->getName()== skel->getName() || chan->getName() == morphName) {
							channelList+=(EA3Block*)chan;
						}
					}
				}
			}
		}
		return(EA3_OK);
	}

	struct AnimClipLess {
		bool operator()(EA3Block *a, EA3Block *b) {
			EA3Field *t1 = a->getByType("time");
			if (!t1) return (a<b);
			EA3Field *t2 = b->getByType("time");
			if (!t2) return (a<b);
			return t1->data[0].f < t2->data[0].f;
		}
	};

	EA3_Status skelGetAnimClipList(const EA3Block *skel,const EA3Array *animClipArray,XbPArray<EA3Block *> &animClips) {
		typedef set<EA3Block *,AnimClipLess > ClipSet;
		ClipSet clipSet;
		if (!animClipArray) return (EA3_OK);
		int num = animClipArray->getNumMembers();
		EA3Block *bone = skel->getByType(XBbone);
		if (!bone) return (EA3_OK);
		EA3Array *xforms = bone->getByType(XBxform);
		if (!xforms) return (EA3_OK);
		for(int i=0;i<num;i++) {
			EA3Block *clip = animClipArray->getByIndex(i);
			EA3Array *clipChan = clip->getByType(XBclipChannel);
			if (!clipChan) continue;
			int num2 = clipChan->getNumMembers();
			for(int j=0;j<num2;j++) {
				EA3Reference *output = clipChan->getByIndex(j)->toBlock()->getByType(XBoutput);
				EA3Field *fld = output->getReference(0);
				if (!fld) continue;
				if (fld->getContainer()->getContainer() == xforms) {
					clipSet.insert(clip);
					break;
				}
			}
		}
    //printf("bone %s\n",skel->getName().getString());
		for(ClipSet::iterator csi=clipSet.begin();csi!=clipSet.end();++csi) {
      EA3Block *clip = *csi;
      float t = -999.0f;
      EA3Field *timeFld = clip->getByType("time");
      if (timeFld) t = timeFld->data[0].f;
      //printf("clip %s %f\n",clip->getName().getString(),t);
			animClips.append(clip);
		}
		return(EA3_OK);
	}

	static void updateClipChanValues(const EA3Block *skel,EA3Array *xforms,EA3Array *clipChans,const XbVec3f &trn,const XbVec3f &angles,const XbVec3f &scale) {
		for(int i=0;i<clipChans->getNumMembers();i++) {
      EA3Block *clipChan = clipChans->getByIndex(i);
      EA3Reference *output = clipChan->getByType(XBoutput);
      if (!output) continue;
      EA3Field *fld = output->getReference(0);
      if (!fld) continue;
      if (fld->getContainer()->getContainer() != xforms) continue;
      EA3Field *outputIx = clipChan->getByType(XBoutputIx);
      if (!outputIx) continue;
      EA3Field *value = clipChan->getByType(XBinputValue);
      if (!value) continue;
      if (fld->getType()==XBeuler) {
        value->data[0].f = angles[outputIx->data[0].i];
      }  
      if (fld->getType()==XBtranslation) {
        value->data[0].f = trn[outputIx->data[0].i];
      }
      if (fld->getType()==XBscale) {
        value->data[0].f = scale[outputIx->data[0].i];
      }
		}
	}
  static void evalClipChans(const EA3Block *skel,EA3Array *xforms,const EA3Array *clipChans,XbVec3f &trn,XbRotation &rot,XbVec3f &scale,unsigned int &mask) {
    trn = XbVec3f(0.0f,0.0f,0.0f);
    scale = XbVec3f(1.0f,1.0f,1.0f);
    XbVec3f angles = trn;
    
    for(int i=0;i<clipChans->getNumMembers();i++) {
      EA3Block *clipChan = clipChans->getByIndex(i);
      EA3Reference *output = clipChan->getByType(XBoutput);
      if (!output) continue;
      EA3Field *fld = output->getReference(0);
      if (!fld) continue;
      if (fld->getContainer()->getContainer() != xforms) continue;
      EA3Field *outputIx = clipChan->getByType(XBoutputIx);
      if (!outputIx) continue;
      EA3Field *value = clipChan->getByType(XBinputValue);
      if (!value) continue;
      if (fld->getType()==XBeuler) {
        angles[outputIx->data[0].i] = value->data[0].f * D2R;
        mask |= 1<<(outputIx->data[0].i+3);
      }  
      if (fld->getType()==XBtranslation) {
        trn[outputIx->data[0].i] = value->data[0].f;
        mask |= 1<<(outputIx->data[0].i+6);
      }
      if (fld->getType()==XBscale) {
        scale[outputIx->data[0].i] = value->data[0].f;
        mask |= 1<<(outputIx->data[0].i);
      }
    }
    XbEuler eul(angles,XbEuler::XYZ,XbEuler::STATIC);
		rot = eul;
  }
  EA3_Status skelEvalAnimClips(const EA3Block *skel,const XbPArray<EA3Block *> &animClips,int frame,XbVec3f &scale,XbEuler &eul,XbVec3f &trans,unsigned int &mask) {
    mask = 0;
    if (animClips.getSize() == 0) return EA3_OK;
    EA3Block *bone = skel->getByType(XBbone);
		if (!bone) return (EA3_OK);
		EA3Array *xforms = bone->getByType(XBxform);
		if (!xforms) return (EA3_OK);
    EA3Block *clip1 = NULL;
    EA3Block *clip2 = NULL;
    float t1=0.0f,t2=0.0f;
    float fps = 30.0f;
    EA3Block *model = animClips[0];
    while(model && model->getType() != XBmodel) model = model->getContainer();
    if (!model) return EA3_OK;
    EA3Block *anim = model->getByType(XBanimation)->toBlock()->getByIndex(0);
    if (anim) {
      animGetFPS(anim,fps);
    }
    float t = float(frame)/fps;
    for(int i=0;i<animClips.getSize();i++) {
      EA3Block *clip = animClips[i];
      EA3Field *timeFld = clip->getByType("time");
      if (!timeFld) continue;
      if (!clip1 || timeFld->data[1].f <= t) {
        clip1 = clip;
        t1 = timeFld->data[1].f;
      }
      if (!clip2 && timeFld->data[0].f >= t) {
        clip2 = clip;
        t2 = timeFld->data[0].f;
      }
      if (clip1 && clip2) break;
    }
    if (!clip1) return EA3_OK;
    
    EA3Array *clip1Chans = clip1->getByType(XBclipChannel);
    
    XbVec3f trn1,scl1,trn2,scl2;
    XbRotation rot1,rot2;
    evalClipChans(skel,xforms,clip1Chans,trn1,rot1,scl1,mask);
    if (clip2) {
      EA3Array *clip2Chans = clip2->getByType(XBclipChannel);
      evalClipChans(skel,xforms,clip2Chans,trn2,rot2,scl2,mask);
    } else {
      rot2 = rot1;
      trn2 = trn1;
      scl2 = scl1;
    }
		float t01lin = (t1==t2)? 1.0f : (t-t1)/(t2-t1);
    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;
    XbVec3f angles = eul.getAngles();
		float dist = XbRotation::distance(rot1,rot2);
		
		if (dist > 1.0f) {
			float q[4];
		  rot2.getValue(q[0],q[1],q[2],q[3]);
			rot2.setValue(-q[0],-q[1],-q[2],-q[3]);
		}
    XbRotation rot = XbRotation::slerp(rot1,rot2,t01lin);
    XbEuler res(rot,XbEuler::XYZ,XbEuler::STATIC);
    XbVec3f angles2 = res.getAngles();
    if (mask & 1) scale[0] = (scl2[0]*t01lin + scl1[0]*(1.0f-t01lin));
    if (mask & 2) scale[1] = (scl2[1]*t01lin + scl1[1]*(1.0f-t01lin));
    if (mask & 4) scale[2] = (scl2[2]*t01lin + scl1[2]*(1.0f-t01lin));
    if (mask & 8) angles[0] = angles2[0];
    if (mask & 16) angles[1] = angles2[1];
    if (mask & 32) angles[2] = angles2[2];
    if (mask & 64) trans[0] = (trn2[0]*t01lin + trn1[0]*(1.0f-t01lin));
    if (mask & 128) trans[1] = (trn2[1]*t01lin + trn1[1]*(1.0f-t01lin));
    if (mask & 256) trans[2] = (trn2[2]*t01lin + trn1[2]*(1.0f-t01lin));
		eul.setAngles(angles);
    return EA3_OK;
  }
	EA3_Status skelUpdateAnimClipValues(const EA3Block *skel,const XbPArray<EA3Block *> &animClips,int frame,XbVec3f &scale,XbEuler &eul,XbVec3f &trans) {
    if (animClips.getSize() == 0) return EA3_OK;
    EA3Block *bone = skel->getByType(XBbone);
		if (!bone) return (EA3_OK);
		EA3Array *xforms = bone->getByType(XBxform);
		if (!xforms) return (EA3_OK);
    float fps = 30.0f;
    EA3Block *model = animClips[0];
    while(model && model->getType() != XBmodel) model = model->getContainer();
    if (!model) return EA3_OK;
    EA3Block *anim = model->getByType(XBanimation)->toBlock()->getByIndex(0);
    if (anim) {
      animGetFPS(anim,fps);
    }
    float t = float(frame)/fps;
    for(int i=0;i<animClips.getSize();i++) {
      EA3Block *clip = animClips[i];
      EA3Field *timeFld = clip->getByType("time");
      if (!timeFld) continue;
      if (timeFld->data[0].f <= t && t < timeFld->data[1].f) {
        EA3Array *clipChans = clip->getByType(XBclipChannel);
				XbVec3f angles = eul.getAngles() * R2D;
				updateClipChanValues(skel,xforms,clipChans,scale,angles,trans);
      }
		}
    
    
		return EA3_OK;
	}

	EA3_Status skelGetMatrix(const EA3Block *skel,XbMatrix &mat, const XbIdentifier &type) {
		mat.makeIdentity();
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,0);
		if (e!=EA3_OK) return(e);
		xformGetMatrix(xfm,mat);
		return(EA3_OK);
	}



	EA3_Status skelSetMatrix(EA3Block *skel,const XbMatrix &mat, const XbIdentifier &type) {
		EA3Block *bone = skelGetBone(skel);
		if (!bone) bone = skel->createBlock(XBbone,skel->getName());
		EA3Block *xfm=NULL;
		EA3_Status e=skelGetXform(skel,xfm,type,1);
		xformSetMatrix(xfm,mat);
		if (e!=EA3_OK) return(e);
		return(EA3_OK);
	}


	EA3_Status skelGetEuler(const EA3Block *skel,XbEuler &eul, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,0);
		if (e!=EA3_OK) {
			eul.setValue(XbVec3f(0,0,0),XbEuler::XYZ,XbEuler::STATIC);
			return(e);
		}
		xformGetEuler(xfm,eul);
		return(EA3_OK);
	}


	EA3_Status skelGetTranslation(const EA3Block *skel,XbVec3f &trn, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,0);
		if (e!=EA3_OK) {
			trn.setValue(0,0,0);
			return(e);
		};
		xformGetTranslation(xfm,trn);
		return(EA3_OK);
	}


	EA3_Status skelGetScale(const EA3Block *skel,XbVec3f &scl, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,0);
		if (e!=EA3_OK) {
			scl.setValue(1.0f,1.0f,1.0f);
			return(e);
		}
		xformGetScale(xfm,scl);
		return(EA3_OK);
	}

	EA3_Status skelGetRotatePivot(const EA3Block *skel,XbVec3f &rp, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,0);
		if (e!=EA3_OK) return(e);
		xformGetRotatePivot(xfm,rp);
		return(EA3_OK);
	}


	EA3_Status skelSetEuler(EA3Block *skel,const XbEuler &eul, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,1);
		if (e!=EA3_OK) return(e);
		xformSetEuler(xfm,eul);
		return(EA3_OK);
	}


	EA3_Status skelSetTranslation(EA3Block *skel,const XbVec3f &trn, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,1);
		if (e!=EA3_OK) return(e);
		xformSetTranslation(xfm,trn);
		return(EA3_OK);
	}


	EA3_Status skelSetScale(EA3Block *skel,const XbVec3f &scl, const XbIdentifier &type) {
		EA3Block *xfm;
		EA3_Status e=skelGetXform(skel,xfm,type,1);
		if (e!=EA3_OK) return(e);
		xformSetScale(xfm,scl);
		return(EA3_OK);
	}


	EA3_Status skelSetGeom(const EA3Block *skel,EA3Block *geom) {
		if (!geom) return EA3_ERROR;
		EA3Block *boneBlock = const_cast<EA3Block *>(skel)->createBlock(XBbone);
		boneBlock->createField(XBtype)->data[0].str = XBgeom;
		EA3Reference *linksPointer = boneBlock->getByType(XBlinks);
		if (linksPointer) {
			if (linksPointer->getNumReferences() > 0)	{
				for (int i=0;i<linksPointer->getNumReferences();i++) {
					if (linksPointer->getReference(i)->getType() == XBgeometry) {
						linksPointer->replaceReference(linksPointer->getReference(i),geom);
						break;
					}
				}
			}
			else	linksPointer->addReference(geom);
		}
		else {
			linksPointer = boneBlock->createReference("links");
			linksPointer->addReference(geom);
		}
		return EA3_OK;
	}


	EA3_Status skelGetBoneType(EA3Block *skel,XbIdentifier &type) {
		if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Block *bone = skelGetBone(skel);
		if (!bone) skel->createBlock(XBbone,skel->getName());
		boneGetType(bone,type);
		return(EA3_OK);
	}



	EA3_Status skelSetBoneType(EA3Block *skel,const XbIdentifier &type) {
		if (skel->getType() != XBskeleton) return(EA3_WRONG_TYPE);
		EA3Block *bone = skelGetBone(skel);
		if (!bone) bone = (EA3Block *)skel->createBlock(XBbone,skel->getName());
		boneSetType(bone,type);
		return(EA3_OK);
	}


	EA3_Status skelGetLight(const EA3Block *skel,EA3Block *&light) {
		const EA3Block *b = skel;
		EA3Block *bone = skelGetBone(skel);
		if (!bone) return(EA3_NOT_FOUND);
		XbIdentifier btype;
		boneGetType(bone,btype);
		if (btype != XBlight) return(EA3_NOT_FOUND);
		EA3Reference *links = bone->getByType(XBlinks);
		if (links) {
			for(int i=0;i<links->getNumReferences();i++) {
				light = links->getReference(i);
				if (light && light->getType()==XBlight) return(EA3_OK);
				light = NULL;
			}
		}
		light = NULL;
		if (g_useOldByNameBehaviour) {
		  light=b->getScene()->getByType(XBlight)->toBlock()->getByName(skel->getName());
		}
		if (!light) return(EA3_NOT_FOUND);
		return(EA3_OK);
	}


	EA3_Status skelGetGeometry(const EA3Block *skel,EA3Block *&geom) {
		const EA3Block *b = skel;
		EA3Block *bone = skelGetBone(skel);
		if (!bone) return(EA3_NO_GEOMETRY);
		XbIdentifier btype;
		boneGetType(bone,btype);
		if (btype != XBgeom) return(EA3_NO_GEOMETRY);
		EA3Reference *links = bone->getByType(XBlinks);
		geom = NULL;
		if (links) {
			for(int i=0;i<links->getNumReferences();i++) {
				geom = links->getReference(i);
				if (geom && geom->getType()==XBgeometry) return(EA3_OK);
				geom = NULL;
			}
		}
		if (g_useOldByNameBehaviour) {
			while(b->getType()==XBskeleton) b=b->getContainer();
			assert(b->getType()==XBmodel);
			EA3Array *geoms= b->getByType(XBgeometry);
			if (!geoms) return(EA3_NO_GEOMETRY);
			geom = geoms->getByName(skel->getName());
		}
		if (!geom) return(EA3_NO_GEOMETRY);
		return(EA3_OK);
	}


	EA3Block *skelGetBone(const EA3Block *skel) {
		return(skel->getByType(XBbone));
	}


	/*** EA3 RECORDED SKELETON *********************************************************/

	EA3RecordedSkeleton::EA3RecordedSkeleton(EA3Block *s,EA3Block *a)
	{
		numFrames = 0;
		anim = a;

		int count;	// cache for loop limit values, to avoid multiple lookup through pointer

		if(a)
		{
			EA3Array *chans = (EA3Array *)a->getByType(XBchannel);
			if (chans)
			{
				count = chans->getNumMembers();
				for(int i=0;i<count;i++)
				{
					EA3Block *chan = (EA3Block *)chans->getByIndex(i);
					if (chan)
					{
						int num = 0;
						channelGetNumFrames(chan,num);
						if(num>numFrames)
							numFrames=num;
					}
				}
			}
			if (numFrames == 0) {
				EA3Field *sf = a->getByType(XBstartFrame);
				EA3Field *ef = a->getByType(XBendFrame);
				if (sf && ef) numFrames = ef->data[0].i - sf->data[0].i + 1;
			}
		}
		global=new XbMatrix[numFrames+2]; 
		local=new XbMatrix[numFrames+2]; 
		par = NULL;
		skel = s;
		record(a);

		EA3Array *ch = (EA3Array*)skel->getByType(XBskeleton);
		if(ch)		// ch shouldn't be tested for != 0 in each loop
		{
			count = ch->getNumMembers();
			children.allocateSize( count );		// Preset the number of children, since it's known
			for(int i=0;i<count;i++)
			{
				EA3RecordedSkeleton *rs = new EA3RecordedSkeleton((EA3Block *)ch->getByIndex(i), this, a, numFrames);
				children+=rs;
			}
		}
	};

	EA3RecordedSkeleton::EA3RecordedSkeleton(EA3Block *s, EA3RecordedSkeleton *p,EA3Block *a,int nf)
	{
		numFrames = nf;
		anim = a;
		global=new XbMatrix[numFrames+2];
		local=new XbMatrix[numFrames+2];
		par = p;
		skel = s;
		record(a);
		EA3Array *ch = (EA3Array*)skel->getByType(XBskeleton);
		for(int i=0;ch && i<ch->getNumMembers();i++)
		{
			EA3RecordedSkeleton *rs = new EA3RecordedSkeleton(ch->getByIndex(i),this,a,numFrames);
			children+=rs;
		}
	}

	EA3RecordedSkeleton::EA3RecordedSkeleton(EA3RecordedSkeleton &Src,EA3RecordedSkeleton *thePar)
	{
		numFrames = Src.numFrames;
		anim = Src.anim;
		par = thePar;
		skel = Src.skel;
		initGlobalInverse = Src.initGlobalInverse;

		global = new XbMatrix[numFrames+2];
		local = new XbMatrix[numFrames+2];

		memcpy(global, Src.global, sizeof(XbMatrix) * (numFrames+2));
		memcpy(local, Src.local, sizeof(XbMatrix) * (numFrames+2));

		int i;
		channelList = Src.channelList;

		int ChildCount = Src.children.getSize();
		if(ChildCount)
		{
			children.allocateSize( ChildCount );
			for(i=0; i<ChildCount; i++)
			{
				EA3RecordedSkeleton *child = new EA3RecordedSkeleton( *(Src.children[i]),this );
				children.append(child);
			}
		}
	}

	EA3_Status skelGetMatrixAtFrame(const EA3Block *skel,XbPArray<EA3Block *> channelList,int frame,XbMatrix &mat)
	{
		XbVec3f scale;
		skelGetScale(skel, scale, XBlocal);
		XbEuler euler;
		XbVec3f translate(0,0,0);
		XbVec3f rotatePivot(0,0,0);
		skelGetEuler(skel,euler,XBlocal);
		skelGetTranslation(skel,translate,XBlocal);
		skelGetRotatePivot(skel,rotatePivot,XBlocal);
		XbMatrix P,PI;
		P.setTranslate(rotatePivot);
		PI.setTranslate(-rotatePivot);
		XbMatrix S; 
		XbMatrix R;
		XbMatrix T; 
		channelListGetSRT(channelList,frame,scale,euler,translate);
		S.setScale(scale);
		R.setRotate(euler);
		T.setTranslate(translate);
		mat = S*PI*R*P*T;

		return(EA3_OK);
	}

	static int RecordingTypes[8] = {0, 0, 0, 0, 0, 0, 0, 0};


	void EA3RecordedSkeleton::record(EA3Block *a)
	{
		// Preset the number of channels to 4 - 6 is very common, but will only require one growth step instead of three
		channelList.allocateSize(4);
		skelGetChannelList(skel, a, channelList);
    EA3Array *animClips = a? a->getContainer()->getContainer()->getByType(XBanimClip)->toArray() : NULL;
		int listSize = channelList.getSize();
    skelGetAnimClipList(skel,animClips,channelList);
		bool hasAnimClips = channelList.getSize() != listSize;

		int i;
		skelGetMatrix(skel, local[1], XBlocal);
		XbVec3f scale;
		// Just do the skelGetScale
		skelGetScale(skel, scale, XBlocal);

		if (skelGetMatrix(skel, local[0], XBinit) != EA3_OK)
		{
			local[0] = local[1];
		}

		if (par)
		{
			global[0] = local[0] * par->global[0];
			global[1] = local[1] * par->global[1];
		}
		else
		{
			global[0] = local[0];
			global[1] = local[1];
		}

		XbEuler euler, lasteuler(XbVec3f(0,0,0), XbEuler::XYZ, XbEuler::STATIC);
		XbVec3f translate(0,0,0);
		XbVec3f rotatePivot(0,0,0);
		skelGetEuler(skel,euler,XBlocal);
		skelGetTranslation(skel,translate,XBlocal);
		skelGetRotatePivot(skel,rotatePivot,XBlocal);
		XbMatrix S,P,PI;
		S.setScale(scale);
		bool bNeedScale = false;
		if (scale[0] != 1.0f || scale[1]!=1.0f || scale[2] !=1.0f) bNeedScale = true;
		// Pivot requires two additional matrix mults and setups; it's very rarely used.
		bool bPivotUsed = false;
		if(rotatePivot[0] || rotatePivot[1] || rotatePivot[2])
		{
			bPivotUsed = true;
			P.setTranslate(rotatePivot);
			PI.setTranslate(-rotatePivot);
		}
		initGlobalInverse = global[0].inverse();

		XbMatrix R;
		R.setRotate(lasteuler);

		ChannelListSRTInfo Info;
		channelListGetSRTInfo(channelList, Info);

		// Figure out which channels are actually active (inactive implies identity transform)
		int ChannelBits = (Info.S.Count != 0) ? 4 : 0;
		ChannelBits |= (Info.R.Count != 0) ? 2 : 0;
		ChannelBits |= (Info.T.Count != 0) ? 1 : 0;
		unsigned int mask = 0;
		if (hasAnimClips) {
			ChannelBits = 7; // not easy to figure out which might be affected
		}

		RecordingTypes[ChannelBits]++;
		switch(ChannelBits)
		{
		case 0:		// No channels active - Everything is the same as the current pose.
			for(i=0; i<numFrames; i++) {
				local[i+2] = local[1];
				if(par)
					global[i+2] = local[1] * par->global[i+2];
				else
					global[i+2] = local[1];
			}
			break;

		case 3:		// Translation + Rotation
			for(i=0;i<numFrames;i++)
			{
				XbMatrix T; 
				channelListGetSRTFast(Info, i+Info.StartFrame, scale, euler, translate);

				if( !(euler == lasteuler) )
				{
					lasteuler = euler;
					R.setRotate(euler);
				}

				if(bPivotUsed == false)
				{
					if (bNeedScale) {
						local[i+2] = S*R;
					} else {
						local[i+2] = R;
					}
					local[i+2][3][0] = translate[0];
					local[i+2][3][1] = translate[1];
					local[i+2][3][2] = translate[2];
				}
				else
				{
					T.setTranslate(translate);
					if (bNeedScale) {
						local[i+2] = S*PI*R*P*T;
					} else {
						local[i+2] = PI*R*P*T;
					}

				}

				if(par)
					global[i+2] = local[i+2] * par->global[i+2];
				else
					global[i+2] = local[i+2];
			}
			break;

		default:		// Everything else
			for(i=0;i<numFrames;i++)
			{
				XbMatrix T; 
				channelListGetSRTFast(Info, i+Info.StartFrame, scale, euler, translate);
				if (hasAnimClips) {
					skelEvalAnimClips(skel,channelList,i+Info.StartFrame,scale,euler,translate,mask);
				}

				S.setScale(scale);
				if( !(euler == lasteuler) )
				{
					lasteuler = euler;
					R.setRotate(euler);
				}
				T.setTranslate(translate);

				if(bPivotUsed == false)
					local[i+2] = S*R*T;	// Skip two matrix mults if unnecessary
				else
					local[i+2] = S*PI*R*P*T;

				if(par)
					global[i+2] = local[i+2] * par->global[i+2];
				else
					global[i+2] = local[i+2];
			}
		}
	}


	void EA3RecordedSkeleton::getInitGlobal(XbMatrix &glb) {
		glb = global[0];
	}
  void EA3RecordedSkeleton::setInitGlobal(const XbMatrix &glb) {
		global[0] = glb;
    initGlobalInverse = glb.inverse();
	}

	void EA3RecordedSkeleton::getInitGlobalInverse(XbMatrix &glb) {
		glb = initGlobalInverse;
	}


	void EA3RecordedSkeleton::getGlobal(XbMatrix &glb) {
		glb = global[1];
	}
  void EA3RecordedSkeleton::setGlobal(const XbMatrix &glb) {
		global[1] = glb;
	}


	void EA3RecordedSkeleton::getInitLocal(XbMatrix &loc) {
		loc = local[0];
	}

  void EA3RecordedSkeleton::setInitLocal(const XbMatrix &loc) {
		local[0] = loc;
	}


	void EA3RecordedSkeleton::getLocal(XbMatrix &loc) {
		loc = local[1];
	}

  void EA3RecordedSkeleton::setLocal(const XbMatrix &loc) {
		local[1] = loc;
	}


	void EA3RecordedSkeleton::getAnimGlobal(XbMatrix &glb,int frame) {
		glb = global[frame+2];
	}

  void EA3RecordedSkeleton::setAnimGlobal(const XbMatrix &glb,int frame) {
		global[frame+2] = glb;
    if (frame == -2) initGlobalInverse = glb.inverse();
	}


	void EA3RecordedSkeleton::getAnimLocal(XbMatrix &loc,int frame) {
		loc = local[frame+2];
	}
  void EA3RecordedSkeleton::setAnimLocal(const XbMatrix &loc,int frame) {
		local[frame+2] = loc;
	}

	void EA3RecordedSkeleton::updateLocalFast(void)
	{
		int i,j;
		int ChildCount = children.getSize();
		if(ChildCount > 1)	// Only call this version when the overhead is worth it
		{
			XbArray<XbMatrix> ParentInv;
			int Count = numFrames+2;

			ParentInv.setSize(Count);
			for(i=0; i<Count; i++)
				ParentInv[i] = global[i].inverse();

			// Execute this loop for all children at the same time to keep the cache warm
			for(i=0; i<children.getSize(); i++)
			{
				XbMatrix *clocal = children[i]->local;
				XbMatrix *cglobal = children[i]->global;

				for(j=0; j<Count; j++)
					clocal[j] = cglobal[j] * ParentInv[j];
			}
			ParentInv.setSize(0);

			// Now get each child to do it as well
			for(i=0; i<children.getSize(); i++)
				children[i]->updateLocalFast();
		}
		else
		{
			for(i=0; i<children.getSize(); i++)
				children[i]->updateLocal(true);
		}
	}

	void EA3RecordedSkeleton::updateLocal(bool lockchildglobal)
	{
		int i,j;
		if(par)
		{
			for(i=0;i<numFrames+2;i++)
			{
				local[i] = global[i] * par->global[i].inverse();
			}
		}
		else
		{
			for(i=0; i<numFrames+2; i++)
			{
				local[i] = global[i];
			}
		}
		// this does not work, but I do not know why yet
		if(lockchildglobal && children.getSize() > 1)
		{
			XbArray<XbMatrix> ParentInv;
			int Count = numFrames+2;

			ParentInv.setSize(Count);
			for(i=0; i<Count; i++)
				ParentInv[i] = global[i].inverse();

			// Execute this loop for all children at the same time to keep the cache warm
			for(i=0; i<children.getSize(); i++)
			{
				XbMatrix *clocal = children[i]->local;
				XbMatrix *cglobal = children[i]->global;

				for(j=0; j<Count; j++)
					clocal[j] = cglobal[j] * ParentInv[j];
			}
			ParentInv.setSize(0);

			// Now get each child to do it as well
			for(i=0; i<children.getSize(); i++)
				children[i]->updateLocalFast();
		}
		else
		{
			for(i=0; i<children.getSize(); i++)
			{
				if(lockchildglobal)
					children[i]->updateLocal();
				else
					children[i]->updateGlobal();
			}
		}
	}


	void EA3RecordedSkeleton::updateGlobal(bool lockchildlocal) {
		int i;
		if (par) {
			for(i=0;i<numFrames+2;i++) {
				global[i] = local[i]*par->global[i];
			}
		} else {
			for(i=0;i<numFrames+2;i++) {
				global[i] = local[i];
			}
		}
		initGlobalInverse = global[0].inverse();
		for(i=0;i<children.getSize();i++) {
			if(lockchildlocal) children[i]->updateGlobal();
			else children[i]->updateLocal();
		}
	}

	void EA3RecordedSkeleton::postMultGlobalBy(XbMatrix &post) {
		for(int i=0;i<numFrames+2;i++) {
			global[i] = XbMatrix(global[i])*post;
		}
		initGlobalInverse = global[0].inverse();
	}
	void EA3RecordedSkeleton::preMultGlobalBy(XbMatrix &pre) {
		for(int i=0;i<numFrames+2;i++) {
			global[i] = pre*XbMatrix(global[i]);
		}
		initGlobalInverse = global[0].inverse();
	}
	void EA3RecordedSkeleton::postMultLocalBy(XbMatrix &post) {
		for(int i=0;i<numFrames+2;i++) {
			local[i] = XbMatrix(local[i])*post;
		}
	}
	void EA3RecordedSkeleton::preMultLocalBy(XbMatrix &pre) {
		for(int i=0;i<numFrames+2;i++) {
			local[i] = pre * XbMatrix(local[i]);
		}
	}


	void EA3RecordedSkeleton::cutChild(EA3RecordedSkeleton *ch) {
		for(int i=0;i<children.getSize();i++) {
			if (children[i] == ch) {
				children.remove(i);
				ch->par = NULL;
				ch->updateLocal();
				EA3Array *childs = (EA3Array*)skel->getByType(XBskeleton);
				if (childs) {
					childs->removeMember(ch->skel);
					if( childs->getNumMembers()==0 ) {
						skel->removeMember(childs);
					}
				}
				delete ch;
			}
		}
	}
	void EA3RecordedSkeleton::addChild(EA3RecordedSkeleton *ch) {
    if (ch->par) {
      for(int i=0;i<ch->par->children.getSize();i++) {
			  if (ch->par->children[i] == ch) {
			  	ch->par->children.remove(i);
        }
      }
    }
    ch->par = this;
    skel->createArray(XBskeleton)->appendMember(ch->skel);
    children.append(ch);
	}
	EA3RecordedSkeleton::~EA3RecordedSkeleton() {
		delete [] local;
		delete [] global;
		for(int i=0;i<children.getSize();i++) {
			delete children[i];
		}
  }

  EA3_Status EA3RecordedSkeleton::geometryApplyTransform(EA3Block *geom, const XbMatrix& matrix) {
    return (EA3_Status)geomApplyTransform(geom, matrix);
  }
  EA3_Status EA3RecordedSkeleton::freezeHierarchy(int recurse) {
    if(!skel) return EA3_OK;
    if(skel->getType()!=XBskeleton) {
      if(!recurse) return EA3_OK;
      EA3_Status r = EA3_OK;
      int e;
      for(int i=0; i<children.getSize(); i++) {
        e = EA3FreezeHierarchy(*children[i], recurse);
        if(e!=EA3_OK) r=(EA3_Status)e;
      }
      return r;
    }
    return (EA3_Status)EA3FreezeHierarchy(*this, recurse);
  }
  EA3_Status EA3RecordedSkeleton::freezeHierarchyWorld(int recurse) {
    if(!skel) return EA3_OK;
    if(skel->getType()!=XBskeleton) {
      if(!recurse) return EA3_OK;
      EA3_Status r = EA3_OK;
      int e;
      for(int i=0; i<children.getSize(); i++) {
        e = EA3FreezeHierarchyWorld(*children[i], recurse);
        if(e!=EA3_OK) r=(EA3_Status)e;
      }
      return r;
    }
    return (EA3_Status)EA3FreezeHierarchyWorld(*this, recurse);
  }
  EA3_Status EA3RecordedSkeleton::reParentSkeleton(EA3RecordedSkeleton *child) {
    if(!child->skel || child->skel->getType()!=XBskeleton) return EA3_OK;
    return (EA3_Status)EA3ReParentSkeleton(this, child);
  }
  EA3_Status EA3RecordedSkeleton::flattenHierarchy() {
    if(!skel) return EA3_OK;
    if(skel->getType()!=XBskeleton) {
      EA3_Status r = EA3_OK;
      int e;
      for(int i=0; i<children.getSize(); i++) {
        e = EA3FlattenHierarchy(*children[i]);
        if(e!=EA3_OK) r=(EA3_Status)e;
      }
      return r;
    }
    return (EA3_Status)EA3FlattenHierarchy(*this);
  }
  EA3_Status	EA3RecordedSkeleton::mergeHierarchies(EA3RecordedSkeleton *slave) {
    if(!skel || skel->getType()!=XBskeleton) return EA3_OK;
    if(!slave->skel || slave->skel->getType()!=XBskeleton) return EA3_OK;
    return (EA3_Status)EA3MergeHierarchies(this, slave);
  }
  EA3_Status	EA3RecordedSkeleton::collapse() {
    if(!skel || skel->getType()!=XBskeleton) return EA3_OK;
    return (EA3_Status)EA3Collapse(*this);
  }

  bool rsApplyFunc(EA3RecordedSkeleton *rs, RecordedSkeletonFunc *f, void *data) {
    if( !(*f)(rs,data) ) return false;
    int i;
    for( i=0; i<rs->children.getSize(); i++ ) if( !rsApplyFunc(rs->children[i], f, data) ) return false;
    return true;
  }

								//@}
} //namespace

