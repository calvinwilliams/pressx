#include "pxagent_in.h"

int app_RegisteAgent( struct PxAgent *p_agent )
{
	struct PxRegisteMessage		msg ;
	
	int				nret = 0 ;
	
	memset( & msg , 0x00 , sizeof(struct PxRegisteMessage ) );
	strncpy( msg.user_name , GetUsernamePtr() , sizeof(msg.user_name)-1 );
	nret = writen( p_agent->connected_session.netaddr.sock , (char*)&msg , sizeof(struct PxRegisteMessage) , NULL ) ;
	if( nret )
	{
		printf( "writen failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	return 0;
}

int app_CreateProcesses( struct PxAgent *p_agent )
{
	pid_t				*pids_array = NULL ;
	int				process_index ;
	int				status ;
	int				thread_index ;
	struct PxPerformanceStatMessage	*perf_stat = NULL ;
	
	int				nret = 0 ;
	
	p_agent->perf_stat_base = (struct PxPerformanceStatMessage *)mmap( NULL , sizeof(struct PxPerformanceStatMessage)*p_agent->run_pressing.process_count*p_agent->run_pressing.thread_count , PROT_READ|PROT_WRITE , MAP_SHARED|MAP_ANONYMOUS , 0 , 0 ) ;
	if( p_agent->perf_stat_base == NULL )
	{
		printf( "mmap failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( p_agent->perf_stat_base , 0x00 , sizeof(struct PxPerformanceStatMessage)*p_agent->run_pressing.process_count*p_agent->run_pressing.thread_count );
	
	signal( SIGCLD , SIG_DFL );
	signal( SIGCHLD , SIG_DFL );
	
	pids_array = (pid_t*)malloc( sizeof(pid_t)*p_agent->run_pressing.process_count ) ;
	if( pids_array == NULL )
	{
		printf( "*** ERROR : malloc failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( pids_array , 0x00 , sizeof(pid_t)*p_agent->run_pressing.process_count );
	
	for( process_index = 0 ; process_index < p_agent->run_pressing.process_count ; process_index++ )
	{
		pids_array[process_index] = fork() ;
		if( pids_array[process_index] == -1 )
		{
			printf( "*** ERROR : fork failed , errno[%d]\n" , errno );
			return -1;
		}
		else if( pids_array[process_index] == 0 )
		{
			nret = app_CreateThreads( p_agent , process_index , p_agent->perf_stat_base+process_index*sizeof(struct PxPerformanceStatMessage)*p_agent->run_pressing.thread_count ) ;
			if( nret )
			{
				printf( "*** ERROR : app_CreateThreads failed[%d] , errno[%d]\n" , nret , errno );
			}
			exit(0);
		}
		
		printf( "create process[%d] ok\n" , process_index );
	}
	
	for( process_index = 0 ; process_index < p_agent->run_pressing.process_count ; process_index++ )
	{
		nret = waitpid( pids_array[process_index] , & status , 0 ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : waitpid failed , errno[%d]\n" , errno );
			return -1;
		}
		
		printf( "destroy process[%d] ok , WEXITSTATUS[%d] WIFSIGNALED[%d] WTERMSIG[%d] WCOREDUMP[%d]\n" , process_index , WEXITSTATUS(status) , WIFSIGNALED(status) , WTERMSIG(status) , WCOREDUMP(status) );
	}
	
	perf_stat = p_agent->perf_stat_base ;
	for( process_index = 0 ; process_index < p_agent->run_pressing.process_count ; process_index++ )
	{
		for( thread_index = 0 ; thread_index < p_agent->run_pressing.thread_count ; thread_index++ )
		{
			nret = writen( p_agent->connected_session.netaddr.sock , (void*)perf_stat , sizeof(struct PxPerformanceStatMessage) , NULL ) ;
			if( nret )
			{
				printf( "writen failed[%d] , errno[%d]\n" , nret , errno );
				return -1;
			}
			
			perf_stat++;
		}
	}
	
	free( pids_array );
	
	munmap( p_agent->perf_stat_base , sizeof(struct PxPerformanceStatMessage)*p_agent->run_pressing.process_count*p_agent->run_pressing.thread_count);
	
	return 0;
}

int app_CreateThreads( struct PxAgent *p_agent , int process_index , struct PxPerformanceStatMessage *perf_stat_base_in_this_process )
{
	struct PxPluginContext	*pxplugin_ctx_array = NULL ;
	pthread_t		*tids_array = NULL ;
	int			thread_index ;
	
	int			nret = 0 ;
	
	pxplugin_ctx_array = (struct PxPluginContext *)malloc( sizeof(struct PxPluginContext)*p_agent->run_pressing.thread_count ) ;
	if( pxplugin_ctx_array == NULL )
	{
		printf( "malloc failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( pxplugin_ctx_array , 0x00 , sizeof(struct PxPluginContext)*p_agent->run_pressing.thread_count );
	
	tids_array = (pthread_t *)malloc( sizeof(pthread_t)*p_agent->run_pressing.thread_count ) ;
	if( tids_array == NULL )
	{
		printf( "malloc failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( tids_array , 0x00 , sizeof(pthread_t)*p_agent->run_pressing.thread_count );
	
	for( thread_index = 0 ; thread_index < p_agent->run_pressing.thread_count ; thread_index++ )
	{
		pxplugin_ctx_array[thread_index].p_agent = p_agent ;
		pxplugin_ctx_array[thread_index].perf_stat = perf_stat_base_in_this_process+thread_index ;
		nret = pthread_create( tids_array+thread_index , NULL , app_ThreadEntry , (void*)(pxplugin_ctx_array+thread_index) ) ;
		if( nret == -1 )
		{
			printf( "pthread_create failed , errno[%d]\n" , errno );
			return -1;
		}
		
		printf( "create process[%d] thread[%d] ok\n" , process_index , thread_index );
	}
	
	for( thread_index = 0 ; thread_index < p_agent->run_pressing.thread_count ; thread_index++ )
	{
		nret = pthread_join( tids_array[thread_index] , NULL ) ;
		if( nret == -1 )
		{
			printf( "pthread_join failed , errno[%d]\n" , errno );
			return -1;
		}
		
		printf( "destroy process[%d] thread[%d] ok\n" , process_index , thread_index );
	}
	
	free( tids_array );
	
	free( pxplugin_ctx_array ) ;
	
	return 0;
}

void *app_ThreadEntry( void *p )
{
	struct PxPluginContext		*p_pxplugin_ctx = (struct PxPluginContext *)p ;
	
	sleep(1);
	
p_pxplugin_ctx->perf_stat->min_run_elapse.tv_sec = 1 ;
p_pxplugin_ctx->perf_stat->min_run_elapse.tv_usec = 0 ;
p_pxplugin_ctx->perf_stat->avg_run_elapse.tv_sec = 1 ;
p_pxplugin_ctx->perf_stat->avg_run_elapse.tv_usec = 0 ;
p_pxplugin_ctx->perf_stat->max_run_elapse.tv_sec = 1 ;
p_pxplugin_ctx->perf_stat->max_run_elapse.tv_usec = 0 ;
	return NULL;
}

