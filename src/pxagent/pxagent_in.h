#ifndef _H_PXAGENT_IN_
#define _H_PXAGENT_IN_

#include "pxutil.h"
#include "pxagent_api.h"

#include "pthread.h"

struct PxConnectedSession
{
	struct NetAddress		netaddr ;
} ;

struct PxPluginContext
{
	struct PxAgent			*p_agent ;
	unsigned int			process_index ;
	unsigned int			thread_index ;
	struct PxPerformanceStatMessage	*perf_stat ;
	void				*user_data ;
} ;

struct PxAgent
{
	struct PxConnectedSession	connected_session ;
	
	struct PxRunPressingMessage	run_pressing ;
	void				*so_handler ;
	funcInitPxPlugin		*pfuncInitPxPlugin ;
	funcRunPxPlugin			*pfuncRunPxPlugin ;
	funcRunPxPlugin			*pfuncRawRunPxPlugin ;
	funcCleanPxPlugin		*pfuncCleanPxPlugin ;
	struct PxPerformanceStatMessage	*perf_stat_base ;
} ;

int agent( struct PxAgent *p_agent );

int comm_CreateClientSocket( struct PxAgent *p_agent );

int app_RegisteAgent( struct PxAgent *p_agent );
int app_LoadPlugin( struct PxAgent *p_agent );
int app_UnloadPlugin( struct PxAgent *p_agent );
int app_CreateProcesses( struct PxAgent *p_agent );
int app_CreateThreads( struct PxAgent *p_agent , int process_index , struct PxPerformanceStatMessage *perf_stat_base_in_this_process );
void *app_ThreadEntry( void *p );

#endif

