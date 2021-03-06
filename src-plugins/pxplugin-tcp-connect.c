#include "pxagent_api.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
	unsigned char		output_flag ;
} ;

/* run parameter
ip port
*/

/* for example
$ pxmanager --listen-ip 192.168.6.21 --listen-port 9527 -p 1 -t 1 -g pxplugin-tcp-connect.so -m "192.168.6.21 80" -n 2
$ pxagent --connect-ip 192.168.6.21 --connect-port 9527
*/

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-tcp-connect | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	user_data->netaddr.addr.sin_family = AF_INET ;
	
	sscanf( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , "%s%d" , user_data->netaddr.ip , & (user_data->netaddr.port) );
	if( user_data->netaddr.ip[0] == '\0' || user_data->netaddr.port <= 0 )
	{
		printf( "pxplugin-tcp-connect | run parameter '%s' invalid for format '(ip) (port)'\n" , GetPxPluginRunParameterPtr(p_pxplugin_ctx) );
		return -1;
	}
	
	user_data->netaddr.addr.sin_addr.s_addr = inet_addr(user_data->netaddr.ip) ;
	user_data->netaddr.addr.sin_port = htons( (unsigned short)user_data->netaddr.port );
	
	user_data->output_flag = 1 ;
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	int			nret = 0 ;
	
	user_data->netaddr.sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( user_data->netaddr.sock == -1 )
	{
		printf( "*** ERROR : socket failed , errno[%d]\n" , errno );
		return -1;
	}
	
	nret = connect( user_data->netaddr.sock , (struct sockaddr *) & (user_data->netaddr.addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "*** ERROR : connect[%s:%d] failed , errno[%d]\n" , user_data->netaddr.ip , user_data->netaddr.port , errno );
		close( user_data->netaddr.sock );
		return -1;
	}
	
	if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
	{
		printf( "connect[%s:%d] ok\n" , user_data->netaddr.ip , user_data->netaddr.port );
	}
	
	close( user_data->netaddr.sock );
	
	user_data->output_flag = 0 ;
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	free( user_data );
	
	return 0;
}

