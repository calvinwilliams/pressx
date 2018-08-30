#ifndef _H_PXMANAGER_IN_
#define _H_PXMANAGER_IN_

#include "pxutil.h"

struct ListenSession
{
	struct NetAddress	netaddr ;
} ;

struct StdinSession
{
	int			fd ;
} ;

struct AcceptedSession
{
	struct NetAddress	netaddr ;
	
	struct list_head	listnode ;
} ;

struct PxManager
{
	unsigned int		process_count ;
	unsigned int		thread_count ;
	struct ListenSession	listen_session ;
	struct StdinSession	stdin_session ;
	
	struct list_head	accepted_session_list ;
} ;

int manager( struct PxManager *p_pxmanager );



#endif

