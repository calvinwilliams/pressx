#include "pxagent_api.h"

#include "ib2api.h"

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct IB2Env		*p_env = NULL ;
	
	int			nret = 0 ;
	
	nret = IB2AllocEnvironment( & p_env ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2AllocEnvironment failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	nret = IB2LoadClientConfig( p_env , NULL ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2LoadClientConfig failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	SetPxPluginUserData( p_pxplugin_ctx , p_env );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct IB2Env		*p_env = NULL ;
	
	p_env = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	
	
	
	
	
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct IB2Env		*p_env = NULL ;
	
	p_env = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	printf( "pxplugin-ib2 | IB2FreeEnvironment\n" );
	IB2FreeEnvironment( & p_env );
	
	return 0;
}

