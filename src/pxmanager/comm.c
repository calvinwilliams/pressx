#include "pxmanager_in.h"

int comm_CreateServerSocket( struct PxManager *p_pxmanager )
{
	int		nret = 0 ;
	
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
		printf( "bind[%s:%d] failed , errno[%d]\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port , errno );
		close( p_pxmanager->listen_session.netaddr.sock );
		return -1;
	}
	
	nret = listen( p_pxmanager->listen_session.netaddr.sock , 10240 ) ;
	if( nret == -1 )
	{
		printf( "listen[%s:%d] failed , errno[%d]\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port , errno );
		close( p_pxmanager->listen_session.netaddr.sock );
		return -1;
	}
	else
	{
		printf( "listen[%s:%d] ok\n" , p_pxmanager->listen_session.netaddr.ip , p_pxmanager->listen_session.netaddr.port );
	}
	
	return 0;
}

int comm_AcceptClientSocket( struct PxManager *p_pxmanager , struct AcceptedSession **pp_accepted_session )
{
	struct AcceptedSession	*p_accepted_session = NULL ;
	socklen_t		socklen ;
	
	p_accepted_session = (struct AcceptedSession *)malloc( sizeof(struct AcceptedSession) ) ;
	if( p_accepted_session == NULL )
	{
		printf( "malloc failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( p_accepted_session , 0x00 , sizeof(struct AcceptedSession) );
	
	/* 接受新连接 */
	socklen = sizeof(struct sockaddr) ;
	p_accepted_session->netaddr.sock = accept( p_pxmanager->listen_session.netaddr.sock , (struct sockaddr *) & (p_accepted_session->netaddr.addr) , & socklen ) ;
	if( p_accepted_session->netaddr.sock == -1 )
	{
		printf( "accept failed , errno[%d]\n" , errno );
		free( p_accepted_session );
		return -1;
	}
	
	GETNETADDRESS( p_accepted_session->netaddr )
	GETNETADDRESS_LOCAL( p_accepted_session->netaddr )
	GETNETADDRESS_REMOTE( p_accepted_session->netaddr )
	printf( "accept socket [%s:%d] ok\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	
	list_add_tail( & (p_accepted_session->listnode) , & (p_pxmanager->accepted_session_list) );
	
	(*pp_accepted_session) = p_accepted_session ;
	
	return 0;
}

int comm_CloseClientSocket( struct PxManager *p_pxmanager , struct AcceptedSession *p_accepted_session )
{
	printf( "close socket [%s:%d]\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	close( p_accepted_session->netaddr.sock );
	
	list_del( & (p_accepted_session->listnode) );
	
	free( p_accepted_session );
	
	return 0;
}

