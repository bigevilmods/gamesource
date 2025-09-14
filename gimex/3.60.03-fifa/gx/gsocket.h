// (c) 1997-2007 Electronic Arts Inc.

#ifndef GSOCKET_H
#define GSOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void GSOCKET;

GSOCKET *GSOCKET_open(short port);
GSOCKET *GSOCKET_accept(GSOCKET * host);
int GSOCKET_close(GSOCKET *mysocket);
int GSOCKET_read(GSOCKET *mysocket, char *msg, size_t len);

GSOCKET *GSOCKET_wopen(const char *adr, short port);
int GSOCKET_write(GSOCKET *mysocket, const char *msg, size_t len);
int GSOCKET_cancel(void);
int GSOCKET_wclose(GSOCKET *mysocket);

#ifdef __cplusplus
}
#endif
#endif // GSOCKET_H


