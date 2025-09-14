// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include <unit/unit.h>

int main(int argc, char **argv)
{
#if defined(USE_TRAP)   // Defined in your build script
    EATest::TestApiDevice trapDevice(argc,argv);
    EATest::SetTestListenerDevice(&trapDevice);
#endif

    // Run the tests.
    return EATest::DoTests(argc, argv);
}
