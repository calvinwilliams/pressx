#include "pxmanager_in.h"

int app_ShowManagerInfo( struct PxManager *p_manager )
{
	printf( "listen ip : %s\n" , p_manager->listen_session.netaddr.ip );
	printf( "listen port : %d\n" , p_manager->listen_session.netaddr.port );
	printf( "process count : %u\n" , p_manager->process_count );
	printf( "thread count : %u\n" , p_manager->thread_count );
	
	return 0;
}

int app_ShowCommSessions( struct PxManager *p_manager )
{
	struct PxAcceptedSession	*p_accepted_session = NULL ;
	
	list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		printf( "%s:%d\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	}
	
	return 0;
}

int app_RegisteAgent( struct PxManager *p_manager , struct PxAcceptedSession *p_accepted_session )
{
	struct PxRegisteMessage		msg ;
	
	int				nret = 0 ;
	
	memset( & msg , 0x00 , sizeof(struct PxRegisteMessage) );
	nret = readn( p_accepted_session->netaddr.sock , (char*)&msg , sizeof(struct PxRegisteMessage) , NULL ) ;
	if( nret == 1 )
	{
		printf( "readn socket closed\n" );
		return 1;
	}
	else if( nret )
	{
		printf( "readn socket failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	strncpy( p_accepted_session->user_name , msg.user_name , sizeof(p_accepted_session->user_name)-1 );
	
	return 0;
}

int app_RunPressing( struct PxManager *p_manager )
{
	struct PxAcceptedSession		*p_accepted_session = NULL ;
	int					host_count ;
	struct PxRunPressingMessage		run_pressing ;
	struct PxPerformanceStatMessage		perf_stat ;
	unsigned int				process_thread_idex ;
	unsigned int				process_thread_count ;
	unsigned int				total_run_count ;
	double					total_run_elapse ;
	struct timeval				min_run_elapse ;
	double					avg_run_elapse ;
	struct timeval				max_run_elapse ;
	
	int					nret = 0 ;
	
	host_count = 0 ;
	list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		memset( & run_pressing , 0x00 , sizeof(struct PxRunPressingMessage) );
		run_pressing.process_count = p_manager->process_count ;
		run_pressing.thread_count = p_manager->thread_count ;
		run_pressing.run_count = p_manager->run_count ;
		strncpy( run_pressing.run_plugin , p_manager->run_plugin , sizeof(run_pressing.run_plugin)-1 );
		strncpy( run_pressing.run_parameter , p_manager->run_parameter , sizeof(run_pressing.run_parameter)-1 );
		nret = writen( p_accepted_session->netaddr.sock , (char*)&run_pressing , sizeof(struct PxRunPressingMessage) , NULL ) ;
		if( nret )
		{
			printf( "writen failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		printf( "%s:%d run pressing ...\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
		host_count++;
	}
	
	process_thread_count = p_manager->process_count * p_manager->thread_count ;
	total_run_count = 0 ;
	total_run_elapse = 0.00 ;
	min_run_elapse.tv_sec = LONG_MAX ;
	min_run_elapse.tv_usec = LONG_MAX ;
	max_run_elapse.tv_sec = LONG_MIN ;
	max_run_elapse.tv_usec = LONG_MIN ;
	
	list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		printf( "waiting %s:%d finishing ... " , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
		
		for( process_thread_idex = 0 ; process_thread_idex < process_thread_count ; process_thread_idex++ )
		{
			memset( & perf_stat , 0x00 , sizeof(struct PxPerformanceStatMessage) );
			nret = readn( p_accepted_session->netaddr.sock , (void*)&perf_stat , sizeof(struct PxPerformanceStatMessage) , NULL ) ;
			if( nret )
			{
				printf( "*** ERROR : readn failed[%d] , errno[%d]\n" , nret , errno );
				return -1;
			}
			
			printf( "run_count[%u] total_run_elapse[%ld.%06ld] min_run_elapse[%ld.%06ld] max_run_elapse[%ld.%06ld]\n" , p_manager->run_count , perf_stat.total_run_elapse.tv_sec,perf_stat.total_run_elapse.tv_usec , perf_stat.min_run_elapse.tv_sec,perf_stat.min_run_elapse.tv_usec , perf_stat.max_run_elapse.tv_sec,perf_stat.max_run_elapse.tv_usec );
			
			total_run_count += p_manager->run_count ;
			total_run_elapse += (double)(perf_stat.total_run_elapse.tv_sec) + ((double)(perf_stat.total_run_elapse.tv_usec))/1000000 ;
			MIN_VAL_TIMEVAL( min_run_elapse , perf_stat.min_run_elapse )
			MAX_VAL_TIMEVAL( max_run_elapse , perf_stat.max_run_elapse )
		}
	}
	avg_run_elapse = total_run_elapse / host_count ;
	
	printf( "all process and thread finished\n" );
	printf( "--------- PERFORMANCE REPORT ---------\n" );
	printf( "          total run count : %u\n" , total_run_count );
	printf( "               min elapse : %ld.%06ld\n" , min_run_elapse.tv_sec , min_run_elapse.tv_usec );
	printf( "               avg elapse : %.6lf\n" , avg_run_elapse / (double)total_run_count );
	printf( "               max elapse : %ld.%06ld\n" , max_run_elapse.tv_sec , max_run_elapse.tv_usec );
	printf( "  transactions per second : %.0lf\n" , (double)total_run_count / avg_run_elapse );
	
	return 0;
}

