#include "pxmanager_in.h"

int app_ShowManagerInfo( struct PxManager *p_manager )
{
	printf( "listen ip : %s\n" , p_manager->listen_session.netaddr.ip );
	printf( "listen port : %d\n" , p_manager->listen_session.netaddr.port );
	printf( "process count : %u\n" , p_manager->process_count );
	printf( "thread count : %u\n" , p_manager->thread_count );
	printf( "run plugin : %s\n" , p_manager->run_plugin );
	printf( "run parameter : '%s'\n" , p_manager->run_parameter );
	printf( "run count : %u\n" , p_manager->run_count );
	
	return 0;
}

int app_ShowCommSessions( struct PxManager *p_manager )
{
	struct PxAcceptedSession	*p_accepted_session = NULL ;
	
	list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		printf( "%s@%s:%d\n" , p_accepted_session->reg_msg.user_name , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	}
	
	return 0;
}

int app_SetProcessCount( struct PxManager *p_manager , unsigned int process_count )
{
	p_manager->process_count = process_count ;
	printf( "set process count to [%u]\n" , p_manager->process_count );
	return 0;
}

int app_SetThreadCount( struct PxManager *p_manager , unsigned int thread_count )
{
	p_manager->thread_count = thread_count ;
	printf( "set thread count to [%u]\n" , p_manager->thread_count );
	return 0;
}

int app_SetRunCount( struct PxManager *p_manager , unsigned int run_count )
{
	p_manager->run_count = run_count ;
	printf( "set run count to [%u]\n" , p_manager->run_count );
	return 0;
}

int app_SetRunParameter( struct PxManager *p_manager , char *run_parameter )
{
	memset( p_manager->run_parameter , 0x00 , sizeof(p_manager->run_parameter) );
	strncpy( p_manager->run_parameter , run_parameter , sizeof(p_manager->run_parameter)-1 );
	printf( "set run parameter to [%s]\n" , p_manager->run_parameter );
	return 0;
}

int app_RegisteAgent( struct PxManager *p_manager , struct PxAcceptedSession *p_accepted_session )
{
	struct PxRegisteMessage		reg_msg ;
	
	int				nret = 0 ;
	
	memset( & reg_msg , 0x00 , sizeof(struct PxRegisteMessage) );
	nret = readn( p_accepted_session->netaddr.sock , (char*) & reg_msg , sizeof(struct PxRegisteMessage) , NULL ) ;
	if( nret == 1 )
	{
		printf( "pxagent closed on connecting\n" );
		return 1;
	}
	else if( nret )
	{
		printf( "readn socket failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	memcpy( & (p_accepted_session->reg_msg) , & reg_msg , sizeof(struct PxRegisteMessage) );
	printf( "pxagent %s@%s:%d connected\n" , p_accepted_session->reg_msg.user_name , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	
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
	int					remain_accepted_session_count ;
	fd_set					read_fds ;
	int					max_fd ;
	unsigned int				total_run_count ;
	double					run_timeval ;
	double					total_run_timeval ;
	double					avg_run_timeval ;
	double					total_tps ;
	struct timeval				min_delay_timeval ;
	struct timeval				max_delay_timeval ;
	
	int					nret = 0 ;
	
	host_count = 0 ;
	list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
	{
		host_count++;
	}
	if( host_count == 0 )
	{
		printf( "no pxagents\n" );
		return 0;
	}
	
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
		
		p_accepted_session->response_flag = 0 ;
		
		printf( "%s@%s:%d run pressing ...\n" , p_accepted_session->reg_msg.user_name , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	}
	
	process_thread_count = p_manager->process_count * p_manager->thread_count ;
	total_run_count = 0 ;
	total_run_timeval = 0.00 ;
	total_tps = 0.00 ;
	min_delay_timeval.tv_sec = LONG_MAX ;
	min_delay_timeval.tv_usec = LONG_MAX ;
	max_delay_timeval.tv_sec = LONG_MIN ;
	max_delay_timeval.tv_usec = LONG_MIN ;
	
	remain_accepted_session_count = host_count ;
	while( remain_accepted_session_count > 0 )
	{
		FD_ZERO( & read_fds );
		max_fd = -1 ;
		list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
		{
			if( p_accepted_session->response_flag == 0 )
			{
				FD_SET( p_accepted_session->netaddr.sock , & read_fds );
				if( p_accepted_session->netaddr.sock > max_fd )
					max_fd = p_accepted_session->netaddr.sock ;
			}
		}
		
		printf( "waiting any finishing ...\n" );
		nret = select( max_fd+1 , & read_fds , NULL , NULL , NULL ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : select failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		list_for_each_entry( p_accepted_session , & (p_manager->accepted_session_list) , struct PxAcceptedSession , listnode )
		{
			if( p_accepted_session->response_flag == 0 )
			{
				if( FD_ISSET( p_accepted_session->netaddr.sock , & read_fds ) )
				{
					printf( "%s@%s:%d finished\n" , p_accepted_session->reg_msg.user_name , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
					
					for( process_thread_idex = 0 ; process_thread_idex < process_thread_count ; process_thread_idex++ )
					{
						memset( & perf_stat , 0x00 , sizeof(struct PxPerformanceStatMessage) );
						nret = readn( p_accepted_session->netaddr.sock , (void*)&perf_stat , sizeof(struct PxPerformanceStatMessage) , NULL ) ;
						if( nret )
						{
							printf( "*** ERROR : readn failed[%d] , errno[%d]\n" , nret , errno );
							return 0;
						}
						
						printf( "run_count[%u] run_timeval[%ld.%06ld] min_delay_timeval[%ld.%06ld] max_delay_timeval[%ld.%06ld]\n" , perf_stat.run_count , perf_stat.run_timeval.tv_sec,perf_stat.run_timeval.tv_usec , perf_stat.min_delay_timeval.tv_sec,perf_stat.min_delay_timeval.tv_usec , perf_stat.max_delay_timeval.tv_sec,perf_stat.max_delay_timeval.tv_usec );
						
						total_run_count += perf_stat.run_count ;
						run_timeval = (double)(perf_stat.run_timeval.tv_sec) + ((double)(perf_stat.run_timeval.tv_usec))/1000000 ;
						total_run_timeval += run_timeval ;
						total_tps += ( perf_stat.run_count / run_timeval ) ;
						MIN_VAL_TIMEVAL( min_delay_timeval , perf_stat.min_delay_timeval )
						MAX_VAL_TIMEVAL( max_delay_timeval , perf_stat.max_delay_timeval )
					}
					
					p_accepted_session->response_flag = 1 ;
					remain_accepted_session_count--;
				}
			}
		}
	}
	
	avg_run_timeval = total_run_timeval / host_count / process_thread_count ;
	
	printf( "all process and thread finished\n" );
	printf( "--------- PERFORMANCE REPORT ---------\n" );
	printf( "          process count : %u\n" , p_manager->process_count );
	printf( "           thread count : %u\n" , p_manager->thread_count );
	printf( "             run plugin : %s\n" , p_manager->run_plugin );
	printf( "          run parameter : '%s'\n" , p_manager->run_parameter );
	printf( "              run count : %u\n" , p_manager->run_count );
	printf( "\n" );
	printf( "        total run count : %u\n" , total_run_count );
	printf( "        avg run timeval : %.6lf (s)\n" , avg_run_timeval );
	printf( "      min delay timeval : %ld.%06ld (s)\n" , min_delay_timeval.tv_sec , min_delay_timeval.tv_usec );
	printf( "      avg delay timeval : %.6lf (s)\n" , total_run_timeval / total_run_count );
	printf( "      max delay timeval : %ld.%06ld (s)\n" , max_delay_timeval.tv_sec , max_delay_timeval.tv_usec );
	printf( "transactions per second : %.0lf\n" , total_tps );
	printf( "\n" );
	
	return 0;
}

