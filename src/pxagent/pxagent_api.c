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

