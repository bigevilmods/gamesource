// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

#if defined(EA_PLATFORM_XENON)
    #include "xenon/rwfilesysmanager.cpp"
    #include "xenon/rwxenondriver.cpp"
#else
    #include "pc/rwfilesysmanager.cpp"
    #include "pc/rwpcdriver.cpp"
#endif

