#include "_Apt.h"
#include "AptStageMembers.h"
#include "MainInline.h"

static unsigned char StageMembersIndex_asso_values[] =
{
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15,  0, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15,  0,
        0,  0, 15,  0, 15,  0, 15, 15,  0, 15,
        15,  0, 15, 15, 15, 15,  5, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15
};

inline unsigned int
StageMembersIndex::hash (const char *str, unsigned int len)
{
  int hval = len;

  switch (hval)
    {
      default:
      case 8:
        hval += StageMembersIndex_asso_values[(unsigned char)str[7]];
        //lint -fallthrough
      case 7:
      case 6:
      case 5:
      case 4:
        hval += StageMembersIndex_asso_values[(unsigned char)str[3]];
        //lint -fallthrough
      case 3:
      case 2:
        hval += StageMembersIndex_asso_values[(unsigned char)str[1]];
        //lint -fallthrough
        break;
    }
  return hval;
}

struct StageMembers *
StageMembersIndex::in_word_set (const char *str, unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 6,
      MIN_WORD_LENGTH = 5,
      MAX_WORD_LENGTH = 14,
      MIN_HASH_VALUE = 5,
      MAX_HASH_VALUE = 14
    };

  static unsigned char lengthtable[] =
    {
       5,  6,  9,  5, 11, 14
    };
  static struct StageMembers wordlist[] =
    {
      {"align",1},
      {"height",2},
      {"scaleMode",4},
      {"width",3},
      {"addListener",100},
      {"removeListener",101}
    };

  static signed char lookup[] =
    {
      -1, -1, -1, -1, -1,  0,  1, -1, -1,  2,  3,  4, -1, -1,
       5
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          int index = lookup[key];

          if (index >= 0)
            {
              if (len == lengthtable[index])
                {
                  const char *s = wordlist[index].szName;

                  if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                    return &wordlist[index];
                }
            }
        }
    }
  return 0;
}
