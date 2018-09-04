#include "pxmanager_in.h"

int manager( struct PxManager *p_manager )
{
	fd_set				read_fds ;
	int				max_fd ;
	char				stdin_command_buffer[ 1024 + 1 ] ;
	char				stdin_command_parameter1[ 1024 + 1 ] ;
	char				stdin_command_parameter2[ 1024 + 1 ] ;
	char				accepted_session_comm_buffer[ 1024 + 1 ] ;
	struct PxAcceptedSession	*p_accepted_session = NULL ;
	struct PxAcceptedSession	*p_next_accepted_session = NULL ;
	
	int				nret = 0 ;
	
	p_manager->stdin_session.fd = fileno( stdin ) ;
	
	nret = comm_CreateServerSocket( p_manager ) ;
	if( nret )
		return nret;
	
	signal( SIGPIPE , SIG_IGN );
	
	printf( "press '?' to get help\n" ); fflush(stdout);
	
	while(1)
	{
		printf( "> " ); fflush(stdout);
		
		FD_ZERO( & read_fds );
		FD_SET( p_manager->stdin_session.fd , & read_fds );
		FD_SET( p_manager->listen_session.netaddr.sock , & read_fds );
		max_fd = MAX(p_manager->stdin_session.fd,p_manager->listen_session.netaddr.sock) ;
		list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
		{
			FD_SET( p_accepted_session->netaddr.sock , & read_fds );
			if( p_accepted_session->netaddr.sock > max_fd )
				max_fd = p_accepted_session->netaddr.sock ;
		}
		nret = select( max_fd+1 , & read_fds , NULL , NULL , NULL ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : select failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		if( FD_ISSET( p_manager->stdin_session.fd , & read_fds ) )
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
			memset( stdin_command_parameter1 , 0x00 , sizeof(stdin_command_parameter1) );
			memset( stdin_command_parameter2 , 0x00 , sizeof(stdin_command_parameter2) );
			sscanf( stdin_command_buffer , "%s%s" , stdin_command_parameter1 , stdin_command_parameter2 );
			if( stdin_command_parameter2[0] == '\'' )
			{
				memset( stdin_command_parameter2 , 0x00 , sizeof(stdin_command_parameter2) );
				sscanf( stdin_command_buffer , "%*s%*[^']'%[^']" , stdin_command_parameter2 );
			}
			
			if( STRMINCMP( "?" , == , stdin_command_buffer ) )
			{
				printf( "? - help\n" );
				printf( "quit - quit manager\n" );
				printf( "info - show info\n" );
				printf( "sessions - show sessions of client\n" );
				printf( "process (process_count) - set process count\n" );
				printf( "thead (thread_count) - set thread count\n" );
				printf( "count (run_count) - set run count\n" );
				printf( "arameter '(run_parameter)' - set run parameter\n" );
				printf( "run - begin running press\n" );
			}
			else if( STRMINCMP( "quit" , == , stdin_command_buffer ) )
			{
				break;
			}
			else if( STRMINCMP( "info" , == , stdin_command_buffer ) )
			{
				nret = app_ShowManagerInfo( p_manager ) ;
				if( nret )
				{
					printf( "app_ShowManagerInfo failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "sessions" , == , stdin_command_buffer ) )
			{
				nret = app_ShowCommSessions( p_manager ) ;
				if( nret )
				{
					printf( "app_ShowCommSessions failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "process" , == , stdin_command_parameter1 ) )
			{
				nret = app_SetProcessCount( p_manager , atoi(stdin_command_parameter2) ) ;
				if( nret )
				{
					printf( "app_SetProcessCount failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "thread" , == , stdin_command_parameter1 ) )
			{
				nret = app_SetThreadCount( p_manager , atoi(stdin_command_parameter2) ) ;
				if( nret )
				{
					printf( "app_SetThreadCount failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "count" , == , stdin_command_parameter1 ) )
			{
				nret = app_SetRunCount( p_manager , atoi(stdin_command_parameter2) ) ;
				if( nret )
				{
					printf( "app_SetRunCount failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "arameter" , == , stdin_command_parameter1 ) )
			{
				nret = app_SetRunParameter( p_manager , stdin_command_parameter2 ) ;
				if( nret )
				{
					printf( "app_SetRunParameter failed[%d]\n" , nret );
					break;
				}
			}
			else if( STRMINCMP( "run" , == , stdin_command_buffer ) )
			{
				nret = app_RunPressing( p_manager ) ;
				if( nret )
				{
					printf( "app_RunPressing failed[%d]\n" , nret );
					break;
				}
			}
			else
			{
				printf( "command[%s] invalid\n" , stdin_command_buffer );
			}
		}
		
		if( FD_ISSET( p_manager->listen_session.netaddr.sock , & read_fds ) )
		{
			struct PxAcceptedSession	*p_accepted_session = NULL ;
			
			nret = comm_AcceptClientSocket( p_manager , & p_accepted_session ) ;
			if( nret )
			{
				printf( "comm_AcceptClientSocket failed[%d]\n" , nret );
				break;
			}
			
			nret = app_RegisteAgent( p_manager , p_accepted_session ) ;
			if( nret )
			{
				printf( "app_RegisteAgent failed[%d]\n" , nret );
				comm_CloseClientSocket( p_manager , p_accepted_session );
				continue;
			}
		}
		
		list_for_each_entry_safe( p_accepted_session , p_next_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
		{
			if( FD_ISSET( p_accepted_session->netaddr.sock , & read_fds ) )
			{
				memset( accepted_session_comm_buffer , 0x00 , sizeof(accepted_session_comm_buffer) );
				nret = read( p_accepted_session->netaddr.sock , accepted_session_comm_buffer , sizeof(accepted_session_comm_buffer)-1 ) ;
				if( nret == 0 )
				{
					printf( "client socket closed\n" );
					list_del( & (p_accepted_session->listnode) );
					close( p_accepted_session->netaddr.sock );
					free( p_accepted_session );
				}
				else if( nret == -1 )
				{
					printf( "*** ERROR : read client socket failed[%d] , errno[%d]\n" , nret , errno );
					list_del( & (p_accepted_session->listnode) );
					close( p_accepted_session->netaddr.sock );
					free( p_accepted_session );
				}
			}
		}
	}
	
	list_for_each_entry_safe( p_accepted_session , p_next_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		list_del( & (p_accepted_session->listnode) );
		close( p_accepted_session->netaddr.sock );
		free( p_accepted_session );
	}
	
	close( p_manager->listen_session.netaddr.sock );
	
	return 0;
}

