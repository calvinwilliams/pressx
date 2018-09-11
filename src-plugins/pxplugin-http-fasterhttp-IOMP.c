#include "pxagent_api.h"

#include <sys/epoll.h>

#include "fasterhttp.h"

struct PxPluginUserData
{
	struct NetAddress	netaddr ;
	unsigned char		output_flag ;
	struct PxIompSession	*p_output_session ;
	char			*request ;
	unsigned char		keepalive_flag ;
	int			request_len ;
	int			iomp_count ;
	int			total_run_count ;
	int			epoll_fd ;
} ;

struct PxIompSession
{
	unsigned int		iomp_index ;
	struct NetAddress	netaddr ;
	struct HttpEnv		*http_env ;
	unsigned char		request_or_response_flag ;
} ;

#define MAXCNT_EPOLL_EVENTS	1024

/* run parameter
ip port http_request_pathfilename iomp_count total_count
*/

/* for example
$ pxmanager --listen-ip 192.168.6.21 --listen-port 9527 -p 1 -t 1 -n 1 -g pxplugin-http-fasterhttp-IOMP-IOMP.so -m "192.168.6.21 80 pxplugin-http.txt 10 100"
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
		printf( "pxplugin-http-fasterhttp-IOMP | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	memset( http_request_pathfilename , 0x00 , sizeof(http_request_pathfilename) );
	sscanf( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , "%s%d%s%d%d" , user_data->netaddr.ip , & (user_data->netaddr.port) , http_request_pathfilename , & (user_data->iomp_count) , & (user_data->total_run_count) );
	if( user_data->netaddr.ip[0] == '\0' || user_data->netaddr.port <= 0 || user_data->iomp_count <= 0 || user_data->total_run_count <= 0 )
	{
		printf( "pxplugin-http-fasterhttp-IOMP | run parameter '%s' invalid for format '(ip) (port) (http_request_pathfilename) (iomp_count) (total_count)'\n" , GetPxPluginRunParameterPtr(p_pxplugin_ctx) );
		return -1;
	}
	
	nret = PXReadEntireFile( http_request_pathfilename , & (user_data->request) , & user_data->request_len ) ;
	if( nret )
	{
		printf( "pxplugin-http-fasterhttp-IOMP | PXReadEntireFile[%s] failed[%d] , errno[%d]\n" , http_request_pathfilename , nret , errno );
		return -1;
	}
	
	if( STRISTR( user_data->request , "HTTP/1.1" ) || STRISTR( user_data->request , "Connection: keep-alive" ) )
	{
		user_data->keepalive_flag = 1 ;
	}
	else
	{
		user_data->keepalive_flag = 0 ;
	}
	
	user_data->epoll_fd = epoll_create( 1024 ) ;
	if( user_data->epoll_fd == -1 )
	{
		printf( "*** ERROR : epoll_create failed[%d] , errno[%d]\n" , user_data->epoll_fd , errno );
		return -1;
	}
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

static int ConnectToServer( struct PxPluginUserData *user_data , struct PxIompSession *s )
{
	int		nret = 0 ;
	
	s->netaddr.sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( s->netaddr.sock == -1 )
	{
		printf( "*** ERROR : socket failed , errno[%d]\n" , errno );
		return -1;
	}
	
	{
		int	onoff = 1 ;
		setsockopt( s->netaddr.sock , IPPROTO_TCP , TCP_NODELAY , (void*) & onoff , sizeof(int) );
	}
	
	strcpy( s->netaddr.ip , user_data->netaddr.ip );
	s->netaddr.port = user_data->netaddr.port ;
	SETNETADDRESS( s->netaddr )
	nret = connect( s->netaddr.sock , (struct sockaddr *) & (s->netaddr.addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "*** ERROR : connect[%s:%d] failed , errno[%d]\n" , s->netaddr.ip , s->netaddr.port , errno );
		close( s->netaddr.sock ); s->netaddr.sock = -1 ;
		return -1;
	}
	
	{
		int	opts ;
		opts = fcntl( s->netaddr.sock , F_GETFL );
		opts |= O_NONBLOCK ;
		fcntl( s->netaddr.sock , F_SETFL , opts );
	}
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	int			run_count ;
	int			run_iomp_count ;
	struct PxIompSession	*s = NULL ;
	struct HttpBuffer	*b = NULL ;
	struct epoll_event	event ;
	struct epoll_event	events[ MAXCNT_EPOLL_EVENTS ] ;
	struct epoll_event	*p_event = NULL ;
	int			i ;
	int			event_count ;
	int			status_code ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	run_count = 0 ;
	run_iomp_count = 0 ;
	user_data->output_flag = 1 ;
	while( run_count < user_data->total_run_count )
	{
		while( run_iomp_count < user_data->iomp_count )
		{
			if( run_count + run_iomp_count >= user_data->total_run_count )
				break;
			
			s = (struct PxIompSession *)malloc( sizeof(struct PxIompSession) ) ;
			if( s == NULL )
			{
				printf( "*** ERROR : malloc failed , errno[%d]\n" , errno );
				return -1;
			}
			memset( s , 0x00 , sizeof(struct PxIompSession) );
			
			if( user_data->p_output_session == NULL )
				user_data->p_output_session = s ;
			
			s->http_env = CreateHttpEnv() ;
			if( s->http_env == NULL )
			{
				printf( "*** ERROR : CreateHttpEnv failed , errno[%d]\n" , errno );
				return -1;
			}
			
			nret = ConnectToServer( user_data , s ) ;
			if( nret )
			{
				printf( "*** ERROR : ConnectToServer failed[%d]\n" , nret );
				return nret;
			}
			
			b = GetHttpRequestBuffer( s->http_env ) ;
			nret = MemcatHttpBuffer( b , user_data->request , user_data->request_len ) ;
			if( nret )
			{
				printf( "*** ERROR : StrcatHttpBuffer failed , errno[%d]\n" , errno );
				return -1;
			}
			
			if( user_data->output_flag == 1 && s == user_data->p_output_session )
			{
				printf( "HTTP REQUEST[%.*s]\n" , GetHttpBufferLength(b) , GetHttpBufferBase(b,NULL) );
			}
			
			memset( & event , 0x00 , sizeof(struct epoll_event) );
			event.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP ;
			event.data.ptr = s ;
			nret = epoll_ctl( user_data->epoll_fd , EPOLL_CTL_ADD , s->netaddr.sock , & event ) ;
			if( nret == -1 )
			{
				printf( "*** ERROR : epoll_ctl[%d][%d] EPOLL_CTL_ADD failed[%d] , errno[%d]\n" , user_data->epoll_fd , s->netaddr.sock , nret , errno );
				return -1;
			}
			else
			{
				if( user_data->output_flag == 1 && s == user_data->p_output_session )
				{
					printf( "epoll_ctl[%d][%d] EPOLL_CTL_ADD ok\n" , user_data->epoll_fd , s->netaddr.sock );
				}
			}
			
			run_iomp_count++;	
			s->iomp_index = run_iomp_count ;
			printf( "create session[%d] to [%s:%d]\n" , s->iomp_index , s->netaddr.ip , s->netaddr.port );
		}
		
		memset( & events , 0x00 , sizeof(events) );
		event_count = epoll_wait( user_data->epoll_fd , events , MAXCNT_EPOLL_EVENTS , 1000 ) ;
		if( event_count == -1 )
		{
			printf( "*** ERROR : epoll_wait failed[%d] , errno[%d]\n" , event_count , errno );
			return -1;
		}
		
		for( i = 0 , p_event = events ; i < event_count ; i++ , p_event++ )
		{
			s = (struct PxIompSession *)(p_event->data.ptr) ;
			
			if( p_event->events & EPOLLOUT )
			{
				nret = SendHttpRequestNonblock( s->netaddr.sock , NULL , s->http_env ) ;
				if( nret == FASTERHTTP_INFO_TCP_SEND_WOULDBLOCK )
				{
					if( user_data->output_flag == 1 && s == user_data->p_output_session )
					{
						printf( "SendHttpRequestNonblock[%d] return FASTERHTTP_INFO_TCP_SEND_WOULDBLOCK\n" , s->netaddr.sock );
					}
				}
				else if( nret )
				{
					printf( "*** ERROR : SendHttpRequestNonblock[%d] failed[%d]\n" , s->netaddr.sock , nret );
					return -1;
				}
				else
				{
					if( user_data->output_flag == 1 && s == user_data->p_output_session )
					{
						printf( "SendHttpRequestNonblock[%d] ok\n" , s->netaddr.sock );
					}
					
					memset( & event , 0x00 , sizeof(struct epoll_event) );
					event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP ;
					event.data.ptr = s ;
					nret = epoll_ctl( user_data->epoll_fd , EPOLL_CTL_MOD , s->netaddr.sock , & event ) ;
					if( nret == -1 )
					{
						printf( "*** ERROR : epoll_ctl[%d][%d] EPOLL_CTL_MOD failed[%d] , errno[%d]\n" , user_data->epoll_fd , s->netaddr.sock , nret , errno );
						return -1;
					}
					else
					{
						if( user_data->output_flag == 1 && s == user_data->p_output_session )
						{
							printf( "epoll_ctl[%d][%d] EPOLL_CTL_MOD ok\n" , user_data->epoll_fd , s->netaddr.sock );
						}
					}
				}
			}
			else if( p_event->events & EPOLLIN )
			{
				nret = ReceiveHttpResponseNonblock( s->netaddr.sock , NULL , s->http_env ) ;
				if( nret == FASTERHTTP_INFO_NEED_MORE_HTTP_BUFFER )
				{
					if( user_data->output_flag == 1 && s == user_data->p_output_session )
					{
						printf( "ReceiveHttpResponseNonblock[%d] return FASTERHTTP_INFO_NEED_MORE_HTTP_BUFFER\n" , s->netaddr.sock );
					}
				}
				else if( nret )
				{
					printf( "*** ERROR : ReceiveHttpResponseNonblock[%d] failed[%d]\n" , s->netaddr.sock , nret );
					return -1;
				}
				else
				{
					if( user_data->output_flag == 1 && s == user_data->p_output_session )
					{
						printf( "ReceiveHttpResponseNonblock[%d] ok\n" , s->netaddr.sock );
						
						b = GetHttpResponseBuffer( s->http_env ) ;
						printf( "HTTP RESPONSE[%.*s]\n" , GetHttpBufferLength(b) , GetHttpBufferBase(b,NULL) );
					}
					
					status_code = GetHttpStatusCode(s->http_env) ;
					if( status_code != HTTP_OK )
					{
						printf( "*** ERROR : response status code [%03d]\n" , status_code );
						return -1;
					}
					
					user_data->output_flag = 0 ;
					
					if( run_count + run_iomp_count < user_data->total_run_count )
					{
						int		keepalive ;
						
						keepalive = CheckHttpKeepAlive( s->http_env ) ;
						if( keepalive == 0 )
						{
							epoll_ctl( user_data->epoll_fd , EPOLL_CTL_DEL , s->netaddr.sock , NULL ) ;
							close( s->netaddr.sock );
							
							nret = ConnectToServer( user_data , s ) ;
							if( nret )
							{
								printf( "*** ERROR : ConnectToServer failed[%d]\n" , nret );
								return nret;
							}
						}
						
						ResetHttpEnv( s->http_env );
						
						b = GetHttpRequestBuffer( s->http_env ) ;
						nret = MemcatHttpBuffer( b , user_data->request , user_data->request_len ) ;
						if( nret )
						{
							printf( "*** ERROR : StrcatHttpBuffer failed , errno[%d]\n" , errno );
							return -1;
						}
						
						memset( & event , 0x00 , sizeof(struct epoll_event) );
						event.events = EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP ;
						event.data.ptr = s ;
						if( keepalive == 0 )
							nret = epoll_ctl( user_data->epoll_fd , EPOLL_CTL_ADD , s->netaddr.sock , & event ) ;
						else
							nret = epoll_ctl( user_data->epoll_fd , EPOLL_CTL_MOD , s->netaddr.sock , & event ) ;
						if( nret == -1 )
						{
							printf( "*** ERROR : epoll_ctl[%d][%d] EPOLL_CTL_MOD failed[%d] , errno[%d]\n" , user_data->epoll_fd , s->netaddr.sock , nret , errno );
							return -1;
						}
						else
						{
							if( user_data->output_flag == 1 && s == user_data->p_output_session )
							{
								printf( "epoll_ctl[%d][%d] EPOLL_CTL_MOD ok\n" , user_data->epoll_fd , s->netaddr.sock );
							}
						}
					}
					else
					{
						close( s->netaddr.sock );
						printf( "destroy session[%d] from [%s:%d]\n" , s->iomp_index , s->netaddr.ip , s->netaddr.port );
						free( s );
						run_iomp_count--;
					}
					
					run_count++;
				}
			}
			else
			{
				printf( "*** ERROR : session[%p] events[%d]\n" , s , p_event->events );
				return -1;
			}
		}
	}
	printf( "run_iomp_count[%d] run_count[%d] user_data->total_run_count[%d]\n" , run_iomp_count , run_count , user_data->total_run_count );
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	/*
	if( user_data->http_env )
	{
		DestroyHttpEnv( user_data->http_env );
	}
	
	if( user_data->netaddr.sock >= 0 )
	{
		printf( "disconnect[%s:%d]\n" , user_data->netaddr.ip , user_data->netaddr.port );
		close( user_data->netaddr.sock ); user_data->netaddr.sock = -1 ;
	}
	*/
	
	close( user_data->epoll_fd );
	
	if( user_data->request )
		free( user_data->request );
	
	free( user_data );
	
	return 0;
}

