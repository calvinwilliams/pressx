#include "pxmanager_in.h"

int manager( struct PxManager *p_pxmanager )
{
	fd_set			read_fds ;
	char			stdin_command_buffer[ 1024 + 1 ] ;
	struct AcceptedSession	*p_accepted_session = NULL ;
	struct AcceptedSession	*p_next_accepted_session = NULL ;
	
	int			nret = 0 ;
	
	p_pxmanager->stdin_session.fd = fileno( stdin ) ;
	
	nret = comm_CreateServerSocket( p_pxmanager ) ;
	if( nret )
		return nret;
	
	signal( SIGPIPE , SIG_IGN );
	
	while(1)
	{
		printf( "> " ); fflush(stdout);
		
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
			if( stdin_command_buffer[0] == '\0' )
				continue;
			
			if( STRMINCMP( "quit" , == , stdin_command_buffer ) )
			{
				break;
			}
			else if( STRMINCMP( "info" , == , stdin_command_buffer ) )
			{
				nret = app_ShowManagerInfo( p_pxmanager ) ;
				if( nret )
				{
					printf( "app_ShowManagerInfo failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "sessions" , == , stdin_command_buffer ) )
			{
				nret = app_ShowCommSessions( p_pxmanager ) ;
				if( nret )
				{
					printf( "app_ShowCommSessions failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "run" , == , stdin_command_buffer ) )
			{
				nret = app_RunPressing( p_pxmanager ) ;
				if( nret )
				{
					printf( "app_RunPressing failed[%d]\n" , nret );
					break;
				}
			}
		}
		if( FD_ISSET( p_pxmanager->listen_session.netaddr.sock , & read_fds ) )
		{
			struct AcceptedSession	*p_accepted_session = NULL ;
			
			nret = comm_AcceptClientSocket( p_pxmanager , & p_accepted_session ) ;
			if( nret )
			{
				printf( "comm_AcceptClientSocket failed[%d]\n" , nret );
				break;
			}
			
			nret = app_RegisteAgent( p_pxmanager , p_accepted_session ) ;
			if( nret )
			{
				printf( "app_RegisteAgent failed[%d]\n" , nret );
				comm_CloseClientSocket( p_pxmanager , p_accepted_session );
				continue;
			}
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

