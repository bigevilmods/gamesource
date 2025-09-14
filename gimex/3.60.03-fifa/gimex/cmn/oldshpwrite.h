// (c) 2002-2007 Electronic Arts Inc.

int OLDSHPWAddShape(OLDShapeListPtr theShape,const GINFO * ginfo, const char *source, int rowbytes, int framenum, int type, int subtype);
int OLDSHPWWriteShapeFile(GSTREAM *g,OLDShapeListPtr theShape,int type, int subtype, int multi,int signature, int oldshpdataalignment,int oldshpclutdataalignment,int oldshpalignment,int oldshpmipmapdataalignment,int oldshppackeddataalignment);
void OLDSHPWFreeShapes(OLDShapeListPtr theShape);

