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

char *GetPxPluginRunParameterPtr( struct PxPluginContext *p_pxplugin_ctx )
{
	return p_pxplugin_ctx->p_agent->run_pressing.run_parameter;
}

void SetPxPluginRunCount( struct PxPluginContext *p_pxplugin_ctx , unsigned int run_count )
{
	if( p_pxplugin_ctx->p_agent->pfuncRawRunPxPlugin )
	{
		p_pxplugin_ctx->perf_stat->run_count = run_count ;
	}
	return;
}

void SetPxPluginRunTimeval( struct PxPluginContext *p_pxplugin_ctx , struct timeval *p_begin_run_timestamp , struct timeval *p_end_run_timestamp )
{
	if( p_pxplugin_ctx->p_agent->pfuncRawRunPxPlugin )
	{
		DIFF_TIMEVAL( p_pxplugin_ctx->perf_stat->run_timeval , (*p_begin_run_timestamp) , (*p_end_run_timestamp) )
	}
	return;
}

void SetPxPluginDelayTimeval( struct PxPluginContext *p_pxplugin_ctx , struct timeval *p_begin_delay_timestamp , struct timeval *p_end_delay_timestamp )
{
	struct timeval		diff_timeval ;
	
	if( p_pxplugin_ctx->p_agent->pfuncRawRunPxPlugin )
	{
		DIFF_TIMEVAL( diff_timeval , (*p_begin_delay_timestamp) , (*p_end_delay_timestamp) )
		
		MIN_VAL_TIMEVAL( p_pxplugin_ctx->perf_stat->min_delay_timeval , diff_timeval )
		MAX_VAL_TIMEVAL( p_pxplugin_ctx->perf_stat->max_delay_timeval , diff_timeval )
	}
	return;
}

