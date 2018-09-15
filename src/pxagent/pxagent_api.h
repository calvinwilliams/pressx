#ifndef _H_PXAGENT_API_
#define _H_PXAGENT_API_

#include <errno.h>
#include <sys/time.h>

#include "pxutil.h"

struct PxPluginContext ;

typedef int funcInitPxPlugin( struct PxPluginContext *p_pxplugin_ctx );
typedef int funcRunPxPlugin( struct PxPluginContext *p_pxplugin_ctx );
typedef int funcRawRunPxPlugin( struct PxPluginContext *p_pxplugin_ctx );
typedef int funcCleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx );

void SetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx , void *user_data );
void *GetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx );

unsigned char GetPxPluginOutputFlag( struct PxPluginContext *p_pxplugin_ctx );

unsigned int GetPxPluginRunCount( struct PxPluginContext *p_pxplugin_ctx );
char *GetPxPluginRunParameterPtr( struct PxPluginContext *p_pxplugin_ctx );

void SetPxPluginRunCount( struct PxPluginContext *p_pxplugin_ctx , unsigned int run_count );
void SetPxPluginRunTimeval( struct PxPluginContext *p_pxplugin_ctx , struct timeval *p_begin_run_timestamp , struct timeval *p_end_run_timestamp );
void SetPxPluginDelayTimeval( struct PxPluginContext *p_pxplugin_ctx , struct timeval *p_begin_delay_timestamp , struct timeval *p_end_delay_timestamp );

#endif

