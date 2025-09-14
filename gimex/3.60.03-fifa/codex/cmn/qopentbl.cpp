// (c) 1995-2007 Electronic Arts Inc.

#include "codex.h"
#include "qopen.h"

#include "nilcodex.h"
#include "refcodex.h"
#include "huffcodex.h"
#include "btreecodex.h"
#include "zipcodex.h"
#include "dxtcodex.h"
#include "jlzcodex.h"
//#include "bzip2codex.h"

struct QFUNCTIONS qfunctions[]=
{
    { CNIL_about,  CNIL_is,  CNIL_size,  CNIL_decode,  CNIL_encode  },
    { REF_about,   REF_is,   REF_size,   REF_decode,   REF_encode   },
    { ZIP_about,   ZIP_is,   ZIP_size,   ZIP_decode,   ZIP_encode   },
    { HUFF_about,  HUFF_is,  HUFF_size,  HUFF_decode,  HUFF_encode  },
    { BTREE_about, BTREE_is, BTREE_size, BTREE_decode, BTREE_encode },
    { DXT_about,   DXT_is,   DXT_size,   DXT_decode,   DXT_encode   },
    { JLZ_about,   JLZ_is,   JLZ_size,   JLZ_decode,   JLZ_encode   },
//    { BZIP2_about, BZIP2_is, BZIP2_size, BZIP2_decode, BZIP2_encode },
    { 0,           0,        0,          0,            0,           },
};
