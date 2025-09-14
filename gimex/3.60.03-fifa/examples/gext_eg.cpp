// (c) 2000-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

gext_eg.cpp - GIMEX example, list GIMEX modules and extensions

DESCRIPTION

This is an example of how to use the GIMEX API. Shows how to access the GIMEX
module information with GABOUT.

This example prints out the list of supported GIMEX file formats and the extensions
that the format supports.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> in order to run this example.

SOURCE
@*********************************************************************************/

#include <stdio.h>
#include "gopen.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif


int main(int argc, char* argv[])
{
	int numGimexModules;
	int module;

	/* initialize the gimex library */
	if (!GIMEX_init())
	{
		printf("could not initialize gimex.dll - make sure gimex.dll is in the same folder as %s\n", argv[0]);
		return 1;
	}

	/* Get the number of modules in the Gimex library. */
	numGimexModules = GIMEX_count();
	printf("%2d Gimex Modules:\n", numGimexModules);
	printf( "-----------------\n");

	/* Start loop at 1 (vs 0) because 0 will always be the Nil Gimex module which is */
	/* only for debugging. */
	for (module = 1; module < numGimexModules; ++module)
	{
		GABOUT* pAbout;

		/* set the current module */
		GIMEX_set(module);

		/* get info about the current module */
		pAbout = GIMEX_about();
		if (pAbout != NULL)
		{
			int extension;
			
			/* print the module name and all the extensions the module supports */
			printf("%2d %s: ", module, pAbout->longtypestr);
			for (extension = 0; extension < MAXEXTENSIONS; ++extension)
			{
				if (pAbout->extensions[extension][0] != '\0')
				{
					printf("%s ", pAbout->extensions[extension]);
				}
			}
			printf("\n");

			/* you must free the pointer returned by GIMEX_about */
			gfree(pAbout);
		}
	}

	/* clean up the library */
	GIMEX_restore();

	return 0;
}

// END ABSTRACT
