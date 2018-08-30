#ifndef _H_PXAGENT_IN_
#define _H_PXAGENT_IN_

#include "pxutil.h"

#include "pthread.h"

struct ConnectedSession
{
	struct NetAddress	netaddr ;
} ;

struct PxSomainParameter
{
	struct PxAgent			*p_pxagent ;
	struct PxPerformanceStatMessage	*perf_stat_base ;
} ;

struct PxAgent
{
	struct ConnectedSession		connected_session ;
	
	struct PxRunPressingMessage	run_pressing ;
	struct PxPerformanceStatMessage	*perf_stat_base ;
} ;

int agent( struct PxAgent *p_pxagent );

int comm_CreateClientSocket( struct PxAgent *p_pxagent );

int app_RegisteAgent( struct PxAgent *p_pxagent );
int app_CreateProcesses( struct PxAgent *p_pxagent );
int app_CreateThreads( struct PxAgent *p_pxagent , int process_index , struct PxPerformanceStatMessage *perf_stat_base_in_this_process );
void *app_ThreadEntry( void *p );

#endif

