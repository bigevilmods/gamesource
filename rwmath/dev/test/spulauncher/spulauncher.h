// (c) 2008 Criterion Software Limited and its licensors. All Rights Reserved
#ifndef SPU_LAUNCHER_H
#define SPU_LAUNCHER_H

// Spawn SPU programs specified in the provided "command line" arguments.
int SPULauncher(int argc, char **argv);

// Returns true if all tests have passed.  Otherwise returns false.
bool HaveAllTestsPassed();

#endif

