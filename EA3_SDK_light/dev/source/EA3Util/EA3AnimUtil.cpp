// (c) 1998-2010 Electronic Arts Inc.

/** \file EA3AnimUtil.cxx
 *
 *  Contains utility functions for operations on animation blocks, channel
 *  blocks, function curve blocks, and key fields.
 */
#include "EA3.h"
#include "EA3AnimUtil.h"
#include "EA3SkelUtil.h"
#include "EA3Expr.h"
#include "mayaCurve.h"

using namespace EA3Expr;
namespace EA3Util {

/** @name ANIMATION FUNCTIONS
 *  Utility functions for working with animations
 */
//@{

XbIdentifier xbTexEul("tex_eul");	// Pre-built these identifiers so we don't have to keep looking for them
XbIdentifier xbTexTrn("tex_trn");
XbIdentifier xbTexScl("tex_scl");


EA3_Status animGetFPS(EA3Block *anim,float &fps) {
  EA3Field *fpsFld = anim->getByType(XBFPS);	// Use the pre-built ID's where possible
  fps = 30.0f;
  if (fpsFld) {
    fps = fpsFld->data[0].f;
  }
  return(EA3_OK);
}

EA3_Status animSetFPS(EA3Block *anim,float fps,bool timeWarp) {
  EA3Field *fpsFld = anim->getByType(XBFPS);
  float oldFps = 30.0f;
  if (fpsFld) {
    oldFps = fpsFld->data[0].f;
  } else {
    fpsFld = anim->createField("FPS");
  }
  fpsFld->data[0].f = fps;
  if (timeWarp) {
    float warp = oldFps/fps;
    findFirst(anim,HasType("key")&&IsArray()&&HasMember(SetValue(0,GetValue(0)*warp)&&0));
  }
  return(EA3_OK);
}


EA3_Status animGetChannels(const EA3Block *anim,const XbString &name,XbPArray<EA3Block *> &outlist) {
  int i;
  EA3Array *chans = (EA3Array *)anim->getByType(XBchannel);
  for(i=0;chans && i<chans->getNumMembers();i++) {
    EA3Base *b = chans->getByIndex(i);
    if(b->getName()==name) outlist+=(EA3Block*)b;
  }
  return(EA3_OK);
}


EA3_Status animCreateDiscreteF3Channels(EA3Block *anim,const XbString &name, EA3Block *skel, XbPArray<EA3Block *> &outlist,unsigned int &types) {
  unsigned int what = 0;
  EA3Array *chans = anim->createArray("channel","");
  EA3Block *bone = skel->getByType(XBbone);
  if (!bone) return(EA3_NOT_FOUND);
  EA3Block *xform = boneGetXform(bone,"local");
  if (!xform) return(EA3_NOT_FOUND);
  EA3Base *b = xform->getByType("euler");
  if (b) {
    EA3Block *bl = (EA3Block *)chans->createBlock("discreteF3Channel",name);
    outlist += bl;
    what |= EULER;
    EA3Field *f = bl->createField("what","");
    f->set("eul");
    //EA3Reference *r = bl->createReference("connectTo","");
    //r->addReference(b);
  }
  b = xform->getByType("translation");
  if (b) {
    EA3Block *bl = (EA3Block *)chans->createBlock("discreteF3Channel",name);
    outlist += bl;
    what |= TRANSLATION;
    EA3Field *f = bl->createField("what","");
    f->set("trn");
    //EA3Reference *r = bl->createReference("connectTo","");
    //r->addReference(b);
  }
  types = what;
//  EA3Reference *links = bone->getByType("links");
	EA3Reference *links = bone->createReference("links");
 if (links) for(int i=0;i<outlist.getSize();i++) links->addReference(outlist[i],true);
  return(EA3_OK);
}

EA3_Status animCreateDiscreteF3Channels(EA3Block *anim,const XbString &name, unsigned int what, XbPArray<EA3Block *> &outlist,unsigned int &types,EA3Block *skel) {
  EA3Array *chans = anim->createArray("channel","");
  if (what & EULER) {
    EA3Block *bl = (EA3Block *)chans->createBlock(XBdiscreteF3Channel,name);
    outlist += bl;
    EA3Field *f = bl->createField("what","");
    f->set("eul");
  }
  if (what & TRANSLATION) {
    EA3Block *bl = (EA3Block *)chans->createBlock(XBdiscreteF3Channel,name);
    outlist += bl;
    EA3Field *f = bl->createField(XBwhat,"");
    f->set("trn");
  }
	if (what & SCALE) {
		EA3Block *bl = (EA3Block *)chans->createBlock(XBdiscreteF3Channel,name);
		outlist += bl;
		EA3Field *f = bl->createField(XBwhat,"");
		f->set("scl");
	}
  if (skel) {
    EA3Block *bone = skel->getByType("bone");
    if (bone) {
			//EA3Reference *links = bone->getByType("links");
			EA3Reference *links = bone->createReference("links");
      if (links) for(int i=0;i<outlist.getSize();i++) links->addReference(outlist[i],true);
    }
  }
  types = what;
  return(EA3_OK);
}

EA3_Status animCreateF1Channel(EA3Block *anim,const XbString &name, XbIdentifier what,unsigned int index,bool discrete,EA3Block *skel, XbPArray<EA3Block *> &outlist) {
  EA3Array *chans = anim->createArray(XBchannel);
  XbIdentifier chanType(discrete? XBdiscreteF1Channel : XBcontinuousF1Channel);
  EA3Block *bl = (EA3Block *)chans->createBlock(chanType,name);
  outlist += bl;
  bl->createField(XBwhat)->data[0].str = what;
  bl->createField(XBindex)->data[0].i = index;
  if (skel) {
    EA3Block *bone = skel->getByType("bone");
    if (bone) {
//      EA3Reference *links = bone->getByType("links");
			EA3Reference *links = bone->createReference("links");
      if (links) for(int i=0;i<outlist.getSize();i++) links->addReference(outlist[i],true);
    }
  }
  return(EA3_OK);
}

EA3_Status animSaveKey(EA3Block *anim,XbPArray<EA3Block *> &clist, XbMatrix &mat, int frame) {
  XbPArray<EA3Block *> eulChan;
  XbPArray<EA3Block *> trnChan;
  EA3Block *chan = 0;
  for(int c=0;c<clist.getSize();c++) {
    chan = clist[c];
    EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
    if (!what) continue;
    if(what->data[0].str == XBeul) {
      eulChan.append(chan);
    }
	  else if(what->data[0].str == XBtrn) {
      trnChan.append(chan);
    }
  }

  XbVec3f trn;
  XbEuler eul;
  XbRotation rot,srot;
  XbVec3f scl;
  XbVec3f ctr(0,0,0);
  //mat.getTransform(trn,rot,scl,srot,ctr);
  eul = XbEuler(mat,XbEuler::XYZ,XbEuler::STATIC);
  trn = mat.getTranslation();
/*
  ctr = mat.getTranslation();
  if ((ctr-trn).length() > 0.01) {
    printf("error decomposing matrix for frame %d\n",frame);
  }
*/
  if (eulChan.getSize()) channelListSaveKey(eulChan,eul,frame);
  if (trnChan.getSize()) channelListSaveKey(trnChan,trn,frame);
  return(EA3_OK);
}

EA3_Status animBuildChannels(EA3Block *anim, XbPArray<EA3Block *> &clist, const XbString &name, EA3Block *skel)
{
	//XbPArray<EA3Block *> eulChan, XbPArray<EA3Block *> trnChan
	EA3Block *chan = 0;
	bool hasEul[3] = {0,0,0};
	bool hasTrn[3] = {0,0,0};
	int c, eulChans = 0, trnChans = 0;

	for(c=0; c<clist.getSize(); c++)
	{
		chan = clist[c];
		EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
		if(!what) continue;

		if(what->data[0].str == XBeul)
		{
			eulChans++;
			if(chan->getType() == XBdiscreteF3Channel)
			{
				hasEul[0] = hasEul[1] = hasEul[2] = true;
			}
			else
			{
				EA3Field *ixF = chan->getByType(XBindex);
				if (ixF && ixF->data[0].i < 3)
					hasEul[ixF->data[0].i] = true;
			}
		}
		else if(what->data[0].str == XBtrn)
		{
			trnChans++;
			if(chan->getType()==XBdiscreteF3Channel)
				hasTrn[0] = hasTrn[1] = hasTrn[2] = true;
			else
			{
				EA3Field *ixF = chan->getByType(XBindex);
				if (ixF && ixF->data[0].i < 3)
					hasTrn[ixF->data[0].i] = true;
			}
		}
	}

	if(eulChans == 0)
	{
		//cerr << "No eul channel in animation list (" << list.getSize() << ")\n";
		unsigned int types;
		if (skel)
			animCreateDiscreteF3Channels(anim,name, EULER, clist, types, skel);
		else
			animCreateDiscreteF3Channels(anim,name, EULER, clist, types);
	}
	else
	{
		for(c=0;c<3;c++)
		{
			if(!hasEul[c])
			{
				EA3Array *chans = anim->getByType(XBchannel);
				EA3Block *newchan = chans->createBlock(XBdiscreteF1Channel,name);
				newchan->createField(XBindex)->data[0].i = c;
				newchan->createField(XBwhat)->data[0].str = XBeul;
				newchan->createArray(XBdata);
				EA3Block *bone = skel->getByType(XBbone);
				EA3Reference *links = bone->getByType(XBlinks);
				if(links)
					links->addReference(newchan);

				clist.append(newchan);
			}
		}
	}

	if(trnChans == 0)
	{
		//cerr << "No trn channel in animation list (" << list.getSize() << ")\n";
		unsigned int types;
		if(skel)
			animCreateDiscreteF3Channels(anim, name, TRANSLATION, clist, types, skel);
		else
			animCreateDiscreteF3Channels(anim, name, TRANSLATION, clist, types);
	}
	else
	{
		for(c=0;c<3;c++)
		{
			if(!hasTrn[c])
			{
				EA3Array *chans = anim->getByType(XBchannel);
				EA3Block *newchan = chans->createBlock(XBdiscreteF1Channel,name);
				newchan->createField(XBindex)->data[0].i = c;
				newchan->createField(XBwhat)->data[0].str=XBtrn;
				newchan->createArray(XBdata);
				EA3Block *bone = skel->getByType(XBbone);
				EA3Reference *links = bone->getByType(XBlinks);
				if(links)
					links->addReference(newchan);

				clist.append(newchan);
			}
		}
	}
	return(EA3_OK);
}


EA3_Status animBuildChannelInfo(EA3Block *anim, XbPArray<EA3Block *> &clist, ChannelInfo &eulChan, ChannelInfo &trnChan)
{
	eulChan.Count = 0;
	trnChan.Count = 0;

	EA3Block *chan = 0;
	for(int c=0; c<clist.getSize(); c++)
	{
		chan = clist[c];

		EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
		if (!what) continue;

		// Figure out if this is a channel we want
		ChannelInfo *pAddTo = 0;
		if(what->data[0].str == XBeul)
			pAddTo = &eulChan;
		else if(what->data[0].str == XBtrn)
			pAddTo = &trnChan;

		if(pAddTo)	// Found something of interest
		{
			// Cache the counter, and increment it
			int i = pAddTo->Count++;
			pAddTo->Chan[i].pChan = chan;
			pAddTo->Chan[i].pData = 0;

			if(chan->getType() == XBdiscreteF3Channel)
			{
				pAddTo->Chan[i].Type = CSRT_F3;

				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(!data) data = chan->createArray(XBdata);
				pAddTo->Chan[i].pData = data;
			}
			else
			{
				// Grab the channel index
				EA3Field *ixF = chan->getByType(XBindex);
				if(ixF && ixF->data[0].i < 3)
					pAddTo->Chan[i].Index = ixF->data[0].i;

				if(chan->getType() == XBdiscreteF1Channel)
				{
					pAddTo->Chan[i].Type = CSRT_F1Discrete;

					EA3Array *data = (EA3Array*)chan->getByType(XBdata);
					if(!data) data = chan->createArray(XBdata);
					pAddTo->Chan[i].pData = data;
				}
				else if(chan->getType() == XBcontinuousF1Channel)
				{
					pAddTo->Chan[i].Type = CSRT_F1Continuous;

					EA3Array *data = (EA3Array*)chan->getByType(XBkey);
					if(!data) data = chan->createArray(XBkey);
					pAddTo->Chan[i].pData = data;
				}
			}
		}
	}
	return(EA3_OK);
}

/*
EA3_Status animSaveKeyFast(EA3Block *anim, XbPArray<EA3Block *> &eulChan, XbPArray<EA3Block *> &trnChan, XbMatrix &mat, int frame)
{
  XbEuler eul(mat, XbEuler::XYZ, XbEuler::STATIC);
  XbVec3f trn = mat.getTranslation();

  if (eulChan.getSize()) channelListSaveKey(eulChan,eul,frame);
  if (trnChan.getSize()) channelListSaveKey(trnChan,trn,frame);
  return(EA3_OK);
}
*/


EA3_Status animSaveKey(EA3Block *anim,XbPArray<EA3Block *> &clist, XbMatrix &mat, int frame, const XbString &name, EA3Block *skel)
{
  XbPArray<EA3Block *> eulChan;
  XbPArray<EA3Block *> trnChan;
  EA3Block *chan = 0;
  bool hasEul[3] = {0,0,0};
  bool hasTrn[3] = {0,0,0};
  int c;

  for(c=0;c<clist.getSize();c++)
  {
    chan = clist[c];
    EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
    if (!what) continue;
    if(what->data[0].str == XBeul)
	{
      eulChan.append(chan);
      if (chan->getType()==XBdiscreteF3Channel) {
        hasEul[0]=hasEul[1]=hasEul[2]=true;
      } else {
        EA3Field *ixF = chan->getByType(XBindex);
        if (ixF && ixF->data[0].i<3) {
          hasEul[ixF->data[0].i]=true;
        }
      }
    }
	else if(what->data[0].str == XBtrn)
	{
      trnChan.append(chan);
      if (chan->getType()==XBdiscreteF3Channel) {
        hasTrn[0]=hasTrn[1]=hasTrn[2]=true;
      } else {
        EA3Field *ixF = chan->getByType(XBindex);
        if (ixF && ixF->data[0].i<3) {
          hasTrn[ixF->data[0].i]=true;
        }
      }
    }
  }

  if(eulChan.getSize()==0)
  {
    //cerr << "No eul channel in animation list (" << list.getSize() << ")\n";
    unsigned int types;
    if (skel) {
      animCreateDiscreteF3Channels(anim,name, EULER, clist, types,skel);
    } else {
      animCreateDiscreteF3Channels(anim,name, EULER, clist, types);
    }
    for(c=0;c<clist.getSize();c++) {
      chan = clist[c];
      EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
      if (!what) continue;
      if (XbString(what->data[0].str) == XBeul) {
        eulChan.append(chan);
      }
    }           
  }
  else
  {
    for(c=0;c<3;c++) {
      if (!hasEul[c]) {
        EA3Array *chans = anim->getByType(XBchannel);
        EA3Block *newchan = chans->createBlock(XBdiscreteF1Channel,name);
        newchan->createField(XBindex)->data[0].i = c;
        newchan->createField(XBwhat)->data[0].str=XBeul;
        newchan->createArray(XBdata);
        EA3Block *bone = skel->getByType(XBbone);
        EA3Reference *links = bone->getByType(XBlinks);
        if (links) links->addReference(newchan);
        eulChan.append(newchan);
        clist.append(newchan);
      }
    }
  }

  if(trnChan.getSize()==0)
  {
    //cerr << "No trn channel in animation list (" << list.getSize() << ")\n";
    unsigned int types;
    if (skel) {
      animCreateDiscreteF3Channels(anim,name, TRANSLATION, clist, types, skel);
    } else {
      animCreateDiscreteF3Channels(anim,name, TRANSLATION, clist, types);
    }
    for(c=0;c<clist.getSize();c++) {
      chan = clist[c];
      EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
      if (!what) continue;
      if (XbString(what->data[0].str) == "trn") {
        trnChan.append(chan);
      }
    }           
  }
  else
  {
    for(c=0;c<3;c++)
	{
      if (!hasTrn[c]) {
        EA3Array *chans = anim->getByType(XBchannel);
        EA3Block *newchan = chans->createBlock(XBdiscreteF1Channel,name);
        newchan->createField(XBindex)->data[0].i = c;
        newchan->createField(XBwhat)->data[0].str=XBtrn;
        newchan->createArray(XBdata);
        EA3Block *bone = skel->getByType(XBbone);
        EA3Reference *links = bone->getByType(XBlinks);
        if (links) links->addReference(newchan);
        trnChan.append(newchan);
        clist.append(newchan);
      }
    }
  }
  XbVec3f trn;
  XbEuler eul;
  XbRotation rot,srot;
  XbVec3f scl;
  XbVec3f ctr(0,0,0);
  //mat.getTransform(trn,rot,scl,srot,ctr);
  eul = XbEuler(mat,XbEuler::XYZ,XbEuler::STATIC);
  trn = mat.getTranslation();
/*
  ctr = mat.getTranslation();
  if ((ctr-trn).length() > 0.01) {
    printf("error decomposing matrix for frame %d\n",frame);
  }
*/
  if (eulChan.getSize()) channelListSaveKey(eulChan,eul,frame);
  if (trnChan.getSize()) channelListSaveKey(trnChan,trn,frame);
  return(EA3_OK);
}




EA3_Status animGetMatrix(const EA3Block *anim,XbPArray<EA3Block *> &clist,int frame,XbMatrix &mat) {
  XbVec3f trn(0.0f,0.0f,0.0f);
  XbEuler eul(XbVec3f(0.0f,0.0f,0.0f),XbEuler::XYZ,XbEuler::STATIC);
  XbVec3f scl(1.0f,1.0f,1.0f);
  return(animGetMatrix(anim,clist,frame,mat,trn,eul,scl));
}

// Function to just get rotation channel data
EA3_Status channelListGetR(XbPArray<EA3Block *> &clist, int frame, XbEuler &eul)
{
	EA3Block *chan;
	int startframe = 0;
	if (clist.getSize()==0)
		return(EA3_OK);

	EA3Field *sf = clist[0]->getContainer()->getContainer()->getByType(XBstartFrame);
	if(sf)
		startframe = sf->data[0].i;

	for(int c=0; c<clist.getSize(); c++)
	{
		chan = clist[c];
		EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
		if (!what) continue;

		if (what->data[0].str == XBeul || what->data[0].str == xbTexEul )
		{
			if(chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data && data->getNumMembers())	// Do this check here to save overhead
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;
					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					eul.setAngles( XbVec3f(f->data[0].f*D2R, f->data[1].f*D2R, f->data[2].f*D2R) );
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data && data->getNumMembers())	// Do this check here to save overhead
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

					XbVec3f ang = eul.getAngles();
					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					ang[ix] = f->data[0].f*D2R;
					eul.setAngles(ang);
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS);
				float FPS = fps? fps->data[0].f : 30.0f;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBkey);
				if(data && data->getNumMembers())
				{
					float time = float(frame)/FPS;
					XbVec3f ang = eul.getAngles();
					float val;
					fcrvEval(chan,time,val);
					ang[ix] = val*D2R;
					eul.setAngles(ang);
				}
			}
		}
	}

	return(EA3_OK);
}

EA3_Status channelListGetSRT(XbPArray<EA3Block *> &clist, int frame, XbVec3f &scl, XbEuler &eul, XbVec3f &trn)
{
	EA3Block *chan;
	int startframe = 0;
	if (clist.getSize()==0)
		return(EA3_OK);

	EA3Field *sf = clist[0]->getContainer()->getContainer()->getByType(XBstartFrame);
	if(sf)
		startframe = sf->data[0].i;

	for(int c=0; c<clist.getSize(); c++)
	{
		chan = clist[c];
		EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
		if (!what) continue;

		if (what->data[0].str == XBeul || what->data[0].str == xbTexEul )
		{
			if(chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data && data->getNumMembers())	// Do this check here to save overhead
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;
					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					eul.setAngles( XbVec3f(f->data[0].f*D2R, f->data[1].f*D2R, f->data[2].f*D2R) );
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data && data->getNumMembers())	// Do this check here to save overhead
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

					XbVec3f ang = eul.getAngles();
					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					ang[ix] = f->data[0].f*D2R;
					eul.setAngles(ang);
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS);
				float FPS = fps? fps->data[0].f : 30.0f;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBkey);
				if(data && data->getNumMembers())
				{
					float time = float(frame)/FPS;
					XbVec3f ang = eul.getAngles();
					float val;
					fcrvEval(chan,time,val);
					ang[ix] = val*D2R;
					eul.setAngles(ang);
				}
			}
		}
		else if(what->data[0].str == XBtrn || what->data[0].str == xbTexTrn)	// else's here reduce work
		{
			if (chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data && data->getNumMembers())
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					trn.setValue(f->data[0].f,f->data[1].f,f->data[2].f);
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data && data->getNumMembers())
				{
					int dataMembers = data->getNumMembers();
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= dataMembers) frameInRange = dataMembers-1;

					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					trn[ix] = f->data[0].f;
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS );
				float FPS = fps? fps->data[0].f : 30.0f;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBkey);

				if(data && data->getNumMembers())
				{
					float time = float(frame)/FPS;
					XbVec3f ang = eul.getAngles();
					float val;
					fcrvEval(chan,time,val);
					trn[ix] = val;
				}
			}
		}
		else if (what->data[0].str == XBscl || what->data[0].str == xbTexScl )	// else's here reduce work
		{
			if (chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data && data->getNumMembers())
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					scl.setValue(f->data[0].f, f->data[1].f, f->data[2].f);
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (data && data->getNumMembers())
				{
					int frameInRange = frame-startframe;
					if(frameInRange < 0) frameInRange = 0;
					else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

					EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
					scl[ix] = f->data[0].f;
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS);
				float FPS = fps? fps->data[0].f : 30.0f;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBkey);

				if(data && data->getNumMembers())
				{
					float time = float(frame)/FPS;
					XbVec3f ang = eul.getAngles();
					float val;
					fcrvEval(chan,time,val);
					scl[ix] = val;
				}
			}
		}
	}

	return(EA3_OK);
}


// Gather channel info for an animation we're about to record
EA3_Status channelListGetSRTInfo(XbPArray<EA3Block *> &clist, ChannelListSRTInfo &Info)
{
	memset(&Info, 0, sizeof(Info));
	Info.fps = 30.f;

	EA3Block *chan;
	int startframe = 0;
	if (clist.getSize()==0)
		return(EA3_OK);

	EA3Field *sf = clist[0]->getContainer()->getContainer()->getByType(XBstartFrame);
	if(sf)
		startframe = sf->data[0].i;

	Info.StartFrame = startframe;

	for(int c=0; c<clist.getSize(); c++)
	{
		chan = clist[c];
		EA3Field *what = (EA3Field *)chan->getByType(XBwhat);
		if (!what) continue;

		if(what->data[0].str == XBeul || what->data[0].str == xbTexEul )
		{
			int i = Info.R.Count;
			Info.R.Chan[i].pChan = chan;
			if(chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data)
				{
					Info.R.Chan[i].pData = data;
					Info.R.Chan[i].Type = CSRT_F3;
					Info.R.Count++;
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if (!data) data = chan->createArray(XBdata);

				if(data)
				{
					Info.R.Chan[i].pData= data;
					Info.R.Chan[i].Type = CSRT_F1Discrete;
					Info.R.Chan[i].Index = ix;
					Info.R.Count++;
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS);
				Info.fps = fps? fps->data[0].f : 30.0f;
				if(ifld) ix=ifld->data[0].i;

				EA3Array *data = (EA3Array*)chan->getByType(XBkey);
				if(data)
				{
					Info.R.Chan[i].pData = data;
					Info.R.Chan[i].Type = CSRT_F1Continuous;
					Info.R.Chan[i].Index = ix;
					Info.R.Count++;
				}
			}
		}
		else if(what->data[0].str == XBtrn || what->data[0].str == xbTexTrn)	// else's here reduce work
		{
			int i = Info.T.Count;
			Info.T.Chan[i].pChan = chan;
			if (chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data)
				{
					Info.T.Chan[i].pData = data;
					Info.T.Chan[i].Type = CSRT_F3;
					Info.T.Count++;
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data)
				{
					Info.T.Chan[i].pData = data;
					Info.T.Chan[i].Type = CSRT_F1Discrete;
					Info.T.Chan[i].Index = ix;
					Info.T.Count++;
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS );
				Info.fps = fps? fps->data[0].f : 30.0f;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBkey);

				if(data)
				{
					Info.T.Chan[i].pData = data;
					Info.T.Chan[i].Type = CSRT_F1Continuous;
					Info.T.Chan[i].Index = ix;
					Info.T.Count++;
				}
			}
		}
		else if (what->data[0].str == XBscl || what->data[0].str == xbTexScl )	// else's here reduce work
		{
			int i = Info.S.Count;
			Info.S.Chan[i].pChan = chan;
			if (chan->getType() == XBdiscreteF3Channel)
			{
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data)
				{
					Info.S.Chan[i].pData = data;
					Info.S.Chan[i].Type = CSRT_F3;
					Info.S.Count++;
				}
			}
			else if(chan->getType() == XBdiscreteF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBdata);
				if(data)
				{
					Info.S.Chan[i].pData = data;
					Info.S.Chan[i].Type = CSRT_F1Discrete;
					Info.S.Chan[i].Index = ix;
					Info.S.Count++;
				}
			}
			else if(chan->getType() == XBcontinuousF1Channel)
			{
				EA3Field *ifld = (EA3Field *)chan->getByType(XBindex);
				int ix = 0;
				EA3Field *fps = chan->getContainer()->getContainer()->getByType(XBFPS);
				Info.fps = fps? fps->data[0].f : 30.0f;
				if (ifld) ix=ifld->data[0].i;
				EA3Array *data = (EA3Array*)chan->getByType(XBkey);

				if(data)
				{
					Info.S.Chan[i].pData = data;
					Info.S.Chan[i].Type = CSRT_F1Continuous;
					Info.S.Chan[i].Index = ix;
					Info.S.Count++;
				}
			}
		}
	}
	return(EA3_OK);
}

EA3_Status channelListGetRFast(const ChannelInfo &Info, int frame, XbEuler &eul, int StartFrame, float fps)
{
int i;

	// Get the rotation
	for(i=0; i<Info.Count; i++)
	{
		const ChanRec &CRec = Info.Chan[i];
		EA3Array *data = CRec.pData;
		switch(CRec.Type)
		{
		case CSRT_None:
			break;

		case CSRT_F3:
			if(data && data->getNumMembers())
			{
				int frameInRange = frame - StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				eul.setAngles( XbVec3f(f->data[0].f*D2R, f->data[1].f*D2R, f->data[2].f*D2R) );
			}
			break;

		case CSRT_F1Discrete:
			if(data && data->getNumMembers())
			{
				int frameInRange = frame - StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

				XbVec3f ang = eul.getAngles();
				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				ang[CRec.Index] = f->data[0].f*D2R;
				eul.setAngles(ang);
			}
			break;

		case CSRT_F1Continuous:
			if(data && data->getNumMembers())
			{
				float time = float(frame) / fps;
				XbVec3f ang = eul.getAngles();
				float val;
				fcrvEval(CRec.pChan, time, val);
				ang[CRec.Index] = val*D2R;
				eul.setAngles(ang);
			}
			break;
		}
	}
	return(EA3_OK);
}


// Use the previously gathered channel information to speed up individual frame value accesses
EA3_Status channelListGetSRTFast(const ChannelListSRTInfo &Info, int frame, XbVec3f &scl, XbEuler &eul, XbVec3f &trn)
{
int i;

	// Get the rotation
	for(i=0; i<Info.R.Count; i++)
	{
		const ChanRec &CRec = Info.R.Chan[i];
		EA3Array *data = CRec.pData;

		switch(CRec.Type)
		{
		case CSRT_None:
			break;

		case CSRT_F3:
			if(data && data->getNumMembers())
			{
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				eul.setAngles( XbVec3f(f->data[0].f*D2R, f->data[1].f*D2R, f->data[2].f*D2R) );
			}
			break;

		case CSRT_F1Discrete:
			if(data && data->getNumMembers())
			{
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= data->getNumMembers()) frameInRange = data->getNumMembers()-1;

				XbVec3f ang = eul.getAngles();
				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				ang[CRec.Index] = f->data[0].f*D2R;
				eul.setAngles(ang);
			}
			break;

		case CSRT_F1Continuous:
			if(data && data->getNumMembers())
			{
				float time = float(frame) / Info.fps;
				XbVec3f ang = eul.getAngles();
				float val;
				fcrvEval(CRec.pChan, time, val);
				ang[CRec.Index] = val*D2R;
				eul.setAngles(ang);
			}
			break;
		}
	}

	// Get the translation
	for(i=0; i<Info.T.Count; i++)
	{
		const ChanRec &CRec = Info.T.Chan[i];
		EA3Array *data = CRec.pData;

		switch(CRec.Type)
		{
		case CSRT_None:
			break;

		case CSRT_F3:
			if(data && data->getNumMembers())
			{
				int dataMembers = data->getNumMembers();
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= dataMembers) frameInRange = dataMembers-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				trn.setValue(f->data[0].f,f->data[1].f,f->data[2].f);
			}
			break;

		case CSRT_F1Discrete:
			if(data && data->getNumMembers())
			{
				int dataMembers = data->getNumMembers();
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= dataMembers) frameInRange = dataMembers-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				trn[CRec.Index] = f->data[0].f;
			}
			break;

		case CSRT_F1Continuous:
			if(data && data->getNumMembers())
			{
				float time = float(frame) / Info.fps;
				XbVec3f ang = eul.getAngles();
				float val;
				fcrvEval(CRec.pChan, time, val);
				trn[CRec.Index] = val;
			}
			break;
		}
	}

	// Get the scale
	for(i=0; i<Info.S.Count; i++)
	{
		const ChanRec &CRec = Info.S.Chan[i];
		EA3Array *data = CRec.pData;

		switch(CRec.Type)
		{
		case CSRT_None:
			break;

		case CSRT_F3:
			if(data && data->getNumMembers())
			{
				int dataMembers = data->getNumMembers();
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= dataMembers) frameInRange = dataMembers-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				scl.setValue(f->data[0].f, f->data[1].f, f->data[2].f);
			}
			break;

		case CSRT_F1Discrete:
			if(data && data->getNumMembers())
			{
				int dataMembers = data->getNumMembers();
				int frameInRange = frame-Info.StartFrame;
				if(frameInRange < 0) frameInRange = 0;
				else if(frameInRange >= dataMembers) frameInRange = dataMembers-1;

				EA3Field *f = (EA3Field *)data->getByIndex(frameInRange);
				scl[CRec.Index] = f->data[0].f;
			}
			break;

		case CSRT_F1Continuous:
			if(data && data->getNumMembers())
			{
				float time = float(frame)/Info.fps;
				XbVec3f ang = eul.getAngles();
				float val;
				fcrvEval(CRec.pChan, time, val);
				scl[CRec.Index] = val;
			}
			break;
		}
	}

	return(EA3_OK);
}

EA3_Status animGetMatrix(const EA3Block *anim,XbPArray<EA3Block *> &clist,int frame,XbMatrix &mat,XbVec3f &trn, XbEuler &eul, XbVec3f &scl) {
  channelListGetSRT(clist,frame,scl,eul,trn);
  mat.setTransform(trn,eul,scl);
  return(EA3_OK);
}


EA3_Status channelSaveKey(EA3Block *channel,const XbVec3f &val,int frame) {
  int startFrame = 0;
  EA3Block *anim = channel->getContainer()->getContainer();
  EA3Field *sf = anim->getByType(XBstartFrame);
  if (sf) startFrame = sf->data[0].i;
  EA3Array *f3Array = (EA3Array *)channel->createArray(XBdata,"");
  EA3Field *f;
  while (f3Array->getNumMembers()<=frame-startFrame) {
    f = f3Array->createField(XBdata,"");
    f->data[0].f = val[0];
    f->data[1].f = val[1];
    f->data[2].f = val[2];
  }
  f = (EA3Field *)f3Array->getByIndex(frame-startFrame);
  f->data[0].f = val[0];
  f->data[1].f = val[1];
  f->data[2].f = val[2];
  return(EA3_OK);
}


EA3_Status channelSaveKey(EA3Block *channel,const XbEuler &eul,int frame)
{
  XbVec3f v = eul.getAngles()*R2D;
  return (channelSaveKey(channel,v,frame)) ;
}

EA3_Status channelSaveKey( EA3Block *channel, float val, int frame ) {
  int startFrame = 0;
  EA3Block *anim = channel->getContainer()->getContainer();
  EA3Field *sf = anim->getByType(XBstartFrame);
  if (sf) startFrame = sf->data[0].i;
  XbIdentifier type = channel->getType();
  bool discrete = type==XBdiscreteF1Channel?true:false;
  EA3Array *fArray = discrete? channel->createArray(XBdata,"") : channel->createArray(XBkey,"");
  EA3Field *f;
  if (discrete) {
    while (fArray->getNumMembers()<=frame-startFrame) {
      f = fArray->createField(XBdata,"");
      f->data[0].f = val;
    }
    f = fArray->getByIndex(frame-startFrame);
    f->data[0].f = val;
  } else {
    EA3Field *k1=NULL,*k2=NULL;
    EA3Field *fps = channel->getContainer()->getContainer()->getByType(XBFPS);
    float FPS = fps? fps->data[0].f : 30.0f;
    float time = float(frame)/FPS;
    int which = -1;
    fcrvFindPreviousKey(channel,time,&which,k1);
    if (k1) {
      if (fabs(k1->data[0].f - time) < 0.01) {
        k2 = k1;
      } else {
        k2 = fArray->getByIndex(which+1);
      }
      if (k2) {
        if (fabs(k2->data[0].f - time) < 0.01) {
        } else {
          k2 = NULL;
        }
      }
      if (!k2) {
        k2 = fArray->createField("linearKey");
        k2->data[0].f = time;
        fcrvSortKeys(channel,fArray);
      }
      k2->data[1].f = val;
    } else {
      k2 = fArray->createField("linearKey");
      k2->data[0].f = time;
      k2->data[1].f = val;
    }
  }
  return(EA3_OK);
}

EA3_Status channelListSaveKey( XbPArray<EA3Block *> &clist, const XbVec3f &val, int frame )
{
	int c;
	for(c=0; c<clist.getSize(); c++)
	{
		EA3Block *chan = clist[c];
		if(chan->getType() == XBdiscreteF1Channel || chan->getType() == XBcontinuousF1Channel)
		{
			EA3Field *ix = chan->getByType(XBindex);
			if(ix)
			{
				EA3_Status st = channelSaveKey(chan,val[ix->data[0].i],frame);
				if (st != EA3_OK) return st;
			}
		}
		else if (chan->getType() == XBdiscreteF3Channel)
		{
			return (channelSaveKey(chan,val,frame));
		}
	}
	return EA3_OK;
}


EA3_Status channelListSaveKey( XbPArray<EA3Block *> &clist, const XbEuler &eul, int frame )
{
	XbEuler oldeul(XbVec3f(0,0,0), XbEuler::XYZ,XbEuler::STATIC);
	channelListGetR(clist, frame-1, oldeul);	// Why grab them all when you only need one?
	XbVec3f v = eul.closest(oldeul.getAngles()).getAngles()*R2D;
	return channelListSaveKey(clist, v, frame);
}

// ----------------------------------------------------------------------------
// Warp speed anim save functions
// ----------------------------------------------------------------------------

EA3_Status channelSaveKeyFast(const ChanRec &Chan, const XbVec3f &val, int frame, int startFrame)
{
	frame -= startFrame;
	EA3Array *f3Array = Chan.pData;
	EA3Field *f;

	while(f3Array->getNumMembers() <= frame)
	{
		f = f3Array->createField(XBdata, "");
		f->data[0].f = val[0];
		f->data[1].f = val[1];
		f->data[2].f = val[2];
	}

	f = (EA3Field *)f3Array->getByIndex(frame);
	f->data[0].f = val[0];
	f->data[1].f = val[1];
	f->data[2].f = val[2];
	return EA3_OK;
}


EA3_Status channelSaveKeyFast(const ChanRec &Chan, const XbEuler &eul, int frame, int startFrame)
{
	XbVec3f v = eul.getAngles()*R2D;
	return channelSaveKeyFast(Chan, v, frame, startFrame);
}


EA3_Status channelSaveKeyFast(const ChannelsInfo &Info, int ChanIndex, float val, int frame)
{
	const ChanRec &CRec = Info.Chan[ChanIndex];

	bool discrete = CRec.Type == CSRT_F1Discrete;
	EA3Array *fArray = CRec.pData; // discrete ? channel->createArray(XBdata,"") : channel->createArray(XBkey,"");
	if(discrete)
	{
		EA3Field *f;
		while(fArray->getNumMembers() <= frame-Info.StartFrame)
		{
			f = fArray->createField(XBdata, "");
			f->data[0].f = val;
		}
		f = fArray->getByIndex(frame - Info.StartFrame);
		f->data[0].f = val;
	}
	else
	{
		EA3Field *k1=NULL,*k2=NULL;

		float time = float(frame) / Info.fps;
		int which = -1;

		fcrvFindPreviousKey(CRec.pChan, time, &which,k1);

		if(k1)
		{
			if(fabs(k1->data[0].f - time) < 0.01)
				k2 = k1;
			else
				k2 = fArray->getByIndex(which+1);

			if(k2)
			{
				if(fabs(k2->data[0].f - time) < 0.01)
				{
				}
				else
					k2 = NULL;
			}

			if(!k2)
			{
				k2 = fArray->createField("linearKey");
				k2->data[0].f = time;
				fcrvSortKeys(CRec.pChan, fArray);
			}
			k2->data[1].f = val;
		}
		else
		{
			k2 = fArray->createField("linearKey");
			k2->data[0].f = time;
			k2->data[1].f = val;
		}
	}
	return EA3_OK;
}

EA3_Status channelListSaveKeyFast(const ChannelsInfo &Info, const XbVec3f &val, int frame)
{
	int c=0;
	do {
		const ChanRec &CRec = Info.Chan[c];
		switch(CRec.Type)
		{
		case CSRT_None:
			break;

		case CSRT_F3:
			return channelSaveKeyFast(CRec, val, frame, Info.StartFrame);

		case CSRT_F1Discrete:
		case CSRT_F1Continuous:
			{
				int Index = CRec.Index;
				EA3_Status st = channelSaveKeyFast(Info, c, val[Index], frame);
				if(st != EA3_OK) return st;
			}
			break;
		}
	} while(++c < Info.Count);
	return EA3_OK;
}


EA3_Status channelListSaveKeyFast(const ChannelsInfo &Info, const XbEuler &eul, int frame)
{
	XbEuler oldeul(XbVec3f(0,0,0), XbEuler::XYZ,XbEuler::STATIC);

	channelListGetRFast(Info, frame-1, oldeul, Info.StartFrame, Info.fps);	// Why grab them all when you only need one?

	XbVec3f v = eul.closest(oldeul.getAngles()).getAngles()*R2D;
	return channelListSaveKeyFast(Info, v, frame);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------





EA3_Status channelGetNumFrames(const EA3Block *channel,int &num) {
  num = 0;
  EA3Array *data = (EA3Array *)channel->getByType(XBdata);
  if(data) {
    num = data->getNumMembers();
    return(EA3_OK);
  }
  EA3Array *keys = (EA3Array *)channel->getByType(XBkey);
  if (keys && keys->getNumMembers()) {
    EA3Field *startFld = channel->getContainer()->getContainer()->getByType( XBstartFrame );
    int startFrame = 0;
    if( startFld ) startFrame = startFld->data[0].i;
    float FPS = 30.0f;
    animGetFPS(channel->getContainer()->getContainer(),FPS);
    EA3Field *last = keys->getByIndex(keys->getNumMembers()-1);
    num = (int)(last->data[0].f*FPS);
    if (float(num) < last->data[0].f) num++;
    num = (num - startFrame) + 1;
  }
  return(EA3_OK);
}



EA3_Status fcrvCreateConstantKey(
        EA3Block *fcrv, float time, float value, EA3Field *&key,bool sortkeys )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) keys = fcrv->createArray( XBkey );

    key = keys->createField( XBconstantKey, "" );
    key->data[0].f = time;
    key->data[1].f = value;

    if (sortkeys) fcrvSortKeys(fcrv, keys );
    return EA3_OK;
}



EA3_Status fcrvCreateLinearKey(
        EA3Block *fcrv, float time, float value, EA3Field *&key,bool sortkeys )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) keys = fcrv->createArray( XBkey );

    key = keys->createField( XBlinearKey, "" );
    key->data[0].f = time;
    key->data[1].f = value;

    if (sortkeys) fcrvSortKeys(fcrv, keys );
    return EA3_OK;
}


EA3_Status fcrvCreateHermite1DKey(
        EA3Block *fcrv,
        float time, float value,
        float leftSlope, float rightSlope,
        EA3Field *&key, bool sortkeys )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) keys = fcrv->createArray( XBkey );

    key = keys->createField( XBhermite1DKey, "" );
    key->data[0].f = time;
    key->data[1].f = value;
    key->data[2].f = leftSlope;
    key->data[3].f = rightSlope;

    if (sortkeys) fcrvSortKeys(fcrv,keys );
    return EA3_OK;
}



EA3_Status fcrvCreateSoftHermiteKey(EA3Block *fcrv,
        float time, float value,
        float leftSlope, float rightSlope,
  EA3Field *&key,
        bool sortkeys )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) keys = fcrv->createArray( XBkey );

    key = keys->createField( XBsoftHermiteKey, "" );
    key->data[0].f = time;
    key->data[1].f = value;
    key->data[2].f = leftSlope;
    key->data[3].f = rightSlope;

    if (sortkeys) fcrvSortKeys(fcrv,keys );
    return EA3_OK;
}



EA3_Status fcrvRemoveKey(EA3Block *fcrv, int which )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    if (keys->removeMember( which )) {
      return(EA3_OK);
    }
    return(EA3_NOT_FOUND);
}

EA3_Status fcrvRemoveKey(EA3Block *fcrv, EA3Field *k )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    if( keys->removeMember( (EA3Base *)k )) {
      return(EA3_OK);
    }
    return(EA3_NOT_FOUND);
}


EA3_Status fcrvRemoveAllKeys(EA3Block *fcrv)
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    if (keys->removeAllMembers()) {
      return(EA3_OK);
    }
    return(EA3_NOT_FOUND);
}



EA3_Status fcrvGetNumKeys(const EA3Block *fcrv, int &num)
{
    num = 0;
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;
    num = keys->getNumMembers();
    return(EA3_OK);
}


int fcrvKeyCompare(EA3Base **k1, EA3Base **k2 )
{
    EA3Field *f1 = (EA3Field*)*k1;
    EA3Field *f2 = (EA3Field*)*k2;
    float diff = f1->data[0].f - f2->data[0].f;
    return (diff < 0.0) ? -1 : ((diff > 0.0) ? 1 : 0);
}

EA3_Status fcrvTranslateTime(EA3Block *fcrv, float delta )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    for( int k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *key = keys->getByIndex( k );
        float t;
        keySetTime(key, ( keyGetTime(key, t),t) + delta );
    }
    return EA3_OK;
}


EA3_Status fcrvScaleTime(
        EA3Block *fcrv, float scale, float origin, bool doSlope )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    for( int k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *key = keys->getByIndex( k );

        float t;
        keySetTime(key, origin + ((keyGetTime(key,t),t) - origin) * scale );

        if( doSlope &&
            (key->getType() == XBhermite1DKey ||
             key->getType() == XBsoftHermiteKey) )
        {
            float ls,rs;
            keySetLSlope(key,(keyGetLSlope(key,ls),ls)/scale);
            keySetRSlope(key,(keyGetRSlope(key,rs),rs)/scale);
        }
    }
    return EA3_OK;
}


EA3_Status fcrvSortKeys(EA3Block *fcrv, EA3Array *keys )
{
    if( keys == NULL )
    {
        keys = fcrv->getByType( XBkey );
        if( ! keys ) return EA3_NOT_FOUND;
    }

    keys->sortMembers( fcrvKeyCompare );
    return EA3_OK;
}



EA3_Status fcrvGetKey(const EA3Block *fcrv, int which, EA3Field *&key)
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    if( which < 0 || which >= keys->getNumMembers() ) return EA3_NOT_FOUND;
    key = keys->getByIndex( which );
    return(EA3_OK);
}

EA3_Status fcrvFindClosestKey(const EA3Block *fcrv, float time, EA3Field *&find)
{
  find = NULL;
  EA3Array *keys = fcrv->getByType(XBkey);
  if(!keys) return EA3_NOT_FOUND;

  float closest = 1e10f;
  int numKeys = keys->getNumMembers();
  for(int k=0; k<numKeys; k++) {
    EA3Field *key = keys->getByIndex(k);
    float curTime = 0.0f;
    keyGetTime(key, curTime);
    float timeDiff = fabs(time - curTime);
    if (timeDiff < closest) {
      closest = timeDiff;
      find = key;
    }
  }

  if (find) return EA3_OK;
  else return EA3_NOT_FOUND;
}

EA3_Status fcrvFindClosestKey(const EA3Block *fcrv, float time, float y, EA3Field *&find)
{
  find = NULL;
  EA3Array *keys = fcrv->getByType( XBkey );
  if( ! keys ) return EA3_NOT_FOUND;
  
  float close = -1;
  float dist;
  int k;
  float minVal=0.0f;
  float maxVal=0.0f;
  float f;
  int num = keys->getNumMembers();
  for(k = 0; k < num; k++ )
  {
    EA3Field *key = keys->getByIndex( k );
    float value = (keyGetValue(key,f),f);
    if (k==0) minVal = maxVal = value;
    if (value<minVal) minVal = value;
    if (value>maxVal) maxVal = value;
  }
  float minTime = keys->getByIndex(0)->toField()->data[0].f;
  float maxTime = keys->getByIndex(num-1)->toField()->data[0].f;
  float timeRange = maxTime-minTime;
  if (timeRange == 0.0f) timeRange = 1.0f;
  float valRange = maxVal - minVal;
  if (valRange == 0.0f) valRange = 1.0f;
  for(k = 0; k < num; k++ )
  {
    EA3Field *key = keys->getByIndex( k );
    float timeDiff = (time - (keyGetTime(key,f),f))/timeRange;
    float valueDiff = (y - (keyGetValue(key,f),f))/valRange;
    dist = timeDiff * timeDiff + valueDiff * valueDiff;
    
    if( close == -1 || dist < close )
    {
      close = dist;
      find = key;
    }
  }
  
  if (find) return EA3_OK;
  else return EA3_NOT_FOUND;
}



EA3_Status fcrvFindClosestSlope(
        const EA3Block *fcrv,
        float time, float y,
        bool &rightSlope, EA3Field *&find )
{
    find = NULL;
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    float dist;
    float close = -1;
    float x0, y0;
    float ls, rs;

    for( int i = 0; i < keys->getNumMembers(); i++ )
    {
        EA3Field *key = keys->getByIndex( i );
        if( key->getType() == XBconstantKey || key->getType() == XBlinearKey )
        {
            continue;
        }

        ls = key->data[2].f;
        rs = key->data[3].f;


        x0 = key->data[0].f;
        y0 = key->data[1].f - ls;
        dist = (time - x0) * (time - x0) + (y - y0) * (y - y0);
        if (close == -1 || dist < close)
        {
            close = dist;
            find = key;
            rightSlope = false;
        }

        x0 = key->data[0].f;
        y0 = key->data[1].f + rs;
        dist = (time - x0) * (time - x0) + (y - y0) * (y - y0);
        if (close == -1 || dist < close)
        {
            close = dist;
            find = key;
            rightSlope = true;
        }
    }

    if (find) return EA3_OK; 
    return EA3_NOT_FOUND;
}


EA3_Status fcrvAutoSlopes(EA3Block *fcrv, float tension )
{
    if( tension <= 0.0 ) return EA3_OK;


    int num = 0;
    fcrvGetNumKeys(fcrv,num);
    for( int k = 0; k < num; k++ )
    {
        EA3Field *thisKey = NULL;
        fcrvGetKey(fcrv, k, thisKey );

        if( thisKey->getType() == XBconstantKey ||
            thisKey->getType() == XBlinearKey )
        {
            continue;
        }

        EA3Field *prevKey = NULL;
        EA3Field *nextKey = NULL;

        if( k > 0 ) fcrvGetKey(fcrv, k - 1 , prevKey);
        int num2;
        fcrvGetNumKeys(fcrv,num2);
        if( k <  num2 - 1 ) fcrvGetKey(fcrv, k + 1 , nextKey);

        float dx, dy;
        if (prevKey == NULL || nextKey == NULL)
        {
            dx = 1.0f / tension;
            dy = 0.0f;
        }
        else
        {
            float f1,f2;
            dx = ( (keyGetTime(nextKey,f1),f1) - (keyGetTime(prevKey,f2),f2 )) / tension;
            dy = ( (keyGetTime(nextKey,f1),f1) - (keyGetTime(prevKey,f2),f2 )) / tension;
        }

        keySetLSlope(thisKey, dx );
        keySetRSlope(thisKey, dy );
    }
    return EA3_OK;
}



EA3_Status fcrvFindPreviousKey(const EA3Block *fcrv, float time, int *which , EA3Field *&find)
{
    find = NULL;
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    int k;
    for( k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *key = keys->getByIndex( k );

        if( key->data[0].f > time )
        {
            if( k == 0 )
            {
                if( which ) *which = -1;
                return EA3_NOT_FOUND;
            }
            else
            {
                if( which ) *which = k - 1;
                find = keys->getByIndex( k - 1 );
                return EA3_OK;
            }
        }
    }

    if( keys->getNumMembers() == 0 )
    {
        if( which ) *which = -1;
        return EA3_NOT_FOUND;
    }
    else
    {
        if( which ) *which = k - 1;
        find = keys->getByIndex( k - 1 );
        return EA3_OK;
    }
}



EA3_Status fcrvFindNextKey(const EA3Block *fcrv, float time, int *which, EA3Field *&find)
{
    find = NULL;
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    for( int k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *key = keys->getByIndex( k );

        if( key->data[0].f >= time )
        {
            if( which ) *which = k;
            find = key;
            return EA3_OK;
        }
    }

    if( which ) *which = -1;
    return EA3_NOT_FOUND;
}


EA3_Status fcrvCycleCurveType(EA3Block *fcrv, float time )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    int ki = keys->getNumMembers() - 1;
        int k;
    for( k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *field = keys->getByIndex( k );
        if( field->data[0].f > time )
        {
            ki = k - 1;
            break;
        }
    }

    if (ki < 0) ki = 0;
    EA3Field *find = keys->getByIndex( ki );
    EA3Field *newKey = NULL;
    float f;

    if( find->getType() == XBconstantKey )
    {
        newKey = keys->createField( XBlinearKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);
    }
    else if( find->getType() == XBlinearKey )
    {
        newKey = keys->createField( XBhermite1DKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);

        if( k < keys->getNumMembers() )
        {
            EA3Field *next = keys->getByIndex( k );
            float diffX = (keyGetTime(next,f),f) - (keyGetTime(find,f),f);
            float diffY = (keyGetValue(next,f),f) - (keyGetValue(find,f),f);
            float ang = atan2f( diffY, diffX );
            newKey->data[2].f = newKey->data[3].f = sinf( ang );
        }
        else if( (ki - 1) >= 0 ) 
        {
            EA3Field *last = keys->getByIndex( ki-1 );
            float diffX = (keyGetTime(find,f),f) - (keyGetTime(last,f),f);
            float diffY = (keyGetValue(find,f),f) - (keyGetValue(last,f),f);
            float ang = atan2f( diffY, diffX );
            newKey->data[2].f = newKey->data[3].f = sinf( ang );
        }
        else
        {
            newKey->data[2].f = newKey->data[3].f = 0.0;
        }
    }
    else if( find->getType() == XBhermite1DKey )
    {
        newKey = keys->createField( XBsoftHermiteKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);
        newKey->data[2].f = (keyGetLSlope(find,f),f);
        newKey->data[3].f = (keyGetRSlope(find,f),f);
    }
    else if( find->getType() == XBsoftHermiteKey )
    {
        newKey = keys->createField( XBconstantKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);
    }

    keys->insertMember( ki, newKey );
    keys->removeMember( find );
    return EA3_OK;
}


EA3_Status fcrvChangeKeyType(
        EA3Block *fcrv, int index, XbIdentifier newType , EA3Field *&find)
{
    float f;
    find = NULL;
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    find = keys->getByIndex( index );
    if( find->getType() == newType ) return EA3_OK;

    EA3Field *newKey = NULL;

    if( newType == XBconstantKey )
    {
        newKey = keys->createField( XBconstantKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);
    }
    else if( newType == XBlinearKey )
    {
        newKey = keys->createField( XBlinearKey, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);
    }
    else if( newType == XBhermite1DKey ||
             newType == XBsoftHermiteKey )
    {
        newKey = keys->createField( newType, "" );
        newKey->data[0].f = (keyGetTime(find,f),f);
        newKey->data[1].f = (keyGetValue(find,f),f);

        if( find->getType() == XBhermite1DKey )
        {
            newKey->data[2].f = (keyGetLSlope(find,f),f);
            newKey->data[3].f = (keyGetRSlope(find,f),f);
        }
        else if( find->getType() == XBsoftHermiteKey )
        {
            newKey->data[2].f = (keyGetLSlope(find,f),f);
            newKey->data[3].f = (keyGetRSlope(find,f),f);
        }
        else if( find->getType() == XBlinearKey )
        {
            if( (index + 1) < keys->getNumMembers() )
            {
                EA3Field *next = keys->getByIndex( index + 1 );
                float diffX = (keyGetTime(next,f),f) - (keyGetTime(find,f),f);
                float diffY = (keyGetValue(next,f),f) - (keyGetValue(find,f),f);
                float ang = atan2f( diffY, diffX );
                newKey->data[2].f = newKey->data[3].f = sinf( ang );
            }
            else if( (index - 1) >= 0 ) 
            {
                EA3Field *last = keys->getByIndex( index - 1 );
                float diffX = (keyGetTime(find,f),f) - (keyGetTime(last,f),f);
                float diffY = (keyGetValue(find,f),f) - (keyGetValue(last,f),f);
                float ang = atan2f( diffY, diffX );
                newKey->data[2].f = newKey->data[3].f = sinf( ang );
            }
            else
            {
                newKey->data[2].f = newKey->data[3].f = 0.0;
            }
        }
        else
        {
            newKey->data[2].f = newKey->data[3].f = 0.0;
        }
    }

    keys->insertMember( index, newKey );
    keys->removeMember( find );
    find = newKey;
    return EA3_OK;
}



EA3_Status fcrvChangeKeyType(
        EA3Block *fcrv, float time, XbIdentifier newType, EA3Field *&find )
{
    int which = -1;
    EA3Field *key;
    fcrvFindPreviousKey( fcrv, time, &which , key);
    if( key ) {
      fcrvChangeKeyType( fcrv, which, newType, find );
      return EA3_OK;
    } 
    find = NULL;
    return EA3_NOT_FOUND;
}



EA3_Status fcrvChangeKeyType(
        EA3Block *fcrv, EA3Field &key, XbIdentifier newType, EA3Field *&find )
{
    int which = -1;
    fcrvGetKeyIndex( fcrv, key ,which );
    find = NULL;
    if( which != -1 ) {
         fcrvChangeKeyType( fcrv, which, newType, find );
         return EA3_OK;
    }
    return EA3_NOT_FOUND;
}



EA3_Status fcrvChangeAllKeysType( EA3Block *fcrv, XbIdentifier newType )
{
    int num;
    fcrvGetNumKeys(fcrv,num);
    for( int k = 0; k < num; k++ )
    {
        EA3Field *f;
        fcrvChangeKeyType( fcrv, k, newType,f );
    }
    return EA3_OK;
}



EA3_Status fcrvEval(const EA3Block *fcrv, float time, float &res )
{
    EA3Array *keys = fcrv->getByType( XBkey );
    if( ! keys || keys->getNumMembers() == 0 ) {
        res = 0.0;
        return EA3_NOT_FOUND;
    }
    if( keys->getNumMembers() == 1 )
    {
        EA3Field *field = keys->getByIndex( 0 );
        res = field->data[1].f;
        return EA3_OK;
    }
 
    int nKeys = keys->getNumMembers();
    EA3Field *firstKey = keys->getByIndex( 0 );
    EA3Field *lastKey = keys->getByIndex( nKeys - 1 );
    float f;
    float firstTime = (keyGetTime(firstKey,f),f);
    float lastTime = (keyGetTime(lastKey,f),f);
//    float firstValue = (keyGetValue(firstKey,f),f);
//    float lastValue = (keyGetValue(lastKey,f),f);
		float offset = 0.0f;

    const EA3Block* eBlock = fcrv;
		EA3Array* eArray = NULL;
		
		if ((eBlock == NULL) || ((eArray = eBlock->getByType( XBkey )->toArray())->getNumMembers()== 0)) {
			return (EA3_NOT_FOUND);
		}; /* if */

    float fr = 30.0f;
    EA3Field *fps = eBlock->getContainer()->getContainer()->toBlock()->getByType( XBFPS );
    if (fps) fr = fps->data[0].f;
    XbIdentifier pre = XBCONSTANT;
    XbIdentifier post = XBCONSTANT;
    if (time < eArray->getByIndex(0)->toField()->data[0].f) {
      EA3Field *preF = eBlock->getByType(XBpreInfinity);
      if (preF) pre = preF->data[0].str;
	    if (getInfinityType(pre) == kInfinityConstant) {
		    return (res=eArray->getByIndex(0)->toField()->data[1].f), EA3_OK;
	    }; /* if */
	    return (res=evaluateInfinities( eBlock, time, fr, true )), EA3_OK;
    }; /* if  */

    if (time > eArray->getByIndex( eArray->getNumMembers()-1 )->toField()->data[0].f) {
      EA3Field *postF = eBlock->getByType(XBpreInfinity);
      if (postF) post = postF->data[0].str;
	    if (getInfinityType(post) == kInfinityConstant) {
		    return (res=(eArray->getByIndex(eArray->getNumMembers()-1)->toField()->data[1].f)), EA3_OK;
	    }; /* if */
	    return (res=evaluateInfinities( eBlock, time, fr, false )), EA3_OK;
    }; /* if  */




    //
    // Assuming all the keys are evenly spaced, <guess> is the
    // estimate of which key <time> is at.
    //

    int guess = int( float(nKeys-1) *
                         ( time - firstTime ) / (lastTime - firstTime) );
    EA3Field *guessKey = keys->getByIndex( guess );

    //
    //  Find the keys, <startKeyIndex> and <endKeyIndex> that will be used to
    //  evaluate the fcurve value at <time>.
    //
    int startKeyIndex=0, endKeyIndex=0;

    while (time < guessKey->data[0].f)
    {
        guessKey = keys->getByIndex( --guess );
    }

    for( int k = guess; k < nKeys; k++ )
    {
        EA3Field *key = keys->getByIndex( k );

        if( fabs( time - (keyGetTime(key,f),f) ) < 0.005 )
        {
            res = offset + (keyGetValue(key,f),f);
            return EA3_OK;
        }
        else if( time < (keyGetTime(key,f),f) )
        {
            startKeyIndex = k-1;
            endKeyIndex = k;
            break;
        }
    }


    //
    //  <time> is between <startKey> and <endKey>. Evaluate for the value.
    //

    EA3Field *startKey = keys->getByIndex( startKeyIndex );
    EA3Field *endKey = keys->getByIndex( endKeyIndex );
    float startTime = startKey->data[0].f;
    float startValue = startKey->data[1].f;
    float endTime = endKey->data[0].f;
    float endValue = endKey->data[1].f;

    if( startKey->getType() == XBconstantKey )
    {
        res = offset + startValue;
        return EA3_OK;
    }
    else if( startKey->getType() == XBlinearKey )
    {
        float t = (time - startTime) / (endTime - startTime);
        res = offset + startValue * (1.0f - t) + endValue * t;
        return EA3_OK;
    }
    else if( startKey->getType() == XBhermite1DKey )
    {
        float res2;
        res = offset + (fcrvEvalHermite1D(
                            *startKey,
                            *endKey,
                            time,res2 ), res2);
        return EA3_OK;
    }
    else if( startKey->getType() == XBsoftHermiteKey )
    {
        float res2;
        res = offset + (fcrvEvalSoftHermite(
                            *startKey,
                            *endKey,
                            time,res2 ) ,res2);
        return EA3_OK;
    }
		else if ((startKey->getType() == XBmayaNW) || (startKey->getType() == XBmayaW)) {
			int index = endKeyIndex;
			float inTanX, inTanY, outTanX, outTanY, crap1, crap2;
			float x[4];
			float y[4];

      bool isMayaW = eArray->getByIndex(0)->getType()==XBmayaW?true:false;
      getTangent( eArray, (index-1), fr, isMayaW, crap1, crap2, outTanX, outTanY );
      getTangent( eArray, (index), fr, isMayaW, inTanX, inTanY, crap1, crap2 );

			if ((outTanX == 0.0) &&	(outTanY == 0.0)) {
				res = eArray->getByIndex( index-1 )->toField()->data[1].f;
			}	else {

				x[0] = eArray->getByIndex( index-1 )->toField()->data[0].f;
				y[0] = eArray->getByIndex( index-1 )->toField()->data[1].f;
				x[1] = x[0] + (outTanX * kOneThird);
				y[1] = y[0] + (outTanY * kOneThird);

				x[3] = eArray->getByIndex( index )->toField()->data[0].f;
				y[3] = eArray->getByIndex( index )->toField()->data[1].f;
				x[2] = x[3] - (inTanX * kOneThird);
				y[2] = y[3] - (inTanY * kOneThird);

				if (eArray->getByIndex(endKeyIndex)->getType() == XBmayaW) {
					res = engineBezierEvaluate( eBlock, time, x, y );
				}	else {
					res = engineHermiteEvaluate( eBlock, time, x, y );
				}; /* if */
			}
      return EA3_OK;
    } else if (startKey->getType() == XBsoft2DKey) {
        /*
      int index = endKeyIndex;
			float x[4];
			float y[4];
      x[0] = eArray->getByIndex( index-1 )->toField()->data[0].f;
      y[0] = eArray->getByIndex( index-1 )->toField()->data[1].f;
      x[1] = x[0] + eArray->getByIndex( index-1 )->toField()->data[4].f;
      y[1] = y[0] + eArray->getByIndex( index-1 )->toField()->data[5].f;

      x[3] = eArray->getByIndex( index )->toField()->data[0].f;
      y[3] = eArray->getByIndex( index )->toField()->data[1].f;
      x[2] = x[3] + eArray->getByIndex( index )->toField()->data[2].f;
      y[2] = y[3] + eArray->getByIndex( index )->toField()->data[3].f;
      res = engineBezierEvaluate( eBlock, time, x, y ); 
      */
        res = 0.0f;
      return EA3_OK;
    }
    res = 0.0;
    return EA3_OK;
}



EA3_Status fcrvEvalHermite1D(
        const EA3Field &startKey, EA3Field &endKey,
        float time, float &res )
{
  float f;
  float p1x = (keyGetTime(&startKey,f),f);
  float p1y = (keyGetValue(&startKey,f),f);
  float r1y = (keyGetRSlope(&startKey,f),f);
  float p2x = (keyGetTime(&endKey,f),f);
  float p2y = (keyGetValue(&endKey,f),f);
  float r2y;
  if( endKey.getType() == XBhermite1DKey ||
      endKey.getType() == XBsoftHermiteKey )
  {
    r2y = (keyGetLSlope(&endKey,f),f);
  }
  else if( endKey.getType() == XBlinearKey )
  {
    int idx = endKey.getIndex();
    EA3Array *keyArray = (EA3Array *)endKey.getContainer();

    if( (idx + 1) < keyArray->getNumMembers() )
    {
      EA3Field *next = keyArray->getByIndex( idx + 1 );
      float diffX = (keyGetTime(next,f),f) - (keyGetTime(&endKey,f),f);
      float diffY = (keyGetValue(next,f),f) - (keyGetValue(&endKey,f),f);
      float ang = atan2f( diffY, diffX );
      r2y = sinf( ang );
    }
    else if( (idx - 1) >= 0 )
    {
      EA3Field *last = keyArray->getByIndex( idx - 1 );
      float diffX = (keyGetTime(&endKey,f),f) - (keyGetTime(last,f),f);
      float diffY = (keyGetValue(&endKey,f),f) - (keyGetValue(last,f),f);
      float ang = atan2f( diffY, diffX );
      r2y = sinf( ang );
    }
    else
    {
      r2y = 0.0;
    }
  }
  else
  {
    r2y = 0.0;
  }

  float t = ( time - p1x ) / ( p2x - p1x );
  float T2 = t*t;
  float T3 = t*t*t;
  float _2T3 = 2*T3;
  float _3T2 = 3*T2;
  float y = (_2T3-_3T2+1)*p1y+ (-_2T3+_3T2)*p2y +
            (T3-2*T2+t)*r1y + (T3-T2)*r2y;
  res = y;
  return EA3_OK;
}


EA3_Status fcrvEvalSoftHermite(
        const EA3Field &startKey, EA3Field &endKey, float time, float &res )
{
  float f;
  float t = time - (keyGetTime(&startKey,f),f);
  float f1,f2;
  float _1_T = 1.0 / ( (keyGetTime(&endKey,f1),f1) - (keyGetTime(&startKey,f2),f2) );
  float _1_T2 = _1_T * _1_T;
  float t2 = t * t;
  float t2_T = t2 * _1_T;
  float t3_T2 = t2 * t * _1_T2;
  float _2t3_T3 = 2.0 * t3_T2 * _1_T;
  float _3t2_T2 = 3.0 * t2 * _1_T2;
  float r2y;

  if( endKey.getType() == XBhermite1DKey ||
      endKey.getType() == XBsoftHermiteKey )
  {
    r2y = (keyGetLSlope(&endKey,f),f);
  }
  else if( endKey.getType() == XBlinearKey )
  {
    int idx = endKey.getIndex();
    EA3Array *keyArray = (EA3Array *)endKey.getContainer();

    if( (idx + 1) < keyArray->getNumMembers() )
    {
      EA3Field *next = keyArray->getByIndex( idx + 1 );
      float diffX = (keyGetTime(next,f),f) - (keyGetTime(&endKey,f),f);
      float diffY = (keyGetValue(next,f),f) - (keyGetValue(&endKey,f),f);
      float ang = atan2f( diffY, diffX );
      r2y = sinf( ang );
    }
    else if( (idx - 1) >= 0 )
    {
      EA3Field *last = keyArray->getByIndex( idx - 1 );
      float diffX = (keyGetTime(&endKey,f),f) - (keyGetTime(last,f),f);
      float diffY = (keyGetValue(&endKey,f),f) - (keyGetValue(last,f),f);
      float ang = atan2f( diffY, diffX );
      r2y = sinf( ang );
    }
    else
    {
      r2y = 0.0;
    }
  }
  else
  {
    r2y = 0.0;
  }

  float y =  (keyGetValue(&startKey,f),f)  * ( _2t3_T3 - _3t2_T2 + 1 )  +
             (keyGetValue(&endKey,f),f)    * ( - _2t3_T3 + _3t2_T2 ) +
             (keyGetRSlope(&startKey,f),f) * ( t3_T2 - t2_T - t2_T + t ) +
             r2y                  * ( t3_T2 - t2_T ) ;
  res = y;
  return EA3_OK;
}



EA3_Status fcrvFirstKey( const EA3Block *fcrv, EA3Field *&key )
{
    key = NULL;
    EA3Array *keys = (EA3Array *)fcrv->getByType( XBkey );
    if( ! keys || keys->getNumMembers() <= 0 ) return EA3_NOT_FOUND;

    key = (EA3Field *)keys->getByIndex( 0 );
    return EA3_OK;
}



EA3_Status fcrvLastKey( const EA3Block *fcrv, EA3Field *&key )
{
    key = NULL;
    EA3Array *keys = (EA3Array *)fcrv->getByType( XBkey );
    if( ! keys || keys->getNumMembers() <= 0 ) return EA3_NOT_FOUND;

    key = (EA3Field *)keys->getByIndex( keys->getNumMembers() - 1 );
    return EA3_OK;
}



EA3_Status fcrvGetKeyIndex( const EA3Block *fcrv, EA3Field &keySeek, int &k )
{
    k = -1;
    EA3Array *keys = (EA3Array *)fcrv->getByType( XBkey );
    if( ! keys ) return EA3_NOT_FOUND;

    for( k = 0; k < keys->getNumMembers(); k++ )
    {
        EA3Field *key = (EA3Field *)keys->getByIndex( k );

        if( key == &keySeek ) return EA3_OK;
    }
    k = -1;
    return EA3_NOT_FOUND;
}



EA3_Status keySetTime( EA3Field *key, float t ) { 
  key->data[0].f = t; 
  return EA3_OK;
}


EA3_Status keySetValue( EA3Field *key, float v )  { 
  key->data[1].f = v; 
  return EA3_OK;
}



EA3_Status keyGetTime( const EA3Field *key, float &t )   { 
  t = key->data[0].f; 
  return EA3_OK;
}



EA3_Status keyGetValue( const EA3Field *key, float &v )  { 
  v =  key->data[1].f; 
  return EA3_OK;
}



EA3_Status keySetLSlope( EA3Field *key, float slope ) { 
  key->data[2].f = slope; 
  return EA3_OK;
}


EA3_Status keySetRSlope( EA3Field *key, float slope ) { 
  key->data[3].f = slope; 
  return EA3_OK;
}



EA3_Status keySetSlope( EA3Field *key, float left, float right )
{
  key->data[2].f = left;
  key->data[3].f = right;
  return EA3_OK;
}



EA3_Status keyGetLSlope( const EA3Field *key, float &ls ) {
  if (key->getNum() > 2) {
    ls = key->data[2].f; 
  } else {
    ls = 0.0f;
  }
  return EA3_OK;
}



EA3_Status keyGetRSlope( const EA3Field *key , float &rs) { 
  if (key->getNum() > 3) {
    rs = key->data[3].f; 
  } else {
    rs = 0.0f;
  }
  return EA3_OK;
}


EA3_Status keyGetSlope( const EA3Field *key, float &left, float &right )
{
  if (key->getNum() > 3) {
    left = key->data[2].f;
    right = key->data[3].f;
  } else {
    left = 0.0f;
    right = 0.0f;
  }
  return EA3_OK;
}

//@}
} //namespace

