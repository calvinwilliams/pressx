#include "pxagent_in.h"

int app_RegisteAgent( struct PxAgent *p_pxagent )
{
	struct PxRegisteMessage		msg ;
	
	int				nret = 0 ;
	
	memset( & msg , 0x00 , sizeof(struct PxRegisteMessage ) );
	strncpy( msg.user_name , GetUsernamePtr() , sizeof(msg.user_name)-1 );
	nret = writen( p_pxagent->connected_session.netaddr.sock , (char*)&msg , sizeof(struct PxRegisteMessage) , NULL ) ;
	if( nret )
	{
		printf( "writen failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	return 0;
}

int app_CreateProcesses( struct PxAgent *p_pxagent )
{
	
	
	
	
	
	
	
	return 0;
}

