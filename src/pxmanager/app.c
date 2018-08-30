#include "pxmanager_in.h"

int app_ShowManagerInfo( struct PxManager *p_pxmanager )
{
	printf( "listen ip : %s\n" , p_pxmanager->listen_session.netaddr.ip );
	printf( "listen port : %d\n" , p_pxmanager->listen_session.netaddr.port );
	printf( "process count : %u\n" , p_pxmanager->process_count );
	printf( "thread count : %u\n" , p_pxmanager->thread_count );
	
	return 0;
}

int app_ShowCommSessions( struct PxManager *p_pxmanager )
{
	struct AcceptedSession	*p_accepted_session = NULL ;
	
	list_for_each_entry( p_accepted_session , & (p_pxmanager->accepted_session_list) , struct AcceptedSession , listnode )
	{
		printf( "%s:%d\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	}
	
	return 0;
}

int app_RegisteAgent( struct PxManager *p_pxmanager , struct AcceptedSession *p_accepted_session )
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

int app_RunPressing( struct PxManager *p_pxmanager )
{
	struct AcceptedSession		*p_accepted_session = NULL ;
	struct PxRunPressingMessage	run_pressing ;
	struct PxPerformanceStatMessage	perf_stat ;
	unsigned int			i ;
	unsigned int			count ;
	double				total_elapse ;
	
	int				nret = 0 ;
	
	list_for_each_entry( p_accepted_session , & (p_pxmanager->accepted_session_list) , struct AcceptedSession , listnode )
	{
		memset( & run_pressing , 0x00 , sizeof(struct PxRunPressingMessage) );
		run_pressing.process_count = p_pxmanager->process_count ;
		run_pressing.thread_count = p_pxmanager->thread_count ;
		strncpy( run_pressing.run_command , p_pxmanager->run_command , sizeof(run_pressing.run_command)-1 );
		nret = writen( p_accepted_session->netaddr.sock , (char*)&run_pressing , sizeof(struct PxRunPressingMessage) , NULL ) ;
		if( nret )
		{
			printf( "writen failed[%d] , errno[%d]\n" , nret , errno );
			return -1;
		}
		
		printf( "%s:%d run pressing ...\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
	}
	
	count = p_pxmanager->process_count * p_pxmanager->thread_count ;
	total_elapse = 0.00 ;
	
	list_for_each_entry( p_accepted_session , & (p_pxmanager->accepted_session_list) , struct AcceptedSession , listnode )
	{
		printf( "waiting %s:%d finishing ...\n" , p_accepted_session->netaddr.remote_ip , p_accepted_session->netaddr.remote_port );
		
		for( i = 0 ; i < count ; i++ )
		{
			memset( & perf_stat , 0x00 , sizeof(struct PxPerformanceStatMessage) );
			nret = readn( p_accepted_session->netaddr.sock , (void*)&perf_stat , sizeof(struct PxPerformanceStatMessage) , NULL ) ;
			if( nret )
			{
				printf( "*** ERROR : readn failed[%d] , errno[%d]\n" , nret , errno );
				return -1;
			}
			
			printf( "total_elapse[%ld.%06ld]\n" , perf_stat.total_elapse.tv_sec , perf_stat.total_elapse.tv_usec );
			
			total_elapse += perf_stat.total_elapse.tv_sec + (double)(perf_stat.total_elapse.tv_usec)/1000000 ;
		}
	}
	
	printf( "all process and thread finished\n" );
	printf( "          total_elapse[%.2lf]\n" , total_elapse );
	printf( "transaction per secons[%.0lf]\n" , count / total_elapse );
	printf( "  delay of transaction[%.6lf]\n" , total_elapse / count );
	
	return 0;
}

