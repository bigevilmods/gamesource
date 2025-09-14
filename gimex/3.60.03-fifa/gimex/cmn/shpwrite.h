// (c) 2002-2007 Electronic Arts Inc.

int SHPWAddShape(ShapeListPtr theShape,const GINFO *ginfo, const char *source, int rowbytes, int framenum, int type, int subtype);
int SHPWWriteShapeFile(GSTREAM *g,ShapeListPtr theShape,int type, int subtype, int multi,int signature, int shpdataalignment,int shpclutdataalignment,int shpalignment,int shpmipmapdataalignment,int shppackeddataalignment);
void SHPWFreeShapes(ShapeListPtr theShape);

