// (c) 1995-2007 Electronic Arts Inc.

#ifndef __CROPEN_H
#define __CROPEN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "crapi.h"

#define aboutcrapi() CRAPI_about()
#define opencrapi(numcolours) crcolours = numcolours, CRAPI_open()
#define readcrapi(crx, source) CRAPI_read(crx, source->ginfo, source->image, source->rowbytes)
#define closecrapi(crx, inversetable) CRAPI_close(crx, crcolours, inversetable, NULL)
#define initcrapi() CRAPI_init()
#define restorecrapi() CRAPI_restore()
#define findcrapi(name) CRAPI_find(name)
#define setcrapi(typenum) CRAPI_set(typenum)
#define getcrapi() CRAPI_get()
#define getnumcrapi() CRAPI_count()

#ifdef __cplusplus
}
#endif
#endif
