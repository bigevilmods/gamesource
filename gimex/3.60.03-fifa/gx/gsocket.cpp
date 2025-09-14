// (c) 1997-2007 Electronic Arts Inc.

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock.h>
// stddef for intptr_t
#include <stddef.h>
#else
// defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET -1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr *  LPSOCKADDR;
#endif
#include <string.h>
#include <stdio.h>
#include "gsocket.h"

#define SD_RECEIVE 2
#define SD_SEND 1

static  SOCKADDR_IN GSOCKET_sockaddr;

// confirm winsock.dll version

static bool GSOCKET_startup(void)
{
#if !defined(_WIN32)
    return true;
#else
    bool ok=false;

    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 0);

    int err = WSAStartup(wVersionRequested, &wsaData);

    if (!err)
    {
        /* Confirm that the Windows Sockets DLL supports 1.1.*/
        /* Note that if the DLL supports versions greater */
        /* than 1.1 in addition to 1.1, it will still return */
        /* 1.1 in wVersion since that is the version we */

        /* requested. */

        if ( (wsaData.wVersion&0xff)>=0x02)
        {
            ok = true;
        }
        if (!ok)
            WSACleanup();
    }
    return ok;
#endif

}

static void GSOCKET_setlinger(SOCKET mysocket)
{
    struct linger opt;
    opt.l_onoff = 1;
    opt.l_linger = 1;
    setsockopt(mysocket, SOL_SOCKET, SO_LINGER, (char *) &opt, sizeof(opt));
}


GSOCKET *GSOCKET_accept(GSOCKET * host)
{
    SOCKET fsocket=0;

    bool ok = GSOCKET_startup();
    if (ok)
    {
        /* Accept connection */
        fsocket = accept((SOCKET) (intptr_t)host, NULL, NULL);
        GSOCKET_setlinger(fsocket);
    }

    return (GSOCKET *) fsocket;
}


GSOCKET *GSOCKET_open(short port)
{
    SOCKET mysocket=0;

    bool ok = GSOCKET_startup();
    if (ok)
    {
        // NWD Changed from SOCK_STREAM to SOCK_DGRAM
        mysocket = socket(AF_INET,SOCK_DGRAM,0);
        GSOCKET_setlinger(mysocket);

        if (mysocket != INVALID_SOCKET)
        {
            memset(&GSOCKET_sockaddr, 0, sizeof(GSOCKET_sockaddr));
            GSOCKET_sockaddr.sin_family = AF_INET;
            GSOCKET_sockaddr.sin_port = htons(port);
            GSOCKET_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(mysocket, (struct sockaddr*)&GSOCKET_sockaddr, sizeof(GSOCKET_sockaddr)) < 0)
            {
                ok = false;
            }
            else
            {
                int err = listen(mysocket, 1);
                if (err < 0)
                    ok = false;
            }
            if (!ok)
            {
#if defined(_WIN32)
                closesocket(mysocket);
#else
                close(mysocket);
#endif
                mysocket = 0;
            }
        }
    }
    return (GSOCKET *) mysocket;
}

int GSOCKET_read(GSOCKET *mysocket, char *msg, size_t len)
{
    size_t bytesleft = len;

    if (len == 0)
        return 1;

    for (;;)
    {
        char * message =  msg + (len - bytesleft);
        int err = recv((SOCKET) (intptr_t)mysocket, message, bytesleft, 0);
#if defined(_WIN32)
        if (err != SOCKET_ERROR)
#else
        if (err > 0)
#endif
        {
            bytesleft -= err;
            if (bytesleft <= 0)
            {
                return 1;
            }
        }
#if !defined(_WIN32)
        else if (err == 0)
        {
            return 0;

        }
#endif
        else
        {
            return 0;
        }
    }
    return 0;
}

int GSOCKET_close(GSOCKET *mysocket)
{
    int ok=1;

    if (mysocket)
    {
#if defined(_WIN32)
		closesocket((SOCKET)mysocket);
#else
        close((SOCKET)(intptr_t)mysocket);
#endif
    }
#if defined(_WIN32)
    WSACleanup();
#endif

    return ok;
}

GSOCKET *GSOCKET_wopen(const char *adr, short port)
{
    SOCKET mysocket=0;

    bool ok = GSOCKET_startup();
    if (ok)
    {
        mysocket = socket(AF_INET,SOCK_DGRAM,0);
        if (mysocket == INVALID_SOCKET)
            mysocket = 0;

        if (mysocket)
        {
			int val=300;	// millisecond timeout on send
		    setsockopt(mysocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &val, sizeof(val));

#if defined(_WIN32)
			val = 0;	// linger not, in the close
		    setsockopt(mysocket, SOL_SOCKET, SO_DONTLINGER, (char *) &val, sizeof(val));
#endif

#if 0
			if (adr[0]>='0' && adr[0]>='9')
			{
                GSOCKET_sockaddr.sin_family = AF_INET;
                GSOCKET_sockaddr.sin_port = htons(port);
				GSOCKET_sockaddr.sin_addr = inet_addr(adr);
			}
			else
#endif
			{
				// note this function is very slow to time out if you are not connected to vpn and attempt to use an EA address

				struct hostent *hp = gethostbyname(adr);
				if (!hp)
				{
	#if defined(_WIN32)
					closesocket(mysocket);
	#else
					close(mysocket);
	#endif
					mysocket = 0;
				}
				else
				{
					GSOCKET_sockaddr.sin_family = AF_INET;
					GSOCKET_sockaddr.sin_port = htons(port);
					memcpy(&GSOCKET_sockaddr.sin_addr, hp->h_addr, hp->h_length);
				}
			}
        }
    }

    return (GSOCKET *) mysocket;
}

int GSOCKET_write(GSOCKET *mysocket, const char *msg, size_t len)
{
    int ok=0;
    int r;

    r = sendto((SOCKET)(intptr_t)mysocket, msg, len, 0, (LPSOCKADDR) &GSOCKET_sockaddr, sizeof(SOCKADDR_IN));

#if defined(_WIN32)
    if (r == SOCKET_ERROR)
#else
    if (r<=0)
#endif
    {
        ok = 0;
    }
    else
        ok = 1;

    return ok;
}

int GSOCKET_wclose(GSOCKET *mysocket)
{
    int ok=1;

    if (mysocket)
    {
#if defined(_WIN32)
        if ( shutdown( (SOCKET)(intptr_t)mysocket, SD_SEND | SD_RECEIVE ) == SOCKET_ERROR )
#else
        if ( shutdown( (SOCKET)(intptr_t)mysocket, SD_SEND | SD_RECEIVE ) > 0 )
#endif
        {
            ok = 0;
        }

#if defined(_WIN32)
        closesocket((SOCKET)(intptr_t)mysocket);
#else
        close((SOCKET)(intptr_t)socket);
#endif
    }
#if defined(_WIN32)
    WSACleanup();
#endif

    return ok;
}

int GSOCKET_cancel(void)
{
    int ok=0;

#if defined(_WIN32)
    ok = !WSACancelBlockingCall();
#endif
    return ok;
}
