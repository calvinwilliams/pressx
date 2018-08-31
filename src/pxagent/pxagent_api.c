#include "pxutil.h"
#include "pxagent_in.h"
#include "pxagent_api.h"

void SetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx , void *user_data )
{
	p_pxplugin_ctx->user_data = user_data ;
	return;
}

void *GetPxPluginUserData( struct PxPluginContext *p_pxplugin_ctx )
{
	return p_pxplugin_ctx->user_data;
}

unsigned int GetPxPluginRunCount( struct PxPluginContext *p_pxplugin_ctx )
{
	return p_pxplugin_ctx->p_agent->run_pressing.run_count;
}

char *GetPxPluginRunCommandPtr( struct PxPluginContext *p_pxplugin_ctx )
{
	return p_pxplugin_ctx->p_agent->run_pressing.run_command;
}

void SetPxPluginMinRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval min_run_elapse )
{
	p_pxplugin_ctx->perf_stat->min_run_elapse.tv_sec = min_run_elapse.tv_sec ;
	p_pxplugin_ctx->perf_stat->min_run_elapse.tv_usec = min_run_elapse.tv_usec ;
	return;
}

void SetPxPluginAvgRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval avg_run_elapse )
{
	p_pxplugin_ctx->perf_stat->avg_run_elapse.tv_sec = avg_run_elapse.tv_sec ;
	p_pxplugin_ctx->perf_stat->avg_run_elapse.tv_usec = avg_run_elapse.tv_usec ;
	return;
}

void SetPxPluginMaxRunElapse( struct PxPluginContext *p_pxplugin_ctx , struct timeval max_run_elapse )
{
	p_pxplugin_ctx->perf_stat->max_run_elapse.tv_sec = max_run_elapse.tv_sec ;
	p_pxplugin_ctx->perf_stat->max_run_elapse.tv_usec = max_run_elapse.tv_usec ;
	return;
}

