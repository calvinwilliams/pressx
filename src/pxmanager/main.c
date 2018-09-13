#include "pxmanager_in.h"

char	_PXMANAGER_VERSION_0_4_0[] = "0.4.0" ;
char	*_PXMANAGER_VERSION = _PXMANAGER_VERSION_0_4_0 ;

static void usage()
{
	printf( "USAGE : manager --listen-ip (ip) --listen-port (port)\n" );
	printf( "                  [ [-p|--process-count] (count) ]\n" );
	printf( "                  [ [-t|--thread-count] (count) ]\n" );
	printf( "                  [ [-g|--run-plugin] (filename)\n" );
	printf( "                  [ [-m|--run-parameter] (cmd) ]\n" );
	printf( "                  [ [-n|--run-count] (count) ]\n" );
	return;
}

static void version()
{
	printf( "manager v%s\n" , _PXMANAGER_VERSION );
	return;
}

int main( int argc , char *argv[] )
{
	struct PxManager	_manager , *p_manager = & _manager ;
	int			i ;
	
	if( argc == 1 )
	{
		usage();
		exit(0);
	}
	
	memset( p_manager , 0x00 , sizeof(struct PxManager) );
	
	for( i = 1 ; i < argc ; i++ )
	{
		if( strcmp( argv[i] , "-v" ) == 0 )
		{
			version();
			exit(0);
		}
		else if( strcmp( argv[i] , "--listen-ip" ) == 0 && i + 1 < argc )
		{
			strncpy( p_manager->listen_session.netaddr.ip , argv[++i] , sizeof(p_manager->listen_session.netaddr.ip)-1 );
		}
		else if( strcmp( argv[i] , "--listen-port" ) == 0 && i + 1 < argc )
		{
			p_manager->listen_session.netaddr.port = atoi(argv[++i]) ;
		}
		else if( ( strcmp( argv[i] , "-p" ) == 0 || strcmp( argv[i] , "--process-count" ) == 0 ) && i + 1 < argc )
		{
			p_manager->process_count = atoi(argv[++i]) ;
		}
		else if( ( strcmp( argv[i] , "-t" ) == 0 || strcmp( argv[i] , "--thread-count" ) == 0 ) && i + 1 < argc )
		{
			p_manager->thread_count = atoi(argv[++i]) ;
		}
		else if( ( strcmp( argv[i] , "-n" ) == 0 || strcmp( argv[i] , "--run-count" ) == 0 ) && i + 1 < argc )
		{
			p_manager->run_count = atoi(argv[++i]) ;
		}
		else if( ( strcmp( argv[i] , "-g" ) == 0 || strcmp( argv[i] , "--run-plugin" ) == 0 ) && i + 1 < argc )
		{
			strncpy( p_manager->run_plugin , argv[++i] , sizeof(p_manager->run_plugin)-1 );
		}
		else if( ( strcmp( argv[i] , "-m" ) == 0 || strcmp( argv[i] , "--run-parameter" ) == 0 ) && i + 1 < argc )
		{
			strncpy( p_manager->run_parameter , argv[++i] , sizeof(p_manager->run_parameter)-1 );
		}
		else
		{
			printf( "*** ERROR : invalid opt[%s]\n" , argv[i] );
			usage();
			exit(7);
		}
	}
	
	if( p_manager->listen_session.netaddr.ip[0] == '\0' || p_manager->listen_session.netaddr.port <= 0 )
	{
		printf( "*** ERROR : expect '--listen-ip' and '--listen-port'\n" );
		usage();
		exit(7);
	}
	
	if( p_manager->process_count <= 0 )
	{
		p_manager->process_count = 1 ;
	}
	
	if( p_manager->thread_count <= 0 )
	{
		p_manager->thread_count = 1 ;
	}
	
	if( p_manager->run_count <= 0 )
	{
		p_manager->run_count = 1 ;
	}
	
	INIT_LIST_HEAD( & (p_manager->accepted_session_list) );
	
	return -manager( p_manager ) ;
}

