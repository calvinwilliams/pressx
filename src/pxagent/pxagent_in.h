#ifndef _H_PXAGENT_IN_
#define _H_PXAGENT_IN_

#include "pxutil.h"

struct ConnectedSession
{
	struct NetAddress	netaddr ;
} ;

struct PxAgent
{
	struct ConnectedSession	connected_session ;
	
	struct PxRunPressing	run_pressing ;
} ;

int agent( struct PxAgent *p_pxagent );

int comm_CreateClientSocket( struct PxAgent *p_pxagent );

int app_RegisteAgent( struct PxAgent *p_pxagent );
int app_CreateProcesses( struct PxAgent *p_pxagent );

#endif

