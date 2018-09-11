#include "pxagent_api.h"

#include "fasterhttp.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
	unsigned char		output_flag ;
	char			*request ;
	int			request_len ;
	struct HttpEnv		*http_env ;
} ;

/* run parameter
ip port http_request_pathfilename
*/

/* for example
$ pxmanager --listen-ip 192.168.6.21 --listen-port 9527 -p 1 -t 1 -n 1 -g pxplugin-http-fasterhttp.so -m "192.168.6.21 80 pxplugin-http.txt"  
*/

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			http_request_pathfilename[ PATH_MAX ] ;
	
	int			nret = 0 ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-http-fasterhttp | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	user_data->netaddr.addr.sin_family = AF_INET ;
	
	memset( http_request_pathfilename , 0x00 , sizeof(http_request_pathfilename) );
	sscanf( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , "%s%d%s" , user_data->netaddr.ip , & (user_data->netaddr.port) , http_request_pathfilename );
	if( user_data->netaddr.ip[0] == '\0' || user_data->netaddr.port <= 0 )
	{
		printf( "pxplugin-http-fasterhttp | run parameter '%s' invalid for format '(ip) (port) (http_request_pathfilename)'\n" , GetPxPluginRunParameterPtr(p_pxplugin_ctx) );
		return -1;
	}
	
	user_data->netaddr.addr.sin_addr.s_addr = inet_addr(user_data->netaddr.ip) ;
	user_data->netaddr.addr.sin_port = htons( (unsigned short)user_data->netaddr.port );
	
	nret = PXReadEntireFile( http_request_pathfilename , & (user_data->request) , & user_data->request_len ) ;
	if( nret )
	{
		printf( "pxplugin-http-fasterhttp | PXReadEntireFile[%s] failed[%d] , errno[%d]\n" , http_request_pathfilename , nret , errno );
		return -1;
	}
	
	if( STRISTR( user_data->request , "HTTP/1.1" ) || STRISTR( user_data->request , "Connection: keep-alive" ) )
	{
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
		else
		{
			printf( "connect[%s:%d] ok\n" , user_data->netaddr.ip , user_data->netaddr.port );
		}
	}
	else
	{
		user_data->netaddr.sock = -1 ;
	}
	
	user_data->http_env = CreateHttpEnv() ;
	if( user_data->http_env == NULL )
	{
		printf( "*** ERROR : CreateHttpEnv failed , errno[%d]\n" , errno );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
		return -1;
	}
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	struct HttpBuffer	*b = NULL ;
	int			status_code ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->netaddr.sock == -1 )
	{
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
		else
		{
			if( user_data->output_flag == 0 )
			{
				printf( "connect[%s:%d] ok\n" , user_data->netaddr.ip , user_data->netaddr.port );
			}
		}
	}
	
	ResetHttpEnv( user_data->http_env );
	
	b = GetHttpRequestBuffer( user_data->http_env ) ;
	nret = MemcatHttpBuffer( b , user_data->request , user_data->request_len ) ;
	if( nret )
	{
		printf( "*** ERROR : StrcatHttpBuffer failed , errno[%d]\n" , errno );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
		return -1;
	}
	
	nret = RequestHttp( user_data->netaddr.sock , NULL , user_data->http_env ) ;
	if( nret )
	{
		printf( "*** ERROR : RequestHttp failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	else
	{
		if( user_data->output_flag == 0 )
		{
			b = GetHttpRequestBuffer( user_data->http_env ) ;
			printf( "HTTP REQUEST[%.*s]\n" , GetHttpBufferLength(b) , GetHttpBufferBase(b,NULL) );
			
			b = GetHttpResponseBuffer( user_data->http_env ) ;
			printf( "HTTP RESPONSE[%.*s]\n" , GetHttpBufferLength(b) , GetHttpBufferBase(b,NULL) );
		}
	}
	
	status_code = GetHttpStatusCode(user_data->http_env) ;
	if( status_code != HTTP_OK )
	{
		printf( "*** ERROR : response status code [%03d]\n" , status_code );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
		return -1;
	}
	
	nret = CheckHttpKeepAlive( user_data->http_env ) ;
	if( nret == 0 )
	{
		if( user_data->output_flag == 0 )
		{
			printf( "disconnect[%s:%d]\n" , user_data->netaddr.ip , user_data->netaddr.port );
		}
		
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
	}
	
	user_data->output_flag = 1 ;
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->http_env )
	{
		DestroyHttpEnv( user_data->http_env );
	}
	
	if( user_data->netaddr.sock >= 0 )
	{
		printf( "disconnect[%s:%d]\n" , user_data->netaddr.ip , user_data->netaddr.port );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
	}
	
	if( user_data->request )
		free( user_data->request );
	
	free( user_data );
	
	return 0;
}

