#ifndef _H_PXAGENT_API_
#define _H_PXAGENT_API_

#include <errno.h>
#include <sys/time.h>

struct PxPluginContext ;

typedef int funcInitPxPlugin( struct PxPluginContext *p_pxplugin_ctx );
typedef int funcRunPxPlugin( struct PxPluginContext *p_pxplugin_ctx );
typedef int funcCleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx );

void SetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx , void *user_data );
void *GetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx );

unsigned int GetPxPluginRunCount( struct PxPluginContext *p_pxplugin_ctx );
char *GetPxPluginRunCommandPtr( struct PxPluginContext *p_pxplugin_ctx );

void SetPxPluginMinRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval min_run_elapse );
void SetPxPluginAvgRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval avg_run_elapse );
void SetPxPluginMaxRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval max_run_elapse );

#endif

