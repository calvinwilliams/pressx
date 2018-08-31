#include "pxagent_in.h"

int comm_CreateClientSocket( struct PxAgent *p_agent )
{
	int		nret = 0 ;
	
	p_agent->connected_session.netaddr.sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( p_agent->connected_session.netaddr.sock == -1 )
	{
		printf( "*** ERROR : socket failed , errno[%d]\n" , errno );
		return -1;
	}
	
	SETNETADDRESS( p_agent->connected_session.netaddr )
	nret = connect( p_agent->connected_session.netaddr.sock , (struct sockaddr *) & (p_agent->connected_session.netaddr.addr) , sizeof(struct sockaddr) ) ;
	if( nret == -1 )
	{
		printf( "*** ERROR : connect[%s:%d] failed , errno[%d]\n" , p_agent->connected_session.netaddr.ip , p_agent->connected_session.netaddr.port , errno );
		close( p_agent->connected_session.netaddr.sock ); p_agent->connected_session.netaddr.sock = -1 ;
		return -1;
	}
	else
	{
		printf( "connect[%s:%d] ok\n" , p_agent->connected_session.netaddr.ip , p_agent->connected_session.netaddr.port );
	}
	
	return 0;
}

