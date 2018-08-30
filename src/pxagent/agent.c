#include "pxagent_in.h"

int agent( struct PxAgent *p_pxagent )
{
	fd_set		read_fds ;
	
	int		nret = 0 ;
	
	p_pxagent->connected_session.netaddr.sock = -1 ;
	
	while(1)
	{
		if( p_pxagent->connected_session.netaddr.sock == -1 )
		{
			nret = comm_CreateClientSocket( p_pxagent ) ;
			if( nret )
			{
				printf( "*** ERROR : comm_CreateClientSocket failed[%d]\n" , nret );
				return nret;
			}
			
			nret = app_RegisteAgent( p_pxagent ) ;
			if( nret )
			{
				printf( "*** ERROR : app_RegisteAgent failed[%d]\n" , nret );
				return nret;
			}
		}
		
		FD_ZERO( & read_fds );
		FD_SET( p_pxagent->connected_session.netaddr.sock , & read_fds );
		nret = select( p_pxagent->connected_session.netaddr.sock+1 , & read_fds , NULL , NULL , NULL ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : select failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		if( FD_ISSET( p_pxagent->connected_session.netaddr.sock , & read_fds ) )
		{
			memset( & (p_pxagent->run_pressing) , 0x00 , sizeof(struct PxRunPressing) );
			nret = readn( p_pxagent->connected_session.netaddr.sock , (char*)&(p_pxagent->run_pressing) , sizeof(struct PxRunPressing) , NULL ) ;
			if( nret == 1 )
			{
				printf( "*** ERROR : readn failed[%d] , errno[%d]\n" , nret , errno );
				close( p_pxagent->connected_session.netaddr.sock ); p_pxagent->connected_session.netaddr.sock = -1 ;
				continue;
			}
			
			nret = app_CreateProcesses( p_pxagent ) ;
			if( nret )
			{
				printf( "*** ERROR : app_CreateProcesses failed[%d]\n" , nret );
				close( p_pxagent->connected_session.netaddr.sock ); p_pxagent->connected_session.netaddr.sock = -1 ;
				continue;
			}
		}
	}
	
	return 0;
}

