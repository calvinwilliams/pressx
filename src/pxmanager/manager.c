#include "pxmanager_in.h"

int manager( struct PxManager *p_pxmanager )
{
	fd_set			read_fds ;
	char			stdin_command_buffer[ 1024 + 1 ] ;
	struct AcceptedSession	*p_accepted_session = NULL ;
	struct AcceptedSession	*p_next_accepted_session = NULL ;
	
	int			nret = 0 ;
	
	p_pxmanager->stdin_session.fd = fileno( stdin ) ;
	
	p_pxmanager->listen_session.netaddr.sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( p_pxmanager->listen_session.netaddr.sock == -1 )
	{
		printf( "socket failed , errno[%d]\n" , errno );
		return -1;
	}
	
	SETNETADDRESS( p_pxmanager->listen_session.netaddr )
	nret = bind( p_pxmanager->listen_session.netaddr.sock , (struct sockaddr *) & (p_pxmanager->listen_session.netaddr.addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "bind[%s:%d]#%d# failed , errno[%d]\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port , p_pxmanager->listen_session.netaddr.sock , errno );
		close( p_pxmanager->listen_session.netaddr.sock );
		return -1;
	}
	
	nret = listen( p_pxmanager->listen_session.netaddr.sock , 10240 ) ;
	if( nret == -1 )
	{
		printf( "listen[%s:%d]#%d# failed , errno[%d]\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port , p_pxmanager->listen_session.netaddr.sock , errno );
		close( p_pxmanager->listen_session.netaddr.sock );
		return -1;
	}
	else
	{
		printf( "listen[%s:%d]#%d# ok\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port , p_pxmanager->listen_session.netaddr.sock );
	}
	
	signal( SIGPIPE , SIG_IGN );
	
	while(1)
	{
		FD_ZERO( & read_fds );
		FD_SET( p_pxmanager->stdin_session.fd , & read_fds );
		FD_SET( p_pxmanager->listen_session.netaddr.sock , & read_fds );
		nret = select( MAX(p_pxmanager->stdin_session.fd,p_pxmanager->listen_session.netaddr.sock)+1 , & read_fds , NULL , NULL , NULL ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : select failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		if( FD_ISSET( p_pxmanager->stdin_session.fd , & read_fds ) )
		{
			memset( stdin_command_buffer , 0x00 , sizeof(stdin_command_buffer) );
			if( fgets( stdin_command_buffer , sizeof(stdin_command_buffer)-1 , stdin ) == NULL )
			{
				printf( "*** ERROR : fgets failed , errno[%d]\n" , errno );
				return -1;
			}
			
			TrimEnter( stdin_command_buffer );
			
			if( STRMINCMP( "quit" , == , stdin_command_buffer ) )
			{
				break;
			}
			if( STRMINCMP( "info" , == , stdin_command_buffer ) )
			{
				printf( "--- INFO ---------\n" );
				printf( "listen ip : %s\n" , p_pxmanager->listen_session.netaddr.ip );
				printf( "listen port : %d\n" , p_pxmanager->listen_session.netaddr.port );
				printf( "process count : %u\n" , p_pxmanager->process_count );
				printf( "thread count : %u\n" , p_pxmanager->thread_count );
				
				printf( "--- ACCEPTED SESSION ---------\n" );
				list_for_each_entry( p_accepted_session , & (p_pxmanager->accepted_session_list) , struct AcceptedSession , listnode )
				{
					printf( "ip : %s        port : %d\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
				}
			}
		}
		if( FD_ISSET( p_pxmanager->listen_session.netaddr.sock , & read_fds ) )
		{
		}
	}
	
	list_for_each_entry_safe( p_accepted_session , p_next_accepted_session , & (p_pxmanager->accepted_session_list) , struct AcceptedSession , listnode )
	{
		list_del( & (p_accepted_session->listnode) );
		
		close( p_accepted_session->netaddr.sock );
		free( p_accepted_session );
	}
	
	close( p_pxmanager->listen_session.netaddr.sock );
	
	return 0;
}

