// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                          Huff Viewer v1.00                       */
/*                          based on lumpyi.c                       */
/*                                                                  */
/*                            by Lydia Chan                         */
/*                                                                  */
/*                             Aug 09, 1999                         */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "spit/spit.h"
#include "spit/stdgimex.h"
#include "huffview.h"

/****************************************************************/
/*  Local Variables                                             */
/****************************************************************/

static int LUMPfiles;
static int LUMPubatchlen;
static int LUMPpbatchlen;

/* percentage calculation status */

char *LUMPYpercentstr(char *buf, size_t olen, size_t nlen)
{
    double reduction=100.0;

    if (nlen && olen)
    {
        reduction = 100.0-(double)nlen*100.0/(double)olen;
    }
    sprintf(buf, "%.1f%%", reduction);
    return(buf);
}


/****************************************************************/
/*  Type of File                                                */
/*  - returns supported types of packed files                   */
/****************************************************************/

int TypeOFile(const unsigned char *s)
{
    unsigned int id;
    unsigned int ltype;
    int type;

    if ((unsigned int) (*((unsigned char *) (s+1)))==0xfb)
    {
        id = (unsigned int) *s & 0x007e;

        switch(id)
        {
            /* fb6 pack types */

            case 0x10:    type = REFPAC;      break;
            case 0x12:    type = ZIPPAC;      break;
            case 0x1e:    type = BZ2PAC;      break;
            case 0x30:    type = HUFFPAC;     break;
            case 0x46:    type = BTREEPAC;    break;

            default:
                type = NOPAC;
        }
    }
    else if (ggetm(s,2)==GMAKEID(0,0,'B','Z'))
    {
        type = BZ2PAC;
    }
    else
    {
        ltype = ggetm(s, 4);

        if (ltype==0x000003f3)
            type = AEXE;
        else
            type = NOPAC;
    }
    return(type);
}

/****************************************************************/
/*  Get Unpacked Length of a File                               */
/****************************************************************/

int GetULen(const unsigned char *ptr, int len)
{
    int tof;
    const unsigned char *s;

    tof = 0;
    if (len>2)
    {
        s = (ptr);
        len -= (int)(s-ptr);

        tof = TypeOFile(s);

        if (tof>0 && tof<=NEWPACKTYPES)
        {
            int id = (unsigned int) *s;
            if (id&0x80)
                len = (int) ggetm(s+2, 4);
            else
                len = (int) ggetm(s+2, 3);
        }
    }

    return(len);
}

/****************************************************************/
/*  View a File                                                 */
/****************************************************************/

int ViewFile(char *ufname, char *name, struct SPITMEM *infile, int n, int offset)
{
    unsigned int id;
    int tof;
    int more;
    int ulen;
    int ok=1;
    const unsigned char *s;
    char *typestr;
    char fname[SPITMAXLINE];
    char statbuf[SPITMAXLINE];
    char storbuf[SPITMAXLINE];

    if (name[0])
    {
        strcpy(fname, name);
        if ((*ufname!=(char)'\0') && (OPTverbose>=4))
        {
            if (OPTverbose>=6)
                strcpy(fname,ufname);
            else
                if (OPTverbose)
                    printf ("equals change ext here %s %s\n", fname, ufname);
            strcat(fname, "@");
            strcat(fname, name);
        }
    }
    else
    {
        sprintf(fname, "%s[%d]", ufname,n);
    }

    if (OPTverbose)
    {
        if (OPTverbose>=4)
            printf("  %-21s ", fname);
        else
            printf("  %-16s ", fname);
    }

    id = 0;
    ulen = 0;
    s = (const unsigned char *) infile->ptr;
    if (infile->len>=2L)
    {
        ulen = GetULen(s, infile->len);
        s = (const unsigned char *)(infile->ptr);
        id = *s;
        id = id*256 + *((unsigned char *) (s+1));
        tof = TypeOFile(s);
    }
    else
    {
        tof = 0;
        if (*name==(char)'.')
            tof = LUMPGROUP;
    }

    if (OPTverbose)
    {
        if (OPTverbose>=4)
        {
            if (offset>0L)
                printf("%4d ",n);
            else
                printf("     ");
        }
    }

    switch (tof)
    {
        case BTREEPAC:      typestr = "BTreed";     break;
        case HUFFPAC:       typestr = "Huffed";     break;
        case REFPAC:        typestr = "Refpack";    break;
        case BZ2PAC:        typestr = "BZip2";      break;
        case ZIPPAC:        typestr = "Zip";        break;

        default:
            if (OPTverbose>=4)
            {
                sprintf(storbuf,"Stored%04x",id);
                typestr = storbuf;
            }
            else
                typestr = "Stored";
            ok = 0; // unknown/stored type
            break;
    }

    if (OPTverbose>=3)
    {
        more = 0;
        if (tof>0 && tof<NEWPACKTYPES)
        {
            if (id&0x0100)
                more = 1;
        }
        if (s != (const unsigned char *)infile->ptr)
            more += 3;
        strcpy(statbuf,typestr);
        statbuf[10-more] = (char) '\0';
        if (more>1)
        {
            strcat(statbuf,"(c)");
            more -= 3;
        }
        if (more)
            strcat(statbuf,"+");

        printf("%-10s", statbuf);
    }

    if (OPTverbose)
    {
        if (OPTverbose>=3)
            printf("%8ld", offset);
        else
            printf("%7ld", offset);
    }

    if (OPTverbose>=3)
    {
        printf("%10ld", ulen);
        printf("%9ld", infile->len);

        LUMPYpercentstr(statbuf, ulen, infile->len);
        printf("  %6s\n", statbuf);
    }
    else
        if (OPTverbose)
            printf("%10ld\n", infile->len);

    LUMPpbatchlen += infile->len;
    LUMPubatchlen += ulen;
    ++LUMPfiles;

    return(ok);
}

void StartViewFile()
{
    LUMPfiles = 0;
    LUMPpbatchlen = 0L;
    LUMPubatchlen = 0L;

    if (OPTverbose>=4)
        puts("  Filename                 # Type        Offset  Unpacked   Packed Reduction");
    else
        if (OPTverbose>=3)
            puts("  Filename         Type        Offset  Unpacked   Packed Reduction");
        else if (OPTverbose)
            puts("  Filename          Offset     FBSize");
}

void StopViewFile()
{
    char statbuf[SPITMAXLINE];

    if (OPTverbose>=4)
        puts("  -----------------                              -------- --------  ------");
    else
        if (OPTverbose>=3)
            puts("  ----------------                     -------- --------  ------");
        else if (OPTverbose)
            puts("  ----------------             ----");

    sprintf(statbuf,"%d files",LUMPfiles);

    if (OPTverbose>=4)
        printf("  %-24s                     ", statbuf);
    else
        if (OPTverbose>=3)
            printf("  %-17s                  ", statbuf);
        else if (OPTverbose)
            printf("  %-17s       ", statbuf);

    if (OPTverbose>=3)
    {
        printf("%10ld", LUMPubatchlen);
        printf("%9ld", LUMPpbatchlen);
        LUMPYpercentstr(statbuf, LUMPubatchlen, LUMPpbatchlen);
        printf("  %6s\n", statbuf);
    }
    else if (OPTverbose)
        printf("%10ld\n", LUMPpbatchlen);
}

