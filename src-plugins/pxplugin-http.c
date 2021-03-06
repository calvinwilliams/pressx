#include "pxagent_api.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
	unsigned char		output_flag ;
	char			*request ;
	int			request_len ;
	unsigned char		keep_alive ;
} ;

/* run parameter
ip port http_request_pathfilename
*/

/* for example
$ pxmanager --listen-ip 192.168.6.21 --listen-port 9527 -p 1 -t 1 -g pxplugin-http.so -m "192.168.6.21 80 pxplugin-http.msg" -n 1
$ pxagent --connect-ip 192.168.6.21 --connect-port 9527
*/

#define CONTENT_LENGTH		"Content-length: "

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			http_request_pathfilename[ PATH_MAX ] ;
	char			*http__1_0 ;
	char			*http__1_1 ;
	char			*connection__keep_alive ;
	
	int			nret = 0 ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-request | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	user_data->netaddr.addr.sin_family = AF_INET ;
	
	memset( http_request_pathfilename , 0x00 , sizeof(http_request_pathfilename) );
	sscanf( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , "%s%d%s" , user_data->netaddr.ip , & (user_data->netaddr.port) , http_request_pathfilename );
	if( user_data->netaddr.ip[0] == '\0' || user_data->netaddr.port <= 0 )
	{
		printf( "pxplugin-http | run parameter '%s' invalid for format '(ip) (port) (http_request_pathfilename)'\n" , GetPxPluginRunParameterPtr(p_pxplugin_ctx) );
		return -1;
	}
	
	user_data->netaddr.addr.sin_addr.s_addr = inet_addr(user_data->netaddr.ip) ;
	user_data->netaddr.addr.sin_port = htons( (unsigned short)user_data->netaddr.port );
	
	user_data->output_flag = 1 ;
	
	nret = PXReadEntireFile( http_request_pathfilename , & (user_data->request) , & user_data->request_len ) ;
	if( nret )
	{
		printf( "pxplugin-http | PXReadEntireFile failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	http__1_0 = STRISTR( user_data->request , "HTTP/1.0" ) ;
	http__1_1 = STRISTR( user_data->request , "HTTP/1.1" ) ;
	connection__keep_alive = STRISTR( user_data->request , "Connection: Keep-Alive" ) ;
	if( http__1_0 && connection__keep_alive )
	{
		user_data->keep_alive = 1 ;
	}
	else if( http__1_1 )
	{
		user_data->keep_alive = 1 ;
	}
	else
	{
		user_data->keep_alive = 0 ;
	}
	printf( "keep_alive[%d]\n" , user_data->keep_alive );
	
	if( user_data->keep_alive == 1 )
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
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			response_buffer[ 4096 + 1 ] ;
	int			response_len ;
	int			header_len ;
	char			*p = NULL ;
	int			status_code ;
	char			*content_length = NULL ;
	int			remain_len ;
	char			remain_buffer[ 4096 + 1 ] ;
	int			block_len ;
	int			len ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->keep_alive == 0 || user_data->netaddr.sock == -1 )
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
			if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
			{
				printf( "connect[%s:%d] ok\n" , user_data->netaddr.ip , user_data->netaddr.port );
			}
		}
	}
	
	nret = writen( user_data->netaddr.sock , user_data->request , user_data->request_len , NULL ) ;
	if( nret )
	{
		printf( "*** ERROR : writen failed , errno[%d]\n" , errno );
		return -1;
	}
	else
	{
		if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
		{
			printf( "request[%.*s]\n" , user_data->request_len , user_data->request );
		}
	}
	
	remain_len = -1 ;
	
	memset( response_buffer , 0x00 , sizeof(response_buffer) );
	response_len = read( user_data->netaddr.sock , response_buffer , sizeof(response_buffer)-1 ) ;
	if( response_len == 0 )
	{
		printf( "*** ERROR : socket closed on first reading\n" );
		return -1;
	}
	else if( response_len == -1 )
	{
		printf( "*** ERROR : first read failed[%d] , errno[%d]\n" , response_len , errno );
		return -1;
	}
	else
	{
		if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
		{
			printf( "first response[%.*s]\n" , response_len , response_buffer );
		}
	}
	
	sscanf( response_buffer , "%*s%d" , & status_code );
	if( status_code != 200 )
	{
		printf( "*** ERROR : response status code [%03d]\n" , status_code );
		return -1;
	}
	
	p = strstr( response_buffer , "\r\n\r\n" ) ;
	if( p == NULL )
	{
		printf( "*** ERROR : big request response[%s]\n" , response_buffer );
		return -1;
	}
	header_len = p+4 - response_buffer ;
	
	content_length = STRISTR( response_buffer , CONTENT_LENGTH ) ;
	if( content_length == NULL )
	{
		if( response_len > 4 && STRCMP( response_buffer+response_len-4 , == , "\r\n\r\n" ) )
		{
			;
		}
		else
		{
			printf( "*** ERROR : big request response[%s]\n" , response_buffer );
			return -1;
		}
	}
	else
	{
		remain_len = atoi(content_length+sizeof(CONTENT_LENGTH)-1) - (response_len-header_len) ;
		
		while( remain_len > 0 )
		{
			block_len = MIN( remain_len , sizeof(remain_buffer)-1 ) ;
			memset( remain_buffer , 0x00 , sizeof(remain_buffer) );
			len = read( user_data->netaddr.sock , remain_buffer , block_len ) ;
			if( len == 0 )
			{
				printf( "*** ERROR : socket closed on continue reading\n" );
				return -1;
			}
			else if( len == -1 )
			{
				printf( "*** ERROR : continue read failed[%d] , errno[%d]\n" , block_len , errno );
				return -1;
			}
			else
			{
				if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
				{
					printf( "continue response[%.*s]\n" , len , remain_buffer );
				}
			}
			
			remain_len -= len ;
		}
	}
	
	if( user_data->keep_alive == 0 || STRISTR( response_buffer , "Connection: Close" ) )
	{
		if( user_data->output_flag == 1 && GetPxPluginOutputFlag(p_pxplugin_ctx) )
		{
			printf( "disconnect[%s:%d]\n" , user_data->netaddr.ip , user_data->netaddr.port );
		}
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
	}
	
	user_data->output_flag = 0 ;
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->keep_alive == 1 )
	{
		printf( "disconnect[%s:%d]\n" , user_data->netaddr.ip , user_data->netaddr.port );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
	}
	
	if( user_data->request )
		free( user_data->request );
	
	free( user_data );
	
	return 0;
}

