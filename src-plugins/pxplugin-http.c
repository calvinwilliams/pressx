#include "pxagent_api.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
	char			*http ;
	int			http_len ;
	char			*http_1_0 ;
	char			*http_1_1 ;
	char			*connection__keep_alive ;
	unsigned char		keep_alive ;
} ;

/* run parameter for GET method
ip:port
GET uri HTTP/1.1
[ request-headers ]

*/

/* run parameter for POST method
ip:port
POST uri HTTP/1.1
[ request-headers ]

[ post data ]
*/

/* run parameter for GET method and keep-alive
ip:port
GET uri HTTP/1.1
[ request-headers ]
Connection: Keep-Alive

*/

#define CONTENT_LENGTH		"Content-length: "

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			*ip_or_and_port = NULL ;
	int			ip_or_and_port_len ;
	char			*ip = NULL ;
	char			*port = NULL ;
	char			*http = NULL ;
	
	int			nret = 0 ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-http | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	user_data->netaddr.addr.sin_family = AF_INET ;
	
	ip_or_and_port = gettok( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , "\n" ) ;
	if( ip_or_and_port == NULL || ip_or_and_port[0] == '\0' )
	{
		printf( "pxplugin-http | expect 'ip[:port]' in run parameter\n" );
		return -1;
	}
	ip_or_and_port_len = strlen( ip_or_and_port ) ;
	
	port = strchr( ip_or_and_port , ':' ) ;
	if( port )
	{
		port[0] = '\0' ;
		ip = ip_or_and_port ;
		user_data->netaddr.addr.sin_addr.s_addr = inet_addr(ip) ;
		port++;
		user_data->netaddr.addr.sin_port = htons( (unsigned short)atoi(port) );
	}
	else
	{
		ip = ip_or_and_port ;
		user_data->netaddr.addr.sin_addr.s_addr = inet_addr(ip) ;
		user_data->netaddr.addr.sin_port = htons( (unsigned short)80 );
	}
	
	http = GetPxPluginRunParameterPtr(p_pxplugin_ctx) + ip_or_and_port_len ;
	if( http[0] == '\0' )
	{
		printf( "pxplugin-http | expect 'http' in run parameter\n" );
		return -1;
	}
	user_data->http = strdup( http ) ;
	if( user_data->http == NULL )
	{
		printf( "pxplugin-http | strdup failed , errno[%d]\n" , errno );
		return -1;
	}
	user_data->http_len = strlen(user_data->http) ;
	
	user_data->http_1_0 = STRISTR( user_data->http , "HTTP/1.0" ) ;
	user_data->http_1_1 = STRISTR( user_data->http , "HTTP/1.1" ) ;
	user_data->connection__keep_alive = STRISTR( user_data->http , "Connection: Keep-Alive" ) ;
	
	if( user_data->http_1_0 && user_data->connection__keep_alive )
	{
		user_data->keep_alive = 1 ;
	}
	else if( user_data->http_1_1 && ! user_data->connection__keep_alive )
	{
		user_data->keep_alive = 1 ;
	}
	else
	{
		user_data->keep_alive = 0 ;
	}
	
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
	char			*p = NULL ;
	int			status_code ;
	char			*content_length = NULL ;
	int			remain_len ;
	int			block_len ;
	int			len ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->keep_alive == 0 )
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
	}
	
	nret = writen( user_data->netaddr.sock , user_data->http , user_data->http_len , NULL ) ;
	if( nret )
	{
		printf( "*** ERROR : writen failed , errno[%d]\n" , errno );
		return -1;
	}
	
	remain_len = -1 ;
	
	memset( response_buffer , 0x00 , sizeof(response_buffer) );
	response_len = read( user_data->netaddr.sock , response_buffer , sizeof(response_buffer)-1 ) ;
	if( response_len == 0 )
	{
		printf( "*** ERROR : socket closed on reading\n" );
		return -1;
	}
	else if( response_len == -1 )
	{
		printf( "*** ERROR : read failed[%d] , errno[%d]\n" , response_len , errno );
		return -1;
	}
	
	p = gettok( response_buffer , " " ) ;
	if( p == NULL )
	{
		printf( "*** ERROR : http response [%s] invalid\n" , response_buffer );
		return -1;
	}
	
	p = gettok( NULL , " " ) ;
	if( p == NULL )
	{
		printf( "*** ERROR : http response [%s] invalid\n" , response_buffer );
		return -1;
	}
	
	status_code = atoi(p);
	if( status_code != 200 )
	{
		printf( "*** ERROR : http status code [%03d]\n" , status_code );
		return -1;
	}
	
	content_length = STRISTR( response_buffer , CONTENT_LENGTH ) ;
	if( content_length == NULL )
	{
		if( response_len > 4 && STRCMP( response_buffer+response_len-4 , == , "\r\n\r\n" ) )
		{
			;
		}
		else
		{
			printf( "*** ERROR : big http response[%s]\n" , response_buffer );
			return -1;
		}
	}
	else
	{
		remain_len = atoi(content_length+sizeof(CONTENT_LENGTH)) - response_len ;
		
		while( remain_len > 0 )
		{
			block_len = MIN( remain_len , sizeof(response_buffer)-1 ) ;
			memset( response_buffer , 0x00 , sizeof(response_buffer) );
			len = read( user_data->netaddr.sock , response_buffer , block_len ) ;
			if( len == 0 )
			{
				printf( "*** ERROR : socket closed on reading\n" );
				return -1;
			}
			else if( len == -1 )
			{
				printf( "*** ERROR : read failed[%d] , errno[%d]\n" , block_len , errno );
				return -1;
			}
			
			remain_len -= len ;
		}
	}
	
	if( user_data->keep_alive == 0 )
	{
		close( user_data->netaddr.sock );
	}
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	if( user_data->keep_alive == 1 )
	{
		close( user_data->netaddr.sock );
	}
	
	if( user_data->http )
		free( user_data->http );
	
	free( user_data );
	
	return 0;
}

