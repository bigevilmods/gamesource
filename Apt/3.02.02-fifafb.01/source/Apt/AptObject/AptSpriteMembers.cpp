#include "_Apt.h"
#include "AptSpriteMembers.h"
#include "MainInline.h"

// Please see the Apt wiki for further detail about Open Source license usage:
// http://docs.ea.com/Apt:User_Guide/Overview#Open_Source_License_Information

/* C++ code produced by gperf version 2.7.2 */
/* Command-line: 'E:\\dev\\wireframe\\TUX\\wf\\ui\\private-apt\\Apt\\dev\\bin\\util\\gperf-2.7.2-bin\\bin\\gperf.exe' -t -K szName -E -D -L C++ -Z SpriteMembersIndex -k 2,6,8 'E:\\dev\\wireframe\\TUX\\wf\\ui\\private-apt\\Apt\\dev\\bin\\util\\..\\..\\source\\Apt\\sprite.gperf'  */

/* maximum key range = 249, duplicates = 1 */

  static unsigned char SpriteMembersIndex_asso_values[] =
    {
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251,  10,  30, 251,
        0,   0, 251,   0, 251, 251,  10,  20, 251, 251,
       20, 251, 251,   5,   0,  75, 251,   0, 251, 251,
      251, 251, 251, 251, 251, 251, 251,  95,   5,  30,
        0,   0,  40,  20,  70,   5, 251, 251,  35,   0,
      124,  80,  60,  20,  65,  15,  55,   0,   0,  10,
       30,  15,   0, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251, 251, 251, 251, 251,
      251, 251, 251, 251, 251, 251
    };

inline unsigned int
SpriteMembersIndex::hash (const char *str, unsigned int len)
{
  int hval = len;

  switch (hval)
    {
      default:
      case 8:
        hval += SpriteMembersIndex_asso_values[(unsigned char)str[7]];
      case 7:
      case 6:
        hval += SpriteMembersIndex_asso_values[(unsigned char)str[5]];
      case 5:
      case 4:
      case 3:
      case 2:
        hval += SpriteMembersIndex_asso_values[(unsigned char)str[1]];
        break;
    }
  return hval;
}

struct SpriteMembers *
SpriteMembersIndex::in_word_set (const char *str, unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 90,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 20,
      MIN_HASH_VALUE = 2,
      MAX_HASH_VALUE = 250
    };

  static struct SpriteMembers wordlist[] =
    {
      {"_z",128},
      {"_url", 16},
      {"getBounds",117},
      {"_visible", 8},
      {"removeTextField",119},
      {"getURL",102},
      {"_y", 2},
      {"isNaN",26},
      {"escape",28},
      {"setMask",122},
      {"getNewTextFormat",123},
      {"hitTest",118},
      {"_soundbuftime", 19},
      {"_x", 1},
      {"_ymouse", 22},
      {"play",107},
      {"_zscale",135},
      {"getTextFormat",124},
      {"setTextFormat",125},
      {"_quality", 20},
      {"_xmouse", 21},
      {"_yscale", 4},
      {"stop",110},
      {"_target", 12},
      {"blendMode",30},
      {"_focusrect", 18},
      {"_xscale", 3},
      {"nextFrame",113},
      {"swapDepths",120},
      {"filters",31},
      {"createTextField",114},
      {"getBytesTotal",112},
      {"getBytesLoaded",111},
      {"createEmptyMovieClip",116},
      {"_width", 9},
      {"_framesloaded", 13},
      {"localToGlobal",127},
      {"removeMovieClip",109},
      {"_currentframe", 5},
      {"duplicateMovieClip",101},
      {"_renderflags",131},
      {"_CustomControlType",36},
      {"_skipeval",35},
      {"_yrotation",130},
      {"getInstanceAtDepth",136},
      {"_name", 14},
      {"onLoad",207},
      {"_droptarget", 15},
      {"onEnterFrame",203},
      {"getDepth",115},
      {"prevFrame",108},
      {"_xrotation",129},
      {"onPress",211},
      {"_height", 10},
      {"onRelease",212},
      {"onMouseWheel",218},
      {"onDragOut",201},
      {"onDragOver",202},
      {"onReleaseOutside",213},
      {"parseInt",32},
      {"extern", 23},
      {"globalToLocal",132},
      {"onSetFocus",216},
      {"_totalframes", 6},
      {"onRollOut",214},
      {"onRollOver",215},
      {"onMouseMove",209},
      {"onKeyDown",204},
      {"loadMovie",105},
      {"loadMovieNum",133},
      {"onMouseDown",208},
      {"Boolean",29},
      {"parseFloat",33},
      {"startDrag",126},
      {"_parent",34},
      {"loadVariables",106},
      {"_alpha", 7},
      {"_highquality", 17},
      {"onKeyUp",205},
      {"_rotation", 11},
      {"onUnload",217},
      {"unloadMovie",121},
      {"attachMovie",100},
      {"unloadMovieNum",134},
      {"gotoAndStop",104},
      {"onMouseUp",210},
      {"onData",200},
      {"unescape",27},
      {"gotoAndPlay",103},
      {"onKillFocus",206}
    };

  static short lookup[] =
    {
        -1,   -1,    0,   -1,    1,   -1,   -1,   -1,
        -1,    2,   -1,   -1,   -1,    3,   -1,    4,
         5,    6,   -1,   -1,    7,    8,    9,   -1,
        -1,   -1,   10,   11,   12,   -1,   -1,   -1,
        13,   -1,   -1,   -1,   -1,   14,   -1,   15,
        -1,   -1,   16, -135,  -73,   -2,   -1,   -1,
        19,   -1,   -1,   -1,   20,   -1,   -1,   -1,
        -1,   21,   -1,   22,   -1,   -1,   23,   -1,
        24,   25,   -1,   -1,   -1,   -1,   -1,   -1,
        26,   -1,   27,   28,   -1,   29,   -1,   -1,
        30,   -1,   -1,   31,   32,   33,   34,   -1,
        35,   -1,   -1,   -1,   -1,   36,   -1,   37,
        -1,   -1,   38,   -1,   -1,   -1,   -1,   39,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   40,   41,   42,
        -1,   -1,   -1,   -1,   -1,   43,   -1,   -1,
        44,   45,   46,   47,   -1,   -1,   -1,   -1,
        48,   -1,   49,   50,   51,   -1,   -1,   -1,
        -1,   -1,   52,   53,   54,   -1,   -1,   55,
        -1,   56,   57,   58,   -1,   -1,   59,   -1,
        60,   -1,   -1,   61,   62,   -1,   -1,   63,
        64,   65,   66,   -1,   -1,   67,   68,   -1,
        -1,   69,   -1,   -1,   70,   -1,   71,   -1,
        -1,   72,   -1,   -1,   -1,   73,   -1,   74,
        -1,   75,   -1,   -1,   76,   77,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   78,   -1,
        -1,   79,   -1,   -1,   80,   -1,   -1,   81,
        82,   -1,   83,   -1,   84,   -1,   -1,   85,
        -1,   86,   -1,   87,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   88,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   89
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          int index = lookup[key];

          if (index >= 0)
            {
              const char *s = wordlist[index].szName;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
          else if (index < -TOTAL_KEYWORDS)
            {
              int offset = - 1 - TOTAL_KEYWORDS - index;
              struct SpriteMembers *wordptr = &wordlist[TOTAL_KEYWORDS + lookup[offset]];
              struct SpriteMembers *wordendptr = wordptr + -lookup[offset + 1];

              while (wordptr < wordendptr)
                {
                  const char *s = wordptr->szName;

                  if (*str == *s && !strcmp (str + 1, s + 1))
                    return wordptr;
                  wordptr++;
                }
            }
        }
    }
  return 0;
}
