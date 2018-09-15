#ifndef _H_PXMANAGER_IN_
#define _H_PXMANAGER_IN_

#include "pxutil.h"

struct PxListenSession
{
	struct NetAddress	netaddr ;
} ;

struct PxStdinSession
{
	int			fd ;
} ;

struct PxAcceptedSession
{
	struct NetAddress	netaddr ;
	struct PxRegisteMessage	reg_msg ;
	unsigned char		response_flag ;
	
	struct list_head	listnode ;
} ;

struct PxManager
{
	unsigned int		process_count ;
	unsigned int		thread_count ;
	unsigned int		run_count ;
	char			run_plugin[ PRESSX_MAXLEN_RUN_PLUGIN + 1 ] ;
	char			run_parameter[ PRESSX_MAXLEN_RUN_PARAMETER + 1 ] ;
	
	struct PxListenSession	listen_session ;
	struct PxStdinSession	stdin_session ;
	
	struct list_head	accepted_session_list ;
} ;

int manager( struct PxManager *p_manager );

int comm_CreateServerSocket( struct PxManager *p_manager );
int comm_AcceptClientSocket( struct PxManager *p_manager , struct PxAcceptedSession **pp_accepted_session );
int comm_CloseClientSocket( struct PxManager *p_manager , struct PxAcceptedSession *p_accepted_session );

int app_ShowManagerInfo( struct PxManager *p_manager );
int app_ShowCommSessions( struct PxManager *p_manager );
int app_SetProcessCount( struct PxManager *p_manager , unsigned int process_count );
int app_SetThreadCount( struct PxManager *p_manager , unsigned int thread_count );
int app_SetRunCount( struct PxManager *p_manager , unsigned int run_count );
int app_SetRunParameter( struct PxManager *p_manager , char *run_parameter );
int app_RegisteAgent( struct PxManager *p_manager , struct PxAcceptedSession *p_accepted_session );
int app_RunPressing( struct PxManager *p_manager );

#endif

