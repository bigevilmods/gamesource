// (c) 1995-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                     Nil CRapi Module  v1.03                      */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*                    Main Module - Mar 24, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 950523 GS initial version                                */
/*    1.01 960617 GS crapi 1.20 update                              */
/*    1.02 990707 LC uses CRINVERSETABLE struct                     */
/*    1.03 030324 ID respectalpha                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* empty crapi that does nothing                                    */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "nil.h"
#include <string.h>


CRABOUT *GCALL NILCR_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('N','I','L','C');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.03" );
        strcpy(about->namestr,      "Nil" );
        strcpy(about->longnamestr,  "Nil" );
    }

    return(about);
}

CRINSTANCE *GCALL NILCR_open(void)
{
    return((CRINSTANCE *)0);
}

int GCALL NILCR_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    return(0);
}

ARGB *GCALL NILCR_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    *inversetable = (CRINVERSETABLE *)0;
    return((ARGB *)0);
}
