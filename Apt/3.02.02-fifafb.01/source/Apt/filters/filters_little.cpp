#include <EABase/eabase.h>
#include "_AptInternalDefines.h"

#if APT_USE_FILTERS
#if !defined(EA_SYSTEM_BIG_ENDIAN)

APT_FILTER_ALIGNED(unsigned char, g_arrFiltersAptFile[]) = {
#       if (EA_PLATFORM_PTR_SIZE == 8)
#           include "./filters_little_apt_64.h"
#       else
#include "./filters_little_apt.h"
#       endif
};

APT_FILTER_ALIGNED(unsigned char, g_arrFiltersConstFile[]) = {
#       if (EA_PLATFORM_PTR_SIZE == 8)
#           include "./filters_little_const_64.h"
#       else
#include "./filters_little_const.h"
#       endif
};

#endif
#endif
