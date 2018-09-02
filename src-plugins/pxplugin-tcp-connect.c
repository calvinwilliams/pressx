#include "pxagent_api.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
} ;

/* run parameter
ip port
*/

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			*p = NULL ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-tcp-connect | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	user_data->netaddr.addr.sin_family = AF_INET ;
	
	p = gettok( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , PRESSX_BLANK_DELIM ) ;
	if( p == NULL || p[0] == '\0' )
	{
		printf( "pxplugin-tcp-connect | expect 'ip' in run parameter\n" );
		return -1;
	}
	user_data->netaddr.addr.sin_addr.s_addr = inet_addr(p) ;
	
	p = gettok( NULL , PRESSX_BLANK_DELIM ) ;
	if( p == NULL )
	{
		printf( "pxplugin-tcp-connect | expect 'port' in run parameter\n" );
		return -1;
	}
	if( atoi(p) <= 0 )
	{
		printf( "pxplugin-tcp-connect | 'port' invalid\n" );
		return -1;
	}
	user_data->netaddr.addr.sin_port = htons( (unsigned short)atoi(p) );
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	user_data->netaddr.sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( user_data->netaddr.sock == -1 )
	{
		printf( "*** ERROR : socket failed , errno[%d]\n" , errno );
		return -1;
	}
	
	SETNETADDRESS( user_data->netaddr )
	nret = connect( user_data->netaddr.sock , (struct sockaddr *) & (user_data->netaddr.addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "*** ERROR : connect[%s:%d] failed , errno[%d]\n" , user_data->netaddr.ip , user_data->netaddr.port , errno );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
		return -1;
	}
	
	close( user_data->netaddr.sock );
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	free( user_data );
	
	return 0;
}

