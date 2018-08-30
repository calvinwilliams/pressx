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
	char			user_name[ MAXLEN_USER_NAME + 1 ] ;
	
	struct list_head	listnode ;
} ;

struct PxManager
{
	unsigned int		process_count ;
	unsigned int		thread_count ;
	char			run_command[ 256 + 1 ] ;
	
	struct ListenSession	listen_session ;
	struct StdinSession	stdin_session ;
	
	struct list_head	accepted_session_list ;
} ;

int manager( struct PxManager *p_pxmanager );

int comm_CreateServerSocket( struct PxManager *p_pxmanager );
int comm_AcceptClientSocket( struct PxManager *p_pxmanager , struct AcceptedSession **pp_accepted_session );
int comm_CloseClientSocket( struct PxManager *p_pxmanager , struct AcceptedSession *p_accepted_session );

int app_ShowManagerInfo( struct PxManager *p_pxmanager );
int app_ShowCommSessions( struct PxManager *p_pxmanager );
int app_RegisteAgent( struct PxManager *p_pxmanager , struct AcceptedSession *p_accepted_session );
int app_RunPressing( struct PxManager *p_pxmanager );

#endif

