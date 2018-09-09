#include "pxagent_in.h"

char	_PXAGENT_VERSION_0_3_0[] = "0.3.0" ;
char	*_PXAGENT_VERSION = _PXAGENT_VERSION_0_3_0 ;

static void usage()
{
	printf( "USAGE : pxagent --connect-ip (ip) --connect-port (port)\n" );
	return;
}

static void version()
{
	printf( "pxagent v%s\n" , _PXAGENT_VERSION );
	return;
}

int main( int argc , char *argv[] )
{
	struct PxAgent		_agent , *p_agent = & _agent ;
	int			i ;
	
	if( argc == 1 )
	{
		usage();
		exit(0);
	}
	
	memset( p_agent , 0x00 , sizeof(struct PxAgent) );
	
	for( i = 1 ; i < argc ; i++ )
	{
		if( strcmp( argv[i] , "-v" ) == 0 )
		{
			version();
			exit(0);
		}
		else if( strcmp( argv[i] , "--connect-ip" ) == 0 && i + 1 < argc )
		{
			strncpy( p_agent->connected_session.netaddr.ip , argv[++i] , sizeof(p_agent->connected_session.netaddr.ip)-1 );
		}
		else if( strcmp( argv[i] , "--connect-port" ) == 0 && i + 1 < argc )
		{
			p_agent->connected_session.netaddr.port = atoi(argv[++i]) ;
		}
		else
		{
			printf( "*** ERROR : invalid opt[%s]\n" , argv[i] );
			usage();
			exit(7);
		}
	}
	
	if( p_agent->connected_session.netaddr.ip[0] == '\0' || p_agent->connected_session.netaddr.port <= 0 )
	{
		printf( "*** ERROR : expect '--connect-ip' and '--connect-port'\n" );
		usage();
		exit(7);
	}
	
	return -agent( p_agent ) ;
}

