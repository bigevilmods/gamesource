// (c) 2003-2007 Electronic Arts Inc.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gimex.h"
#include "gopen.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif


typedef struct TYPESTRS
{
    char * extension;
	char * longname;
}TYPESTRS;

char *strTemp;

GABOUT *about;
TYPESTRS * list = 0;
TYPESTRS *finalList = 0;
unsigned int type = 0;
unsigned int count;// = GIMEX_count();
unsigned int i = 0;
unsigned int m = 0;
char * temp1;
char * temp2;


TYPESTRS *GX_generateGimexTypes()
{
    count = GIMEX_count();

    printf("count = %d\n", count);

    list = (TYPESTRS *)galloc((sizeof(TYPESTRS)*count)+1*sizeof(TYPESTRS));
    memset(list,0,(sizeof(TYPESTRS)*count)+1*sizeof(TYPESTRS));
    if(list)
    {
    	for (type=0;type<count; ++type)
    	{
    	    GIMEX_set(type);

    	    about = GIMEX_about();

    	    if (about)
    	    {
        		list[type].extension = (char *)galloc(strlen(about->shorttypestr)+5); // create memory for my two new entries
        		list[type].longname  = (char *)galloc(strlen(about->longtypestr)+5);
        		strcpy(list[type].extension,about->shorttypestr); // copy the items over into my buffers
        		strcpy(list[type].longname,about->longtypestr);

        		for(i = 0; i < strlen(list[type].extension);i++) // keep the extensions uniform, all lowercase.
        		{
        		    list[type].extension[i] = tolower(list[type].extension[i]);
        		}

        		temp1 = list[type].extension; // copy the pointers so I can do a dirty insertion sort.

		        temp2 = list[type].longname;

        		for(i = 0; i < type;i++) // insertion sort them alphabetically.
                {
        		    if(strcmp(list[type].extension,list[i].extension) < 0)
        		    {
            			for(m = type; m > i; m--)
            			{
            			    list[m].extension = list[m-1].extension;
            			    list[m].longname  = list[m-1].longname;
            			}

		            	list[i].extension = temp1;
            			list[i].longname  = temp2;
            			break;
        		    }
        		}
    	    }

    	    gfree((char *)about);
    	}
    }

    return(list);
}

void main()
{
    GIMEX_init();       // load and initialize library

    finalList = GX_generateGimexTypes();

    for (int i=0; finalList[i].extension; ++i)
        printf(".%-4s %s\n", finalList[i].extension,finalList[i].longname);

    GIMEX_restore();
}
