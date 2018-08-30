#include "pxmanager_in.h"

char	_PXMANAGER_VERSION_0_1_0[] = "0.1.0" ;
char	*_PXMANAGER_VERSION = _PXMANAGER_VERSION_0_1_0 ;

static void usage()
{
	printf( "USAGE : pxmanager --listen-ip (ip) --listen-port (port)\n" );
	printf( "                  [ --process-count (count) ]\n" );
	printf( "                  [ --thread-count (count) ]\n" );
	printf( "                  [ --run-command (cmd) ]\n" );
	return;
}

static void version()
{
	printf( "pxmanager v%s\n" , _PXMANAGER_VERSION );
	return;
}

int main( int argc , char *argv[] )
{
	struct PxManager	pxmanager , *p_pxmanager = & pxmanager ;
	int			i ;
	
	if( argc == 1 )
	{
		usage();
		exit(0);
	}
	
	memset( & pxmanager , 0x00 , sizeof(struct PxManager) );
	
	for( i = 1 ; i < argc ; i++ )
	{
		if( strcmp( argv[i] , "-v" ) == 0 )
		{
			version();
			exit(0);
		}
		else if( strcmp( argv[i] , "--listen-ip" ) == 0 && i + 1 < argc )
		{
			strncpy( p_pxmanager->listen_session.netaddr.ip , argv[++i] , sizeof(p_pxmanager->listen_session.netaddr.ip)-1 );
		}
		else if( strcmp( argv[i] , "--listen-port" ) == 0 && i + 1 < argc )
		{
			p_pxmanager->listen_session.netaddr.port = atoi(argv[++i]) ;
		}
		else if( strcmp( argv[i] , "--process-count" ) == 0 && i + 1 < argc )
		{
			p_pxmanager->process_count = atoi(argv[++i]) ;
		}
		else if( strcmp( argv[i] , "--thread-count" ) == 0 && i + 1 < argc )
		{
			p_pxmanager->thread_count = atoi(argv[++i]) ;
		}
		else if( strcmp( argv[i] , "--run-command" ) == 0 && i + 1 < argc )
		{
			strncpy( p_pxmanager->run_command , argv[++i] , sizeof(p_pxmanager->run_command)-1 );
		}
		else
		{
			printf( "*** ERROR : invalid opt[%s]\n" , argv[i] );
			usage();
			exit(7);
		}
	}
	
	if( p_pxmanager->listen_session.netaddr.ip[0] == '\0' || p_pxmanager->listen_session.netaddr.port <= 0 )
	{
		printf( "*** ERROR : expect '--listen-ip' and '--listen-port'\n" );
		usage();
		exit(7);
	}
	
	if( p_pxmanager->process_count <= 0 )
	{
		p_pxmanager->process_count = 1 ;
	}
	
	if( p_pxmanager->thread_count <= 0 )
	{
		p_pxmanager->thread_count = 1 ;
	}
	
	INIT_LIST_HEAD( & (p_pxmanager->accepted_session_list) );
	
	return -manager( p_pxmanager ) ;
}

